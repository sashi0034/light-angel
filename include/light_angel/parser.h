#pragma once

#include "node.h"
#include <memory>

namespace light_angel
{
    struct ParseResult
    {
        std::unique_ptr<Node_Script> script;
        int errorCount = 0;

        bool ok() const { return errorCount == 0; }
    };

    ParseResult Parse(str_view source);
} // namespace light_angel
