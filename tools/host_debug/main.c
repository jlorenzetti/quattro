/**
 * @file main.c
 * @brief Host-side debug harness: deterministic run, textual board, gravity.
 */

#include <stdio.h>
#include <stdlib.h>

#include "ascii_renderer.h"
#include "game_state.h"
#include "types.h"

static const char CONTROLS[] =
    "A/D MOVE  Z/X ROTATE  SPC DROP  G TICK  Q QUIT";

static Command char_to_command(char c) {
    switch (c) {
        case 'a': return CMD_MOVE_LEFT;
        case 'd': return CMD_MOVE_RIGHT;
        case 'z': return CMD_ROTATE_CCW;
        case 'x': return CMD_ROTATE_CW;
        case ' ': return CMD_SOFT_DROP;
        case 'g': return CMD_NONE; /* gravity tick handled separately */
        default: return CMD_NONE;
    }
}

int main(int argc, char **argv) {
    uint32_t seed = 12345u;
    if (argc >= 2) seed = (uint32_t)atoi(argv[1]);

    GameState state;
    game_start(&state, seed, 0);

    printf("\nquattro\nseed %lu\n\n", (unsigned long)seed);

    for (;;) {
        ascii_render_status(&state);
        ascii_render_board(&state);
        printf("\n%s\n", CONTROLS);

        if (game_is_over(&state)) {
            printf("GAME OVER — Q quit\n");
        }
        printf("> ");
        fflush(stdout);

        int c = getchar();
        if (c == EOF || c == 'q') break;
        if (c == '\n') continue;

        if ((char)c == 'g') {
            if (!game_is_over(&state)) game_tick_gravity(&state);
        } else if (!game_is_over(&state)) {
            Command cmd = char_to_command((char)c);
            if (cmd != CMD_NONE) game_apply_command(&state, cmd);
        }

        /* Consume rest of line so the trailing newline does not trigger another render. */
        while (c != '\n') {
            c = getchar();
            if (c == EOF || c == '\n') break;
        }
        putchar('\n');
    }

    return 0;
}
