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
/** PETSCII: border (same block char, distinct color). */
#define CHAR_BORDER 0xA0

/** C64 color: locked blocks (primary content; white). */
#define COLOR_LOCKED  1
/** C64 color: active piece (distinct from locked; cyan). */
#define COLOR_ACTIVE  3
/** C64 color: empty area (black, same as background). */
#define COLOR_EMPTY   0
/** C64 color: frame border (light grey; distinct from locked). */
#define COLOR_BORDER  15
/** C64 color: HUD text (light grey; matches frame). */
#define COLOR_HUD     15
/** C64 color: game over text (white on frame-color band). */
#define COLOR_GAMEOVER 1

/** VIC-II: border and background for unified screen. */
#define C64_BORDER_COL    (*(volatile unsigned char *)0xD020)
#define C64_BG_COL       (*(volatile unsigned char *)0xD021)

void video_init(void) {
    C64_BORDER_COL = COLOR_EMPTY;
    C64_BG_COL = COLOR_EMPTY;
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

static void put_string_at(unsigned int col, unsigned int row, const char *s,
                         unsigned int len, unsigned int color) {
    unsigned int i;
    for (i = 0; i < len && (col + i) < C64_SCREEN_COLS; i++) {
        unsigned int off = C64_SCREEN_OFFSET(col + i, row);
        C64_SCREEN_RAM[off] = (unsigned char)s[i];
        C64_COLOR_RAM[off] = (unsigned char)color;
    }
}

/** Write string in reverse PETSCII (char + 128) so glyph uses global bg, rest uses color. */
static void put_string_reverse_at(unsigned int col, unsigned int row, const char *s,
                                  unsigned int len, unsigned int bg_color) {
    unsigned int i;
    for (i = 0; i < len && (col + i) < C64_SCREEN_COLS; i++) {
        unsigned int off = C64_SCREEN_OFFSET(col + i, row);
        C64_SCREEN_RAM[off] = (unsigned char)((unsigned char)s[i] + 0x80u);
        C64_COLOR_RAM[off] = (unsigned char)bg_color;
    }
}

static void put_string(unsigned int col, unsigned int row, const char *s, unsigned int len) {
    put_string_at(col, row, s, len, COLOR_HUD);
}

static void put_number(unsigned int col, unsigned int row, uint32_t value, unsigned int width) {
    unsigned int i;
    if (width > 10) width = 10;
    for (i = width; i > 0; i--) {
        unsigned int off = C64_SCREEN_OFFSET(col + i - 1, row);
        C64_SCREEN_RAM[off] = (unsigned char)('0' + (value % 10));
        C64_COLOR_RAM[off] = COLOR_HUD;
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
    unsigned int r, c;
    unsigned int board_left = BOARD_SCREEN_X;
    unsigned int board_right = BOARD_SCREEN_X + QUATTRO_BOARD_WIDTH - 1;
    unsigned int row_mid = 12;  /* vertical centre of visible board */
    /* Three-row band: solid block (0xA0) with light grey. */
    for (r = row_mid - 1; r <= row_mid + 1; r++) {
        for (c = board_left; c <= board_right; c++) {
            unsigned int off = C64_SCREEN_OFFSET(c, r);
            C64_SCREEN_RAM[off] = CHAR_BLOCK;
            C64_COLOR_RAM[off] = COLOR_BORDER;
        }
    }
    /* "GAME  OVER" (space between) in reverse, centred on middle row. */
    put_string_reverse_at(15, row_mid, "GAME  OVER", 10, COLOR_BORDER);
}

static void clear_screen(void) {
    unsigned int i;
    for (i = 0; i < (unsigned int)(C64_SCREEN_COLS * C64_SCREEN_ROWS); i++) {
        C64_SCREEN_RAM[i] = CHAR_EMPTY;
        C64_COLOR_RAM[i] = COLOR_EMPTY;
    }
}

void video_clear(void) {
    clear_screen();
}

/** Block wordmark: 4x6 per letter, one empty column between letters. */
#define TITLE_LETTER_W  4
#define TITLE_LETTER_H  6
#define TITLE_LETTER_GAP 1
#define TITLE_WORDMARK_COLOR 1

static const uint8_t title_letters[7][6] = {
    /* Q */ { 0x0Fu, 0x09u, 0x09u, 0x09u, 0x0Bu, 0x0Eu },
    /* U */ { 0x09u, 0x09u, 0x09u, 0x09u, 0x09u, 0x0Fu },
    /* A */ { 0x07u, 0x09u, 0x09u, 0x0Fu, 0x09u, 0x09u },
    /* T */ { 0x0Fu, 0x02u, 0x02u, 0x02u, 0x02u, 0x02u },
    /* T */ { 0x0Fu, 0x02u, 0x02u, 0x02u, 0x02u, 0x02u },
    /* R */ { 0x0Fu, 0x09u, 0x0Fu, 0x0Au, 0x09u, 0x09u },
    /* O */ { 0x0Fu, 0x09u, 0x09u, 0x09u, 0x09u, 0x0Fu },
};

static void draw_block_letter(unsigned int start_col, unsigned int start_row,
                               const uint8_t *rows) {
    unsigned int r, c;
    for (r = 0; r < TITLE_LETTER_H; r++) {
        for (c = 0; c < TITLE_LETTER_W; c++) {
            if ((rows[r] >> (TITLE_LETTER_W - 1 - c)) & 1u) {
                unsigned int col = start_col + c;
                unsigned int row = start_row + r;
                if (col < C64_SCREEN_COLS && row < C64_SCREEN_ROWS) {
                    unsigned int off = C64_SCREEN_OFFSET(col, row);
                    C64_SCREEN_RAM[off] = CHAR_BLOCK;
                    C64_COLOR_RAM[off] = TITLE_WORDMARK_COLOR;
                }
            }
        }
    }
}

void video_draw_title(void) {
    unsigned int i;
    unsigned int word_w = 7 * TITLE_LETTER_W + 6 * TITLE_LETTER_GAP;
    unsigned int start_col = (C64_SCREEN_COLS - word_w) / 2;
    unsigned int start_row = 6;

    clear_screen();
    for (i = 0; i < 7u; i++) {
        draw_block_letter(start_col + i * (TITLE_LETTER_W + TITLE_LETTER_GAP), start_row,
                         title_letters[i]);
    }
    put_string_at(13, 23, "PRESS ANY KEY", 13, COLOR_HUD);
}

void video_draw_start_help(uint8_t start_level) {
    unsigned int base_row = 8;
    unsigned int col_start;
    const unsigned int block_w = 14;

    clear_screen();
    col_start = (C64_SCREEN_COLS - 14) / 2;
    put_string_at(col_start, base_row, "START LEVEL  ", 13, COLOR_HUD);
    C64_SCREEN_RAM[C64_SCREEN_OFFSET(col_start + 13, base_row)] =
        (unsigned char)('0' + (start_level % 10));
    C64_COLOR_RAM[C64_SCREEN_OFFSET(col_start + 13, base_row)] = COLOR_HUD;

    base_row += 2;
    col_start = (C64_SCREEN_COLS - block_w) / 2;
    put_string_at(col_start, base_row, "0-9     SET", 11, COLOR_HUD);
    put_string_at(col_start, base_row + 1, "RETURN  START", 13, COLOR_HUD);

    base_row += 3;
    put_string_at(col_start, base_row, "A/D     MOVE", 12, COLOR_HUD);
    put_string_at(col_start, base_row + 1, "Z/X     ROTATE", 14, COLOR_HUD);
    put_string_at(col_start, base_row + 2, "SPACE   DROP", 12, COLOR_HUD);
}

void video_draw_replay_prompt(void) {
    const char *msg = "RETURN AGAIN";
    unsigned int len = 12;
    unsigned int col = (C64_SCREEN_COLS - len) / 2;
    unsigned int row = C64_SCREEN_ROWS - 1;
    put_string_at(col, row, msg, len, COLOR_HUD);
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
