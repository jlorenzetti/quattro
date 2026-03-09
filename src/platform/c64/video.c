/**
 * @file video.c
 * @brief C64 video init and board/piece drawing for Quattro.
 */

#include <stdint.h>

#include "board.h"
#include "memory_map.h"
#include "piece.h"
#include "video.h"

/** PETSCII: space. */
#define CHAR_EMPTY  0x20
/** PETSCII: full block (same for locked and active; color distinguishes). */
#define CHAR_BLOCK  0xA0

/** C64 color: locked blocks. */
#define COLOR_LOCKED  1
/** C64 color: active piece (different from locked). */
#define COLOR_ACTIVE  3
/** C64 color: empty area. */
#define COLOR_EMPTY   0
/** PETSCII: border (same block char, distinct color). */
#define CHAR_BORDER  0xA0
/** C64 color: frame border. */
#define COLOR_BORDER 2

void video_init(void) {
    unsigned int i;
    for (i = 0; i < (unsigned int)(C64_SCREEN_COLS * C64_SCREEN_ROWS); i++) {
        C64_SCREEN_RAM[i] = CHAR_EMPTY;
        C64_COLOR_RAM[i] = COLOR_EMPTY;
    }
}

void video_draw_frame(void) {
    unsigned int r, c;
    unsigned int left_col = BOARD_SCREEN_X - 1;
    unsigned int right_col = BOARD_SCREEN_X + QUATTRO_BOARD_WIDTH;
    unsigned int bottom_row = BOARD_SCREEN_Y + QUATTRO_VISIBLE_HEIGHT;

    for (r = BOARD_SCREEN_Y; r <= bottom_row; r++) {
        C64_SCREEN_RAM[C64_SCREEN_OFFSET(left_col, r)] = CHAR_BORDER;
        C64_COLOR_RAM[C64_SCREEN_OFFSET(left_col, r)] = COLOR_BORDER;
        C64_SCREEN_RAM[C64_SCREEN_OFFSET(right_col, r)] = CHAR_BORDER;
        C64_COLOR_RAM[C64_SCREEN_OFFSET(right_col, r)] = COLOR_BORDER;
    }
    for (c = left_col; c <= right_col; c++) {
        C64_SCREEN_RAM[C64_SCREEN_OFFSET(c, bottom_row)] = CHAR_BORDER;
        C64_COLOR_RAM[C64_SCREEN_OFFSET(c, bottom_row)] = COLOR_BORDER;
    }
}

static void put_string(unsigned int col, unsigned int row, const char *s, unsigned int len) {
    unsigned int i;
    for (i = 0; i < len && (col + i) < C64_SCREEN_COLS; i++) {
        unsigned int off = C64_SCREEN_OFFSET(col + i, row);
        C64_SCREEN_RAM[off] = (unsigned char)s[i];
        C64_COLOR_RAM[off] = COLOR_EMPTY;
    }
}

static void put_number(unsigned int col, unsigned int row, uint32_t value, unsigned int width) {
    unsigned int i;
    if (width > 10) width = 10;
    for (i = width; i > 0; i--) {
        unsigned int off = C64_SCREEN_OFFSET(col + i - 1, row);
        C64_SCREEN_RAM[off] = (unsigned char)('0' + (value % 10));
        C64_COLOR_RAM[off] = COLOR_EMPTY;
        value /= 10;
    }
}

void video_draw_hud(const GameState *state) {
    const ScoreState *s = &state->score;
    unsigned int row = HUD_SCREEN_ROW;
    put_string(HUD_SCREEN_COL, row, "SCORE", 5);
    put_number(HUD_SCREEN_COL + 6, row, (uint32_t)s->score, 6);
    row += 2;
    put_string(HUD_SCREEN_COL, row, "LINES", 5);
    put_number(HUD_SCREEN_COL + 6, row, (uint32_t)s->lines_cleared, 4);
    row += 2;
    put_string(HUD_SCREEN_COL, row, "LEVEL", 5);
    put_number(HUD_SCREEN_COL + 6, row, (uint32_t)s->level, 2);
}

void video_draw_game_over(void) {
    unsigned int col = 15;
    unsigned int row = 12;
    put_string(col, row, "GAME OVER", 9);
}

void video_draw_board(const GameState *state) {
    const Board *board = &state->board;
    int8_t ax[PIECE_BLOCK_COUNT], ay[PIECE_BLOCK_COUNT];

    for (uint8_t i = 0; i < PIECE_BLOCK_COUNT; i++) {
        Point p = active_piece_get_block_position(&state->active, i);
        ax[i] = p.x;
        ay[i] = p.y;
    }

    /* Visible rows only: board y in [QUATTRO_HIDDEN_ROWS, QUATTRO_BOARD_HEIGHT) -> screen row 0..19 + BOARD_SCREEN_Y */
    for (int8_t by = (int8_t)QUATTRO_HIDDEN_ROWS; by < (int8_t)QUATTRO_BOARD_HEIGHT; by++) {
        unsigned int screen_row = (unsigned int)(BOARD_SCREEN_Y + (int)(by - (int8_t)QUATTRO_HIDDEN_ROWS));
        for (int8_t bx = 0; bx < QUATTRO_BOARD_WIDTH; bx++) {
            unsigned int screen_col = (unsigned int)(BOARD_SCREEN_X + (int)bx);
            unsigned int offset = C64_SCREEN_OFFSET((int)screen_col, (int)screen_row);

            int is_active = 0;
            for (uint8_t i = 0; i < PIECE_BLOCK_COUNT; i++) {
                if (ax[i] == bx && ay[i] == by) { is_active = 1; break; }
            }

            if (is_active) {
                C64_SCREEN_RAM[offset] = CHAR_BLOCK;
                C64_COLOR_RAM[offset] = COLOR_ACTIVE;
            } else {
                Cell c = board_get_cell(board, bx, by);
                if (c != CELL_EMPTY) {
                    C64_SCREEN_RAM[offset] = CHAR_BLOCK;
                    C64_COLOR_RAM[offset] = COLOR_LOCKED;
                } else {
                    C64_SCREEN_RAM[offset] = CHAR_EMPTY;
                    C64_COLOR_RAM[offset] = COLOR_EMPTY;
                }
            }
        }
    }
}
