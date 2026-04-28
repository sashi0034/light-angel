
#include "light_angel/parser.h"
#include <cstdio>

namespace
{
    int g_pass = 0;
    int g_fail = 0;

    void expectParseOk(light_angel::str_view source, const char* label)
    {
        auto r = light_angel::Parse(source);
        if (r.ok())
        {
            ++g_pass;
        }
        else
        {
            ++g_fail;
            std::fprintf(stderr, "FAIL [expected ok, errorCount=%d]: %s\n", r.errorCount, label);
        }
    }

    void expectParseFail(light_angel::str_view source, const char* label)
    {
        auto r = light_angel::Parse(source);
        if (!r.ok())
        {
            ++g_pass;
        }
        else
        {
            ++g_fail;
            std::fprintf(stderr, "FAIL [expected error]: %s\n", label);
        }
    }
} // namespace

#define EXPECT_PARSE_OK(src)   expectParseOk(src, src)
#define EXPECT_PARSE_FAIL(src) expectParseFail(src, src)

// -----------------------------------------------
// Parse-success cases

static void testParseOk()
{
    EXPECT_PARSE_OK("");
    EXPECT_PARSE_OK(";");
    EXPECT_PARSE_OK("void foo() {}");
    EXPECT_PARSE_OK("int foo() { return 1; }");
    EXPECT_PARSE_OK("int x = 1;");
    EXPECT_PARSE_OK("class Foo {}");
    EXPECT_PARSE_OK("class Foo { int x; }");
    EXPECT_PARSE_OK("class Foo : Bar {}");
    EXPECT_PARSE_OK("enum Color { Red, Green, Blue }");
    EXPECT_PARSE_OK("namespace Foo { void bar() {} }");
    EXPECT_PARSE_OK("interface IFoo { void bar(); }");
    EXPECT_PARSE_OK("void foo() { if (x) {} }");
    EXPECT_PARSE_OK("void foo() { for (int i = 0; i < 10; i++) {} }");
    EXPECT_PARSE_OK("void foo() { while (true) {} }");
    EXPECT_PARSE_OK("void foo(int a, int b) {}");
    EXPECT_PARSE_OK("void foo(void) {}");
    EXPECT_PARSE_OK("typedef int MyInt;");
    EXPECT_PARSE_OK("using namespace Foo;");
}

// -----------------------------------------------
// Parse-failure cases

static void testParseFail()
{
    EXPECT_PARSE_FAIL("@@@ invalid tokens @@@");
    EXPECT_PARSE_FAIL("class { }"); // missing class name
}

// -----------------------------------------------

int main()
{
    testParseOk();
    testParseFail();

    std::printf("Results: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
