#include "test_utils.h"

void runParserTests();
// Add declarations for new test suites here, e.g.:
// void runLexerTests();

int main()
{
    runParserTests();
    // runLexerTests();

    std::printf("Results: %d passed, %d failed\n", test::g_pass, test::g_fail);
    return test::g_fail == 0 ? 0 : 1;
}
