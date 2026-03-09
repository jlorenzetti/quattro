/**
 * @file test_board.c
 * @brief Tests for board module.
 */

#include <stdbool.h>
#include <stddef.h>

#include "board.h"
#include "types.h"

int test_board_run(void) {
    Board board;
    board_clear(&board);

    if (!board_is_inside(0, 0)) return 1;
    if (!board_is_inside(9, 21)) return 1;
    if (board_is_inside(-1, 0)) return 1;
    if (board_is_inside(10, 0)) return 1;
    if (board_is_inside(0, 22)) return 1;

    if (board_get_cell(&board, 0, 0) != CELL_EMPTY) return 1;
    board_set_cell(&board, 3, 5, CELL_FROM_PIECE(PIECE_T));
    if (board_get_cell(&board, 3, 5) != CELL_FROM_PIECE(PIECE_T)) return 1;

    for (int8_t x = 0; x < QUATTRO_BOARD_WIDTH; x++) {
        board_set_cell(&board, x, 10, CELL_FROM_PIECE(PIECE_I));
    }
    if (!board_is_row_full(&board, 10)) return 1;
    if (board_is_row_full(&board, 9)) return 1;

    uint8_t cleared = board_clear_full_rows(&board);
    if (cleared != 1) return 1;
    if (board_get_cell(&board, 3, 10) != CELL_EMPTY) return 1;

    return 0;
}
