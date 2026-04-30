#include "test_utils.h"
#include "light_angel/parser/parser.h"

namespace
{

void expectParseOk(light_angel::str_view source, const char* label)
{
    auto r = light_angel::Parse(source);
    if (r.ok())
        test::pass(label);
    else
        test::fail(label, "expected ok");
}

void expectParseError(light_angel::str_view source, const char* label)
{
    auto r = light_angel::Parse(source);
    if (!r.ok())
        test::pass(label);
    else
        test::fail(label, "expected error");
}

} // namespace

#define EXPECT_PARSE_OK(src)    expectParseOk(src, src)
#define EXPECT_PARSE_ERROR(src) expectParseError(src, src)

static void testParseOk()
{
    EXPECT_PARSE_OK("");
    EXPECT_PARSE_OK("\xEF\xBB\xBF"
                    "");
    EXPECT_PARSE_OK(";");
    EXPECT_PARSE_OK("\xEF\xBB\xBF"
                    "void foo() {}");
    EXPECT_PARSE_OK("void foo() {}");
    EXPECT_PARSE_OK("int foo() { return 1; }");
    EXPECT_PARSE_OK("int x = 1;");
    EXPECT_PARSE_OK("int MyValue = 0; float MyFloat = 15.f;");
    EXPECT_PARSE_OK("const uint Flag1 = 0x01;");
    EXPECT_PARSE_OK("class Foo {}");
    EXPECT_PARSE_OK("class Foo { int x; }");
    EXPECT_PARSE_OK("class Foo { void bar() { value++; } int value; }");
    EXPECT_PARSE_OK("class Foo { void bar() const { } }");
    EXPECT_PARSE_OK("class Foo : Bar {}");
    EXPECT_PARSE_OK("class Foo { int value { get; set; } }");
    EXPECT_PARSE_OK("enum Color { Red, Green, Blue }");
    EXPECT_PARSE_OK("enum Foo { fizz, buzz, }");
    EXPECT_PARSE_OK("enum MyEnum { eValue0, eValue2 = 2, eValue3, eValue200 = eValue2 * 100 }");
    EXPECT_PARSE_OK("namespace Foo { void bar() {} }");
    EXPECT_PARSE_OK("interface IFoo { void bar(); }");
    EXPECT_PARSE_OK("interface IFoo { int value { get; set; } }");
    EXPECT_PARSE_OK("void foo() { if (x) {} }");
    EXPECT_PARSE_OK("void foo() { for (int i = 0; i < 10; i++) {} }");
    EXPECT_PARSE_OK("void foo() { while (true) {} }");
    EXPECT_PARSE_OK("void foo(int a, int b) {}");
    EXPECT_PARSE_OK("void foo(void) {}");
    EXPECT_PARSE_OK("funcdef bool CALLBACK(int, int);");
    EXPECT_PARSE_OK("typedef int MyInt;");
    EXPECT_PARSE_OK("using namespace Foo;");
    EXPECT_PARSE_OK("bool foo = not true; bool bar = not not false;");
    EXPECT_PARSE_OK("double e0 = 1e10; double e1 = 1e+10; double e2 = 1e-10;");
    EXPECT_PARSE_OK("double e3 = 1.5e10; double e4 = 1.5e+10; double e5 = 1.5e-10;");
    EXPECT_PARSE_OK("double e6 = .5e10; double e7 = .5e+10; double e8 = .5e-10;");
    EXPECT_PARSE_OK(R"(
        class int_array {
            int_array@ f(int &in) {repeat int} { }
        }

        class dictionary {
            dictionary@ f(int &in) {repeat {string, ?}} { }
        }

        class grid {
            grid@ f(int &in) {repeat {repeat_same int}} { }
        }
    )");
    EXPECT_PARSE_OK(R"(
        enum Test {
            A = 1,
            B = 2
        }

        void Main() {
            Test x = Test(1);
            Test y = Test(Test::A + Test::B | Test::A);
            bool z = (y & Test::A) != 0;
            int v = 1;
            bool w = v == Test::A;
        }
    )");
    EXPECT_PARSE_OK(R"(
        void test() {
            double e0 = 1e10;
            e0 = 1e+10;
            e0 = 1e-10;
            e0 = 1.5e10;
            e0 = 1.5e+10;
            e0 = 1.5e-10;
            e0 = .5e10;
            e0 = .5e+10;
            e0 = .5e-10;
        }
    )");
    EXPECT_PARSE_OK(R"(
        void f() {
            // This is a comment
            array<array<int>> my_array;
            my_array[0].push_back((1 >> 4) >>> 1);
            my_array[0][0] >>>= 123;
        }
    )");
    EXPECT_PARSE_OK(R"(
        void f() {
            array<array<array<int>>> array3d(123);
            array<array<array<array<int>>>> array4d(123);
            array<array<array<array<array<int>>>>> array5d(123);
        }
    )");
    EXPECT_PARSE_OK(R"(
        bool is_null(const Obj@ const handle) {
            if (handle !is null) {
                return false;
            }
            else {
                return true;
            }
        }
    )");
}

static void testParseError()
{
    EXPECT_PARSE_ERROR("@@@ invalid tokens @@@");
    EXPECT_PARSE_ERROR("class { }");
    EXPECT_PARSE_ERROR("mixin");
    EXPECT_PARSE_ERROR("funcdef");
    EXPECT_PARSE_ERROR("typedef int ;");
    EXPECT_PARSE_ERROR("funcdef void ();");
    EXPECT_PARSE_ERROR("void 123() {}");
    EXPECT_PARSE_ERROR("void test() { value = ; }");
    EXPECT_PARSE_ERROR("void foo(");
    EXPECT_PARSE_ERROR("class Foo { int value { get; get; } }");
    EXPECT_PARSE_ERROR("class Foo { int value { set; set; } }");
}

void runParserTests()
{
    testParseOk();
    testParseError();
}
