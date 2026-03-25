/**
 * @file perf.h
 * @brief Optional C64 frame-section timing (build with -DQUATTRO_PERF=1).
 *
 * Uses CIA2 Timer A as a free-running countdown for lightweight cycle deltas.
 * Linked only for `make c64_perf` (see Makefile).
 */

#ifndef QUATTRO_PLATFORM_C64_PERF_H
#define QUATTRO_PLATFORM_C64_PERF_H

#include <stdbool.h>
#include <stdint.h>

#ifdef QUATTRO_PERF

#include "types.h"

/** Section indices for perf_section_start/end. */
typedef enum {
    PERF_INPUT = 0,
    PERF_APPLY,
    PERF_BOARD,
    PERF_HUD,
    PERF_GRAVITY,
    PERF_SECTION_COUNT
} PerfSection;

/**
 * @brief Initializes CIA2 Timer A for profiling (free-running countdown).
 */
void perf_init(void);

/**
 * @brief Call once at the end of each APP_GAME frame (after gravity block).
 */
void perf_frame_end(void);

/**
 * @brief True after perf_dump and the build should halt the loop (see main).
 */
bool perf_halted(void);

/**
 * @brief Marks the start of a timed section (reads timer).
 */
void perf_section_start(PerfSection section);

/**
 * @brief Marks the end of a timed section (accumulates sum/count; discards insane deltas).
 */
void perf_section_end(PerfSection section);

/**
 * @brief Ends the PERF_APPLY section; pass the same `command` passed to `game_apply_command`.
 *         Discarded samples are attributed to a command bucket (none / move / rotate / soft-drop).
 */
void perf_section_end_apply(Command command);

#endif

#endif
