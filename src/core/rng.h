#ifndef QUATTRO_CORE_RNG_H
#define QUATTRO_CORE_RNG_H

#include "types.h"

void rng_seed(RngState *rng, uint32_t seed);
uint32_t rng_next_u32(RngState *rng);
PieceKind rng_next_piece(RngState *rng);

#endif
