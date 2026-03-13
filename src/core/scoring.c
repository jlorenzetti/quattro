/**
 * @file scoring.c
 * @brief Score and level progression for Quattro.
 */

#include "scoring.h"

/** Points per line (classic-style: 1=100, 2=300, 3=500, 4=800). */
static const uint32_t LINE_SCORE[] = { 0, 100, 300, 500, 800 };

void scoring_reset(ScoreState *score, uint8_t start_level) {
    if (!score) return;
    if (start_level > 9) start_level = 9;
    score->score = 0;
    score->lines_cleared = 0;
    score->initial_level = start_level;
    score->level = start_level;
}

void scoring_apply_line_clear(ScoreState *score, uint8_t cleared_lines) {
    if (!score) return;
    if (cleared_lines > 4) cleared_lines = 4;
    score->score += LINE_SCORE[cleared_lines];
    score->lines_cleared += cleared_lines;
    score->level = scoring_compute_level(score->lines_cleared, score->initial_level);
}

uint8_t scoring_compute_level(uint16_t total_lines_cleared, uint8_t start_level) {
    uint16_t level = (uint16_t)start_level + total_lines_cleared / 10;
    if (level > 255) level = 255;
    return (uint8_t)level;
}
