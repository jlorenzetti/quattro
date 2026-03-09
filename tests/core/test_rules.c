/**
 * @file test_rules.c
 * @brief Tests for rules module.
 */

#include "board.h"
#include "rules.h"
#include "types.h"

int test_rules_run(void) {
    Board board;
    board_clear(&board);

    /* O at spawn (3,0) is placeable on empty board */
    if (!rules_can_place_piece(&board, PIECE_O, ROT_0, QUATTRO_SPAWN_X, QUATTRO_SPAWN_Y)) return 1;
    if (rules_can_place_piece(&board, PIECE_I, ROT_0, 10, 0)) return 1;

    /* O at (3,0) occupies (4,0),(5,0),(4,1),(5,1); block (4,0) to forbid spawn */
    board_set_cell(&board, 4, 0, CELL_FROM_PIECE(PIECE_L));
    if (rules_can_place_piece(&board, PIECE_O, ROT_0, QUATTRO_SPAWN_X, QUATTRO_SPAWN_Y)) return 1;

    board_clear(&board);
    /* T ROT_0 at (4,5): blocks at (6,5),(5,6),(6,6),(7,6) */
    ActivePiece piece = { .kind = PIECE_T, .rotation = ROT_0, .x = 4, .y = 5 };
    if (!rules_can_move(&board, &piece, 0, 1)) return 1;
    if (rules_can_move(&board, &piece, -10, 0)) return 1;

    Rotation cw = rotation_rotate_cw(ROT_0);
    if (cw != ROT_90) return 1;
    Rotation ccw = rotation_rotate_ccw(ROT_90);
    if (ccw != ROT_0) return 1;

    if (!rules_can_rotate(&board, &piece, ROT_90)) return 1;

    rules_lock_active_piece(&board, &piece);
    if (board_get_cell(&board, 6, 6) != CELL_FROM_PIECE(PIECE_T)) return 1;

    board_clear(&board);
    ActivePiece spawn = rules_make_spawn_piece(PIECE_O);
    if (spawn.x != QUATTRO_SPAWN_X || spawn.y != QUATTRO_SPAWN_Y) return 1;
    if (!rules_can_spawn_piece(&board, PIECE_O)) return 1;

    return 0;
}
