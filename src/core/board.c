/**
 * @file board.c
 * @brief Board state and row operations for Quattro.
 */

#include <stddef.h>

#include "board.h"

void board_clear(Board *board) {
    if (!board) return;
    for (size_t i = 0; i < QUATTRO_BOARD_WIDTH * QUATTRO_BOARD_HEIGHT; i++) {
        board->cells[i] = CELL_EMPTY;
    }
}

Cell board_get_cell(const Board *board, int8_t x, int8_t y) {
    if (!board || !board_is_inside(x, y)) return CELL_EMPTY;
    return board->cells[(size_t)y * QUATTRO_BOARD_WIDTH + (size_t)x];
}

void board_set_cell(Board *board, int8_t x, int8_t y, Cell value) {
    if (!board || !board_is_inside(x, y)) return;
    board->cells[(size_t)y * QUATTRO_BOARD_WIDTH + (size_t)x] = value;
}

bool board_is_inside(int8_t x, int8_t y) {
    return x >= 0 && x < QUATTRO_BOARD_WIDTH &&
           y >= 0 && y < (int8_t)QUATTRO_BOARD_HEIGHT;
}

bool board_is_row_full(const Board *board, int8_t y) {
    if (!board || y < 0 || y >= (int8_t)QUATTRO_BOARD_HEIGHT) return false;
    for (int8_t x = 0; x < QUATTRO_BOARD_WIDTH; x++) {
        if (board_get_cell(board, x, y) == CELL_EMPTY) return false;
    }
    return true;
}

uint8_t board_clear_full_rows(Board *board) {
    if (!board) return 0;
    uint8_t cleared = 0;
    int write_y = (int)QUATTRO_BOARD_HEIGHT - 1;
    for (int read_y = (int)QUATTRO_BOARD_HEIGHT - 1; read_y >= 0; read_y--) {
        if (board_is_row_full(board, (int8_t)read_y)) {
            cleared++;
            continue;
        }
        if (write_y != read_y) {
            for (int8_t x = 0; x < QUATTRO_BOARD_WIDTH; x++) {
                board_set_cell(board, x, (int8_t)write_y, board_get_cell(board, x, (int8_t)read_y));
            }
        }
        write_y--;
    }
    for (; write_y >= 0; write_y--) {
        for (int8_t x = 0; x < QUATTRO_BOARD_WIDTH; x++) {
            board_set_cell(board, x, (int8_t)write_y, CELL_EMPTY);
        }
    }
    return cleared;
}
