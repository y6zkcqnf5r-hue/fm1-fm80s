// SPDX-License-Identifier: GPL-3.0-only
/* Minimal unit-test helpers for host tests (tests/ may use libc, math.h and double).
 *
 *   static void test_x(void) { CHECK(a == b); CHECK_EQ(x, 3); CHECK_NEAR(y, 0.5, 1e-3); }
 *   TEST_MAIN(RUN(test_x); RUN(test_y);)
 */
#pragma once
#include <stdio.h>

static int test_fails, test_checks;
static const char *test_name = "";

#define CHECK(c)                                                                            \
    do {                                                                                    \
        test_checks++;                                                                      \
        if (!(c)) {                                                                         \
            test_fails++;                                                                   \
            printf("  FAIL %s %s:%d: %s\n", test_name, __FILE__, __LINE__, #c);             \
        }                                                                                   \
    } while (0)

#define CHECK_EQ(a, b)                                                                      \
    do {                                                                                    \
        long long a_ = (long long)(a), b_ = (long long)(b);                                 \
        test_checks++;                                                                      \
        if (a_ != b_) {                                                                     \
            test_fails++;                                                                   \
            printf("  FAIL %s %s:%d: %s == %s (%lld != %lld)\n", test_name, __FILE__,       \
                   __LINE__, #a, #b, a_, b_);                                               \
        }                                                                                   \
    } while (0)

#define CHECK_NEAR(a, b, tol)                                                               \
    do {                                                                                    \
        double a_ = (double)(a), b_ = (double)(b);                                          \
        test_checks++;                                                                      \
        if (!(a_ - b_ <= (tol) && b_ - a_ <= (tol))) {                                      \
            test_fails++;                                                                   \
            printf("  FAIL %s %s:%d: %s ~ %s (%g vs %g, tol %g)\n", test_name, __FILE__,    \
                   __LINE__, #a, #b, a_, b_, (double)(tol));                                \
        }                                                                                   \
    } while (0)

#define RUN(fn)                                                                             \
    do {                                                                                    \
        test_name = #fn;                                                                    \
        fn();                                                                               \
    } while (0)

#define TEST_MAIN(body)                                                                     \
    int main(void)                                                                          \
    {                                                                                       \
        body                                                                                \
        printf("  %s: %d checks, %d failed\n", __FILE__, test_checks, test_fails);          \
        return test_fails ? 1 : 0;                                                          \
    }
