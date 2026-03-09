/**
 * @file video.h
 * @brief C64 video init and board/piece drawing for Quattro.
 */

#ifndef QUATTRO_PLATFORM_C64_VIDEO_H
#define QUATTRO_PLATFORM_C64_VIDEO_H

#include "game_state.h"

/** Clears screen and sets default colors. */
void video_init(void);

/** Draws playfield frame (left, right, bottom border). Call every redraw before board. */
void video_draw_frame(void);

/** Draws board and active piece (active as overlay, same char different color). */
void video_draw_board(const GameState *state);

/** Draws HUD: SCORE, LINES, LEVEL at fixed positions. */
void video_draw_hud(const GameState *state);

/** Draws GAME OVER text. Call once when game has ended. */
void video_draw_game_over(void);

#endif
