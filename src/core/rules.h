#ifndef QUATTRO_CORE_RULES_H
#define QUATTRO_CORE_RULES_H

#include "types.h"

Rotation rotation_rotate_cw(Rotation rotation);
Rotation rotation_rotate_ccw(Rotation rotation);

bool rules_can_place_piece(
    const Board *board,
    PieceKind kind,
    Rotation rotation,
    int8_t x,
    int8_t y
);

bool rules_can_move(
    const Board *board,
    const ActivePiece *piece,
    int8_t dx,
    int8_t dy
);

bool rules_can_rotate(
    const Board *board,
    const ActivePiece *piece,
    Rotation target_rotation
);

void rules_lock_active_piece(Board *board, const ActivePiece *piece);
ActivePiece rules_make_spawn_piece(PieceKind kind);
bool rules_can_spawn_piece(const Board *board, PieceKind kind);

#endif
