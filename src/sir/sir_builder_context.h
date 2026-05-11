#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "light_angel/parser/node.h"
#include "light_angel/parser/string_type.h"
#include "light_angel/parser/token.h"
#include "light_angel/sir/sir.h"

namespace light_angel
{
    struct SirScope
    {
        // name -> symbol id
        std::unordered_map<std::string, SirSymbolId> names;
    };

    struct SirBuilderContext
    {
        str_view source;
        SirModule module;
        std::vector<Diagnostic> diagnostics;
        std::vector<SirScope> scopes;
        SirFunctionId currentFunction = InvalidFunction;

        SirSymbolId lookupName(const std::string& name);
        std::string spanText(SourceSpan span) const;
        std::string getTokenText(const TokenView& tok) const;
        SirExprId makeErrorExpr(SourceSpan span);
        SirStatementId makeErrorStatement(SourceSpan span);
        SirExprId makeImplicitCastIfNeeded(SirExprId src, SirTypeId target, SourceSpan span);
        void reportDiagnostic(SourceSpan span, std::string msg); // TODO: severity を引数に取るように
        void pushScope();
        void popScope();
        void declareInScope(const std::string& name, SirSymbolId id);
        void initBuiltinTypes();
        SirTypeId getBuiltinType(str_view keyword) const;
    };

} // namespace light_angel
