#pragma once

#include "node.h"
#include "token.h"
#include <memory>
#include <vector>

namespace light_angel
{
    struct ParseResult
    {
        std::unique_ptr<Node_Script> script;
        std::vector<Diagnostic> diagnostics;

        bool ok() const
        {
            return diagnostics.empty();
        }

        // Returns all diagnostics formatted with source context.
        // Each entry shows: error header, source line, and caret marker.
        // Pass a filename (e.g., "script.as") to include it in the location prefix.
        str_t formatDiagnostics(str_view source, str_view filename = {}) const;
    };

    ParseResult Parse(str_view source);
} // namespace light_angel
