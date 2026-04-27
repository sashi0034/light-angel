#pragma once
#include "light_angel/token.h"


#include <cstdint>
#include <string_view>

namespace light_angel
{
    class LexerState
    {
    public:
        explicit LexerState(std::string_view source)
            : m_source(source)
        {
        }

        uint32_t offset() const
        {
            return m_offset;
        }

        char peek() const
        {
            if (isEnd()) return '\0';
            return m_source[m_offset];
        }

        bool isEnd() const
        {
            return m_offset >= m_source.size();
        }

        bool startsWith(std::string_view prefix) const
        {
            return m_source.substr(m_offset, prefix.size()) == prefix;
        }

        bool isNextLineBreak() const
        {
            char c = peek();
            return c == '\n' || c == '\r';
        }

        bool isNextWhitespace() const
        {
            char c = peek();
            return c == ' ' || c == '\t' || c == '\n' || c == '\r';
        }

        void advance(uint32_t count = 1)
        {
            m_offset += count;
        }

    private:
        std::string_view m_source;
        uint32_t m_offset{};
    };

    LexicalToken Lex(LexerState& lexer);
} // namespace light_angel
