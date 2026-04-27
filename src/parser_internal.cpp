#include "parser_internal.h"

namespace
{
    using namespace light_angel;

    ParserContext g_activeContext{};
} // namespace

namespace light_angel
{
    ParserContext& GetActiveParser()
    {
        return g_activeContext;
    }
} // namespace light_angel
