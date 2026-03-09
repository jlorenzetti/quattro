#ifndef QUATTRO_CORE_BOARD_H
#define QUATTRO_CORE_BOARD_H

#include "types.h"

void board_clear(Board *board);
Cell board_get_cell(const Board *board, int8_t x, int8_t y);
void board_set_cell(Board *board, int8_t x, int8_t y, Cell value);
bool board_is_inside(int8_t x, int8_t y);
bool board_is_row_full(const Board *board, int8_t y);
uint8_t board_clear_full_rows(Board *board);

#endif
