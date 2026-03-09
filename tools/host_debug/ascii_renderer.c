/**
 * @file ascii_renderer.c
 * @brief Textual board rendering for host-side debugging.
 */

#include <stdio.h>

#include "ascii_renderer.h"
#include "board.h"
#include "game_state.h"
#include "piece.h"

static char cell_char(const GameState *state, int8_t x, int8_t y) {
    const Board *board = &state->board;
    for (uint8_t i = 0; i < PIECE_BLOCK_COUNT; i++) {
        Point p = active_piece_get_block_position(&state->active, i);
        if (p.x == x && p.y == y) return 'A';
    }
    Cell c = board_get_cell(board, x, y);
    if (c == CELL_EMPTY) return '.';
    return (char)('0' + c);
}

void ascii_render_board(const GameState *state) {
    if (!state) return;
    printf("+----------+\n");
    for (int8_t y = 0; y < (int8_t)QUATTRO_BOARD_HEIGHT; y++) {
        printf("|");
        for (int8_t x = 0; x < QUATTRO_BOARD_WIDTH; x++) {
            putchar(cell_char(state, x, y));
        }
        printf("|\n");
    }
    printf("+----------+\n");
}

void ascii_render_status(const GameState *state) {
    if (!state) return;
    printf("Next: %u  Score: %lu  Lines: %u  Level: %u  %s\n",
           (unsigned)state->next_piece,
           (unsigned long)state->score.score,
           (unsigned)state->score.lines_cleared,
           (unsigned)state->score.level,
           state->phase == GAME_PHASE_GAME_OVER ? "GAME OVER" : "");
}
