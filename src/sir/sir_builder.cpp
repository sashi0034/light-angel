#include "sir_builder.h"

#include <cctype>
#include <utility>

namespace light_angel
{
    // -----------------------------------------------
    // Public entry point / builder helpers
    // -----------------------------------------------

    SirBuildResult BuildSirModule(const Node_Script& script, str_view source)
    {
        SirBuilder builder(source);
        return builder.build(script);
    }

    SirBuilder::SirBuilder(str_view source)
        : m_source(source)
    {
        initBuiltinTypes();
        pushScope(); // global scope
    }

    SirBuildResult SirBuilder::build(const Node_Script& script)
    {
        buildScript(script);
        SirBuildResult result;
        result.module = std::move(m_module);
        result.diagnostics = std::move(m_diagnostics);
        return result;
    }

    void SirBuilder::initBuiltinTypes()
    {
        auto addSimple = [&](SirTypeKind kind, const char* name)
        {
            SirTypeInfo t;
            t.kind = kind;
            t.name = name;
            return m_module.addType(std::move(t));
        };
        auto addPrim = [&](PrimitiveTypeKind pk, const char* name)
        {
            SirTypeInfo t;
            t.kind = SirTypeKind::Primitive;
            t.name = name;
            SirTypeInfo::primitive_data pd;
            pd.primitiveKind = pk;
            t.data = pd;
            return m_module.addType(std::move(t));
        };
        m_module.errorType = addSimple(SirTypeKind::Error, "<error>");
        m_module.voidType = addSimple(SirTypeKind::Void, "void");
        m_module.boolType = addPrim(PrimitiveTypeKind::Bool, "bool");
        m_module.intType = addPrim(PrimitiveTypeKind::Int, "int");
        m_module.int8Type = addPrim(PrimitiveTypeKind::Int8, "int8");
        m_module.int16Type = addPrim(PrimitiveTypeKind::Int16, "int16");
        m_module.int32Type = addPrim(PrimitiveTypeKind::Int32, "int32");
        m_module.int64Type = addPrim(PrimitiveTypeKind::Int64, "int64");
        m_module.uintType = addPrim(PrimitiveTypeKind::UInt, "uint");
        m_module.uint8Type = addPrim(PrimitiveTypeKind::UInt8, "uint8");
        m_module.uint16Type = addPrim(PrimitiveTypeKind::UInt16, "uint16");
        m_module.uint32Type = addPrim(PrimitiveTypeKind::UInt32, "uint32");
        m_module.uint64Type = addPrim(PrimitiveTypeKind::UInt64, "uint64");
        m_module.floatType = addPrim(PrimitiveTypeKind::Float, "float");
        m_module.doubleType = addPrim(PrimitiveTypeKind::Double, "double");
        m_module.stringType = addSimple(SirTypeKind::NativeType, "string");
        m_module.nullType = addSimple(SirTypeKind::Null, "null_t");
    }

    SirTypeId SirBuilder::builtinTypeFromKeyword(str_view kw) const
    {
        if (kw == "void") return m_module.voidType;

        if (kw == "bool") return m_module.boolType;

        if (kw == "int") return m_module.intType;
        if (kw == "int8") return m_module.int8Type;
        if (kw == "int16") return m_module.int16Type;
        if (kw == "int32") return m_module.int32Type;
        if (kw == "int64") return m_module.int64Type;

        if (kw == "uint") return m_module.uintType;
        if (kw == "uint8") return m_module.uint8Type;
        if (kw == "uint16") return m_module.uint16Type;
        if (kw == "uint32") return m_module.uint32Type;
        if (kw == "uint64") return m_module.uint64Type;

        if (kw == "float") return m_module.floatType;

        if (kw == "double") return m_module.doubleType;

        if (kw == "string") return m_module.stringType;

        return InvalidType;
    }

    void SirBuilder::pushScope()
    {
        m_scopes.emplace_back();
    }

    void SirBuilder::popScope()
    {
        m_scopes.pop_back();
    }

    void SirBuilder::declareInScope(const std::string& name, SirSymbolId id)
    {
        if (m_scopes.empty()) return;

        m_scopes.back().names[name] = id;
    }

    SirSymbolId SirBuilder::lookupName(const std::string& name) const
    {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it)
        {
            auto found = it->names.find(name);
            if (found != it->names.end()) return found->second;
        }

        return InvalidSymbol;
    }

    std::string SirBuilder::spanText(SourceSpan span) const
    {
        if (span.offset >= m_source.size()) return {};

        size_t end = static_cast<size_t>(span.offset) + static_cast<size_t>(span.length);
        if (end > m_source.size()) end = m_source.size();

        return std::string(m_source.substr(span.offset, end - span.offset));
    }

    std::string SirBuilder::tokenText(const TokenView& tok) const
    {
        return spanText(tok.span());
    }

    void SirBuilder::diag(SourceSpan span, std::string msg)
    {
        Diagnostic d;
        d.severity = Diagnostic::Severity::Error;
        d.span = span;
        d.message = std::move(msg);
        m_diagnostics.push_back(std::move(d));
    }

    SirExprId SirBuilder::makeErrorExpr(SourceSpan span)
    {
        SirExpr e;
        e.span = span;
        e.type = m_module.errorType;
        e.data = SirErrorExpr{};
        return m_module.addExpr(std::move(e));
    }

    SirStatementId SirBuilder::makeErrorStatement(SourceSpan span)
    {
        SirStatement s;
        s.span = span;
        s.data = SirErrorStatement{};
        return m_module.addStatement(std::move(s));
    }

    SirExprId SirBuilder::makeImplicitCastIfNeeded(SirExprId src, SirTypeId target, SourceSpan span)
    {
        // TODO: full implicit conversion rules. For now only wrap when types differ
        // and neither is the error type. The cast leaves overload semantics to a
        // later pass.
        if (src == InvalidExpr || target == InvalidType) return src;

        SirTypeId srcType = m_module.exprs[src].type;
        if (srcType == target) return src;

        if (srcType == m_module.errorType || target == m_module.errorType) return src;

        SirExpr cast;
        cast.span = span;
        cast.type = target;
        cast.data = SirImplicitCastExpr{src};
        return m_module.addExpr(std::move(cast));
    }

    // -----------------------------------------------
    // Non-BNF helpers
    // -----------------------------------------------

    SirStatementId SirBuilder::buildStatBlockAsStatement(const Node_StatBlock& sb)
    {
        SirBlockId blk = buildStatBlock(sb);
        SirStatement s;
        s.span = sb.span;
        s.data = SirBlockStatement{blk};
        return m_module.addStatement(std::move(s));
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
    void SirBuilder::buildScript(const Node_Script& script)
    {
        for (const auto& child : script.children)
        {
            if (!child) continue;

            switch (child->kind)
            {
            case NodeKind::Func:
                buildFunction(AsNode<Node_Func>(*child));
                break;
            // TODO: NAMESPACE / USING / ENUM / TYPEDEF / CLASS / INTERFACE / FUNCDEF / VIRTUALPROP / VAR / IMPORT
            default:
                diag(child->span, "top-level node kind not yet supported in SIR builder");
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
    void SirBuilder::buildFunction(const Node_Func& func)
    {
        SirFunction sf;
        sf.span = func.span;
        sf.name = tokenText(func.identifier);
        sf.isDestructor = func.isDestructor;
        sf.isExternal = func.attr.has(EntityAttribute::External);

        if (func.returnType)
        {
            sf.returnType = buildType(*func.returnType);
        }
        else
        {
            // Destructor / no return type
            sf.returnType = m_module.voidType;
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
                sp.name = tokenText(p->identifier);
                sp.span = p->span;
                sp.type.type = p->type ? buildType(*p->type) : m_module.errorType;
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
        sf.functionType = m_module.addType(std::move(ftype));

        // Function symbol (declared in enclosing scope BEFORE body is built so it
        // can be referenced recursively).
        SirSymbol sym;
        sym.kind = SirSymbolKind::Function;
        sym.name = sf.name;
        sym.span = func.span;
        sym.type = sf.functionType;
        SirSymbolId symId = m_module.addSymbol(std::move(sym));
        sf.symbol = symId;
        SirFunctionId fnId = m_module.addFunction(std::move(sf));
        m_module.symbols[symId].function = fnId;
        declareInScope(m_module.functions[fnId].name, symId);

        if (!func.body)
        {
            // Forward decl / external.
            return;
        }

        // Build body in its own scope with parameters declared.
        SirFunctionId prev = m_currentFunction;
        m_currentFunction = fnId;
        pushScope();

        for (auto& sp : m_module.functions[fnId].parameters)
        {
            if (sp.name.empty()) continue;

            SirSymbol psym;
            psym.kind = SirSymbolKind::Parameter;
            psym.name = sp.name;
            psym.span = sp.span;
            psym.type = sp.type.type;
            sp.symbol = m_module.addSymbol(std::move(psym));
            declareInScope(sp.name, sp.symbol);
        }

        SirBlockId body = buildStatBlock(*func.body);
        m_module.functions[fnId].body = body;

        popScope();
        m_currentFunction = prev;
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
    SirStatementId SirBuilder::buildVarDecl(const Node_Var& var)
    {
        if (!var.type)
        {
            diag(var.span, "internal: var without type");
            return makeErrorStatement(var.span);
        }

        SirTypeId varType = buildType(*var.type);

        // Multi-declarator var: emit one SirVarDeclStatement per declarator.
        // The block this is added to is the caller's responsibility; here we
        // synthesize statements and return the LAST one. Earlier ones are still
        // appended via direct module insertion -- but the caller only knows about
        // one return id. To keep things clean, we wrap multiple decls in a synthetic
        // block when there is more than one.
        if (var.decls.empty())
        {
            return makeErrorStatement(var.span);
        }

        auto buildOne = [&](const Node_Var::Declaration& decl) -> SirStatementId
        {
            SirSymbol sym;
            sym.kind = SirSymbolKind::LocalVar;
            sym.name = tokenText(decl.identifier);
            sym.span = var.span;
            sym.type = varType;
            SirSymbolId sid = m_module.addSymbol(std::move(sym));
            declareInScope(m_module.symbols[sid].name, sid);

            SirExprId init = InvalidExpr;
            if (decl.init)
            {
                if (decl.init->kind == NodeKind::Assign)
                {
                    init = buildAssign(static_cast<const Node_Assign&>(*decl.init));
                    init = makeImplicitCastIfNeeded(init, varType, var.span);
                }
                else
                {
                    // TODO: InitList / ArgList initializers.
                    diag(decl.init->span, "var initializer kind not yet supported");
                    init = makeErrorExpr(decl.init->span);
                }
            }

            SirVarDeclStatement vds;
            vds.symbol = sid;
            vds.type = varType;
            vds.initializer = init;
            SirStatement s;
            s.span = var.span;
            s.data = vds;
            return m_module.addStatement(std::move(s));
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

        SirBlockId bid = m_module.addBlock(std::move(block));
        SirStatement s;
        s.span = var.span;
        s.data = SirBlockStatement{bid};
        return m_module.addStatement(std::move(s));
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
    SirBlockId SirBuilder::buildStatBlock(const Node_StatBlock& sb)
    {
        SirBlock block;
        block.span = sb.span;

        pushScope();
        for (const auto& child : sb.statements)
        {
            if (!child) continue;

            SirStatementId stmt = buildStatementNode(*child);
            if (stmt != InvalidStatement)
            {
                block.statements.push_back(stmt);
            }
        }

        popScope();

        return m_module.addBlock(std::move(block));
    }

    // **BNF** PARAMLIST ::= '(' ['void' | (PARAMETER {',' PARAMETER})] ')'
    // TODO

    // **BNF** PARAMETER ::= TYPE TYPEMODIFIER [IDENTIFIER] ['...' | ('=' (EXPR | 'void'))]
    // TODO

    // **BNF** TYPEMODIFIER ::= ['&' ['in' | 'out' | 'inout'] ['+'] ['if_handle_then_const']]
    // TODO

    // **BNF** TYPE ::= ['const'] SCOPE DATATYPE ['<' TYPE {',' TYPE} '>'] { ('[' ']') | ('@' ['const']) }
    SirTypeId SirBuilder::buildType(const Node_Type& typeNode)
    {
        if (!typeNode.dataType)
        {
            diag(typeNode.span, "internal: type node missing datatype");
            return m_module.errorType;
        }

        // BNF: DATATYPE ::= (IDENTIFIER | PRIMITIVETYPE | '?' | 'auto')
        std::string text = tokenText(typeNode.dataType->token);
        SirTypeId base = builtinTypeFromKeyword(text);
        if (base == InvalidType)
        {
            // TODO: user-defined types / scope resolution / template instances.
            diag(typeNode.span, "unresolved type '" + text + "'");
            return m_module.errorType;
        }

        // TODO: array '[ ]' and handle '@' postfixes - currently unsupported, return base.
        if (!typeNode.postfixes.empty())
        {
            diag(typeNode.span, "type postfixes not yet supported in SIR");
            // Continue with base type; emit as Error to avoid silently dropping.
            return m_module.errorType;
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
    SirStatementId SirBuilder::buildStatementNode(const NodeBase& node)
    {
        switch (node.kind)
        {
        case NodeKind::Var:
            return buildVarDecl(static_cast<const Node_Var&>(node));
        case NodeKind::Statement:
            {
                // STATEMENT ::= (IF | FOR | FOREACH | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)
                const auto& st = static_cast<const Node_Statement&>(node);
                if (!st.child) return makeErrorStatement(st.span);

                return buildStatementNode(*st.child);
            }
        case NodeKind::StatBlock:
            return buildStatBlockAsStatement(static_cast<const Node_StatBlock&>(node));
        case NodeKind::Return:
            return buildReturn(static_cast<const Node_Return&>(node));
        case NodeKind::ExprStat:
            return buildExprStat(static_cast<const Node_ExprStat&>(node));
        // TODO: If / While / DoWhile / For / ForEach / Switch / Break / Continue / Try / Using
        default:
            diag(node.span, "statement kind not yet supported in SIR builder");
            return makeErrorStatement(node.span);
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
    SirStatementId SirBuilder::buildExprStat(const Node_ExprStat& es)
    {
        SirExprStatement xs;
        if (es.assign)
        {
            xs.expr = buildAssign(*es.assign);
        }

        SirStatement s;
        s.span = es.span;
        s.data = xs;
        return m_module.addStatement(std::move(s));
    }

    // **BNF** TRY ::= 'try' STATBLOCK 'catch' STATBLOCK
    // TODO

    // **BNF** RETURN ::= 'return' [ASSIGN] ';'
    SirStatementId SirBuilder::buildReturn(const Node_Return& ret)
    {
        SirReturnStatement rs;
        if (ret.value)
        {
            SirExprId v = buildAssign(*ret.value);
            SirTypeId expected =
                m_currentFunction != InvalidFunction ? m_module.functions[m_currentFunction].returnType : InvalidType;
            if (expected != InvalidType)
            {
                v = makeImplicitCastIfNeeded(v, expected, ret.span);
            }

            rs.value = v;
        }

        SirStatement s;
        s.span = ret.span;
        s.data = rs;
        return m_module.addStatement(std::move(s));
    }

    // **BNF** CASE ::= (('case' EXPR) | 'default') ':' {STATEMENT}
    // TODO

    // **BNF** EXPR ::= EXPRTERM {EXPROP EXPRTERM}
    SirExprId SirBuilder::buildExpr(const Node_Expr& expr)
    {
        if (!expr.first) return makeErrorExpr(expr.span);

        SirExprId acc = buildExprTerm(*expr.first);

        // Left-fold for now. TODO: respect operator precedence properly when this
        // is not already done by the parser.
        for (const auto& opTerm : expr.rest)
        {
            if (!opTerm.term) continue;

            SirExprId rhs = buildExprTerm(*opTerm.term);

            std::string opText = tokenText(opTerm.op);
            bool ok = false;
            SirBinaryOp bop = mapBinaryOp(opText, ok);
            if (!ok)
            {
                diag(opTerm.term->span, "unsupported binary operator '" + opText + "'");
                acc = makeErrorExpr(opTerm.term->span);
                continue;
            }

            // Type: comparison/logical -> bool, otherwise lhs type. TODO: proper rules.
            SirTypeId resultType = isComparisonOp(bop) ? m_module.boolType : m_module.exprs[acc].type;

            SirExpr e;
            e.span = opTerm.term->span;
            e.type = resultType;
            e.data = SirBinaryExpr{bop, acc, rhs, InvalidFunction};
            acc = m_module.addExpr(std::move(e));
        }

        return acc;
    }

    // **BNF** EXPRTERM ::= ([TYPE '='] INITLIST) | ({EXPRPREOP} EXPRVALUE {EXPRPOSTOP})
    SirExprId SirBuilder::buildExprTerm(const Node_ExprTerm& term)
    {
        if (term.form == Node_ExprTerm::Form::InitListForm)
        {
            // TODO: init-list expressions.
            diag(term.span, "init-list expressions not yet supported in SIR");
            return makeErrorExpr(term.span);
        }

        if (!term.exprValue) return makeErrorExpr(term.span);

        SirExprId base = buildExprValue(*term.exprValue);

        // TODO: prefix / postfix operators (postOps include '.member', '[...]', '(...)', '++', '--').
        if (!term.preOps.empty() || !term.postOps.empty())
        {
            diag(term.span, "prefix/postfix operators not yet supported in SIR");
        }

        return base;
    }

    // **BNF** EXPRVALUE ::= CONSTRUCTORCALL | FUNCCALL | VARACCESS | CAST | LITERAL | '(' ASSIGN ')' | LAMBDA
    SirExprId SirBuilder::buildExprValue(const NodeBase& value)
    {
        switch (value.kind)
        {
        case NodeKind::Literal:
            return buildLiteral(static_cast<const Node_Literal&>(value));
        case NodeKind::VarAccess:
            return buildVarAccess(static_cast<const Node_VarAccess&>(value));
        case NodeKind::FuncCall:
            return buildFuncCall(static_cast<const Node_FuncCall&>(value));
        case NodeKind::Assign:
            // parenthesized expression
            return buildAssign(static_cast<const Node_Assign&>(value));
        case NodeKind::ExprValue:
            {
                const auto& ev = static_cast<const Node_ExprValue&>(value);
                if (!ev.value) return makeErrorExpr(ev.span);

                return buildExprValue(*ev.value);
            }
        // TODO: ConstructorCall / Cast / Lambda
        default:
            diag(value.span, "expression value kind not yet supported in SIR");
            return makeErrorExpr(value.span);
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
    SirExprId SirBuilder::buildLiteral(const Node_Literal& lit)
    {
        std::string text = tokenText(lit.token);

        SirLiteralExpr le;
        SirTypeId type = m_module.errorType;

        if (text == "true" || text == "false")
        {
            le.kind = SirLiteralKind::Bool;
            le.boolValue = (text == "true");
            type = m_module.boolType;
        }
        else if (text == "null")
        {
            le.kind = SirLiteralKind::Null;
            type = m_module.nullType;
        }
        else if (text == "void")
        {
            le.kind = SirLiteralKind::Void;
            type = m_module.voidType;
        }
        else if (!text.empty() && (text.front() == '"' || text.front() == '\''))
        {
            le.kind = SirLiteralKind::String;
            // TODO: string escape unprocessing.
            le.stringValue = text;
            type = m_module.stringType;
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
                type = m_module.doubleType;
            }
            else
            {
                le.kind = SirLiteralKind::Int;
                // strtoll handles 0x / 0 prefixes; for 0b / 0o we'd need custom parsing.
                // TODO: full BITS literal parsing (binary/octal/decimal/hex prefixes).
                le.intValue = std::strtoll(text.c_str(), nullptr, 0);
                type = m_module.intType;
            }
        }
        else
        {
            diag(lit.span, "unrecognized literal '" + text + "'");
            return makeErrorExpr(lit.span);
        }

        SirExpr e;
        e.span = lit.span;
        e.type = type;
        e.data = std::move(le);
        return m_module.addExpr(std::move(e));
    }

    // **BNF** FUNCCALL ::= SCOPE IDENTIFIER ['<' TYPE {',' TYPE} '>'] ARGLIST
    SirExprId SirBuilder::buildFuncCall(const Node_FuncCall& fc)
    {
        std::string name = tokenText(fc.identifier);
        // TODO: scope-qualified lookup, template args, overload resolution.
        SirSymbolId sid = lookupName(name);
        SirFunctionId fnId = InvalidFunction;
        SirTypeId resultType = m_module.errorType;
        if (sid != InvalidSymbol && m_module.symbols[sid].kind == SirSymbolKind::Function)
        {
            fnId = m_module.symbols[sid].function;
            if (fnId != InvalidFunction)
            {
                resultType = m_module.functions[fnId].returnType;
            }
        }
        else
        {
            diag(fc.span, "unresolved function '" + name + "'");
        }

        std::vector<SirExprId> args;
        if (fc.args)
        {
            // BNF: ARGLIST ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'
            for (const auto& a : fc.args->args)
            {
                if (!a.value)
                {
                    args.push_back(makeErrorExpr(fc.span));
                    continue;
                }

                SirExprId argExpr = buildAssign(*a.value);

                // Apply implicit cast to parameter type if known.
                if (fnId != InvalidFunction)
                {
                    size_t idx = args.size();
                    const auto& params = m_module.functions[fnId].parameters;
                    if (idx < params.size())
                    {
                        argExpr = makeImplicitCastIfNeeded(argExpr, params[idx].type.type, fc.span);
                    }
                }

                args.push_back(argExpr);
            }
        }

        SirExpr e;
        e.span = fc.span;
        e.type = resultType;
        e.data = SirCallExpr{fnId, std::move(args)};
        return m_module.addExpr(std::move(e));
    }

    // **BNF** VARACCESS ::= SCOPE IDENTIFIER
    SirExprId SirBuilder::buildVarAccess(const Node_VarAccess& va)
    {
        std::string name = tokenText(va.identifier);
        // TODO: scope-qualified names.
        SirSymbolId sid = lookupName(name);
        if (sid == InvalidSymbol)
        {
            diag(va.span, "unresolved name '" + name + "'");
            return makeErrorExpr(va.span);
        }

        SirExpr e;
        e.span = va.span;
        e.type = m_module.symbols[sid].type;
        e.data = SirSymbolRefExpr{sid};
        return m_module.addExpr(std::move(e));
    }

    // **BNF** ARGLIST ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'
    // TODO

    // **BNF** ASSIGN ::= CONDITION [ ASSIGNOP ASSIGN ]
    SirExprId SirBuilder::buildAssign(const Node_Assign& assign)
    {
        if (!assign.condition)
        {
            return makeErrorExpr(assign.span);
        }

        SirExprId lhs = buildCondition(*assign.condition);
        if (!assign.rhs || assign.op.isEmpty())
        {
            return lhs;
        }

        SirExprId rhs = buildAssign(*assign.rhs);
        std::string op = tokenText(assign.op);

        // BNF: ASSIGNOP ::= '=' | '+=' | '-=' | '*=' | '/=' | '|=' | '&=' | '^=' | '%=' | '**=' | '<<=' | '>>=' | '>>>='
        if (op == "=")
        {
            SirTypeId targetType = m_module.exprs[lhs].type;
            rhs = makeImplicitCastIfNeeded(rhs, targetType, assign.span);
            SirExpr e;
            e.span = assign.span;
            e.type = targetType;
            e.data = SirAssignExpr{lhs, rhs};
            return m_module.addExpr(std::move(e));
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
            diag(assign.span, "unsupported assignment operator '" + op + "'");
            return makeErrorExpr(assign.span);
        }

        SirTypeId targetType = m_module.exprs[lhs].type;

        SirExpr binop;
        binop.span = assign.span;
        binop.type = targetType; // TODO: proper type propagation.
        binop.data = SirBinaryExpr{bop, lhs, rhs, InvalidFunction};
        SirExprId binopId = m_module.addExpr(std::move(binop));

        SirExpr e;
        e.span = assign.span;
        e.type = targetType;
        e.data = SirAssignExpr{lhs, binopId};
        return m_module.addExpr(std::move(e));
    }

    // **BNF** CONDITION ::= EXPR ['?' ASSIGN ':' ASSIGN]
    SirExprId SirBuilder::buildCondition(const Node_Condition& cond)
    {
        if (!cond.expr) return makeErrorExpr(cond.span);

        SirExprId base = buildExpr(*cond.expr);
        if (!cond.thenExpr) return base;

        // TODO: ternary expression representation in SIR.
        diag(cond.span, "ternary '?:' not yet supported in SIR");
        return makeErrorExpr(cond.span);
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

} // namespace light_angel
