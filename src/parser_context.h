#pragma once
#include "lexer.h"
#include "light_angel/token.h"
#include <vector>

namespace light_angel
{
    struct ParserContext
    {
        str_view source;

        explicit ParserContext(str_view source) noexcept;
        ~ParserContext();

        str_view textOf(const TokenView& view) const
        {
            auto s = view.span();
            return source.substr(s.offset, s.length);
        }

        str_view slice(SourceSpan span) const
        {
            return source.substr(span.offset, span.length);
        }

        bool isEnd() const
        {
            return peekKind() == TokenKind::EndOfFile;
        }

        const LexicalToken& peek(uint32_t step = 0) const
        {
            ensureAhead(step);
            return m_tokens[m_cursor + step];
        }

        str_view peekText(uint32_t step = 0) const
        {
            return textOf(peek(step).view);
        }

        TokenKind peekKind(uint32_t step = 0) const
        {
            return peek(step).kind;
        }

        bool check(str_view s, uint32_t step = 0) const
        {
            return peekText(step) == s;
        }

        bool checkKind(TokenKind k, uint32_t step = 0) const
        {
            return peekKind(step) == k;
        }

        void advance()
        {
            if (!isEnd()) ++m_cursor;
        }

        uint32_t save() const
        {
            return m_cursor;
        }

        void rewindTo(uint32_t c)
        {
            m_cursor = c;
        }

        TokenView consumeView()
        {
            TokenView v = peek().view;
            advance();
            return v;
        }

        bool consume(str_view s)
        {
            if (check(s))
            {
                advance();
                return true;
            }

            return false;
        }

        /// @brief Returns true if the current token is a Punctuator starting with '>'.
        /// @details Covers '>', '>>', '>>>', '>>>>', '>>=', '>>>=', etc.
        bool canConsumeGt(uint32_t step = 0) const
        {
            if (peekKind(step) != TokenKind::Punctuator) return false;

            str_view text = peekText(step);
            return !text.empty() && text[0] == '>';
        }

        /// @brief Consumes exactly one '>' from the front of the current token.
        /// @details If the token is a multi-char '>…' (e.g., '>>'), it is split:
        /// the current token becomes '>' and the remainder is inserted as
        /// the next token, then the cursor advances past the single '>'.
        void consumeOneGt();

        void advanceError();

        void reportError(SourceSpan span, str_view message);

        const std::vector<Diagnostic>& diagnostics() const
        {
            return m_diagnostics;
        }

        std::vector<Diagnostic> takeDiagnostics()
        {
            return std::move(m_diagnostics);
        }

        /// @brief Returns the span covering tokens from startCursor up to (but not including) cursor.
        SourceSpan spanFrom(uint32_t startCursor) const
        {
            if (startCursor >= static_cast<uint32_t>(m_tokens.size())) return SourceSpan{};

            uint32_t endIdx = (m_cursor > 0) ? m_cursor - 1 : startCursor;
            if (endIdx >= static_cast<uint32_t>(m_tokens.size()))
                endIdx = static_cast<uint32_t>(m_tokens.size()) - 1;

            auto start = m_tokens[startCursor].view.span();
            auto end = m_tokens[endIdx].view.span();
            if (end.offset + end.length < start.offset) return start;

            return SourceSpan{start.offset, end.offset + end.length - start.offset};
        }

    private:
        // Lex tokens until m_tokens has at least (m_cursor + step + 1) entries.
        void ensureAhead(uint32_t step) const;

        mutable LexerState m_lexer;
        mutable std::vector<LexicalToken> m_tokens;
        mutable std::vector<Diagnostic> m_diagnostics;
        uint32_t m_cursor{};
    };

    ParserContext& GetActiveParser();
} // namespace light_angel
