#include "parser_context.h"

namespace
{
    light_angel::ParserContext* g_activeContextPtr = nullptr;
} // namespace

namespace light_angel
{
    ParserContext::ParserContext(str_view src) noexcept
        : source(src), m_lexer(src, 0)
    {
        g_activeContextPtr = this;
    }

    ParserContext::~ParserContext()
    {
        if (g_activeContextPtr == this)
            g_activeContextPtr = nullptr;
    }

    void ParserContext::ensureAhead(uint32_t step) const
    {
        const uint32_t needed = m_cursor + step + 1;
        while (static_cast<uint32_t>(m_tokens.size()) < needed)
        {
            if (!m_tokens.empty() && m_tokens.back().kind == TokenKind::EndOfFile)
                break;

            LexicalToken token = Lex(m_lexer);
            if (token.kind == TokenKind::Comment)
                continue;

            m_tokens.push_back(std::move(token));
        }
    }

    void ParserContext::consumeOneGt()
    {
        ensureAhead(0);
        LexicalToken& token = m_tokens[m_cursor];
        SourceSpan span = token.view.span();
        if (span.length > 1)
        {
            token = LexicalToken{TokenKind::Punctuator, TokenView{SourceSpan{span.offset, 1}}};
            m_tokens.insert(
                m_tokens.begin() + m_cursor + 1,
                LexicalToken{TokenKind::Punctuator, TokenView{SourceSpan{span.offset + 1, span.length - 1}}}
            );
        }

        advance();
    }

    ParserContext& GetActiveParser()
    {
        return *g_activeContextPtr;
    }
} // namespace light_angel
