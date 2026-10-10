// SPDX-License-Identifier: GPL-3.0-only
/* Lint self-test: this file must pass. Words that are banned in code are fine in comments:
 * float, double, malloc(), printf(), APP_WEB, 0xFFC00080, a 64-bit / division. */
#include <stdint.h>

static const char *msg = "float double printf( malloc( APP_HOST 0xFFC00080 / %";   /* strings too */

int32_t good_scale(int32_t x)
{
    int64_t p = (int64_t)x * 3;      /* 64-bit multiply is allowed */
    int32_t q = (int32_t)(p >> 2);   /* shift, no 64-bit division */
    return q / 7 + (msg[0] == 'f');  /* 32-bit division is allowed */
}
