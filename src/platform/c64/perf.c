/**
 * @file perf.c
 * @brief CIA2 Timer A based section timing for QUATTRO_PERF builds.
 *
 * After QUATTRO_PERF_FRAMES steady APP_GAME frames, writes average cycle counts
 * to the bottom of the screen (high-contrast) and halts the main loop (see perf_halted).
 */

#ifdef QUATTRO_PERF

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "memory_map.h"
#include "perf.h"

/** CIA2: Timer A low/high, control A. */
#define CIA2_TA_LO (*(volatile unsigned char *)0xDD04)
#define CIA2_TA_HI (*(volatile unsigned char *)0xDD05)
#define CIA2_CRA (*(volatile unsigned char *)0xDD0E)

/** Screen / color RAM (same layout as video). */
#define PERF_SCREEN  C64_SCREEN_RAM
#define PERF_COLOR_R C64_COLOR_RAM

/** Foreground color: white on black (readable in VICE without reverse hacks). */
#define PERF_FG_COLOR 1u

/**
 * Dump on bottom screen rows (frozen frame at halt).
 * Row 21: PERF_APPLY bad samples by command bucket (an/am/ar/sd).
 * Row 22: per-section bad counts and max accepted delta.
 * Row 23: section averages; row 24: CIA-only calibration.
 */
#define PERF_ROW_APPLY_BUCKS 21u
#define PERF_ROW_STATS 22u
#define PERF_ROW_MAIN 23u
#define PERF_ROW_CAL  24u

#define PERF_OFF(row, col) (C64_SCREEN_OFFSET((col), (row)))

/** Reject per-sample deltas above this as multi-wrap / garbage (order-of-magnitude guard). */
#define PERF_DELTA_SANITY_MAX 50000u

/** Steady-state APP_GAME frames to aggregate before dump + halt. */
#ifndef QUATTRO_PERF_FRAMES
#define QUATTRO_PERF_FRAMES 500u
#endif

static uint16_t g_stash[PERF_SECTION_COUNT];
static uint32_t g_sum[PERF_SECTION_COUNT];
static uint32_t g_count[PERF_SECTION_COUNT];
/** Samples discarded (multi-wrap or failed sanity). */
static uint32_t g_bad_samples[PERF_SECTION_COUNT];
/** Largest accepted delta per section (cycles). */
static uint32_t g_max_delta[PERF_SECTION_COUNT];
/**
 * PERF_APPLY discards attributed by command class: none, move (L/R), rotate (CW/CCW), soft drop.
 */
static uint32_t g_apply_bad_bucket[4];
static uint32_t g_frames_measured;
static bool g_halted;

/* CIA-only micro-calibration: 200 vs 5000 XOR iterations (same inner loop). */
static uint16_t g_cal_short_cia;
/** Total cycles for 5000 iterations (25 chunks × 200; summed as uint32_t to avoid timer multi-wrap). */
static uint32_t g_cal_long_sum;
/** Volatile sink so calibration delay loops are not optimized away at -Os. */
static volatile uint16_t g_cal_sink;

/** Short loop length; long path runs 25× this (5000 XORs total), one timer read per chunk. */
#define PERF_CAL_SHORT_ITERS 200u
#define PERF_CAL_LONG_CHUNKS 25u

/**
 * @brief Reads CIA2 Timer A (16-bit). Read LSB first (CIA latch).
 */
static uint16_t perf_read_timer(void) {
    unsigned char lo = CIA2_TA_LO;
    unsigned char hi = CIA2_TA_HI;
    return (uint16_t)lo | ((uint16_t)hi << 8u);
}

/**
 * @brief Arms CIA2 Timer A: continuous countdown from 0xFFFF at Phi2 rate.
 */
static void perf_timer_arm(void) {
    CIA2_CRA = 0u;
    CIA2_TA_LO = 0xFFu;
    CIA2_TA_HI = 0xFFu;
    CIA2_CRA = 0x11u;
}

/**
 * @brief Elapsed cycles for a down-counting 16-bit free-running timer across at most one wrap.
 */
static uint32_t perf_elapsed_down(uint16_t t0, uint16_t t1) {
    if (t1 <= t0) {
        return (uint32_t)(t0 - t1);
    }
    /* One wrap: counted down past 0 and reloaded. */
    return (uint32_t)t0 + (0x10000u - (uint32_t)t1);
}

static void perf_clear_row(unsigned int row) {
    unsigned int c;
    unsigned int base = PERF_OFF(row, 0u);
    for (c = 0; c < C64_SCREEN_COLS; c++) {
        PERF_SCREEN[base + c] = (unsigned char)' ';
        PERF_COLOR_R[base + c] = (unsigned char)PERF_FG_COLOR;
    }
}

static void perf_put_hex2(unsigned int off, uint8_t v) {
    static const char hex[] = "0123456789ABCDEF";
    PERF_SCREEN[off + 0u] = (unsigned char)hex[(v >> 4u) & 0x0Fu];
    PERF_SCREEN[off + 1u] = (unsigned char)hex[v & 0x0Fu];
    PERF_COLOR_R[off + 0u] = (unsigned char)PERF_FG_COLOR;
    PERF_COLOR_R[off + 1u] = (unsigned char)PERF_FG_COLOR;
}

static void perf_put_hex4(unsigned int off, uint16_t v) {
    static const char hex[] = "0123456789ABCDEF";
    unsigned int i;
    for (i = 0; i < 4u; i++) {
        PERF_SCREEN[off + i] = (unsigned char)hex[(v >> (12u - (i * 4u))) & 0x0Fu];
        PERF_COLOR_R[off + i] = (unsigned char)PERF_FG_COLOR;
    }
}

static void perf_put_hex8(unsigned int off, uint32_t v) {
    static const char hex[] = "0123456789ABCDEF";
    unsigned int i;
    for (i = 0; i < 8u; i++) {
        PERF_SCREEN[off + i] =
            (unsigned char)hex[(v >> (28u - (i * 4u))) & 0x0Fu];
        PERF_COLOR_R[off + i] = (unsigned char)PERF_FG_COLOR;
    }
}

static void perf_put_char(unsigned int off, char ch) {
    PERF_SCREEN[off] = (unsigned char)ch;
    PERF_COLOR_R[off] = (unsigned char)PERF_FG_COLOR;
}

static uint16_t perf_avg(unsigned int s) {
    return g_count[s] > 0u ? (uint16_t)(g_sum[s] / g_count[s]) : 0u;
}

static void perf_calibrate(void) {
    volatile uint16_t i;
    uint16_t t0, t1;
    unsigned int chunk;
    uint32_t long_sum = 0u;

    g_cal_sink = 0u;
    t0 = perf_read_timer();
    for (i = 0; i < PERF_CAL_SHORT_ITERS; i++) {
        g_cal_sink ^= i;
    }
    t1 = perf_read_timer();
    g_cal_short_cia = (uint16_t)perf_elapsed_down(t0, t1);

    for (chunk = 0; chunk < PERF_CAL_LONG_CHUNKS; chunk++) {
        t0 = perf_read_timer();
        for (i = 0; i < PERF_CAL_SHORT_ITERS; i++) {
            g_cal_sink ^= i;
        }
        t1 = perf_read_timer();
        long_sum += perf_elapsed_down(t0, t1);
    }
    g_cal_long_sum = long_sum;
}

/**
 * @brief Maps gameplay Command to a 0..3 bucket for apply bad attribution.
 */
static unsigned int perf_apply_bucket_index(Command cmd) {
    switch (cmd) {
    case CMD_NONE:
        return 0u;
    case CMD_MOVE_LEFT:
    case CMD_MOVE_RIGHT:
        return 1u;
    case CMD_ROTATE_CW:
    case CMD_ROTATE_CCW:
        return 2u;
    case CMD_SOFT_DROP:
        return 3u;
    default:
        return 0u;
    }
}

/**
 * @brief Row 21: `an##am##ar##sd##` — bad PERF_APPLY discards per bucket (2 hex each, capped 0xFF).
 */
static void perf_dump_apply_bad_row(unsigned int base) {
    static const char lab[4][2] = { { 'a', 'n' }, { 'a', 'm' }, { 'a', 'r' }, { 's', 'd' } };
    unsigned int b;
    for (b = 0; b < 4u; b++) {
        unsigned int off = base + b * 4u;
        uint8_t n = (g_apply_bad_bucket[b] > 0xFFu) ? 0xFFu : (uint8_t)g_apply_bad_bucket[b];
        perf_put_char(off + 0u, lab[b][0]);
        perf_put_char(off + 1u, lab[b][1]);
        perf_put_hex2(off + 2u, n);
    }
}

/**
 * @brief One row: per section `Lbb#mmmm` — letter, bad count (2 hex, capped 0xFF), `#`, max delta (4 hex).
 */
static void perf_dump_stats_row(unsigned int base) {
    static const char letters[] = "IABHG";
    unsigned int s;
    for (s = 0; s < (unsigned int)PERF_SECTION_COUNT; s++) {
        unsigned int off = base + s * 8u;
        uint8_t bad8 =
            (g_bad_samples[s] > 0xFFu) ? 0xFFu : (uint8_t)g_bad_samples[s];
        uint16_t max16 = (g_max_delta[s] > 0xFFFFu) ? 0xFFFFu : (uint16_t)g_max_delta[s];
        perf_put_char(off + 0u, letters[s]);
        perf_put_hex2(off + 1u, bad8);
        perf_put_char(off + 3u, '#');
        perf_put_hex4(off + 4u, max16);
    }
}

static void perf_dump(void) {
    unsigned int base_apply = PERF_OFF(PERF_ROW_APPLY_BUCKS, 0u);
    unsigned int base_stats = PERF_OFF(PERF_ROW_STATS, 0u);
    unsigned int base0 = PERF_OFF(PERF_ROW_MAIN, 0u);
    unsigned int base1 = PERF_OFF(PERF_ROW_CAL, 0u);
    unsigned int i;
    unsigned int any_bad = 0u;

    perf_clear_row(PERF_ROW_APPLY_BUCKS);
    perf_clear_row(PERF_ROW_STATS);
    perf_clear_row(PERF_ROW_MAIN);
    perf_clear_row(PERF_ROW_CAL);

    perf_dump_apply_bad_row(base_apply);
    perf_dump_stats_row(base_stats);

    /* Row 23: I/A/B/H/G averages + optional X if any bad samples. */
    perf_put_char(base0 + 0u, 'I');
    perf_put_hex4(base0 + 1u, perf_avg((unsigned int)PERF_INPUT));
    perf_put_char(base0 + 5u, ' ');
    perf_put_char(base0 + 6u, 'A');
    perf_put_hex4(base0 + 7u, perf_avg((unsigned int)PERF_APPLY));
    perf_put_char(base0 + 11u, ' ');
    perf_put_char(base0 + 12u, 'B');
    perf_put_hex4(base0 + 13u, perf_avg((unsigned int)PERF_BOARD));
    perf_put_char(base0 + 17u, ' ');
    perf_put_char(base0 + 18u, 'H');
    perf_put_hex4(base0 + 19u, perf_avg((unsigned int)PERF_HUD));
    perf_put_char(base0 + 23u, ' ');
    perf_put_char(base0 + 24u, 'G');
    perf_put_hex4(base0 + 25u, perf_avg((unsigned int)PERF_GRAVITY));

    for (i = 0; i < (unsigned int)PERF_SECTION_COUNT; i++) {
        if (g_bad_samples[i] > 0u) { any_bad = 1u; break; }
    }
    perf_put_char(base0 + 30u, any_bad ? 'X' : ' ');

    /* Row 24: CIA-only calibration; C2 is 8 hex digits (full 5000-iter cycle sum). */
    perf_put_char(base1 + 0u, 'C');
    perf_put_char(base1 + 1u, '1');
    perf_put_hex4(base1 + 2u, g_cal_short_cia);
    perf_put_char(base1 + 6u, ' ');
    perf_put_char(base1 + 7u, 'C');
    perf_put_char(base1 + 8u, '2');
    perf_put_hex8(base1 + 9u, g_cal_long_sum);
}

void perf_init(void) {
    unsigned int i;
    for (i = 0; i < (unsigned int)PERF_SECTION_COUNT; i++) {
        g_stash[i] = 0u;
        g_sum[i] = 0u;
        g_count[i] = 0u;
        g_bad_samples[i] = 0u;
        g_max_delta[i] = 0u;
    }
    for (i = 0; i < 4u; i++) {
        g_apply_bad_bucket[i] = 0u;
    }
    g_frames_measured = 0u;
    g_halted = false;
    perf_timer_arm();
    perf_calibrate();
}

void perf_frame_end(void) {
    if (g_halted) return;
    g_frames_measured++;
    if (g_frames_measured >= QUATTRO_PERF_FRAMES) {
        perf_dump();
        g_halted = true;
    }
}

bool perf_halted(void) { return g_halted; }

void perf_section_start(PerfSection section) {
    if (g_halted) return;
    g_stash[(unsigned int)section] = perf_read_timer();
}

static void perf_section_end_impl(PerfSection section, const Command *apply_cmd) {
    uint16_t t1;
    uint32_t delta;
    if (g_halted) return;
    t1 = perf_read_timer();
    delta = perf_elapsed_down(g_stash[(unsigned int)section], t1);
    if (delta > PERF_DELTA_SANITY_MAX) {
        g_bad_samples[(unsigned int)section]++;
        if (section == PERF_APPLY && apply_cmd != NULL) {
            g_apply_bad_bucket[perf_apply_bucket_index(*apply_cmd)]++;
        }
        return;
    }
    g_sum[(unsigned int)section] += delta;
    g_count[(unsigned int)section]++;
    if (delta > g_max_delta[(unsigned int)section]) {
        g_max_delta[(unsigned int)section] = delta;
    }
}

void perf_section_end(PerfSection section) { perf_section_end_impl(section, NULL); }

void perf_section_end_apply(Command command) {
    perf_section_end_impl(PERF_APPLY, &command);
}

#endif
