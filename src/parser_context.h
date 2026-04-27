#pragma once
#include "lexer.h"
#include "light_angel/token.h"

namespace light_angel
{
    struct ParserContext
    {
        str_view source;

        LexerState lexer;

        explicit ParserContext(str_view source) noexcept
            : source(source), lexer(LexerState(source))
        {
        }

        str_view slice(SourceSpan span) const
        {
            return source.substr(span.offset, span.length);
        }
    };

    ParserContext& GetActiveParser();
} // namespace light_angel
