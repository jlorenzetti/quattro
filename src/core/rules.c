/**
 * @file rules.c
 * @brief Movement, rotation, placement and spawn rules for Quattro.
 */

#include "board.h"
#include "piece.h"
#include "rules.h"

Rotation rotation_rotate_cw(Rotation rotation) {
    return (Rotation)((rotation + 1) % ROTATION_COUNT);
}

Rotation rotation_rotate_ccw(Rotation rotation) {
    return (Rotation)((rotation + ROTATION_COUNT - 1) % ROTATION_COUNT);
}

bool rules_can_place_piece(
    const Board *board,
    PieceKind kind,
    Rotation rotation,
    int8_t x,
    int8_t y
) {
    if (!board) return false;
    for (uint8_t i = 0; i < PIECE_BLOCK_COUNT; i++) {
        Point off = piece_get_block_offset(kind, rotation, i);
        int8_t bx = (int8_t)((int)x + (int)off.x);
        int8_t by = (int8_t)((int)y + (int)off.y);
        if (!board_is_inside(bx, by)) return false;
        if (board_get_cell(board, bx, by) != CELL_EMPTY) return false;
    }
    return true;
}

bool rules_can_move(
    const Board *board,
    const ActivePiece *piece,
    int8_t dx,
    int8_t dy
) {
    if (!board || !piece) return false;
    return rules_can_place_piece(board, piece->kind, piece->rotation,
                                 (int8_t)((int)piece->x + (int)dx),
                                 (int8_t)((int)piece->y + (int)dy));
}

bool rules_can_rotate(
    const Board *board,
    const ActivePiece *piece,
    Rotation target_rotation
) {
    if (!board || !piece) return false;
    return rules_can_place_piece(board, piece->kind, target_rotation, piece->x, piece->y);
}

void rules_lock_active_piece(Board *board, const ActivePiece *piece) {
    if (!board || !piece) return;
    Cell cell = CELL_FROM_PIECE(piece->kind);
    for (uint8_t i = 0; i < PIECE_BLOCK_COUNT; i++) {
        Point p = active_piece_get_block_position(piece, i);
        if (board_is_inside(p.x, p.y)) {
            board_set_cell(board, p.x, p.y, cell);
        }
    }
}

ActivePiece rules_make_spawn_piece(PieceKind kind) {
    ActivePiece piece = {
        .kind = kind,
        .rotation = ROT_0,
        .x = QUATTRO_SPAWN_X,
        .y = QUATTRO_SPAWN_Y
    };
    return piece;
}

bool rules_can_spawn_piece(const Board *board, PieceKind kind) {
    if (!board) return false;
    ActivePiece spawn = rules_make_spawn_piece(kind);
    return rules_can_place_piece(board, spawn.kind, spawn.rotation, spawn.x, spawn.y);
}
