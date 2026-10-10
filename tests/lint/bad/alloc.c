// SPDX-License-Identifier: GPL-3.0-only
// expect: alloc
#include <stdint.h>
void *p(void) { return malloc(16); }
