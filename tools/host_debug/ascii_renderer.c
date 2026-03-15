/**
 * @file ascii_renderer.c
 * @brief Textual board rendering for host-side debugging.
 */

#include <stdio.h>

#include "ascii_renderer.h"
#include "board.h"
#include "game_state.h"
#include "piece.h"

/**
 * Prints a single cell (2 chars): "[]" if filled (locked or active), " ." if empty.
 */
static void put_cell(const GameState *state, int8_t x, int8_t y) {
    bool filled = false;
    for (uint8_t i = 0; i < PIECE_BLOCK_COUNT; i++) {
        Point p = active_piece_get_block_position(&state->active, i);
        if (p.x == x && p.y == y) {
            filled = true;
            break;
        }
    }
    if (!filled) {
        Cell c = board_get_cell(&state->board, x, y);
        filled = (c != CELL_EMPTY);
    }
    fputs(filled ? "[]" : " .", stdout);
}

void ascii_render_status(const GameState *state) {
    if (!state) return;
    printf("LINES: %04u\n", (unsigned)state->score.lines_cleared);
    printf("LEVEL: %02u\n", (unsigned)state->score.level);
    printf("SCORE: %04lu\n", (unsigned long)state->score.score);
}

void ascii_render_board(const GameState *state) {
    if (!state) return;
    putchar('\n');
    /* Well open at top; each row: <! ... !> */
    for (int8_t y = 0; y < (int8_t)QUATTRO_BOARD_HEIGHT; y++) {
        printf("<!");
        for (int8_t x = 0; x < QUATTRO_BOARD_WIDTH; x++)
            put_cell(state, x, y);
        printf("!>\n");
    }
    printf("<!");
    for (int8_t x = 0; x < QUATTRO_BOARD_WIDTH * 2; x++) putchar('=');
    printf("!>\n");
}
