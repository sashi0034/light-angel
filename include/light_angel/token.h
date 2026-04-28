#pragma once
#include <cstdint>

#include "string_type.h"

namespace light_angel
{
    struct SourceSpan
    {
        uint32_t offset;
        uint32_t length;
    };

    struct LineColumn
    {
        uint32_t line;
        uint32_t column;
    };

    class TokenView
    {
    public:
        TokenView() = default;

        TokenView(SourceSpan span);

        SourceSpan span() const
        {
            return m_span;
        }

        bool isEmpty() const
        {
            return m_span.length == 0;
        }

    private:
        SourceSpan m_span{};

#if defined(LIGHT_ANGEL_DEBUG)
        str_t m_text;
#endif
    };

    enum class TokenKind : uint8_t
    {
        Name, // identifier or keyword
        Number,
        String,
        Bits, // hex/binary bit-field literals (e.g. 0xFF, 0b101)
        Punctuator,
        Comment,
        Unknown, // unrecognized character (lex error)
        EndOfFile,
    };

    struct Diagnostic
    {
        enum class Severity
        {
            Error
        };

        Severity severity = Severity::Error;
        SourceSpan span{};
        str_t message;
    };

    struct LexicalToken
    {
        TokenKind kind;
        TokenView view;
    };
} // namespace light_angel
