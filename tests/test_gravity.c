/**
 * @file test_gravity.c
 * @brief Host-side checks for gravity interval lookup (curve and clamp).
 */

#include "gravity.h"

int test_gravity_run(void) {
    if (gravity_interval_for_level(0) < 10) return 1;
    if (gravity_interval_for_level(9) < 1 || gravity_interval_for_level(9) > 10) return 1;
    if (gravity_interval_for_level(29) != 1) return 1;
    if (gravity_interval_for_level(99) != 1) return 1;
    return 0;
}
