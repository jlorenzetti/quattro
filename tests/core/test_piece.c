/**
 * @file test_piece.c
 * @brief Tests for piece module.
 */

#include "piece.h"
#include "types.h"

/** Invariant: every shape lives in 4×4 box and has exactly 4 distinct blocks. */
static int test_piece_4x4_invariant(void) {
    for (PieceKind kind = PIECE_I; kind < PIECE_KIND_COUNT; kind++) {
        for (Rotation rot = ROT_0; rot < ROTATION_COUNT; rot++) {
            Point blocks[PIECE_BLOCK_COUNT];
            for (uint8_t i = 0; i < PIECE_BLOCK_COUNT; i++) {
                Point p = piece_get_block_offset(kind, rot, i);
                if (p.x < 0 || p.x > 3 || p.y < 0 || p.y > 3) return 1;
                blocks[i] = p;
            }
            for (uint8_t i = 0; i < PIECE_BLOCK_COUNT; i++) {
                for (uint8_t j = i + 1; j < PIECE_BLOCK_COUNT; j++) {
                    if (blocks[i].x == blocks[j].x && blocks[i].y == blocks[j].y) return 1;
                }
            }
        }
    }
    return 0;
}

int test_piece_run(void) {
    if (test_piece_4x4_invariant() != 0) return 1;

    Point off = piece_get_block_offset(PIECE_I, ROT_0, 0);
    if (off.x != 0 || off.y != 0) return 1;
    off = piece_get_block_offset(PIECE_I, ROT_0, 1);
    if (off.x != 1 || off.y != 0) return 1;

    /* O centered in 4x4: (1,0),(2,0),(1,1),(2,1) */
    off = piece_get_block_offset(PIECE_O, ROT_0, 0);
    if (off.x != 1 || off.y != 0) return 1;
    off = piece_get_block_offset(PIECE_O, ROT_0, 3);
    if (off.x != 2 || off.y != 1) return 1;

    /* O at spawn (3,0): first block offset (1,0) -> board (4,0) */
    ActivePiece ap = { .kind = PIECE_O, .rotation = ROT_0, .x = QUATTRO_SPAWN_X, .y = QUATTRO_SPAWN_Y };
    Point p = active_piece_get_block_position(&ap, 0);
    if (p.x != 4 || p.y != 0) return 1;

    return 0;
}
