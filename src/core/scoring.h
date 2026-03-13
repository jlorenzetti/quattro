#ifndef QUATTRO_CORE_SCORING_H
#define QUATTRO_CORE_SCORING_H

#include "types.h"

/** Resets score and sets initial level (0–9). Level progresses as initial_level + lines_cleared/10. */
void scoring_reset(ScoreState *score, uint8_t start_level);
void scoring_apply_line_clear(ScoreState *score, uint8_t cleared_lines);
uint8_t scoring_compute_level(uint16_t total_lines_cleared, uint8_t start_level);

#endif
