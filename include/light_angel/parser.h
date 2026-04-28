#pragma once

#include "node.h"
#include <memory>

namespace light_angel
{
    std::unique_ptr<Node_Script> Parse(str_view source);
} // namespace light_angel
