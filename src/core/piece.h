#ifndef QUATTRO_CORE_PIECE_H
#define QUATTRO_CORE_PIECE_H

#include "types.h"

Point piece_get_block_offset(PieceKind kind, Rotation rotation, uint8_t index);
Point active_piece_get_block_position(const ActivePiece *piece, uint8_t index);

#endif
