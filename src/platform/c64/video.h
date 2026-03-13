/**
 * @file video.h
 * @brief C64 video init and board/piece drawing for Quattro.
 */

#ifndef QUATTRO_PLATFORM_C64_VIDEO_H
#define QUATTRO_PLATFORM_C64_VIDEO_H

#include "game_state.h"

/** Clears screen and sets default colors. */
void video_init(void);

/** Clears screen RAM and color RAM (e.g. when entering gameplay from start/help). */
void video_clear(void);

/** Draws playfield frame (left, right, bottom border). Call every redraw before board. */
void video_draw_frame(void);

/** Draws board and active piece (active as overlay, same char different color). */
void video_draw_board(const GameState *state);

/** Draws HUD: SCORE, LINES, LEVEL at fixed positions. */
void video_draw_hud(const GameState *state);

/** Draws GAME OVER band on field. Call when in game-over state. */
void video_draw_game_over(void);

/** Draws title screen: game name and one prompt line (e.g. PRESS ANY KEY). */
void video_draw_title(void);

/** Draws start/help screen: start level (0–9), controls, RETURN START. */
void video_draw_start_help(uint8_t start_level);

/** Draws replay prompt below game-over area. Call after video_draw_game_over. */
void video_draw_replay_prompt(void);

#endif
