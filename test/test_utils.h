#pragma once

#include <cstdio>

namespace test
{

inline int g_pass = 0;
inline int g_fail = 0;

inline void pass(const char* label)
{
    ++g_pass;
    (void)label;
}

inline void fail(const char* label, const char* reason)
{
    ++g_fail;
    std::fprintf(stderr, "FAIL [%s]: %s\n", reason, label);
}

} // namespace test

#define EXPECT_TRUE(expr)                                            \
    do                                                               \
    {                                                                \
        if (expr)                                                    \
            test::pass(#expr);                                       \
        else                                                         \
            test::fail(#expr, "expected true");                      \
    } while (0)

#define EXPECT_FALSE(expr)                                           \
    do                                                               \
    {                                                                \
        if (!(expr))                                                 \
            test::pass(#expr);                                       \
        else                                                         \
            test::fail(#expr, "expected false");                     \
    } while (0)
