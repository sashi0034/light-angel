#include "sir_builder.h"

#include "sir_builder_context.h"

#include <cctype>
#include <utility>
#include <vector>

namespace
{
    using namespace light_angel;

    SirBuildResult build(SirBuilderContext& builder, const Node_Script& script);

    void buildScript(SirBuilderContext& builder, const Node_Script& script);
    void buildFunction(SirBuilderContext& builder, const Node_Func& func);
    SirStatementId buildVarDecl(SirBuilderContext& builder, const Node_Var& var);
    SirBlockId buildStatBlock(SirBuilderContext& builder, const Node_StatBlock& sb);
    SirTypeId buildType(SirBuilderContext& builder, const Node_Type& typeNode);
    SirStatementId buildStatementNode(SirBuilderContext& builder, const NodeBase& node);
    SirStatementId buildExprStat(SirBuilderContext& builder, const Node_ExprStat& es);
    SirStatementId buildReturn(SirBuilderContext& builder, const Node_Return& ret);
    SirExprId buildExpr(SirBuilderContext& builder, const Node_Expr& expr);
    SirExprId buildExprTerm(SirBuilderContext& builder, const Node_ExprTerm& term);
    SirExprId buildExprValue(SirBuilderContext& builder, const NodeBase& value);
    SirExprId buildLiteral(SirBuilderContext& builder, const Node_Literal& lit);
    SirExprId buildFuncCall(SirBuilderContext& builder, const Node_FuncCall& fc);
    SirExprId buildVarAccess(SirBuilderContext& builder, const Node_VarAccess& va);
    SirExprId buildAssign(SirBuilderContext& builder, const Node_Assign& assign);
    SirExprId buildCondition(SirBuilderContext& builder, const Node_Condition& cond);

    SirStatementId buildStatBlockAsStatement(SirBuilderContext& builder, const Node_StatBlock& sb);

    SirBuilderContext makeBuilder(str_view source)
    {
        SirBuilderContext builder;
        builder.source = source;
        builder.initBuiltinTypes();
        builder.pushScope(); // global scope
        return builder;
    }

    SirBuildResult build(SirBuilderContext& builder, const Node_Script& script)
    {
        buildScript(builder, script);
        SirBuildResult result;
        result.module = std::move(builder.module);
        result.diagnostics = std::move(builder.diagnostics);
        return result;
    }

    // -----------------------------------------------
    // Non-BNF helpers
    // -----------------------------------------------

    SirStatementId buildStatBlockAsStatement(SirBuilderContext& builder, const Node_StatBlock& sb)
    {
        SirBlockId blk = buildStatBlock(builder, sb);
        SirStatement s;
        s.span = sb.span;
        s.data = SirBlockStatement{blk};
        return builder.module.addStatement(std::move(s));
    }

    static SirBinaryOp mapBinaryOp(const std::string& op, bool& ok)
    {
        ok = true;
        if (op == "+") return SirBinaryOp::Add;

        if (op == "-") return SirBinaryOp::Sub;

        if (op == "*") return SirBinaryOp::Mul;

        if (op == "/") return SirBinaryOp::Div;

        if (op == "%") return SirBinaryOp::Mod;

        if (op == "**") return SirBinaryOp::Pow;

        if (op == "==") return SirBinaryOp::Eq;

        if (op == "!=") return SirBinaryOp::Ne;

        if (op == "<") return SirBinaryOp::Lt;

        if (op == "<=") return SirBinaryOp::Le;

        if (op == ">") return SirBinaryOp::Gt;

        if (op == ">=") return SirBinaryOp::Ge;

        if (op == "is") return SirBinaryOp::Is;

        if (op == "!is") return SirBinaryOp::NotIs;

        if (op == "&&" || op == "and") return SirBinaryOp::LAnd;

        if (op == "||" || op == "or") return SirBinaryOp::LOr;

        if (op == "^^" || op == "xor") return SirBinaryOp::LXor;

        if (op == "&") return SirBinaryOp::BAnd;

        if (op == "|") return SirBinaryOp::BOr;

        if (op == "^") return SirBinaryOp::BXor;

        if (op == "<<") return SirBinaryOp::Shl;

        if (op == ">>") return SirBinaryOp::Shr;

        if (op == ">>>") return SirBinaryOp::UShr;

        ok = false;
        return SirBinaryOp::Add;
    }

    static bool isComparisonOp(SirBinaryOp op)
    {
        switch (op)
        {
        case SirBinaryOp::Eq:
        case SirBinaryOp::Ne:
        case SirBinaryOp::Lt:
        case SirBinaryOp::Le:
        case SirBinaryOp::Gt:
        case SirBinaryOp::Ge:
        case SirBinaryOp::Is:
        case SirBinaryOp::NotIs:
        case SirBinaryOp::LAnd:
        case SirBinaryOp::LOr:
        case SirBinaryOp::LXor:
            return true;
        default:
            return false;
        }
    }

    // -----------------------------------------------
    // BNF
    // -----------------------------------------------

    // **BNF** SCRIPT ::= {IMPORT | ENUM | TYPEDEF | CLASS | INTERFACE | FUNCDEF | VIRTUALPROP | VAR | FUNC | NAMESPACE | USING | ';'}
    void buildScript(SirBuilderContext& builder, const Node_Script& script)
    {
        for (const auto& child : script.children)
        {
            if (!child) continue;

            switch (child->kind)
            {
            case NodeKind::Func:
                buildFunction(builder, AsNode<Node_Func>(*child));
                break;
            // TODO: NAMESPACE / USING / ENUM / TYPEDEF / CLASS / INTERFACE / FUNCDEF / VIRTUALPROP / VAR / IMPORT
            default:
                builder.reportDiagnostic(child->span, "top-level node kind not yet supported in SIR builder");
                break;
            }
        }
    }

    // **BNF** NAMESPACE ::= 'namespace' IDENTIFIER {'::' IDENTIFIER} '{' SCRIPT '}'
    // TODO

    // **BNF** USING ::= 'using' 'namespace' IDENTIFIER {'::' IDENTIFIER} ';'
    // TODO

    // **BNF** ENUM ::= {'shared' | 'external'} 'enum' IDENTIFIER [ ':' ('int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64') ] (';' | ('{' IDENTIFIER ['=' EXPR] {',' IDENTIFIER ['=' EXPR]} '}'))
    // TODO

    // **BNF** CLASS ::= ['mixin'] {'shared' | 'abstract' | 'final' | 'external'} 'class' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | FUNC | VAR | FUNCDEF} '}'))
    // TODO

    // **BNF** TYPEDEF ::= 'typedef' PRIMITIVETYPE IDENTIFIER ';'
    // TODO

    // **BNF** FUNC ::= {'shared' | 'external'} ['private' | 'protected'] [((TYPE ['&']) | '~')] IDENTIFIER ['<' TYPE {',' TYPE} '>'] PARAMLIST [LISTPATTERN] ['const'] FUNCATTR (';' | STATBLOCK)
    void buildFunction(SirBuilderContext& builder, const Node_Func& func)
    {
        SirFunction sf;
        sf.span = func.span;
        sf.name = builder.getTokenText(func.identifier);
        sf.isDestructor = func.isDestructor;
        sf.isExternal = func.attr.has(EntityAttribute::External);

        if (func.returnType)
        {
            sf.returnType = buildType(builder, *func.returnType);
        }
        else
        {
            // Destructor / no return type
            sf.returnType = builder.module.voidType;
        }

        // Parameters
        // BNF: PARAMLIST ::= '(' ['void' | (PARAMETER {',' PARAMETER})] ')'
        // BNF: PARAMETER ::= TYPE TYPEMODIFIER [IDENTIFIER] ['...' | ('=' (EXPR | 'void'))]
        std::vector<SirTypeId> paramTypes;
        if (func.params)
        {
            for (const auto& p : func.params->params)
            {
                if (!p) continue;

                SirParameter sp;
                sp.name = builder.getTokenText(p->identifier);
                sp.span = p->span;
                sp.type.type = p->type ? buildType(builder, *p->type) : builder.module.errorType;
                sp.refDir = RefDirection::In;
                sf.parameters.push_back(std::move(sp));
            }
        }

        // Function type entry
        SirTypeInfo ftype;
        ftype.kind = SirTypeKind::Function;
        ftype.name = sf.name;
        SirTypeInfo::function_data fprops;
        fprops.returnType = sf.returnType;
        fprops.parameterTypes = paramTypes;
        ftype.data = std::move(fprops);
        sf.functionType = builder.module.addType(std::move(ftype));

        // Function symbol (declared in enclosing scope BEFORE body is built so it
        // can be referenced recursively).
        SirSymbol sym;
        sym.kind = SirSymbolKind::Function;
        sym.name = sf.name;
        sym.span = func.span;
        sym.type = sf.functionType;
        SirSymbolId symId = builder.module.addSymbol(std::move(sym));
        sf.symbol = symId;
        SirFunctionId fnId = builder.module.addFunction(std::move(sf));
        builder.module.symbols[symId].function = fnId;
        builder.declareInScope(builder.module.functions[fnId].name, symId);

        if (!func.body)
        {
            // Forward decl / external.
            return;
        }

        // Build body in its own scope with parameters declared.
        SirFunctionId prev = builder.currentFunction;
        builder.currentFunction = fnId;
        builder.pushScope();

        for (auto& sp : builder.module.functions[fnId].parameters)
        {
            if (sp.name.empty()) continue;

            SirSymbol psym;
            psym.kind = SirSymbolKind::Parameter;
            psym.name = sp.name;
            psym.span = sp.span;
            psym.type = sp.type.type;
            sp.symbol = builder.module.addSymbol(std::move(psym));
            builder.declareInScope(sp.name, sp.symbol);
        }

        SirBlockId body = buildStatBlock(builder, *func.body);
        builder.module.functions[fnId].body = body;

        builder.popScope();
        builder.currentFunction = prev;
    }

    // **BNF** FUNCATTR ::= {'override' | 'final' | 'explicit' | 'property' | 'delete' | 'nodiscard'}
    // TODO

    // **BNF** LISTPATTERN ::= '{' LISTENTRY {',' LISTENTRY} '}'
    // TODO

    // **BNF** LISTENTRY ::= (('repeat' | 'repeat_same') (('{' LISTENTRY '}') | TYPE)) | (TYPE {',' TYPE})
    // TODO

    // **BNF** INTERFACE ::= {'external' | 'shared'} 'interface' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | INTERFACEMETHOD} '}'))
    // TODO

    // **BNF** VAR ::= ['private' | 'protected'] TYPE IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST] {',' IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST]} ';'
    SirStatementId buildVarDecl(SirBuilderContext& builder, const Node_Var& var)
    {
        if (!var.type)
        {
            builder.reportDiagnostic(var.span, "internal: var without type");
            return builder.makeErrorStatement(var.span);
        }

        SirTypeId varType = buildType(builder, *var.type);

        // Multi-declarator var: emit one SirVarDeclStatement per declarator.
        // The block this is added to is the caller's responsibility; here we
        // synthesize statements and return the LAST one. Earlier ones are still
        // appended via direct module insertion -- but the caller only knows about
        // one return id. To keep things clean, we wrap multiple decls in a synthetic
        // block when there is more than one.
        if (var.decls.empty())
        {
            return builder.makeErrorStatement(var.span);
        }

        auto buildOne = [&](const Node_Var::Declaration& decl) -> SirStatementId
        {
            SirSymbol sym;
            sym.kind = SirSymbolKind::LocalVar;
            sym.name = builder.getTokenText(decl.identifier);
            sym.span = var.span;
            sym.type = varType;
            SirSymbolId sid = builder.module.addSymbol(std::move(sym));
            builder.declareInScope(builder.module.symbols[sid].name, sid);

            SirExprId init = InvalidExpr;
            if (decl.init)
            {
                if (decl.init->kind == NodeKind::Assign)
                {
                    init = buildAssign(builder, static_cast<const Node_Assign&>(*decl.init));
                    init = builder.makeImplicitCastIfNeeded(init, varType, var.span);
                }
                else
                {
                    // TODO: InitList / ArgList initializers.
                    builder.reportDiagnostic(decl.init->span, "var initializer kind not yet supported");
                    init = builder.makeErrorExpr(decl.init->span);
                }
            }

            SirVarDeclStatement vds;
            vds.symbol = sid;
            vds.type = varType;
            vds.initializer = init;
            SirStatement s;
            s.span = var.span;
            s.data = vds;
            return builder.module.addStatement(std::move(s));
        };

        if (var.decls.size() == 1u)
        {
            return buildOne(var.decls.front());
        }

        // Multiple declarators -> wrap in a synthetic block.
        SirBlock block;
        block.span = var.span;
        for (const auto& decl : var.decls)
        {
            block.statements.push_back(buildOne(decl));
        }

        SirBlockId bid = builder.module.addBlock(std::move(block));
        SirStatement s;
        s.span = var.span;
        s.data = SirBlockStatement{bid};
        return builder.module.addStatement(std::move(s));
    }

    // **BNF** IMPORT ::= 'import' TYPE ['&'] IDENTIFIER PARAMLIST FUNCATTR 'from' STRING ';'
    // TODO

    // **BNF** FUNCDEF ::= {'external' | 'shared'} 'funcdef' TYPE ['&'] IDENTIFIER PARAMLIST ';'
    // TODO

    // **BNF** VIRTUALPROP ::= ['private' | 'protected'] TYPE ['&'] IDENTIFIER '{' {('get' | 'set') ['const'] FUNCATTR (STATBLOCK | ';')} '}'
    // TODO

    // **BNF** INTERFACEMETHOD ::= TYPE ['&'] IDENTIFIER PARAMLIST ['const'] FUNCATTR ';'
    // TODO

    // **BNF** STATBLOCK ::= '{' {VAR | STATEMENT | USING} '}'
    SirBlockId buildStatBlock(SirBuilderContext& builder, const Node_StatBlock& sb)
    {
        SirBlock block;
        block.span = sb.span;

        builder.pushScope();
        for (const auto& child : sb.statements)
        {
            if (!child) continue;

            SirStatementId stmt = buildStatementNode(builder, *child);
            if (stmt != InvalidStatement)
            {
                block.statements.push_back(stmt);
            }
        }

        builder.popScope();

        return builder.module.addBlock(std::move(block));
    }

    // **BNF** PARAMLIST ::= '(' ['void' | (PARAMETER {',' PARAMETER})] ')'
    // TODO

    // **BNF** PARAMETER ::= TYPE TYPEMODIFIER [IDENTIFIER] ['...' | ('=' (EXPR | 'void'))]
    // TODO

    // **BNF** TYPEMODIFIER ::= ['&' ['in' | 'out' | 'inout'] ['+'] ['if_handle_then_const']]
    // TODO

    // **BNF** TYPE ::= ['const'] SCOPE DATATYPE ['<' TYPE {',' TYPE} '>'] { ('[' ']') | ('@' ['const']) }
    SirTypeId buildType(SirBuilderContext& builder, const Node_Type& typeNode)
    {
        if (!typeNode.dataType)
        {
            builder.reportDiagnostic(typeNode.span, "internal: type node missing datatype");
            return builder.module.errorType;
        }

        // BNF: DATATYPE ::= (IDENTIFIER | PRIMITIVETYPE | '?' | 'auto')
        std::string text = builder.getTokenText(typeNode.dataType->token);
        SirTypeId base = builder.getBuiltinType(text);
        if (base == InvalidType)
        {
            // TODO: user-defined types / scope resolution / template instances.
            builder.reportDiagnostic(typeNode.span, "unresolved type '" + text + "'");
            return builder.module.errorType;
        }

        // TODO: array '[ ]' and handle '@' postfixes - currently unsupported, return base.
        if (!typeNode.postfixes.empty())
        {
            builder.reportDiagnostic(typeNode.span, "type postfixes not yet supported in SIR");
            // Continue with base type; emit as Error to avoid silently dropping.
            return builder.module.errorType;
        }

        return base;
    }

    // **BNF** INITLIST ::= '{' [ASSIGN | INITLIST] {',' [ASSIGN | INITLIST]} '}'
    // TODO

    // **BNF** SCOPE ::= ['::'] {IDENTIFIER '::'} [IDENTIFIER ['<' TYPE {',' TYPE} '>'] '::']
    // TODO

    // **BNF** DATATYPE ::= (IDENTIFIER | PRIMITIVETYPE | '?' | 'auto')
    // TODO

    // **BNF** PRIMITIVETYPE ::= 'void' | 'int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64' | 'float' | 'double' | 'bool'
    // TODO

    // **BNF** STATEMENT ::= (IF | FOR | FOREACH | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)
    SirStatementId buildStatementNode(SirBuilderContext& builder, const NodeBase& node)
    {
        switch (node.kind)
        {
        case NodeKind::Var:
            return buildVarDecl(builder, static_cast<const Node_Var&>(node));
        case NodeKind::Statement:
            {
                // STATEMENT ::= (IF | FOR | FOREACH | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)
                const auto& st = static_cast<const Node_Statement&>(node);
                if (!st.child) return builder.makeErrorStatement(st.span);

                return buildStatementNode(builder, *st.child);
            }
        case NodeKind::StatBlock:
            return buildStatBlockAsStatement(builder, static_cast<const Node_StatBlock&>(node));
        case NodeKind::Return:
            return buildReturn(builder, static_cast<const Node_Return&>(node));
        case NodeKind::ExprStat:
            return buildExprStat(builder, static_cast<const Node_ExprStat&>(node));
        // TODO: If / While / DoWhile / For / ForEach / Switch / Break / Continue / Try / Using
        default:
            builder.reportDiagnostic(node.span, "statement kind not yet supported in SIR builder");
            return builder.makeErrorStatement(node.span);
        }
    }

    // **BNF** SWITCH ::= 'switch' '(' ASSIGN ')' '{' {CASE} '}'
    // TODO

    // **BNF** BREAK ::= 'break' ';'
    // TODO

    // **BNF** FOR ::= 'for' '(' (VAR | EXPRSTAT) EXPRSTAT [ASSIGN {',' ASSIGN}] ')' STATEMENT
    // TODO

    // **BNF** FOREACH ::= 'foreach' '(' TYPE IDENTIFIER {',' TYPE IDENTIFIER} ':' ASSIGN ')' STATEMENT
    // TODO

    // **BNF** WHILE ::= 'while' '(' ASSIGN ')' STATEMENT
    // TODO

    // **BNF** DOWHILE ::= 'do' STATEMENT 'while' '(' ASSIGN ')' ';'
    // TODO

    // **BNF** IF ::= 'if' '(' ASSIGN ')' STATEMENT ['else' STATEMENT]
    // TODO

    // **BNF** CONTINUE ::= 'continue' ';'
    // TODO

    // **BNF** EXPRSTAT ::= [ASSIGN] ';'
    SirStatementId buildExprStat(SirBuilderContext& builder, const Node_ExprStat& es)
    {
        SirExprStatement xs;
        if (es.assign)
        {
            xs.expr = buildAssign(builder, *es.assign);
        }

        SirStatement s;
        s.span = es.span;
        s.data = xs;
        return builder.module.addStatement(std::move(s));
    }

    // **BNF** TRY ::= 'try' STATBLOCK 'catch' STATBLOCK
    // TODO

    // **BNF** RETURN ::= 'return' [ASSIGN] ';'
    SirStatementId buildReturn(SirBuilderContext& builder, const Node_Return& ret)
    {
        SirReturnStatement rs;
        if (ret.value)
        {
            SirExprId v = buildAssign(builder, *ret.value);
            SirTypeId expected =
                builder.currentFunction != InvalidFunction ? builder.module.functions[builder.currentFunction].returnType : InvalidType;
            if (expected != InvalidType)
            {
                v = builder.makeImplicitCastIfNeeded(v, expected, ret.span);
            }

            rs.value = v;
        }

        SirStatement s;
        s.span = ret.span;
        s.data = rs;
        return builder.module.addStatement(std::move(s));
    }

    // **BNF** CASE ::= (('case' EXPR) | 'default') ':' {STATEMENT}
    // TODO

    // **BNF** EXPR ::= EXPRTERM {EXPROP EXPRTERM}
    SirExprId buildExpr(SirBuilderContext& builder, const Node_Expr& expr)
    {
        if (!expr.first) return builder.makeErrorExpr(expr.span);

        SirExprId acc = buildExprTerm(builder, *expr.first);

        // Left-fold for now. TODO: respect operator precedence properly when this
        // is not already done by the parser.
        for (const auto& opTerm : expr.rest)
        {
            if (!opTerm.term) continue;

            SirExprId rhs = buildExprTerm(builder, *opTerm.term);

            std::string opText = builder.getTokenText(opTerm.op);
            bool ok = false;
            SirBinaryOp bop = mapBinaryOp(opText, ok);
            if (!ok)
            {
                builder.reportDiagnostic(opTerm.term->span, "unsupported binary operator '" + opText + "'");
                acc = builder.makeErrorExpr(opTerm.term->span);
                continue;
            }

            // Type: comparison/logical -> bool, otherwise lhs type. TODO: proper rules.
            SirTypeId resultType = isComparisonOp(bop) ? builder.module.boolType : builder.module.exprs[acc].type;

            SirExpr e;
            e.span = opTerm.term->span;
            e.type = resultType;
            e.data = SirBinaryExpr{bop, acc, rhs, InvalidFunction};
            acc = builder.module.addExpr(std::move(e));
        }

        return acc;
    }

    // **BNF** EXPRTERM ::= ([TYPE '='] INITLIST) | ({EXPRPREOP} EXPRVALUE {EXPRPOSTOP})
    SirExprId buildExprTerm(SirBuilderContext& builder, const Node_ExprTerm& term)
    {
        if (term.form == Node_ExprTerm::Form::InitListForm)
        {
            // TODO: init-list expressions.
            builder.reportDiagnostic(term.span, "init-list expressions not yet supported in SIR");
            return builder.makeErrorExpr(term.span);
        }

        if (!term.exprValue) return builder.makeErrorExpr(term.span);

        SirExprId base = buildExprValue(builder, *term.exprValue);

        // TODO: prefix / postfix operators (postOps include '.member', '[...]', '(...)', '++', '--').
        if (!term.preOps.empty() || !term.postOps.empty())
        {
            builder.reportDiagnostic(term.span, "prefix/postfix operators not yet supported in SIR");
        }

        return base;
    }

    // **BNF** EXPRVALUE ::= CONSTRUCTORCALL | FUNCCALL | VARACCESS | CAST | LITERAL | '(' ASSIGN ')' | LAMBDA
    SirExprId buildExprValue(SirBuilderContext& builder, const NodeBase& value)
    {
        switch (value.kind)
        {
        case NodeKind::Literal:
            return buildLiteral(builder, static_cast<const Node_Literal&>(value));
        case NodeKind::VarAccess:
            return buildVarAccess(builder, static_cast<const Node_VarAccess&>(value));
        case NodeKind::FuncCall:
            return buildFuncCall(builder, static_cast<const Node_FuncCall&>(value));
        case NodeKind::Assign:
            // parenthesized expression
            return buildAssign(builder, static_cast<const Node_Assign&>(value));
        case NodeKind::ExprValue:
            {
                const auto& ev = static_cast<const Node_ExprValue&>(value);
                if (!ev.value) return builder.makeErrorExpr(ev.span);

                return buildExprValue(builder, *ev.value);
            }
        // TODO: ConstructorCall / Cast / Lambda
        default:
            builder.reportDiagnostic(value.span, "expression value kind not yet supported in SIR");
            return builder.makeErrorExpr(value.span);
        }
    }

    // **BNF** CONSTRUCTORCALL ::= TYPE ARGLIST
    // TODO

    // **BNF** EXPRPREOP ::= '-' | '+' | '!' | '++' | '--' | '~' | '@'
    // TODO

    // **BNF** EXPRPOSTOP ::= ('.' (FUNCCALL | IDENTIFIER)) | ('[' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ']') | ARGLIST | '++' | '--'
    // TODO

    // **BNF** CAST ::= 'cast' '<' TYPE '>' '(' ASSIGN ')'
    // TODO

    // **BNF** LAMBDA ::= 'function' '(' [LAMBDAPARAM {',' LAMBDAPARAM}] ')' STATBLOCK
    // TODO

    // **BNF** LAMBDAPARAM ::= [TYPE TYPEMODIFIER] [IDENTIFIER]
    // TODO

    // **BNF** LITERAL ::= NUMBER | STRING | BITS | 'true' | 'false' | 'null' | 'void'
    SirExprId buildLiteral(SirBuilderContext& builder, const Node_Literal& lit)
    {
        std::string text = builder.getTokenText(lit.token);

        SirLiteralExpr le;
        SirTypeId type = builder.module.errorType;

        if (text == "true" || text == "false")
        {
            le.kind = SirLiteralKind::Bool;
            le.boolValue = (text == "true");
            type = builder.module.boolType;
        }
        else if (text == "null")
        {
            le.kind = SirLiteralKind::Null;
            type = builder.module.nullType;
        }
        else if (text == "void")
        {
            le.kind = SirLiteralKind::Void;
            type = builder.module.voidType;
        }
        else if (!text.empty() && (text.front() == '"' || text.front() == '\''))
        {
            le.kind = SirLiteralKind::String;
            // TODO: string escape unprocessing.
            le.stringValue = text;
            type = builder.module.stringType;
        }
        else if (!text.empty() && (std::isdigit(static_cast<unsigned char>(text.front())) || text.front() == '.'))
        {
            // NUMBER or BITS.
            bool isFloat = (text.find('.') != std::string::npos) ||
                           (text.find('e') != std::string::npos) ||
                           (text.find('E') != std::string::npos);
            if (isFloat)
            {
                le.kind = SirLiteralKind::Double;
                le.floatValue = std::strtod(text.c_str(), nullptr);
                type = builder.module.doubleType;
            }
            else
            {
                le.kind = SirLiteralKind::Int;
                // strtoll handles 0x / 0 prefixes; for 0b / 0o we'd need custom parsing.
                // TODO: full BITS literal parsing (binary/octal/decimal/hex prefixes).
                le.intValue = std::strtoll(text.c_str(), nullptr, 0);
                type = builder.module.intType;
            }
        }
        else
        {
            builder.reportDiagnostic(lit.span, "unrecognized literal '" + text + "'");
            return builder.makeErrorExpr(lit.span);
        }

        SirExpr e;
        e.span = lit.span;
        e.type = type;
        e.data = std::move(le);
        return builder.module.addExpr(std::move(e));
    }

    // **BNF** FUNCCALL ::= SCOPE IDENTIFIER ['<' TYPE {',' TYPE} '>'] ARGLIST
    SirExprId buildFuncCall(SirBuilderContext& builder, const Node_FuncCall& fc)
    {
        std::string name = builder.getTokenText(fc.identifier);
        // TODO: scope-qualified lookup, template args, overload resolution.
        SirSymbolId sid = builder.lookupName(name);
        SirFunctionId fnId = InvalidFunction;
        SirTypeId resultType = builder.module.errorType;
        if (sid != InvalidSymbol && builder.module.symbols[sid].kind == SirSymbolKind::Function)
        {
            fnId = builder.module.symbols[sid].function;
            if (fnId != InvalidFunction)
            {
                resultType = builder.module.functions[fnId].returnType;
            }
        }
        else
        {
            builder.reportDiagnostic(fc.span, "unresolved function '" + name + "'");
        }

        std::vector<SirExprId> args;
        if (fc.args)
        {
            // BNF: ARGLIST ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'
            for (const auto& a : fc.args->args)
            {
                if (!a.value)
                {
                    args.push_back(builder.makeErrorExpr(fc.span));
                    continue;
                }

                SirExprId argExpr = buildAssign(builder, *a.value);

                // Apply implicit cast to parameter type if known.
                if (fnId != InvalidFunction)
                {
                    size_t idx = args.size();
                    const auto& params = builder.module.functions[fnId].parameters;
                    if (idx < params.size())
                    {
                        argExpr = builder.makeImplicitCastIfNeeded(argExpr, params[idx].type.type, fc.span);
                    }
                }

                args.push_back(argExpr);
            }
        }

        SirExpr e;
        e.span = fc.span;
        e.type = resultType;
        e.data = SirCallExpr{fnId, std::move(args)};
        return builder.module.addExpr(std::move(e));
    }

    // **BNF** VARACCESS ::= SCOPE IDENTIFIER
    SirExprId buildVarAccess(SirBuilderContext& builder, const Node_VarAccess& va)
    {
        std::string name = builder.getTokenText(va.identifier);
        // TODO: scope-qualified names.
        SirSymbolId sid = builder.lookupName(name);
        if (sid == InvalidSymbol)
        {
            builder.reportDiagnostic(va.span, "unresolved name '" + name + "'");
            return builder.makeErrorExpr(va.span);
        }

        SirExpr e;
        e.span = va.span;
        e.type = builder.module.symbols[sid].type;
        e.data = SirSymbolRefExpr{sid};
        return builder.module.addExpr(std::move(e));
    }

    // **BNF** ARGLIST ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'
    // TODO

    // **BNF** ASSIGN ::= CONDITION [ ASSIGNOP ASSIGN ]
    SirExprId buildAssign(SirBuilderContext& builder, const Node_Assign& assign)
    {
        if (!assign.condition)
        {
            return builder.makeErrorExpr(assign.span);
        }

        SirExprId lhs = buildCondition(builder, *assign.condition);
        if (!assign.rhs || assign.op.isEmpty())
        {
            return lhs;
        }

        SirExprId rhs = buildAssign(builder, *assign.rhs);
        std::string op = builder.getTokenText(assign.op);

        // BNF: ASSIGNOP ::= '=' | '+=' | '-=' | '*=' | '/=' | '|=' | '&=' | '^=' | '%=' | '**=' | '<<=' | '>>=' | '>>>='
        if (op == "=")
        {
            SirTypeId targetType = builder.module.exprs[lhs].type;
            rhs = builder.makeImplicitCastIfNeeded(rhs, targetType, assign.span);
            SirExpr e;
            e.span = assign.span;
            e.type = targetType;
            e.data = SirAssignExpr{lhs, rhs};
            return builder.module.addExpr(std::move(e));
        }

        // Compound assignment a OP= b is normalized to a = a OP b.
        SirBinaryOp bop = SirBinaryOp::Add;
        bool ok = true;
        if (op == "+=")
            bop = SirBinaryOp::Add;
        else if (op == "-=")
            bop = SirBinaryOp::Sub;
        else if (op == "*=")
            bop = SirBinaryOp::Mul;
        else if (op == "/=")
            bop = SirBinaryOp::Div;
        else if (op == "%=")
            bop = SirBinaryOp::Mod;
        else if (op == "**=")
            bop = SirBinaryOp::Pow;
        else if (op == "|=")
            bop = SirBinaryOp::BOr;
        else if (op == "&=")
            bop = SirBinaryOp::BAnd;
        else if (op == "^=")
            bop = SirBinaryOp::BXor;
        else if (op == "<<=")
            bop = SirBinaryOp::Shl;
        else if (op == ">>=")
            bop = SirBinaryOp::Shr;
        else if (op == ">>>=")
            bop = SirBinaryOp::UShr;
        else
            ok = false;

        if (!ok)
        {
            builder.reportDiagnostic(assign.span, "unsupported assignment operator '" + op + "'");
            return builder.makeErrorExpr(assign.span);
        }

        SirTypeId targetType = builder.module.exprs[lhs].type;

        SirExpr binop;
        binop.span = assign.span;
        binop.type = targetType; // TODO: proper type propagation.
        binop.data = SirBinaryExpr{bop, lhs, rhs, InvalidFunction};
        SirExprId binopId = builder.module.addExpr(std::move(binop));

        SirExpr e;
        e.span = assign.span;
        e.type = targetType;
        e.data = SirAssignExpr{lhs, binopId};
        return builder.module.addExpr(std::move(e));
    }

    // **BNF** CONDITION ::= EXPR ['?' ASSIGN ':' ASSIGN]
    SirExprId buildCondition(SirBuilderContext& builder, const Node_Condition& cond)
    {
        if (!cond.expr) return builder.makeErrorExpr(cond.span);

        SirExprId base = buildExpr(builder, *cond.expr);
        if (!cond.thenExpr) return base;

        // TODO: ternary expression representation in SIR.
        builder.reportDiagnostic(cond.span, "ternary '?:' not yet supported in SIR");
        return builder.makeErrorExpr(cond.span);
    }

    // **BNF** EXPROP ::= MATHOP | COMPOP | LOGICOP | BITOP
    // TODO

    // **BNF** BITOP ::= '&' | '|' | '^' | '<<' | '>>' | '>>>'
    // TODO

    // **BNF** MATHOP ::= '+' | '-' | '*' | '/' | '%' | '**'
    // TODO

    // **BNF** COMPOP ::= '==' | '!=' | '<' | '<=' | '>' | '>=' | 'is' | '!is'
    // TODO

    // **BNF** LOGICOP ::= '&&' | '||' | '^^' | 'and' | 'or' | 'xor'
    // TODO

    // **BNF** ASSIGNOP ::= '=' | '+=' | '-=' | '*=' | '/=' | '|=' | '&=' | '^=' | '%=' | '**=' | '<<=' | '>>=' | '>>>='
    // TODO

} // namespace

namespace light_angel
{
    SirBuildResult BuildSirModule(const Node_Script& script, str_view source)
    {
        SirBuilderContext builder = makeBuilder(source);
        return build(builder, script);
    }
} // namespace light_angel
