#ifndef QUATTRO_CORE_TYPES_H
#define QUATTRO_CORE_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#define QUATTRO_BOARD_WIDTH 10
#define QUATTRO_VISIBLE_HEIGHT 20
#define QUATTRO_HIDDEN_ROWS 2
#define QUATTRO_BOARD_HEIGHT (QUATTRO_VISIBLE_HEIGHT + QUATTRO_HIDDEN_ROWS)
#define QUATTRO_PREVIEW_SIZE 4
#define PIECE_BLOCK_COUNT 4

/** Spawn position: column (top-left origin for piece bounding box). */
#define QUATTRO_SPAWN_X 3
/** Spawn position: row (top of board, hidden area). */
#define QUATTRO_SPAWN_Y 0

typedef enum {
    PIECE_I = 0,
    PIECE_O,
    PIECE_T,
    PIECE_S,
    PIECE_Z,
    PIECE_J,
    PIECE_L,
    PIECE_KIND_COUNT
} PieceKind;

typedef enum {
    ROT_0 = 0,
    ROT_90,
    ROT_180,
    ROT_270,
    ROTATION_COUNT
} Rotation;

typedef uint8_t Cell;

#define CELL_EMPTY 0
#define CELL_FROM_PIECE(kind) ((Cell)((kind) + 1))

typedef struct {
    int8_t x;
    int8_t y;
} Point;

typedef struct {
    Cell cells[QUATTRO_BOARD_WIDTH * QUATTRO_BOARD_HEIGHT];
} Board;

typedef struct {
    PieceKind kind;
    Rotation rotation;
    int8_t x;
    int8_t y;
} ActivePiece;

typedef struct {
    uint32_t score;
    uint16_t lines_cleared;
    uint8_t level;
} ScoreState;

typedef enum {
    GAME_PHASE_RUNNING = 0,
    GAME_PHASE_GAME_OVER
} GamePhase;

typedef enum {
    CMD_NONE = 0,
    CMD_MOVE_LEFT,
    CMD_MOVE_RIGHT,
    CMD_ROTATE_CW,
    CMD_ROTATE_CCW,
    CMD_SOFT_DROP
} Command;

typedef struct {
    uint32_t state;
} RngState;

typedef struct {
    Board board;
    ActivePiece active;
    PieceKind next_piece;
    ScoreState score;
    GamePhase phase;
    RngState rng;
} GameState;

#endif
