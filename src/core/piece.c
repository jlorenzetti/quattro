/**
 * @file piece.c
 * @brief Piece block offsets and active-piece block positions for Quattro.
 */

#include "piece.h"

/**
 * Block offsets from piece origin, per kind and rotation.
 * Convention: spawn is top-left of a conceptual 4x4 cell; all shapes are
 * centered in that 4x4 so one spawn (QUATTRO_SPAWN_X, QUATTRO_SPAWN_Y) gives
 * consistent visual centering on the 10-wide board. Origin + offset = board position.
 * Index [kind][rotation][block_index].
 */
static const Point PIECE_OFFSETS[PIECE_KIND_COUNT][ROTATION_COUNT][PIECE_BLOCK_COUNT] = {
    /* I: full width at ROT_0/180; vertical centered at ROT_90/270 */
    { { {0,0}, {1,0}, {2,0}, {3,0} },
      { {1,0}, {1,1}, {1,2}, {1,3} },
      { {0,0}, {1,0}, {2,0}, {3,0} },
      { {1,0}, {1,1}, {1,2}, {1,3} } },
    /* O: 2x2 centered in 4x4 */
    { { {1,0}, {2,0}, {1,1}, {2,1} },
      { {1,0}, {2,0}, {1,1}, {2,1} },
      { {1,0}, {2,0}, {1,1}, {2,1} },
      { {1,0}, {2,0}, {1,1}, {2,1} } },
    /* T */
    { { {2,0}, {1,1}, {2,1}, {3,1} },
      { {1,0}, {1,1}, {1,2}, {2,1} },
      { {1,0}, {2,0}, {3,0}, {2,1} },
      { {2,0}, {2,1}, {2,2}, {1,1} } },
    /* S */
    { { {2,0}, {3,0}, {1,1}, {2,1} },
      { {1,0}, {1,1}, {2,1}, {2,2} },
      { {1,0}, {2,0}, {2,1}, {3,1} },
      { {2,0}, {2,1}, {1,1}, {1,2} } },
    /* Z */
    { { {1,0}, {2,0}, {2,1}, {3,1} },
      { {2,0}, {1,1}, {2,1}, {1,2} },
      { {1,0}, {2,0}, {2,1}, {3,1} },
      { {2,0}, {1,1}, {2,1}, {1,2} } },
    /* J */
    { { {1,0}, {1,1}, {2,1}, {3,1} },
      { {1,0}, {2,0}, {1,1}, {1,2} },
      { {1,0}, {2,0}, {3,0}, {3,1} },
      { {2,0}, {2,1}, {2,2}, {1,2} } },
    /* L */
    { { {3,0}, {1,1}, {2,1}, {3,1} },
      { {1,0}, {1,1}, {1,2}, {2,2} },
      { {1,0}, {2,0}, {3,0}, {1,1} },
      { {1,0}, {2,0}, {2,1}, {2,2} } },
};

Point piece_get_block_offset(PieceKind kind, Rotation rotation, uint8_t index) {
    if (kind >= PIECE_KIND_COUNT || rotation >= ROTATION_COUNT || index >= PIECE_BLOCK_COUNT) {
        Point p = {0, 0};
        return p;
    }
    return PIECE_OFFSETS[kind][rotation][index];
}

Point active_piece_get_block_position(const ActivePiece *piece, uint8_t index) {
    Point p = {0, 0};
    if (!piece || index >= PIECE_BLOCK_COUNT) return p;
    Point off = piece_get_block_offset(piece->kind, piece->rotation, index);
    p.x = (int8_t)((int)piece->x + (int)off.x);
    p.y = (int8_t)((int)piece->y + (int)off.y);
    return p;
}
