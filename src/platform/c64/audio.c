/**
 * @file audio.c
 * @brief Minimal SID pulse one-shots (micro SFX) for the C64 build.
 *
 * - **Voice 2 (menu):** title motive and start confirm. Title: five-note A-minor figure, triangle + low-pass per
 *   note with a short cutoff sweep; filter bypass after. Start confirm: two pulse blips (C→A'). Gameplay cues use
 *   pulse on voice 1 so the title can use triangle + filter without carrying colour into play.
 * - **Voice 1 (gameplay):** rotate, lock, line-clear arpeggio, game over. ~50% pulse width for level.
 *
 * Splitting menu and gameplay voices avoids envelope carry-over on the first lock after start.
 *
 * **Boot:** clear the SID register window, then park each used voice: non-zero frequency, PW 0x800, ADSR zero,
 * waveform selected, **gate off**. Avoid pulse at frequency 0.
 *
 * **Each one-shot:** quiesce the other voice, park active, load patch (gate off), short settle, gate on,
 * `spin_iters` hold, gate off, repark. Delays are busy-wait iterations, not milliseconds.
 *
 * Model/emulator variance (weak attack, clicks) is possible; register use is conventional (no TEST strobe).
 */

#include "audio.h"

#ifndef QUATTRO_AUDIO
#define QUATTRO_AUDIO 1
#endif

#if QUATTRO_AUDIO

#define SID ((volatile uint8_t *)0xD400UL)
#define SID_MODE_VOL ((volatile uint8_t *)0xD418UL)

/** Filter cutoff 11-bit (`$D415` / `$D416`). */
#define SID_OFF_FILTER_FC_LO 0x15U
#define SID_OFF_FILTER_FC_HI 0x16U
/** Resonance (low nibble) + voice routing to filter (bits 4–6 = voices 1–3). */
#define SID_OFF_FILTER_RES_ROUT 0x17U

/** Byte offset from `$D400` to each voice block: `freq_lo`, `freq_hi`, `pw_lo`, `pw_hi`, `ctrl`, `ad`, `sr`. */
enum {
    SID_V1 = 0,
    SID_V2 = 7
};

/** Pulse waveform selected, gate bit clear (EG in release toward zero). */
#define VOICE_CTRL_PULSE_GATE_OFF 0x40U
/** Pulse waveform selected, gate bit set (EG runs attack/decay/sustain while gate held). */
#define VOICE_CTRL_PULSE_GATE_ON 0x41U
/** Triangle waveform, gate off (SID ignores pulse width for triangle). */
#define VOICE_CTRL_TRI_GATE_OFF 0x10U
/** Triangle waveform, gate on. */
#define VOICE_CTRL_TRI_GATE_ON 0x11U

/** Master volume 15 + low-pass filter mode bit (`$D418` bit 4). */
#define SID_MODE_VOL_LP 0x1FU

/** ~50% duty: 12-bit pulse width value 0x800 → `PW_LO`/`PW_HI` (not 0x0008 total, which is ~narrow). */
#define BLIP_PW_LO 0x00U
#define BLIP_PW_HI 0x08U

/** Tonic reference for line clear / start confirm (`sid_am_low_root` = one octave down). Title: `k_sid_title_a_root`. */
static const uint16_t k_sid_am_tonic_hi = 0x2E00U;

/** Gameplay single-blip frequency words (pulse SFX on voice 1). */
static const uint16_t k_sid_freq_rotate = 0x2200U;
static const uint16_t k_sid_freq_lock = 0x1600U;
static const uint16_t k_sid_freq_game_over = 0x1200U;

/** Low **A minor** root for title arpeggio (`play_title_motive`). */
static const uint16_t k_sid_title_a_root = 0x1380U;

/** Inter-note gap after each title note (`spin_iters`). */
#define SPIN_TITLE_GAP_ITERS 260U
/** Silence between line-clear arp steps (same value as title/start gap for shared pacing). */
#define ARP_GAP_LINE_CLEAR_ITERS 260U
/** Gap between the two start-confirm blips. */
#define ARP_GAP_START_CONFIRM_ITERS 260U

/** Park frequency: lowest non-zero word (avoids freq 0 with pulse enabled). */
#define VOICE_PARK_FREQ_LO 0x01U
#define VOICE_PARK_FREQ_HI 0x00U

/** Busy-wait length while gate is on (blip duration; PW dominates perceived level). */
#define BLIP_GATE_HOLD_ITERS 280U
/** Longer hold when retriggering quickly (rotate). */
#define BLIP_GATE_HOLD_ROTATE_ITERS 380U
/** Longer hold when retriggering quickly (lock). */
#define BLIP_GATE_HOLD_LOCK_ITERS 430U

/** After both voices are parked to idle, before loading the note patch (SID settle). */
#define BLIP_ARM_SETTLE_ITERS 48U
/** After PW/ADSR/frequency writes, before raising gate (EG sees stable registers). */
#define BLIP_PRE_GATE_SETTLE_ITERS 24U
/** After gate-off, before rewriting to idle row (gate bit + EG latch). */
#define BLIP_POST_GATE_SETTLE_ITERS 40U
/** Settle between gate low → reload in `blip_pulse_step_retrigger` (line clear). */
#define BLIP_RETRIGGER_SETTLE_ITERS 72U

/**
 * @brief Busy-wait for SID timing (not calibrated to milliseconds).
 *
 * Volatile loop plus asm memory barrier so `-Os` does not remove or merge the delay.
 */
static void spin_iters(unsigned n) {
    volatile unsigned i;
    for (i = 0; i < n; ++i) {
        __asm__ volatile("" ::: "memory");
    }
}

/**
 * @brief Write 11-bit SID low-pass filter cutoff (`$D415` / `$D416`).
 * @param fc11 Cutoff index 0–2047.
 */
static void sid_filter_cutoff_write(unsigned fc11) {
    if (fc11 > 2047U) {
        fc11 = 2047U;
    }
    SID[SID_OFF_FILTER_FC_LO] = (uint8_t)(fc11 & 7U);
    SID[SID_OFF_FILTER_FC_HI] = (uint8_t)(fc11 >> 3);
}

/**
 * @brief Route voice 2 through the filter and enable low-pass + master volume (title only).
 */
static void sid_filter_arm_title_v2_lp(void) {
    SID[SID_OFF_FILTER_RES_ROUT] = 0x28U;
    *SID_MODE_VOL = SID_MODE_VOL_LP;
}

/**
 * @brief Disable filter routing and filter modes; restore direct output at full volume (post-title).
 */
static void sid_filter_bypass(void) {
    SID[SID_OFF_FILTER_FC_LO] = 0U;
    SID[SID_OFF_FILTER_FC_HI] = 0U;
    SID[SID_OFF_FILTER_RES_ROUT] = 0U;
    *SID_MODE_VOL = 0x0FU;
}

/**
 * @brief Equal-temperament frequency step up (12-TET), for short structural fragments only.
 * @param f SID frequency word.
 * @param semitones Number of semitones to add (approximate; ~1.059463× per step).
 */
static uint16_t freq_word_up_semitones(uint16_t f, unsigned semitones) {
    uint32_t x = f;
    for (unsigned s = 0; s < semitones; ++s) {
        x = (x * 1059U + 500U) / 1000U;
    }
    if (x > 0xFFFFU) {
        return 0xFFFFU;
    }
    return (uint16_t)x;
}

/**
 * @brief Equal-temperament frequency step down (12-TET), inverse of `freq_word_up_semitones`.
 */
static uint16_t freq_word_down_semitones(uint16_t f, unsigned semitones) {
    uint32_t x = f;
    for (unsigned s = 0; s < semitones; ++s) {
        x = (x * 944U + 500U) / 1000U;
    }
    if (x < 1U) {
        return 1U;
    }
    if (x > 0xFFFFU) {
        return 0xFFFFU;
    }
    return (uint16_t)x;
}

/** One octave below `k_sid_am_tonic_hi` (shared anchor for line clear + start confirm). */
static uint16_t sid_am_low_root(void) {
    return freq_word_down_semitones(k_sid_am_tonic_hi, 12U);
}

/**
 * Linear interpolation of 11-bit filter cutoff between two endpoints (inclusive endpoints).
 * @param step Index 0 … steps-1.
 */
static unsigned sid_fc_lerp(unsigned fc_start, unsigned fc_end, unsigned step, unsigned steps) {
    if (steps <= 1U) {
        return fc_end;
    }
    if (fc_end >= fc_start) {
        return fc_start +
               (unsigned)(((unsigned long)fc_end - (unsigned long)fc_start) * step) / (steps - 1U);
    }
    return fc_start -
           (unsigned)(((unsigned long)fc_start - (unsigned long)fc_end) * step) / (steps - 1U);
}

/**
 * @brief Force pulse + gate off (only the control register).
 * @param base Voice base offset (`SID_V1` or `SID_V2`).
 */
static void voice_sid_gate_off_pulse(unsigned base) {
    SID[base + 4] = VOICE_CTRL_PULSE_GATE_OFF;
}

/**
 * @brief Full idle row for a voice: safe frequency, PW, ADSR zero, pulse with gate off.
 * @param base Voice base offset (`SID_V1` or `SID_V2`).
 */
static void voice_sid_park(unsigned base) {
    SID[base + 0] = VOICE_PARK_FREQ_LO;
    SID[base + 1] = VOICE_PARK_FREQ_HI;
    SID[base + 2] = BLIP_PW_LO;
    SID[base + 3] = BLIP_PW_HI;
    SID[base + 5] = 0x00U;
    SID[base + 6] = 0x00U;
    SID[base + 4] = VOICE_CTRL_PULSE_GATE_OFF;
}

/**
 * @brief Idle row for triangle: minimum frequency, ADSR zero, triangle with gate off.
 * @param base Voice base offset (`SID_V1` or `SID_V2`).
 */
static void voice_sid_park_triangle(unsigned base) {
    SID[base + 0] = VOICE_PARK_FREQ_LO;
    SID[base + 1] = VOICE_PARK_FREQ_HI;
    SID[base + 2] = 0U;
    SID[base + 3] = 0U;
    SID[base + 5] = 0x00U;
    SID[base + 6] = 0x00U;
    SID[base + 4] = VOICE_CTRL_TRI_GATE_OFF;
}

/**
 * @brief Park the non-playing voice before a blip (keeps one oscillator at idle).
 * @param active_base Voice that will play this blip.
 */
static void voice_sid_quiesce_other(unsigned active_base) {
    unsigned other = (active_base == SID_V1) ? SID_V2 : SID_V1;
    voice_sid_park(other);
}

/**
 * @brief With gate off: set pulse width, ADSR, then frequency (standard order before gate-on).
 * @param base Voice base offset.
 * @param freq 16-bit SID frequency word.
 * @param attack_decay High nibble = attack, low = decay.
 * @param sustain_release High nibble = sustain level, low = release rate.
 */
static void voice_sid_load_pulse_patch(unsigned base, uint16_t freq, uint8_t attack_decay,
                                       uint8_t sustain_release) {
    SID[base + 2] = BLIP_PW_LO;
    SID[base + 3] = BLIP_PW_HI;
    SID[base + 5] = attack_decay;
    SID[base + 6] = sustain_release;
    SID[base + 0] = (uint8_t)(freq & 0xFFU);
    SID[base + 1] = (uint8_t)(freq >> 8);
}

/**
 * @brief With gate off: ADSR + frequency for triangle (no meaningful pulse width).
 * @param base Voice base offset.
 * @param freq 16-bit SID frequency word.
 * @param attack_decay High nibble = attack, low = decay.
 * @param sustain_release High nibble = sustain level, low = release rate.
 */
static void voice_sid_load_triangle_patch(unsigned base, uint16_t freq, uint8_t attack_decay,
                                          uint8_t sustain_release) {
    SID[base + 5] = attack_decay;
    SID[base + 6] = sustain_release;
    SID[base + 0] = (uint8_t)(freq & 0xFFU);
    SID[base + 1] = (uint8_t)(freq >> 8);
    SID[base + 4] = VOICE_CTRL_TRI_GATE_OFF;
}

/**
 * @brief End of sounding phase: gate off, brief settle, return voice to idle row.
 * @param base Voice base offset.
 */
static void voice_sid_finish_blip(unsigned base) {
    voice_sid_gate_off_pulse(base);
    spin_iters(BLIP_POST_GATE_SETTLE_ITERS);
    voice_sid_park(base);
}

/**
 * @brief End triangle blip: gate off triangle, then pulse idle row (gameplay/menu use pulse on this voice).
 * @param base Voice base offset.
 */
static void voice_sid_finish_blip_triangle(unsigned base) {
    SID[base + 4] = VOICE_CTRL_TRI_GATE_OFF;
    spin_iters(BLIP_POST_GATE_SETTLE_ITERS);
    voice_sid_park(base);
}

/**
 * @brief One pulse cue: idle both relevant voices, program patch, gate on/off, idle active voice.
 * @param base Voice to use (`SID_V1` or `SID_V2`).
 * @param gate_hold_iters Busy-wait iterations while gate is held high.
 */
static void blip_pulse(unsigned base, uint16_t freq, uint8_t attack_decay, uint8_t sustain_release,
                       unsigned gate_hold_iters) {
    voice_sid_quiesce_other(base);
    voice_sid_park(base);
    spin_iters(BLIP_ARM_SETTLE_ITERS);
    voice_sid_load_pulse_patch(base, freq, attack_decay, sustain_release);
    spin_iters(BLIP_PRE_GATE_SETTLE_ITERS);
    SID[base + 4] = VOICE_CTRL_PULSE_GATE_ON;
    spin_iters(gate_hold_iters);
    voice_sid_finish_blip(base);
}

/**
 * @brief Line-clear step: reload frequency with ADSR held; gate off → settle → on → off (avoids full park between
 *        close notes so retrigger stays reliable).
 */
static void blip_pulse_step_retrigger(unsigned base, uint16_t freq, uint8_t attack_decay, uint8_t sustain_release,
                                      unsigned gate_hold_iters) {
    /* Gate low before reprogramming the step. */
    voice_sid_gate_off_pulse(base);
    spin_iters(BLIP_RETRIGGER_SETTLE_ITERS);
    voice_sid_load_pulse_patch(base, freq, attack_decay, sustain_release);
    spin_iters(BLIP_PRE_GATE_SETTLE_ITERS);
    SID[base + 4] = VOICE_CTRL_PULSE_GATE_ON;
    spin_iters(gate_hold_iters);
    voice_sid_gate_off_pulse(base);
    spin_iters(BLIP_RETRIGGER_SETTLE_ITERS);
}

/**
 * @brief Title-only: triangle note through low-pass filter with cutoff sweep while gate is high.
 * @param base Voice to use (`SID_V2` for title).
 * @param fc11_start Filter cutoff at gate-on (dark).
 * @param fc11_end Filter cutoff at end of sweep (opens through the note).
 */
static void blip_triangle_lp_sweep(unsigned base, uint16_t freq, uint8_t attack_decay, uint8_t sustain_release,
                                   unsigned gate_hold_iters, unsigned fc11_start, unsigned fc11_end) {
    voice_sid_quiesce_other(base);
    voice_sid_park_triangle(base);
    spin_iters(BLIP_ARM_SETTLE_ITERS);
    voice_sid_load_triangle_patch(base, freq, attack_decay, sustain_release);
    spin_iters(BLIP_PRE_GATE_SETTLE_ITERS);
    sid_filter_cutoff_write(fc11_start);
    SID[base + 4] = VOICE_CTRL_TRI_GATE_ON;
    {
        const unsigned steps = 12U;
        unsigned chunk = gate_hold_iters / steps;
        if (chunk == 0U) {
            chunk = 1U;
        }
        for (unsigned s = 0; s < steps; ++s) {
            sid_filter_cutoff_write(sid_fc_lerp(fc11_start, fc11_end, s, steps));
            spin_iters(chunk);
        }
    }
    voice_sid_finish_blip_triangle(base);
}

/**
 * @brief Default gate-hold duration.
 */
static void blip_pulse_default_hold(unsigned base, uint16_t freq, uint8_t attack_decay,
                                    uint8_t sustain_release) {
    blip_pulse(base, freq, attack_decay, sustain_release, BLIP_GATE_HOLD_ITERS);
}

/**
 * @brief One row of the title five-note figure: semitone offset from `k_sid_title_a_root`, AD high nibble,
 * gate-hold length, and filter sweep endpoints (11-bit cutoff).
 */
typedef struct {
    unsigned semi_from_root;
    uint8_t attack_decay;
    unsigned gate_hold_iters;
    unsigned fc11_start;
    unsigned fc11_end;
} TitleMotiveNoteSpec;

static const TitleMotiveNoteSpec k_title_motive_notes[] = {
    {0U, 0x02U, 200U, 0x140U, 0x400U},
    {3U, 0x02U, 200U, 0x160U, 0x430U},
    {7U, 0x02U, 230U, 0x180U, 0x460U},
    {11U, 0x03U, 270U, 0x1A0U, 0x520U},
    {12U, 0x05U, 500U, 0x200U, 0x700U},
};

/**
 * @brief Title: `k_title_motive_notes` on voice 2, then `sid_filter_bypass()`.
 */
static void play_title_motive(void) {
    const unsigned note_count = (unsigned)(sizeof k_title_motive_notes / sizeof k_title_motive_notes[0]);

    sid_filter_arm_title_v2_lp();
    for (unsigned k = 0; k < note_count; ++k) {
        const TitleMotiveNoteSpec *row = &k_title_motive_notes[k];
        uint16_t f = freq_word_up_semitones(k_sid_title_a_root, row->semi_from_root);
        blip_triangle_lp_sweep(SID_V2, f, row->attack_decay, 0xA0U, row->gate_hold_iters, row->fc11_start,
                               row->fc11_end);
        if (k + 1U < note_count) {
            spin_iters(SPIN_TITLE_GAP_ITERS);
        }
    }
    sid_filter_bypass();
}

/**
 * @brief Line clear: three-note pulse arp E→G#→A' (A minor) on voice 1; `blip_pulse_step_retrigger` + `ARP_GAP_*`.
 */
static void play_line_clear_arp(void) {
    const uint16_t a_low = sid_am_low_root();
    const uint16_t f0 = freq_word_up_semitones(a_low, 7U);
    const uint16_t f1 = freq_word_up_semitones(a_low, 11U);
    const uint16_t f2 = freq_word_up_semitones(a_low, 12U);

    voice_sid_quiesce_other(SID_V1);
    voice_sid_park(SID_V1);
    spin_iters(BLIP_ARM_SETTLE_ITERS);

    blip_pulse_step_retrigger(SID_V1, f0, 0x04U, 0xA0U, 300U);
    spin_iters(ARP_GAP_LINE_CLEAR_ITERS);
    blip_pulse_step_retrigger(SID_V1, f1, 0x04U, 0xA0U, 300U);
    spin_iters(ARP_GAP_LINE_CLEAR_ITERS);
    blip_pulse_step_retrigger(SID_V1, f2, 0x06U, 0xC0U, 320U);

    voice_sid_park(SID_V1);
}

/**
 * @brief Start confirm: two pulse notes C→A' on voice 2 (`BLIP_GATE_HOLD_ROTATE/LOCK`, `ARP_GAP_START_CONFIRM`).
 */
static void play_start_confirm_gesture(void) {
    const uint16_t a_low = sid_am_low_root();
    const uint16_t freqs[2] = {
        freq_word_up_semitones(a_low, 3U),
        freq_word_up_semitones(a_low, 12U),
    };

    blip_pulse(SID_V2, freqs[0], 0x06U, 0x90U, BLIP_GATE_HOLD_ROTATE_ITERS);
    spin_iters(ARP_GAP_START_CONFIRM_ITERS);
    blip_pulse(SID_V2, freqs[1], 0x05U, 0x98U, BLIP_GATE_HOLD_LOCK_ITERS);
}

#endif /* QUATTRO_AUDIO */

void audio_init(void) {
#if QUATTRO_AUDIO
    for (unsigned i = 0; i < 25U; ++i) {
        SID[i] = 0;
    }
    *SID_MODE_VOL = 0x0FU;
    voice_sid_park(SID_V1);
    voice_sid_park(SID_V2);
#endif
}

void audio_play(QuattroSoundId id) {
#if QUATTRO_AUDIO
    if (id >= QUATTRO_SND_COUNT) {
        return;
    }

    switch (id) {
    case QUATTRO_SND_TITLE:
        play_title_motive();
        break;
    case QUATTRO_SND_START_CONFIRM:
        play_start_confirm_gesture();
        break;
    case QUATTRO_SND_ROTATE:
        blip_pulse(SID_V1, k_sid_freq_rotate, 0x02U, 0x90U, BLIP_GATE_HOLD_ROTATE_ITERS);
        break;
    case QUATTRO_SND_LOCK:
        blip_pulse(SID_V1, k_sid_freq_lock, 0x04U, 0xA0U, BLIP_GATE_HOLD_LOCK_ITERS);
        break;
    case QUATTRO_SND_LINE_CLEAR:
        play_line_clear_arp();
        break;
    case QUATTRO_SND_GAME_OVER:
        blip_pulse_default_hold(SID_V1, k_sid_freq_game_over, 0x06U, 0xC0U);
        break;
    default:
        break;
    }
#else
    (void)id;
#endif
}
