# Core spec (first concrete pass)

## Global constants

```c
#define QUATTRO_BOARD_WIDTH 10
#define QUATTRO_VISIBLE_HEIGHT 20
#define QUATTRO_HIDDEN_ROWS 2
#define QUATTRO_BOARD_HEIGHT (QUATTRO_VISIBLE_HEIGHT + QUATTRO_HIDDEN_ROWS)
#define QUATTRO_PREVIEW_SIZE 4
#define PIECE_BLOCK_COUNT 4
```

## Core data model

### `PieceKind`
```c
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
```

### `Rotation`
```c
typedef enum {
    ROT_0 = 0,
    ROT_90,
    ROT_180,
    ROT_270,
    ROTATION_COUNT
} Rotation;
```

### `Cell`
```c
typedef uint8_t Cell;
#define CELL_EMPTY 0
#define CELL_FROM_PIECE(kind) ((Cell)((kind) + 1))
```

### `Point`
```c
typedef struct {
    int8_t x;
    int8_t y;
} Point;
```

### `Board`
```c
typedef struct {
    Cell cells[QUATTRO_BOARD_WIDTH * QUATTRO_BOARD_HEIGHT];
} Board;
```

### `ActivePiece`
```c
typedef struct {
    PieceKind kind;
    Rotation rotation;
    int8_t x;
    int8_t y;
} ActivePiece;
```

### `ScoreState`
```c
typedef struct {
    uint32_t score;
    uint16_t lines_cleared;
    uint8_t level;
} ScoreState;
```

### `GamePhase`
```c
typedef enum {
    GAME_PHASE_RUNNING = 0,
    GAME_PHASE_GAME_OVER
} GamePhase;
```

### `Command`
```c
typedef enum {
    CMD_NONE = 0,
    CMD_MOVE_LEFT,
    CMD_MOVE_RIGHT,
    CMD_ROTATE_CW,
    CMD_ROTATE_CCW,
    CMD_SOFT_DROP
} Command;
```

### `RngState`
```c
typedef struct {
    uint32_t state;
} RngState;
```

### `GameState`
```c
typedef struct {
    Board board;
    ActivePiece active;
    PieceKind next_piece;
    ScoreState score;
    GamePhase phase;
    RngState rng;
} GameState;
```

## Board API
```c
void board_clear(Board *board);
Cell board_get_cell(const Board *board, int8_t x, int8_t y);
void board_set_cell(Board *board, int8_t x, int8_t y, Cell value);
bool board_is_inside(int8_t x, int8_t y);
bool board_is_row_full(const Board *board, int8_t y);
uint8_t board_clear_full_rows(Board *board);
```

## Piece API
```c
Point piece_get_block_offset(PieceKind kind, Rotation rotation, uint8_t index);
Point active_piece_get_block_position(const ActivePiece *piece, uint8_t index);
```

Piece offsets are defined inside a shared 4×4 local box. Spawn coordinates refer to the top-left corner of that box, not to a per-piece natural width.

## Rules API
```c
bool rules_can_place_piece(
    const Board *board,
    PieceKind kind,
    Rotation rotation,
    int8_t x,
    int8_t y
);

bool rules_can_move(
    const Board *board,
    const ActivePiece *piece,
    int8_t dx,
    int8_t dy
);

Rotation rotation_rotate_cw(Rotation rotation);
Rotation rotation_rotate_ccw(Rotation rotation);

bool rules_can_rotate(
    const Board *board,
    const ActivePiece *piece,
    Rotation target_rotation
);

void rules_lock_active_piece(Board *board, const ActivePiece *piece);
ActivePiece rules_make_spawn_piece(PieceKind kind);
bool rules_can_spawn_piece(const Board *board, PieceKind kind);
```

## Scoring API
```c
void scoring_reset(ScoreState *score);
void scoring_apply_line_clear(ScoreState *score, uint8_t cleared_lines);
uint8_t scoring_compute_level(uint16_t total_lines_cleared);
```

## RNG API
```c
void rng_seed(RngState *rng, uint32_t seed);
uint32_t rng_next_u32(RngState *rng);
PieceKind rng_next_piece(RngState *rng);
```

## Game API
```c
void game_start(GameState *state, uint32_t seed);
void game_apply_command(GameState *state, Command command);
void game_tick_gravity(GameState *state);
bool game_is_over(const GameState *state);
```

## Core semantics
### `game_start()`
1. clear board
2. reset score
3. set phase running
4. seed RNG
5. generate `next_piece`
6. promote `next_piece` to active
7. generate new `next_piece`
8. check active spawn validity
9. if invalid -> game over

### `game_apply_command()`
- `MOVE_LEFT`: move if legal
- `MOVE_RIGHT`: move if legal
- `ROTATE_CW`: rotate if legal
- `ROTATE_CCW`: rotate if legal
- `SOFT_DROP`: descend by one cell if legal

No lock on failed soft drop in the first pass.

### `game_tick_gravity()`
- if the active piece can descend, descend
- otherwise:
  - lock piece
  - clear rows
  - update score
  - promote next piece
  - generate a new next piece
  - check spawn validity
  - set game over if needed

## Host-side prototype contract
The first host-side spike must support:
- deterministic seeded runs
- textual board rendering for debugging
- direct command injection
- gravity ticks
- final state inspection
