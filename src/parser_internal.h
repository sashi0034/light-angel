#pragma once
#include "light_angel/token.h"


#include <string_view>

namespace light_angel
{
    struct ParserContext
    {
        std::string_view source;

        std::string_view slice(SourceSpan span) const
        {
            return source.substr(span.offset, span.length);
        }
    };

    ParserContext& GetActiveParser();
} // namespace light_angel
