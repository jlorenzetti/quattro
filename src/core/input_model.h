/**
 * @file input_model.h
 * @brief Quattro-owned input model: maps per-frame logical button state to Commands.
 *
 * This module owns gameplay-relevant input semantics (press vs hold, repeat timing),
 * independent of platform-specific key repeat (e.g. C64 KERNAL).
 */

#ifndef QUATTRO_CORE_INPUT_MODEL_H
#define QUATTRO_CORE_INPUT_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#include "types.h"

/**
 * @brief Logical input snapshot for a single frame.
 *
 * This is device-agnostic: platform layers (keyboard, joystick, etc.) should
 * populate these booleans from their own hardware state.
 */
typedef struct InputFrame {
    /* Held state (true while the button is down). */
    bool move_left_held;
    bool move_right_held;
    bool soft_drop_held;
    bool rotate_cw_held;
    bool rotate_ccw_held;
} InputFrame;

/**
 * @brief Input model configuration (DAS / repeat timings in frames).
 *
 * These values are intended to be tuned via config for playtest, but the
 * defaults encode the Phase 3 PAL-inspired baselines.
 */
typedef struct InputConfig {
    /* Lateral movement (PAL-inspired baseline). */
    uint8_t das_delay_frames;      /**< Initial delay before lateral auto-repeat. */
    uint8_t das_repeat_frames;     /**< Interval between lateral repeats once active. */

    /* Soft-drop hold (baseline candidate). */
    uint8_t soft_drop_delay_frames;    /**< Initial delay before soft-drop auto-repeat. */
    uint8_t soft_drop_repeat_frames;   /**< Interval between soft-drop repeats once active. */
} InputConfig;

/**
 * @brief Input model state across frames.
 */
typedef struct InputModel {
    InputConfig config;

    /* Lateral timers. */
    uint16_t left_frames_held;
    uint16_t right_frames_held;
    uint16_t last_left_repeat_frame;
    uint16_t last_right_repeat_frame;
    bool     left_repeat_active;
    bool     right_repeat_active;

    /* Soft-drop timers. */
    uint16_t soft_drop_frames_held;
    uint16_t last_soft_drop_repeat_frame;
    bool     soft_drop_repeat_active;

    /* Rotation edge detection (rotation stays edge-triggered). */
    bool prev_rotate_cw_held;
    bool prev_rotate_ccw_held;

    /* Global frame counter for this model (monotonic, wraps naturally). */
    uint32_t frame_counter;
} InputModel;

/**
 * @brief Initializes an InputModel with Phase 3 baseline configuration.
 *
 * Baseline:
 * - Lateral: DAS delay 12 frames, repeat interval 4 frames.
 * - Soft drop: delay 3 frames, repeat interval 2 frames.
 */
void input_model_init(InputModel *model);

/**
 * @brief Overrides the input configuration (e.g. for tuning).
 */
void input_model_set_config(InputModel *model, const InputConfig *config);

/**
 * @brief Resets per-gameplay state (timers, frame counter).
 *
 * Use when starting a new game so that input timers do not leak between runs.
 */
void input_model_reset(InputModel *model);

/**
 * @brief Steps the input model by one frame and returns the Command for this frame.
 *
 * The platform is responsible for calling this exactly once per game frame
 * with its best-effort logical input snapshot.
 *
 * Rules encoded (Phase 3 spec):
 * - Lateral movement uses frame-based DAS + auto-repeat.
 * - Soft drop uses frame-based delay + repeat while held.
 * - Rotation is edge-triggered by default (no repeat/hold behaviour).
 *
 * @param model Input model state (must have been initialised).
 * @param frame Logical input snapshot for this frame.
 * @return Command for this frame, or CMD_NONE if no gameplay action should occur.
 */
Command input_model_step(InputModel *model, const InputFrame *frame);

#endif

