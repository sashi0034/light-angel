#include "light_angel/token.h"

#include "parser_context.h"

namespace light_angel
{
    TokenView::TokenView(SourceSpan span)
        : m_span(span)
    {
#if defined(LIGHT_ANGEL_DEBUG)
        m_text = GetActiveParser().slice(span);
#endif
    }
} // namespace light_angel
