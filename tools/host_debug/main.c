/**
 * @file main.c
 * @brief Host-side debug harness: deterministic run, textual board, command injection, gravity.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ascii_renderer.h"
#include "game_state.h"
#include "types.h"

static Command char_to_command(char c) {
    switch (c) {
        case 'j': case 'h': return CMD_MOVE_LEFT;
        case 'k': case 'l': return CMD_MOVE_RIGHT;
        case 'c': case 'z': return CMD_ROTATE_CCW;
        case 'x': case 'v': return CMD_ROTATE_CW;
        case ' ': case 's': return CMD_SOFT_DROP;
        case 'g': return CMD_NONE; /* gravity tick handled separately */
        default: return CMD_NONE;
    }
}

int main(int argc, char **argv) {
    uint32_t seed = 12345u;
    if (argc >= 2) seed = (uint32_t)atoi(argv[1]);

    GameState state;
    game_start(&state, seed);

    printf("Quattro host debug — seed %lu\n", (unsigned long)seed);
    printf("j=left k=right c=CCW x=CW space=soft drop g=gravity tick q=quit\n\n");

    while (!game_is_over(&state)) {
        ascii_render_board(&state);
        ascii_render_status(&state);
        printf("> "); fflush(stdout);

        int c = getchar();
        if (c == EOF || c == 'q') break;
        if (c == '\n') continue;

        if ((char)c == 'g') {
            game_tick_gravity(&state);
            continue;
        }
        Command cmd = char_to_command((char)c);
        if (cmd != CMD_NONE) game_apply_command(&state, cmd);
    }

    ascii_render_board(&state);
    ascii_render_status(&state);
    printf("Final score: %lu\n", (unsigned long)state.score.score);

    return 0;
}
