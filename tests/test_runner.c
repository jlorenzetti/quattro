/**
 * @file test_runner.c
 * @brief Minimal test harness for Quattro core.
 */

#include <stdio.h>
#include <stdlib.h>

extern int test_board_run(void);
extern int test_piece_run(void);
extern int test_rules_run(void);
extern int test_game_state_run(void);
extern int test_game_state_full_run(void);
extern int test_gravity_run(void);

int main(void) {
    int failed = 0;
    if (test_board_run() != 0) { fprintf(stderr, "FAIL board\n"); failed = 1; }
    if (test_piece_run() != 0) { fprintf(stderr, "FAIL piece\n"); failed = 1; }
    if (test_rules_run() != 0) { fprintf(stderr, "FAIL rules\n"); failed = 1; }
    if (test_game_state_run() != 0) { fprintf(stderr, "FAIL game_state\n"); failed = 1; }
    if (test_game_state_full_run() != 0) { fprintf(stderr, "FAIL game_state_full\n"); failed = 1; }
    if (test_gravity_run() != 0) { fprintf(stderr, "FAIL gravity\n"); failed = 1; }
    if (!failed) printf("OK\n");
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
