/**
 * @file input_model.c
 * @brief Quattro-owned input model implementation.
 */

#include "input_model.h"

static void input_model_init_config(InputConfig *config) {
    if (!config) return;
    config->das_delay_frames = 6u;
    config->das_repeat_frames = 2u;
    config->soft_drop_delay_frames = 3u;
    config->soft_drop_repeat_frames = 1u;
}

void input_model_init(InputModel *model) {
    if (!model) return;
    input_model_init_config(&model->config);
    input_model_reset(model);
}

void input_model_set_config(InputModel *model, const InputConfig *config) {
    if (!model || !config) return;
    model->config = *config;
}

void input_model_reset(InputModel *model) {
    if (!model) return;
    model->lateral_dominant = INPUT_LATERAL_NONE;

    model->left_frames_held = 0u;
    model->right_frames_held = 0u;
    model->last_left_repeat_frame = 0u;
    model->last_right_repeat_frame = 0u;
    model->left_repeat_active = false;
    model->right_repeat_active = false;

    model->soft_drop_frames_held = 0u;
    model->last_soft_drop_repeat_frame = 0u;
    model->soft_drop_repeat_active = false;

    model->prev_rotate_cw_held = false;
    model->prev_rotate_ccw_held = false;

    model->prev_move_left_held = false;
    model->prev_move_right_held = false;

    model->frame_counter = 0u;
}

static bool is_edge_pressed(bool held, bool prev_held) {
    return held && !prev_held;
}

static Command lateral_step_left(InputModel *model) {
    if (model->left_frames_held == 0u) {
        model->left_frames_held = 1u;
        model->left_repeat_active = false;
        model->last_left_repeat_frame = model->frame_counter;
        return CMD_MOVE_LEFT;
    }

    model->left_frames_held++;
    if (!model->left_repeat_active &&
        model->left_frames_held > model->config.das_delay_frames) {
        model->left_repeat_active = true;
        model->last_left_repeat_frame = model->frame_counter;
        return CMD_MOVE_LEFT;
    }
    if (model->left_repeat_active &&
        (uint16_t)(model->frame_counter - model->last_left_repeat_frame) >=
            model->config.das_repeat_frames) {
        model->last_left_repeat_frame = model->frame_counter;
        return CMD_MOVE_LEFT;
    }

    return CMD_NONE;
}

static Command lateral_step_right(InputModel *model) {
    if (model->right_frames_held == 0u) {
        model->right_frames_held = 1u;
        model->right_repeat_active = false;
        model->last_right_repeat_frame = model->frame_counter;
        return CMD_MOVE_RIGHT;
    }

    model->right_frames_held++;
    if (!model->right_repeat_active &&
        model->right_frames_held > model->config.das_delay_frames) {
        model->right_repeat_active = true;
        model->last_right_repeat_frame = model->frame_counter;
        return CMD_MOVE_RIGHT;
    }
    if (model->right_repeat_active &&
        (uint16_t)(model->frame_counter - model->last_right_repeat_frame) >=
            model->config.das_repeat_frames) {
        model->last_right_repeat_frame = model->frame_counter;
        return CMD_MOVE_RIGHT;
    }

    return CMD_NONE;
}

Command input_model_step(InputModel *model, const InputFrame *frame) {
    if (!model || !frame) return CMD_NONE;

    model->frame_counter++;

    /* Rotation: edge-triggered only (no repeat/hold semantics). */
    if (is_edge_pressed(frame->rotate_cw_held, model->prev_rotate_cw_held)) {
        model->prev_rotate_cw_held = frame->rotate_cw_held;
        model->prev_rotate_ccw_held = frame->rotate_ccw_held;
        return CMD_ROTATE_CW;
    }
    if (is_edge_pressed(frame->rotate_ccw_held, model->prev_rotate_ccw_held)) {
        model->prev_rotate_cw_held = frame->rotate_cw_held;
        model->prev_rotate_ccw_held = frame->rotate_ccw_held;
        return CMD_ROTATE_CCW;
    }

    model->prev_rotate_cw_held = frame->rotate_cw_held;
    model->prev_rotate_ccw_held = frame->rotate_ccw_held;

    /* Lateral movement: stable conflict handling + DAS + repeat. */
    {
        bool prev_left_held = model->prev_move_left_held;
        bool prev_right_held = model->prev_move_right_held;
        bool left_edge = is_edge_pressed(frame->move_left_held, prev_left_held);
        bool right_edge = is_edge_pressed(frame->move_right_held, prev_right_held);

        model->prev_move_left_held = frame->move_left_held;
        model->prev_move_right_held = frame->move_right_held;

        if (!frame->move_left_held) {
            model->left_frames_held = 0u;
            model->left_repeat_active = false;
        }
        if (!frame->move_right_held) {
            model->right_frames_held = 0u;
            model->right_repeat_active = false;
        }

        /* Newest held direction wins, deterministically. */
        if (left_edge) model->lateral_dominant = INPUT_LATERAL_LEFT;
        if (right_edge) model->lateral_dominant = INPUT_LATERAL_RIGHT;

        /* If dominant is released while the opposite remains held, hand off with fresh DAS. */
        if (model->lateral_dominant == INPUT_LATERAL_LEFT &&
            prev_left_held && !frame->move_left_held && frame->move_right_held) {
            model->lateral_dominant = INPUT_LATERAL_RIGHT;
            model->right_frames_held = 0u;
            model->right_repeat_active = false;
            return lateral_step_right(model);
        }
        if (model->lateral_dominant == INPUT_LATERAL_RIGHT &&
            prev_right_held && !frame->move_right_held && frame->move_left_held) {
            model->lateral_dominant = INPUT_LATERAL_LEFT;
            model->left_frames_held = 0u;
            model->left_repeat_active = false;
            return lateral_step_left(model);
        }

        if (frame->move_left_held && !frame->move_right_held) model->lateral_dominant = INPUT_LATERAL_LEFT;
        if (frame->move_right_held && !frame->move_left_held) model->lateral_dominant = INPUT_LATERAL_RIGHT;
        if (!frame->move_left_held && !frame->move_right_held) model->lateral_dominant = INPUT_LATERAL_NONE;

        if (model->lateral_dominant == INPUT_LATERAL_LEFT && frame->move_left_held) {
            Command c = lateral_step_left(model);
            if (c != CMD_NONE) return c;
        } else if (model->lateral_dominant == INPUT_LATERAL_RIGHT && frame->move_right_held) {
            Command c = lateral_step_right(model);
            if (c != CMD_NONE) return c;
        }
    }

    /* Soft drop: delay + repeat while held. */
    if (frame->soft_drop_held) {
        if (model->soft_drop_frames_held == 0u) {
            model->soft_drop_frames_held = 1u;
            model->soft_drop_repeat_active = false;
            model->last_soft_drop_repeat_frame = model->frame_counter;
            /* Initial press: immediate 1-cell soft drop. */
            return CMD_SOFT_DROP;
        } else {
            model->soft_drop_frames_held++;
            if (!model->soft_drop_repeat_active &&
                model->soft_drop_frames_held > model->config.soft_drop_delay_frames) {
                model->soft_drop_repeat_active = true;
                model->last_soft_drop_repeat_frame = model->frame_counter;
                return CMD_SOFT_DROP;
            }
            if (model->soft_drop_repeat_active &&
                (uint16_t)(model->frame_counter - model->last_soft_drop_repeat_frame) >=
                    model->config.soft_drop_repeat_frames) {
                model->last_soft_drop_repeat_frame = model->frame_counter;
                return CMD_SOFT_DROP;
            }
        }
    } else {
        model->soft_drop_frames_held = 0u;
        model->soft_drop_repeat_active = false;
    }

    return CMD_NONE;
}

