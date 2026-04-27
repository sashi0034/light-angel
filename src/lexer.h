#pragma once
#include "light_angel/token.h"

#include <cstdint>

namespace light_angel
{
    class LexerState
    {
    public:
        explicit LexerState(str_view source)
            : m_source(source)
        {
        }

        uint32_t offset() const
        {
            return m_offset;
        }

        char_t peek() const
        {
            if (isEnd()) return '\0';
            return m_source[m_offset];
        }

        char_t peekAt(uint32_t relativeOffset) const
        {
            const uint32_t pos = m_offset + relativeOffset;
            if (pos >= m_source.size()) return '\0';
            return m_source[pos];
        }

        bool isEnd() const
        {
            return m_offset >= m_source.size();
        }

        bool startsWith(str_view prefix) const
        {
            return m_source.substr(m_offset, prefix.size()) == prefix;
        }

        bool isNextLineBreak() const
        {
            char_t c = peek();
            return c == '\n' || c == '\r';
        }

        bool isNextWhitespace() const
        {
            char_t c = peek();
            return c == ' ' || c == '\t' || c == '\n' || c == '\r';
        }

        void advance(uint32_t count = 1)
        {
            m_offset += count;
        }

    private:
        str_view m_source;
        uint32_t m_offset{};
    };

    LexicalToken Lex(LexerState& lexer);
} // namespace light_angel
