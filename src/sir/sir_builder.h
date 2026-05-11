#pragma once

#include "light_angel/parser/node.h"
#include "light_angel/parser/string_type.h"
#include "light_angel/sir/sir.h"

namespace light_angel
{
    // Build a SirModule from a parsed top-level Script node.
    // The source string used for parsing must be passed in so that token
    // text can be inspected (literal values, identifiers, operators).
    SirBuildResult BuildSirModule(const Node_Script& script, str_view source);
} // namespace light_angel
