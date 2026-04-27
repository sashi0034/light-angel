#include "parser_context.h"

namespace
{
    using namespace light_angel;

    constexpr str_view emptySource{};

    ParserContext g_activeContext{ParserContext(emptySource)};
} // namespace

namespace light_angel
{
    ParserContext& GetActiveParser()
    {
        return g_activeContext;
    }
} // namespace light_angel
