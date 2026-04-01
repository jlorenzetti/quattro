/**
 * @file audio.h
 * @brief Minimal SID micro-SFX for C64. Mute at compile time with `-DQUATTRO_AUDIO=0`.
 *        See `docs/notes/phase-4-micro-sfx.md` and `docs/notes/sound-v1-vocabulary.md`.
 */

#ifndef QUATTRO_C64_AUDIO_H
#define QUATTRO_C64_AUDIO_H

#include <stdint.h>

/** One-shot cues aligned with `docs/notes/sound-v1-vocabulary.md`. */
typedef enum {
    QUATTRO_SND_TITLE = 0,
    QUATTRO_SND_START_CONFIRM,
    QUATTRO_SND_ROTATE,
    QUATTRO_SND_LOCK,
    QUATTRO_SND_LINE_CLEAR,
    QUATTRO_SND_GAME_OVER,
    QUATTRO_SND_COUNT
} QuattroSoundId;

/** Silence SID voices and set master volume (call once at boot). */
void audio_init(void);

/** Fire a short cue (menu on SID voice 2, gameplay on voice 1; see `audio.c`). */
void audio_play(QuattroSoundId id);

#endif
