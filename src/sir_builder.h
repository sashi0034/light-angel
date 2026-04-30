#pragma once

#include "light_angel/node.h"
#include "light_angel/sir.h"
#include "light_angel/string_type.h"
#include "light_angel/token.h"

#include <unordered_map>
#include <vector>

namespace light_angel
{
    // Build a SirModule from a parsed top-level Script node.
    // The source string used for parsing must be passed in so that token
    // text can be inspected (literal values, identifiers, operators).
    SirBuildResult BuildSirModule(const Node_Script& script, str_view source);

    // -----------------------------------------------
    // Builder (exposed for testing / extension)
    // -----------------------------------------------

    struct SirScope
    {
        // name -> symbol id
        std::unordered_map<std::string, SirSymbolId> names;
    };

    class SirBuilder
    {
    public:
        SirBuilder(str_view source);

        SirBuildResult build(const Node_Script& script);

    private:
        void buildScript(const Node_Script& script);
        void buildFunction(const Node_Func& func);
        SirStatementId buildVarDecl(const Node_Var& var);
        SirBlockId buildStatBlock(const Node_StatBlock& sb);
        SirTypeId buildType(const Node_Type& typeNode);
        SirStatementId buildStatementNode(const NodeBase& node);
        SirStatementId buildExprStat(const Node_ExprStat& es);
        SirStatementId buildReturn(const Node_Return& ret);
        SirExprId buildExpr(const Node_Expr& expr);
        SirExprId buildExprTerm(const Node_ExprTerm& term);
        SirExprId buildExprValue(const NodeBase& value);
        SirExprId buildLiteral(const Node_Literal& lit);
        SirExprId buildFuncCall(const Node_FuncCall& fc);
        SirExprId buildVarAccess(const Node_VarAccess& va);
        SirExprId buildAssign(const Node_Assign& assign);
        SirExprId buildCondition(const Node_Condition& cond);

        // Non-BNF helpers
        SirStatementId buildStatBlockAsStatement(const Node_StatBlock& sb);

        // Helpers
        SirSymbolId lookupName(const std::string& name) const;
        std::string spanText(SourceSpan span) const;
        std::string tokenText(const TokenView& tok) const;

        SirExprId makeErrorExpr(SourceSpan span);
        SirStatementId makeErrorStatement(SourceSpan span);

        SirExprId makeImplicitCastIfNeeded(SirExprId src, SirTypeId target, SourceSpan span);

        void diag(SourceSpan span, std::string msg);

        void pushScope();
        void popScope();
        void declareInScope(const std::string& name, SirSymbolId id);

        // Builtin type setup
        void initBuiltinTypes();
        SirTypeId builtinTypeFromKeyword(str_view keyword) const;

    private:
        str_view m_source;
        SirModule m_module;
        std::vector<Diagnostic> m_diagnostics;

        std::vector<SirScope> m_scopes;

        // Currently-being-built function context
        SirFunctionId m_currentFunction = InvalidFunction;
    };
} // namespace light_angel
