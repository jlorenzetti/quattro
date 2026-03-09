#include "rng.h"

void rng_seed(RngState *rng, uint32_t seed) {
    if (rng) rng->state = seed ? seed : 1u;
}

uint32_t rng_next_u32(RngState *rng) {
    if (!rng) return 1u;
    rng->state = rng->state * 1664525u + 1013904223u;
    return rng->state;
}

PieceKind rng_next_piece(RngState *rng) {
    return (PieceKind)(rng_next_u32(rng) % PIECE_KIND_COUNT);
}
