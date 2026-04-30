#include "light_angel/parser/parser.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    std::string source;
    std::string filename;

    if (argc >= 2)
    {
        filename = argv[1];
        std::ifstream f(filename);
        if (!f)
        {
            std::fprintf(stderr, "sandbox: cannot open '%s'\n", argv[1]);
            return 1;
        }

        source.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>{});
    }
    else
    {
        filename = "<stdin>";
        source.assign(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>{});
    }

    auto result = light_angel::Parse(source);

    if (result.ok())
    {
        std::printf("Parse OK  (%zu top-level nodes)\n", result.script->children.size());
        return 0;
    }

    std::fputs(result.formatDiagnostics(source, filename).c_str(), stderr);
    return 1;
}
