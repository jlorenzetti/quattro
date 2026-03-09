#ifndef QUATTRO_CORE_SCORING_H
#define QUATTRO_CORE_SCORING_H

#include "types.h"

void scoring_reset(ScoreState *score);
void scoring_apply_line_clear(ScoreState *score, uint8_t cleared_lines);
uint8_t scoring_compute_level(uint16_t total_lines_cleared);

#endif
