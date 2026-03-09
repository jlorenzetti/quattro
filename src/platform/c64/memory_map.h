/**
 * @file memory_map.h
 * @brief C64 screen and color RAM layout for Quattro.
 */

#ifndef QUATTRO_PLATFORM_C64_MEMORY_MAP_H
#define QUATTRO_PLATFORM_C64_MEMORY_MAP_H

/** Default screen RAM base (C64). */
#define C64_SCREEN_RAM  ((volatile unsigned char *)0x0400)

/** Color RAM base (C64). */
#define C64_COLOR_RAM   ((volatile unsigned char *)0xD800)

/** Screen width in columns. */
#define C64_SCREEN_COLS 40

/** Screen height in rows. */
#define C64_SCREEN_ROWS 25

/** Board top-left column on screen (margin). */
#define BOARD_SCREEN_X 2

/** Board top-left row on screen (margin). */
#define BOARD_SCREEN_Y 2

/** Linear screen offset from (col, row). */
#define C64_SCREEN_OFFSET(col, row) ((unsigned int)(row) * C64_SCREEN_COLS + (unsigned int)(col))

/** HUD: first column for labels/values (right of board). */
#define HUD_SCREEN_COL 14
/** HUD: first row for SCORE line. */
#define HUD_SCREEN_ROW 2

#endif
