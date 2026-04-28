#include "light_angel/parser.h"

#include "parser_context.h"

namespace
{
    using namespace light_angel;

    // forward declarations
    std::unique_ptr<Node_Script> parseScript(ParserContext& ctx, str_view stopAt = {});
    std::unique_ptr<Node_Namespace> parseNamespace(ParserContext& ctx);
    std::unique_ptr<Node_Using> parseUsing(ParserContext& ctx);
    std::unique_ptr<Node_Enum> parseEnum(ParserContext& ctx);
    std::unique_ptr<Node_Class> parseClass(ParserContext& ctx);
    std::unique_ptr<Node_TypeDef> parseTypeDef(ParserContext& ctx);
    std::unique_ptr<Node_Func> parseFunc(ParserContext& ctx);
    std::unique_ptr<Node_ListPattern> parseListPattern(ParserContext& ctx);
    std::unique_ptr<Node_ListEntry> parseListEntry(ParserContext& ctx);
    std::unique_ptr<Node_Interface> parseInterface(ParserContext& ctx);
    std::unique_ptr<Node_Var> parseVar(ParserContext& ctx);
    std::unique_ptr<Node_Import> parseImport(ParserContext& ctx);
    std::unique_ptr<Node_FuncDef> parseFuncDef(ParserContext& ctx);
    std::unique_ptr<Node_VirtualProp> parseVirtualProp(ParserContext& ctx);
    std::unique_ptr<Node_InterfaceMethod> parseInterfaceMethod(ParserContext& ctx);
    std::unique_ptr<Node_StatBlock> parseStatBlock(ParserContext& ctx);
    std::unique_ptr<Node_ParamList> parseParamList(ParserContext& ctx);
    std::unique_ptr<Node_Parameter> parseParameter(ParserContext& ctx);
    TypeModifier parseTypeModifier(ParserContext& ctx);
    std::unique_ptr<Node_Type> parseType(ParserContext& ctx);
    std::unique_ptr<Node_InitList> parseInitList(ParserContext& ctx);
    std::unique_ptr<Node_Scope> parseScope(ParserContext& ctx);
    std::unique_ptr<Node_DataType> parseDataType(ParserContext& ctx);
    FuncAttr parseFuncAttr(ParserContext& ctx);
    std::unique_ptr<NodeBase> parseStatement(ParserContext& ctx);
    std::unique_ptr<Node_Switch> parseSwitch(ParserContext& ctx);
    std::unique_ptr<Node_Break> parseBreak(ParserContext& ctx);
    std::unique_ptr<Node_For> parseFor(ParserContext& ctx);
    std::unique_ptr<Node_ForEach> parseForEach(ParserContext& ctx);
    std::unique_ptr<Node_While> parseWhile(ParserContext& ctx);
    std::unique_ptr<Node_DoWhile> parseDoWhile(ParserContext& ctx);
    std::unique_ptr<Node_If> parseIf(ParserContext& ctx);
    std::unique_ptr<Node_Continue> parseContinue(ParserContext& ctx);
    std::unique_ptr<Node_ExprStat> parseExprStat(ParserContext& ctx);
    std::unique_ptr<Node_Try> parseTry(ParserContext& ctx);
    std::unique_ptr<Node_Return> parseReturn(ParserContext& ctx);
    std::unique_ptr<Node_Case> parseCase(ParserContext& ctx);
    std::unique_ptr<Node_Expr> parseExpr(ParserContext& ctx);
    std::unique_ptr<Node_ExprTerm> parseExprTerm(ParserContext& ctx);
    std::unique_ptr<NodeBase> parseExprValue(ParserContext& ctx);
    std::unique_ptr<Node_ConstructorCall> parseConstructorCall(ParserContext& ctx);
    std::unique_ptr<Node_ExprPostOp> parseExprPostOp(ParserContext& ctx);
    std::unique_ptr<Node_Cast> parseCast(ParserContext& ctx);
    std::unique_ptr<Node_Lambda> parseLambda(ParserContext& ctx);
    std::unique_ptr<Node_LambdaParam> parseLambdaParam(ParserContext& ctx);
    std::unique_ptr<Node_Literal> parseLiteral(ParserContext& ctx);
    std::unique_ptr<Node_FuncCall> parseFuncCall(ParserContext& ctx);
    std::unique_ptr<Node_VarAccess> parseVarAccess(ParserContext& ctx);
    std::unique_ptr<Node_ArgList> parseArgList(ParserContext& ctx);
    std::unique_ptr<Node_Assign> parseAssign(ParserContext& ctx);
    std::unique_ptr<Node_Condition> parseCondition(ParserContext& ctx);

    // -----------------------------------------------
    // Helpers

    bool isPrimType(str_view s)
    {
        return s == "void" || s == "int" || s == "int8" || s == "int16" ||
               s == "int32" || s == "int64" || s == "uint" || s == "uint8" ||
               s == "uint16" || s == "uint32" || s == "uint64" ||
               s == "float" || s == "double" || s == "bool";
    }

    bool isExprPreOp(str_view s)
    {
        return s == "-" || s == "+" || s == "!" || s == "++" || s == "--" ||
               s == "~" || s == "@";
    }

    bool isExprOp(str_view s)
    {
        return s == "+" || s == "-" || s == "*" || s == "/" || s == "%" ||
               s == "**" || s == "==" || s == "!=" || s == "<" || s == "<=" ||
               s == ">" || s == ">=" || s == "is" || s == "!is" || s == "&&" ||
               s == "||" || s == "^^" || s == "and" || s == "or" || s == "xor" ||
               s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>" ||
               s == ">>>";
    }

    bool isAssignOp(str_view s)
    {
        return s == "=" || s == "+=" || s == "-=" || s == "*=" || s == "/=" ||
               s == "|=" || s == "&=" || s == "^=" || s == "%=" || s == "**=" ||
               s == "<<=" || s == ">>=" || s == ">>>=";
    }

    bool expectToken(ParserContext& ctx, str_view s)
    {
        if (ctx.check(s))
        {
            ctx.advance();
            return true;
        }

        return false;
    }

    void skipMetadata(ParserContext& ctx)
    {
        while (ctx.check("["))
        {
            int depth = 0;
            while (!ctx.isEnd())
            {
                if (ctx.check("["))
                {
                    depth++;
                    ctx.advance();
                }
                else if (ctx.check("]"))
                {
                    depth--;
                    ctx.advance();
                    if (depth == 0) break;
                }
                else
                    ctx.advance();
            }
        }
    }

    EntityAttribute parseEntityAttribute(ParserContext& ctx)
    {
        EntityAttribute attr;
        while (!ctx.isEnd())
        {
            str_view t = ctx.peekText();
            if (t == "shared")
            {
                attr.flags |= EntityAttribute::Shared;
                ctx.advance();
            }
            else if (t == "external")
            {
                attr.flags |= EntityAttribute::External;
                ctx.advance();
            }
            else if (t == "abstract")
            {
                attr.flags |= EntityAttribute::Abstract;
                ctx.advance();
            }
            else if (t == "final")
            {
                attr.flags |= EntityAttribute::Final;
                ctx.advance();
            }
            else
                break;
        }

        return attr;
    }

    std::optional<AccessModifier> parseAccessModifier(ParserContext& ctx)
    {
        if (ctx.check("private"))
        {
            ctx.advance();
            return AccessModifier::Private;
        }

        if (ctx.check("protected"))
        {
            ctx.advance();
            return AccessModifier::Protected;
        }

        return std::nullopt;
    }

    bool parseTemplateTypeArgs(
        ParserContext& ctx,
        std::vector<std::unique_ptr<Node_Type>>& out
    )
    {
        if (!ctx.check("<")) return false;

        const uint32_t saved = ctx.save();
        ctx.advance();

        while (!ctx.isEnd())
        {
            auto type = parseType(ctx);
            if (!type)
            {
                ctx.rewindTo(saved);
                out.clear();
                return false;
            }

            out.push_back(std::move(type));

            if (ctx.check(">"))
            {
                ctx.advance();
                return true;
            }

            if (!ctx.check(","))
            {
                ctx.rewindTo(saved);
                out.clear();
                return false;
            }

            ctx.advance();
        }

        ctx.rewindTo(saved);
        out.clear();
        return false;
    }

    bool canParseLambda(ParserContext& ctx)
    {
        if (!ctx.check("function") || !ctx.check("(", 1)) return false;

        int depth = 0;
        uint32_t i = 1;
        while (true)
        {
            const LexicalToken& token = ctx.peek(i);
            if (token.kind == TokenKind::EndOfFile) return false;

            str_view t = ctx.textOf(token.view);
            if (t == "(")
                depth++;
            else if (t == ")")
            {
                if (depth == 0) return ctx.check("{", i + 1);

                depth--;
            }

            i++;
        }
    }

    // **BNF** SCRIPT ::= {IMPORT | ENUM | TYPEDEF | CLASS | INTERFACE | FUNCDEF | VIRTUALPROP | VAR | FUNC | NAMESPACE | USING | ';'}
    std::unique_ptr<Node_Script> parseScript(ParserContext& ctx, str_view stopAt)
    {
        const uint32_t start = ctx.save();
        auto node = std::make_unique<Node_Script>();

        while (!ctx.isEnd())
        {
            if (!stopAt.empty() && ctx.check(stopAt)) break;

            if (ctx.check(";"))
            {
                ctx.advance();
                continue;
            }

            if (auto n = parseImport(ctx))
            {
                node->children.push_back(std::move(n));
                continue;
            }

            if (auto n = parseTypeDef(ctx))
            {
                node->children.push_back(std::move(n));
                continue;
            }

            if (auto n = parseNamespace(ctx))
            {
                node->children.push_back(std::move(n));
                continue;
            }

            if (auto n = parseUsing(ctx))
            {
                node->children.push_back(std::move(n));
                continue;
            }

            if (auto n = parseClass(ctx))
            {
                node->children.push_back(std::move(n));
                continue;
            }

            if (auto n = parseInterface(ctx))
            {
                node->children.push_back(std::move(n));
                continue;
            }

            if (auto n = parseEnum(ctx))
            {
                node->children.push_back(std::move(n));
                continue;
            }

            if (auto n = parseFuncDef(ctx))
            {
                node->children.push_back(std::move(n));
                continue;
            }

            if (auto n = parseFunc(ctx))
            {
                node->children.push_back(std::move(n));
                continue;
            }

            if (auto n = parseVirtualProp(ctx))
            {
                node->children.push_back(std::move(n));
                continue;
            }

            if (auto n = parseVar(ctx))
            {
                node->children.push_back(std::move(n));
                continue;
            }

            ctx.advanceError(); // error recovery: skip unrecognized token
        }

        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** NAMESPACE ::= 'namespace' IDENTIFIER {'::' IDENTIFIER} '{' SCRIPT '}'
    std::unique_ptr<Node_Namespace> parseNamespace(ParserContext& ctx)
    {
        if (!ctx.check("namespace")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        std::vector<TokenView> identifiers;
        while (!ctx.isEnd())
        {
            if (!ctx.checkKind(TokenKind::Name)) break;

            identifiers.push_back(ctx.consumeView());
            if (!ctx.check("::")) break;

            ctx.advance();
        }

        if (identifiers.empty()) return nullptr;

        if (!expectToken(ctx, "{")) return nullptr;

        auto script = parseScript(ctx, "}");
        expectToken(ctx, "}");

        auto node = std::make_unique<Node_Namespace>(ctx.spanFrom(start));
        node->identifiers = std::move(identifiers);
        node->script = std::move(script);
        return node;
    }

    // **BNF** USING ::= 'using' 'namespace' IDENTIFIER {'::' IDENTIFIER} ';'
    std::unique_ptr<Node_Using> parseUsing(ParserContext& ctx)
    {
        if (!ctx.check("using")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        if (!ctx.check("namespace"))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        ctx.advance();

        std::vector<TokenView> identifiers;
        while (!ctx.isEnd())
        {
            if (!ctx.checkKind(TokenKind::Name)) break;

            identifiers.push_back(ctx.consumeView());
            if (!ctx.check("::")) break;

            ctx.advance();
        }

        if (identifiers.empty())
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        expectToken(ctx, ";");

        auto node = std::make_unique<Node_Using>(ctx.spanFrom(start));
        node->identifiers = std::move(identifiers);
        return node;
    }

    // **BNF** ENUM ::= {'shared' | 'external'} 'enum' IDENTIFIER [ ':' ('int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64') ] (';' | ('{' IDENTIFIER ['=' EXPR] {',' IDENTIFIER ['=' EXPR]} '}'))
    std::unique_ptr<Node_Enum> parseEnum(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        auto attr = parseEntityAttribute(ctx);

        if (!ctx.check("enum"))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        ctx.advance();

        if (!ctx.checkKind(TokenKind::Name)) return nullptr;

        auto ident = ctx.consumeView();

        auto node = std::make_unique<Node_Enum>();
        node->attr = attr;
        node->identifier = ident;

        if (ctx.check(":"))
        {
            ctx.advance();
            if (ctx.checkKind(TokenKind::Name) && isPrimType(ctx.peekText()))
                node->baseType = ctx.consumeView();
        }

        if (ctx.check(";"))
        {
            ctx.advance();
            node->isForwardDecl = true;
            node->span = ctx.spanFrom(start);
            return node;
        }

        expectToken(ctx, "{");
        bool first = true;

        while (!ctx.isEnd() && !ctx.check("}"))
        {
            if (!first)
            {
                if (!ctx.check(",")) break;

                ctx.advance();
                if (ctx.check("}")) break;
            }

            first = false;

            if (!ctx.checkKind(TokenKind::Name)) break;

            Node_Enum::EnumValue ev;
            ev.name = ctx.consumeView();
            if (ctx.check("="))
            {
                ctx.advance();
                ev.expr = parseExpr(ctx);
            }

            node->values.push_back(std::move(ev));
        }

        expectToken(ctx, "}");
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** CLASS ::= ['mixin'] {'shared' | 'abstract' | 'final' | 'external'} 'class' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | FUNC | VAR | FUNCDEF} '}'))
    std::unique_ptr<Node_Class> parseClass(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();

        skipMetadata(ctx);

        EntityAttribute attr;
        if (ctx.check("mixin"))
        {
            attr.flags |= EntityAttribute::Mixin;
            ctx.advance();
        }

        {
            auto extra = parseEntityAttribute(ctx);
            attr.flags |= extra.flags;
        }

        if (!ctx.check("class"))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        ctx.advance();

        if (!ctx.checkKind(TokenKind::Name)) return nullptr;

        auto ident = ctx.consumeView();

        auto node = std::make_unique<Node_Class>();
        node->attr = attr;
        node->identifier = ident;

        if (ctx.check(";"))
        {
            ctx.advance();
            node->isForwardDecl = true;
            node->span = ctx.spanFrom(start);
            return node;
        }

        if (ctx.check(":"))
        {
            ctx.advance();
            bool first = true;
            while (!ctx.isEnd() && !ctx.check("{"))
            {
                if (!first)
                {
                    if (!ctx.check(",")) break;

                    ctx.advance();
                }

                first = false;
                Node_Class::BaseClass b;
                b.scope = parseScope(ctx);
                if (ctx.checkKind(TokenKind::Name)) b.identifier = ctx.consumeView();

                node->bases.push_back(std::move(b));
            }
        }

        expectToken(ctx, "{");

        while (!ctx.isEnd() && !ctx.check("}"))
        {
            if (auto n = parseFuncDef(ctx))
            {
                node->members.push_back(std::move(n));
                continue;
            }

            if (auto n = parseFunc(ctx))
            {
                node->members.push_back(std::move(n));
                continue;
            }

            if (auto n = parseVirtualProp(ctx))
            {
                node->members.push_back(std::move(n));
                continue;
            }

            if (auto n = parseVar(ctx))
            {
                node->members.push_back(std::move(n));
                continue;
            }

            ctx.advanceError();
        }

        expectToken(ctx, "}");
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** TYPEDEF ::= 'typedef' PRIMITIVETYPE IDENTIFIER ';'
    std::unique_ptr<Node_TypeDef> parseTypeDef(ParserContext& ctx)
    {
        if (!ctx.check("typedef")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        if (!ctx.checkKind(TokenKind::Name) || !isPrimType(ctx.peekText())) return nullptr;

        auto primTok = ctx.consumeView();

        if (!ctx.checkKind(TokenKind::Name)) return nullptr;

        auto ident = ctx.consumeView();

        expectToken(ctx, ";");

        auto node = std::make_unique<Node_TypeDef>(ctx.spanFrom(start));
        node->primitiveType = primTok;
        node->identifier = ident;
        return node;
    }

    // **BNF** FUNC ::= {'shared' | 'external'} ['private' | 'protected'] [((TYPE ['&']) | '~')] IDENTIFIER ['<' TYPE {',' TYPE} '>'] PARAMLIST [LISTPATTERN] ['const'] FUNCATTR (';' | STATBLOCK)
    std::unique_ptr<Node_Func> parseFunc(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();

        skipMetadata(ctx);
        auto attr = parseEntityAttribute(ctx);
        auto access = parseAccessModifier(ctx);

        bool isDestructor = false;
        std::unique_ptr<Node_Type> returnType;
        bool isReturnRef = false;

        if (ctx.check("~"))
        {
            isDestructor = true;
            ctx.advance();
        }
        else if (ctx.checkKind(TokenKind::Name) && ctx.check("(", 1))
        {
            // constructor
        }
        else
        {
            returnType = parseType(ctx);
            if (!returnType)
            {
                ctx.rewindTo(start);
                return nullptr;
            }

            isReturnRef = ctx.consume("&");
        }

        if (!ctx.checkKind(TokenKind::Name))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        auto ident = ctx.consumeView();

        std::vector<std::unique_ptr<Node_Type>> templateParams;
        parseTemplateTypeArgs(ctx, templateParams);

        auto params = parseParamList(ctx);
        if (!params)
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        auto listPattern = parseListPattern(ctx);
        if (listPattern)
        {
            if (!expectToken(ctx, ";"))
            {
                ctx.rewindTo(start);
                return nullptr;
            }

            auto node = std::make_unique<Node_Func>(ctx.spanFrom(start));
            node->attr = attr;
            node->access = access;
            node->isDestructor = isDestructor;
            node->returnType = std::move(returnType);
            node->isReturnRef = isReturnRef;
            node->identifier = ident;
            node->templateParams = std::move(templateParams);
            node->params = std::move(params);
            node->listPattern = std::move(listPattern);
            return node;
        }

        bool isConst = ctx.consume("const");
        auto funcAttr = parseFuncAttr(ctx);

        std::unique_ptr<Node_StatBlock> body;
        if (ctx.check(";"))
            ctx.advance();
        else
            body = parseStatBlock(ctx);

        auto node = std::make_unique<Node_Func>(ctx.spanFrom(start));
        node->attr = attr;
        node->access = access;
        node->isDestructor = isDestructor;
        node->returnType = std::move(returnType);
        node->isReturnRef = isReturnRef;
        node->identifier = ident;
        node->templateParams = std::move(templateParams);
        node->params = std::move(params);
        node->isConst = isConst;
        node->funcAttr = funcAttr;
        node->body = std::move(body);
        return node;
    }

    // **BNF** LISTPATTERN ::= '{' LISTENTRY {',' LISTENTRY} '}'
    std::unique_ptr<Node_ListPattern> parseListPattern(ParserContext& ctx)
    {
        if (!ctx.check("{")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        auto node = std::make_unique<Node_ListPattern>();
        bool first = true;

        while (!ctx.isEnd() && !ctx.check("}"))
        {
            if (!first)
            {
                if (!ctx.check(",")) break;

                ctx.advance();
            }

            first = false;

            auto entry = parseListEntry(ctx);
            if (!entry)
            {
                ctx.rewindTo(start);
                return nullptr;
            }

            node->entries.push_back(std::move(entry));
        }

        if (node->entries.empty())
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        if (!expectToken(ctx, "}"))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** LISTENTRY ::= (('repeat' | 'repeat_same') (('{' LISTENTRY '}') | TYPE)) | (TYPE {',' TYPE})
    std::unique_ptr<Node_ListEntry> parseListEntry(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();

        bool isRepeat = ctx.check("repeat") || ctx.check("repeat_same");
        if (isRepeat)
        {
            bool isSame = ctx.check("repeat_same");
            ctx.advance();

            auto node = std::make_unique<Node_ListEntry>(ctx.spanFrom(start));
            node->isRepeatSame = isSame;

            if (ctx.check("{"))
            {
                ctx.advance();
                node->entryKind = Node_ListEntry::Kind::RepeatSub;
                node->subEntry = parseListEntry(ctx);
                expectToken(ctx, "}");
            }
            else
            {
                node->entryKind = Node_ListEntry::Kind::RepeatType;
                if (auto t = parseType(ctx)) node->types.push_back(std::move(t));
            }

            node->span = ctx.spanFrom(start);
            return node;
        }

        auto node = std::make_unique<Node_ListEntry>(ctx.spanFrom(start));
        node->entryKind = Node_ListEntry::Kind::TypeList;

        while (!ctx.isEnd())
        {
            auto t = parseType(ctx);
            if (!t) break;

            node->types.push_back(std::move(t));

            if (!ctx.check(",")) break;

            const uint32_t savedPos = ctx.save();
            ctx.advance();
            if (ctx.check("repeat") || ctx.check("repeat_same"))
            {
                ctx.rewindTo(savedPos);
                break;
            }
        }

        if (node->types.empty())
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** INTERFACE ::= {'external' | 'shared'} 'interface' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | INTERFACEMETHOD} '}'))
    std::unique_ptr<Node_Interface> parseInterface(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        auto attr = parseEntityAttribute(ctx);

        if (!ctx.check("interface"))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        ctx.advance();

        if (!ctx.checkKind(TokenKind::Name)) return nullptr;

        auto ident = ctx.consumeView();

        auto node = std::make_unique<Node_Interface>();
        node->attr = attr;
        node->identifier = ident;

        if (ctx.check(";"))
        {
            ctx.advance();
            node->isForwardDecl = true;
            node->span = ctx.spanFrom(start);
            return node;
        }

        if (ctx.check(":"))
        {
            ctx.advance();
            bool first = true;
            while (!ctx.isEnd() && !ctx.check("{"))
            {
                if (!first)
                {
                    if (!ctx.check(",")) break;

                    ctx.advance();
                }

                first = false;
                Node_Interface::BaseInterface b;
                b.scope = parseScope(ctx);
                if (ctx.checkKind(TokenKind::Name)) b.identifier = ctx.consumeView();

                node->bases.push_back(std::move(b));
            }
        }

        expectToken(ctx, "{");

        while (!ctx.isEnd() && !ctx.check("}"))
        {
            if (auto im = parseInterfaceMethod(ctx))
            {
                node->members.push_back(std::move(im));
                continue;
            }

            if (auto vp = parseVirtualProp(ctx))
            {
                node->members.push_back(std::move(vp));
                continue;
            }

            ctx.advanceError();
        }

        expectToken(ctx, "}");
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** VAR ::= ['private' | 'protected'] TYPE IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST] {',' IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST]} ';'
    std::unique_ptr<Node_Var> parseVar(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();

        skipMetadata(ctx);
        auto access = parseAccessModifier(ctx);

        auto type = parseType(ctx);
        if (!type)
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        if (!ctx.checkKind(TokenKind::Name))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        auto node = std::make_unique<Node_Var>();
        node->access = access;
        node->type = std::move(type);

        bool first = true;
        while (!ctx.isEnd())
        {
            if (!first)
            {
                if (!ctx.check(",")) break;

                ctx.advance();
            }

            first = false;

            if (!ctx.checkKind(TokenKind::Name)) break;

            Node_Var::VarDecl decl;
            decl.identifier = ctx.consumeView();

            if (ctx.check("="))
            {
                ctx.advance();
                if (auto il = parseInitList(ctx))
                    decl.init = std::move(il);
                else
                    decl.init = parseAssign(ctx);
            }
            else if (auto al = parseArgList(ctx))
            {
                decl.init = std::move(al);
            }

            node->decls.push_back(std::move(decl));
        }

        if (node->decls.empty())
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        if (!expectToken(ctx, ";"))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** IMPORT ::= 'import' TYPE ['&'] IDENTIFIER PARAMLIST FUNCATTR 'from' STRING ';'
    std::unique_ptr<Node_Import> parseImport(ParserContext& ctx)
    {
        if (!ctx.check("import")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        auto type = parseType(ctx);
        if (!type) return nullptr;

        bool isRef = ctx.consume("&");

        if (!ctx.checkKind(TokenKind::Name)) return nullptr;

        auto ident = ctx.consumeView();

        auto params = parseParamList(ctx);
        if (!params) return nullptr;

        auto funcAttr = parseFuncAttr(ctx);

        if (!ctx.check("from")) return nullptr;

        ctx.advance();

        if (!ctx.checkKind(TokenKind::String)) return nullptr;

        auto fromStr = ctx.consumeView();

        expectToken(ctx, ";");

        auto node = std::make_unique<Node_Import>(ctx.spanFrom(start));
        node->type = std::move(type);
        node->isRef = isRef;
        node->identifier = ident;
        node->params = std::move(params);
        node->funcAttr = funcAttr;
        node->fromString = fromStr;
        return node;
    }

    // **BNF** FUNCDEF ::= {'external' | 'shared'} 'funcdef' TYPE ['&'] IDENTIFIER PARAMLIST ';'
    std::unique_ptr<Node_FuncDef> parseFuncDef(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        auto attr = parseEntityAttribute(ctx);

        if (!ctx.check("funcdef"))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        ctx.advance();

        auto returnType = parseType(ctx);
        if (!returnType) return nullptr;

        bool isRef = ctx.consume("&");

        if (!ctx.checkKind(TokenKind::Name)) return nullptr;

        auto ident = ctx.consumeView();

        auto params = parseParamList(ctx);
        if (!params) return nullptr;

        expectToken(ctx, ";");

        auto node = std::make_unique<Node_FuncDef>(ctx.spanFrom(start));
        node->attr = attr;
        node->returnType = std::move(returnType);
        node->isRef = isRef;
        node->identifier = ident;
        node->params = std::move(params);
        return node;
    }

    // **BNF** VIRTUALPROP ::= ['private' | 'protected'] TYPE ['&'] IDENTIFIER '{' {('get' | 'set') ['const'] FUNCATTR (STATBLOCK | ';')} '}'
    std::unique_ptr<Node_VirtualProp> parseVirtualProp(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();

        skipMetadata(ctx);
        auto access = parseAccessModifier(ctx);

        auto type = parseType(ctx);
        if (!type)
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        bool isRef = ctx.consume("&");

        if (!ctx.checkKind(TokenKind::Name))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        auto ident = ctx.consumeView();

        if (!ctx.check("{"))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        ctx.advance();

        std::vector<Node_VirtualProp::Accessor> accessors;
        while (!ctx.isEnd() && !ctx.check("}"))
        {
            if (!ctx.check("get") && !ctx.check("set"))
            {
                ctx.advanceError();
                continue;
            }

            Node_VirtualProp::Accessor acc;
            acc.isGet = ctx.check("get");
            ctx.advance();
            acc.isConst = ctx.consume("const");
            acc.attr = parseFuncAttr(ctx);
            if (ctx.check(";"))
            {
                ctx.advance();
            }
            else
            {
                acc.body = parseStatBlock(ctx);
            }

            accessors.push_back(std::move(acc));
        }

        expectToken(ctx, "}");

        auto node = std::make_unique<Node_VirtualProp>(ctx.spanFrom(start));
        node->access = access;
        node->type = std::move(type);
        node->isRef = isRef;
        node->identifier = ident;
        node->accessors = std::move(accessors);
        return node;
    }

    // **BNF** INTERFACEMETHOD ::= TYPE ['&'] IDENTIFIER PARAMLIST ['const'] FUNCATTR ';'
    std::unique_ptr<Node_InterfaceMethod> parseInterfaceMethod(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();

        auto type = parseType(ctx);
        if (!type) return nullptr;

        bool isRef = ctx.consume("&");

        if (!ctx.checkKind(TokenKind::Name))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        auto ident = ctx.consumeView();

        auto params = parseParamList(ctx);
        if (!params)
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        bool isConst = ctx.consume("const");
        auto funcAttr = parseFuncAttr(ctx);

        if (!expectToken(ctx, ";"))
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        auto node = std::make_unique<Node_InterfaceMethod>(ctx.spanFrom(start));
        node->type = std::move(type);
        node->isRef = isRef;
        node->identifier = ident;
        node->params = std::move(params);
        node->isConst = isConst;
        node->funcAttr = funcAttr;
        return node;
    }

    // **BNF** STATBLOCK ::= '{' {VAR | STATEMENT | USING} '}'
    std::unique_ptr<Node_StatBlock> parseStatBlock(ParserContext& ctx)
    {
        if (!ctx.check("{")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        auto node = std::make_unique<Node_StatBlock>();

        while (!ctx.isEnd() && !ctx.check("}"))
        {
            if (auto u = parseUsing(ctx))
            {
                node->statements.push_back(std::move(u));
                continue;
            }

            if (auto v = parseVar(ctx))
            {
                node->statements.push_back(std::move(v));
                continue;
            }

            if (auto s = parseStatement(ctx))
            {
                node->statements.push_back(std::move(s));
                continue;
            }

            ctx.advanceError();
        }

        expectToken(ctx, "}");
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** PARAMLIST ::= '(' ['void' | (PARAMETER {',' PARAMETER})] ')'
    std::unique_ptr<Node_ParamList> parseParamList(ParserContext& ctx)
    {
        if (!ctx.check("(")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        auto node = std::make_unique<Node_ParamList>();

        if (ctx.check("void") && ctx.check(")", 1))
        {
            ctx.advance();
            ctx.advance();
            node->isVoid = true;
            node->span = ctx.spanFrom(start);
            return node;
        }

        bool first = true;
        while (!ctx.isEnd() && !ctx.check(")"))
        {
            if (!first)
            {
                if (!ctx.check(",")) break;

                ctx.advance();
            }

            first = false;

            auto param = parseParameter(ctx);
            if (!param) break;

            node->params.push_back(std::move(param));
        }

        expectToken(ctx, ")");
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** PARAMETER ::= TYPE TYPEMODIFIER [IDENTIFIER] ['...' | ('=' (EXPR | 'void'))]
    std::unique_ptr<Node_Parameter> parseParameter(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();

        auto type = parseType(ctx);
        if (!type) return nullptr;

        auto mod = parseTypeModifier(ctx);

        TokenView ident;
        if (ctx.checkKind(TokenKind::Name)) ident = ctx.consumeView();

        bool isVariadic = false;
        bool hasVoidDefault = false;
        std::unique_ptr<Node_Expr> defaultExpr;

        if (ctx.check("..."))
        {
            ctx.advance();
            isVariadic = true;
        }
        else if (ctx.check("="))
        {
            ctx.advance();
            if (ctx.check("void"))
            {
                ctx.advance();
                hasVoidDefault = true;
            }
            else
            {
                defaultExpr = parseExpr(ctx);
            }
        }

        auto node = std::make_unique<Node_Parameter>(ctx.spanFrom(start));
        node->type = std::move(type);
        node->typeModifier = mod;
        node->identifier = ident;
        node->isVariadic = isVariadic;
        node->hasVoidDefault = hasVoidDefault;
        node->defaultExpr = std::move(defaultExpr);
        return node;
    }

    // **BNF** TYPEMODIFIER ::= ['&' ['in' | 'out' | 'inout'] ['+'] ['if_handle_then_const']]
    TypeModifier parseTypeModifier(ParserContext& ctx)
    {
        TypeModifier mod;
        if (!ctx.check("&")) return mod;

        mod.isRef = true;
        ctx.advance();

        if (ctx.check("in"))
        {
            mod.refDir = RefDirection::In;
            ctx.advance();
        }
        else if (ctx.check("out"))
        {
            mod.refDir = RefDirection::Out;
            ctx.advance();
        }
        else if (ctx.check("inout"))
        {
            mod.refDir = RefDirection::InOut;
            ctx.advance();
        }

        if (ctx.check("+")) ctx.advance();

        if (ctx.check("if_handle_then_const"))
        {
            mod.ifHandleThenConst = true;
            ctx.advance();
        }

        return mod;
    }

    // **BNF** TYPE ::= ['const'] SCOPE DATATYPE ['<' TYPE {',' TYPE} '>'] { ('[' ']') | ('@' ['const']) }
    std::unique_ptr<Node_Type> parseType(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();

        bool isConst = false;
        if (ctx.check("const"))
        {
            isConst = true;
            ctx.advance();
        }

        auto scope = parseScope(ctx);
        auto dataType = parseDataType(ctx);

        if (!dataType)
        {
            ctx.rewindTo(start);
            return nullptr;
        }

        std::vector<std::unique_ptr<Node_Type>> templateArgs;
        parseTemplateTypeArgs(ctx, templateArgs);

        std::vector<Node_Type::Postfix> postfixes;
        while (!ctx.isEnd())
        {
            if (ctx.check("[") && ctx.check("]", 1))
            {
                ctx.advance();
                ctx.advance();
                postfixes.push_back({Node_Type::PostfixKind::Array});
            }
            else if (ctx.check("@"))
            {
                ctx.advance();
                if (ctx.check("+")) ctx.advance();

                if (ctx.check("const"))
                {
                    ctx.advance();
                    postfixes.push_back({Node_Type::PostfixKind::HandleConst});
                }
                else
                {
                    postfixes.push_back({Node_Type::PostfixKind::Handle});
                }
            }
            else
                break;
        }

        auto node = std::make_unique<Node_Type>(ctx.spanFrom(start));
        node->isConst = isConst;
        node->scope = std::move(scope);
        node->dataType = std::move(dataType);
        node->templateArgs = std::move(templateArgs);
        node->postfixes = std::move(postfixes);
        return node;
    }

    // **BNF** INITLIST ::= '{' [ASSIGN | INITLIST] {',' [ASSIGN | INITLIST]} '}'
    std::unique_ptr<Node_InitList> parseInitList(ParserContext& ctx)
    {
        if (!ctx.check("{")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        auto node = std::make_unique<Node_InitList>();
        bool first = true;

        while (!ctx.isEnd() && !ctx.check("}"))
        {
            if (!first)
            {
                if (!ctx.check(",")) break;

                ctx.advance();
                if (ctx.check("}")) break;
            }

            first = false;

            if (auto inner = parseInitList(ctx))
            {
                node->items.push_back(std::move(inner));
                continue;
            }

            if (auto assign = parseAssign(ctx))
            {
                node->items.push_back(std::move(assign));
                continue;
            }

            break;
        }

        expectToken(ctx, "}");
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** SCOPE ::= ['::'] {IDENTIFIER '::'} [IDENTIFIER ['<' TYPE {',' TYPE} '>'] '::']
    std::unique_ptr<Node_Scope> parseScope(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        bool isGlobal = false;

        if (ctx.check("::"))
        {
            ctx.advance();
            isGlobal = true;
        }

        std::vector<Node_Scope::ScopePart> parts;

        while (!ctx.isEnd() && ctx.checkKind(TokenKind::Name))
        {
            if (ctx.check("::", 1))
            {
                Node_Scope::ScopePart part;
                part.identifier = ctx.consumeView();
                ctx.advance();
                parts.push_back(std::move(part));
                continue;
            }

            if (ctx.check("<", 1))
            {
                const uint32_t savedPos = ctx.save();
                auto identTok = ctx.consumeView();
                std::vector<std::unique_ptr<Node_Type>> args;
                if (parseTemplateTypeArgs(ctx, args) && ctx.check("::"))
                {
                    ctx.advance();
                    Node_Scope::ScopePart part;
                    part.identifier = identTok;
                    part.templateArgs = std::move(args);
                    parts.push_back(std::move(part));
                    continue;
                }

                ctx.rewindTo(savedPos);
            }

            break;
        }

        if (!isGlobal && parts.empty()) return nullptr;

        auto node = std::make_unique<Node_Scope>(ctx.spanFrom(start));
        node->isGlobal = isGlobal;
        node->parts = std::move(parts);
        return node;
    }

    // **BNF** DATATYPE ::= (IDENTIFIER | PRIMITIVETYPE | '?' | 'auto')
    std::unique_ptr<Node_DataType> parseDataType(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        if (ctx.checkKind(TokenKind::Name) || ctx.check("?"))
        {
            auto token = ctx.consumeView();
            auto node = std::make_unique<Node_DataType>(ctx.spanFrom(start));
            node->token = token;
            return node;
        }

        return nullptr;
    }

    // **BNF** PRIMITIVETYPE ::= 'void' | 'int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64' | 'float' | 'double' | 'bool'
    // n/a

    // **BNF** FUNCATTR ::= {'override' | 'final' | 'explicit' | 'property' | 'delete' | 'nodiscard'}
    FuncAttr parseFuncAttr(ParserContext& ctx)
    {
        FuncAttr attr;
        while (!ctx.isEnd())
        {
            str_view t = ctx.peekText();
            if (t == "override")
            {
                attr.flags |= FuncAttr::Override;
                ctx.advance();
            }
            else if (t == "final")
            {
                attr.flags |= FuncAttr::Final;
                ctx.advance();
            }
            else if (t == "explicit")
            {
                attr.flags |= FuncAttr::Explicit;
                ctx.advance();
            }
            else if (t == "property")
            {
                attr.flags |= FuncAttr::Property;
                ctx.advance();
            }
            else if (t == "delete")
            {
                attr.flags |= FuncAttr::Delete;
                ctx.advance();
            }
            else if (t == "nodiscard")
            {
                attr.flags |= FuncAttr::Nodiscard;
                ctx.advance();
            }
            else
                break;
        }

        return attr;
    }

    // **BNF** STATEMENT ::= (IF | FOR | FOREACH | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)
    std::unique_ptr<NodeBase> parseStatement(ParserContext& ctx)
    {
        if (auto n = parseIf(ctx)) return n;

        if (auto n = parseFor(ctx)) return n;

        if (auto n = parseForEach(ctx)) return n;

        if (auto n = parseWhile(ctx)) return n;

        if (auto n = parseReturn(ctx)) return n;

        if (auto n = parseStatBlock(ctx)) return n;

        if (auto n = parseBreak(ctx)) return n;

        if (auto n = parseContinue(ctx)) return n;

        if (auto n = parseDoWhile(ctx)) return n;

        if (auto n = parseSwitch(ctx)) return n;

        if (auto n = parseTry(ctx)) return n;

        if (auto n = parseExprStat(ctx)) return n;

        return nullptr;
    }

    // **BNF** SWITCH ::= 'switch' '(' ASSIGN ')' '{' {CASE} '}'
    std::unique_ptr<Node_Switch> parseSwitch(ParserContext& ctx)
    {
        if (!ctx.check("switch")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        if (!expectToken(ctx, "(")) return nullptr;

        auto assign = parseAssign(ctx);
        if (!assign) return nullptr;

        expectToken(ctx, ")");
        expectToken(ctx, "{");

        auto node = std::make_unique<Node_Switch>();
        node->expr = std::move(assign);

        while (!ctx.isEnd() && !ctx.check("}"))
        {
            auto c = parseCase(ctx);
            if (!c)
            {
                ctx.advanceError();
                continue;
            }

            node->cases.push_back(std::move(c));
        }

        expectToken(ctx, "}");
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** BREAK ::= 'break' ';'
    std::unique_ptr<Node_Break> parseBreak(ParserContext& ctx)
    {
        if (!ctx.check("break")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();
        expectToken(ctx, ";");
        return std::make_unique<Node_Break>(ctx.spanFrom(start));
    }

    // **BNF** FOR ::= 'for' '(' (VAR | EXPRSTAT) EXPRSTAT [ASSIGN {',' ASSIGN}] ')' STATEMENT
    std::unique_ptr<Node_For> parseFor(ParserContext& ctx)
    {
        if (!ctx.check("for")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        if (!expectToken(ctx, "(")) return nullptr;

        auto node = std::make_unique<Node_For>();

        if (auto v = parseVar(ctx))
            node->init = std::move(v);
        else if (auto es = parseExprStat(ctx))
            node->init = std::move(es);
        else
            return nullptr;

        node->cond = parseExprStat(ctx);

        bool first = true;
        while (!ctx.isEnd() && !ctx.check(")"))
        {
            if (!first)
            {
                if (!ctx.check(",")) break;

                ctx.advance();
            }

            first = false;
            auto assign = parseAssign(ctx);
            if (!assign) break;

            node->incs.push_back(std::move(assign));
        }

        expectToken(ctx, ")");
        node->body = parseStatement(ctx);
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** FOREACH ::= 'foreach' '(' TYPE IDENTIFIER {',' TYPE IDENTIFIER} ':' ASSIGN ')' STATEMENT
    std::unique_ptr<Node_ForEach> parseForEach(ParserContext& ctx)
    {
        if (!ctx.check("foreach")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        if (!expectToken(ctx, "(")) return nullptr;

        auto node = std::make_unique<Node_ForEach>();
        bool first = true;

        while (!ctx.isEnd() && !ctx.check(":"))
        {
            if (!first)
            {
                if (!ctx.check(",")) break;

                ctx.advance();
            }

            first = false;

            Node_ForEach::IterVar v;
            v.type = parseType(ctx);
            if (!v.type) break;

            if (ctx.checkKind(TokenKind::Name)) v.identifier = ctx.consumeView();

            node->vars.push_back(std::move(v));
        }

        expectToken(ctx, ":");
        node->range = parseAssign(ctx);
        expectToken(ctx, ")");
        node->body = parseStatement(ctx);
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** WHILE ::= 'while' '(' ASSIGN ')' STATEMENT
    std::unique_ptr<Node_While> parseWhile(ParserContext& ctx)
    {
        if (!ctx.check("while")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        if (!expectToken(ctx, "(")) return nullptr;

        auto cond = parseAssign(ctx);
        if (!cond) return nullptr;

        if (!expectToken(ctx, ")")) return nullptr;

        auto node = std::make_unique<Node_While>();
        node->cond = std::move(cond);
        node->body = parseStatement(ctx);
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** DOWHILE ::= 'do' STATEMENT 'while' '(' ASSIGN ')' ';'
    std::unique_ptr<Node_DoWhile> parseDoWhile(ParserContext& ctx)
    {
        if (!ctx.check("do")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        auto node = std::make_unique<Node_DoWhile>();
        node->body = parseStatement(ctx);
        if (!expectToken(ctx, "while")) return nullptr;

        if (!expectToken(ctx, "(")) return nullptr;

        node->cond = parseAssign(ctx);
        expectToken(ctx, ")");
        expectToken(ctx, ";");
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** IF ::= 'if' '(' ASSIGN ')' STATEMENT ['else' STATEMENT]
    std::unique_ptr<Node_If> parseIf(ParserContext& ctx)
    {
        if (!ctx.check("if")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        if (!expectToken(ctx, "(")) return nullptr;

        auto cond = parseAssign(ctx);
        if (!cond) return nullptr;

        if (!expectToken(ctx, ")")) return nullptr;

        auto node = std::make_unique<Node_If>();
        node->cond = std::move(cond);
        node->thenBranch = parseStatement(ctx);

        if (ctx.check("else"))
        {
            ctx.advance();
            node->elseBranch = parseStatement(ctx);
        }

        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** CONTINUE ::= 'continue' ';'
    std::unique_ptr<Node_Continue> parseContinue(ParserContext& ctx)
    {
        if (!ctx.check("continue")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();
        expectToken(ctx, ";");
        return std::make_unique<Node_Continue>(ctx.spanFrom(start));
    }

    // **BNF** EXPRSTAT ::= [ASSIGN] ';'
    std::unique_ptr<Node_ExprStat> parseExprStat(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();

        if (ctx.check(";"))
        {
            ctx.advance();
            return std::make_unique<Node_ExprStat>(ctx.spanFrom(start));
        }

        auto assign = parseAssign(ctx);
        if (!assign) return nullptr;

        expectToken(ctx, ";");
        auto node = std::make_unique<Node_ExprStat>(ctx.spanFrom(start));
        node->assign = std::move(assign);
        return node;
    }

    // **BNF** TRY ::= 'try' STATBLOCK 'catch' STATBLOCK
    std::unique_ptr<Node_Try> parseTry(ParserContext& ctx)
    {
        if (!ctx.check("try")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        auto node = std::make_unique<Node_Try>();
        node->tryBlock = parseStatBlock(ctx);
        if (!node->tryBlock) return nullptr;

        if (!expectToken(ctx, "catch")) return nullptr;

        node->catchBlock = parseStatBlock(ctx);
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** RETURN ::= 'return' [ASSIGN] ';'
    std::unique_ptr<Node_Return> parseReturn(ParserContext& ctx)
    {
        if (!ctx.check("return")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        auto node = std::make_unique<Node_Return>();
        if (!ctx.check(";")) node->value = parseAssign(ctx);

        expectToken(ctx, ";");
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** CASE ::= (('case' EXPR) | 'default') ':' {STATEMENT}
    std::unique_ptr<Node_Case> parseCase(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();

        auto node = std::make_unique<Node_Case>();
        if (ctx.check("case"))
        {
            ctx.advance();
            node->expr = parseExpr(ctx);
        }
        else if (ctx.check("default"))
        {
            ctx.advance();
        }
        else
        {
            return nullptr;
        }

        expectToken(ctx, ":");

        while (!ctx.isEnd() && !ctx.check("case") && !ctx.check("default") && !ctx.check("}"))
        {
            auto s = parseStatement(ctx);
            if (!s) break;

            node->statements.push_back(std::move(s));
        }

        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** EXPR ::= EXPRTERM {EXPROP EXPRTERM}
    std::unique_ptr<Node_Expr> parseExpr(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        auto first = parseExprTerm(ctx);
        if (!first) return nullptr;

        auto node = std::make_unique<Node_Expr>(ctx.spanFrom(start));
        node->first = std::move(first);

        while (!ctx.isEnd() && isExprOp(ctx.peekText()))
        {
            Node_Expr::OpTerm ot;
            ot.op = ctx.consumeView();
            ot.term = parseExprTerm(ctx);
            if (!ot.term) break;

            node->rest.push_back(std::move(ot));
        }

        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** EXPRTERM ::= ([TYPE '='] INITLIST) | ({EXPRPREOP} EXPRVALUE {EXPRPOSTOP})
    std::unique_ptr<Node_ExprTerm> parseExprTerm(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();

        if (ctx.check("{"))
        {
            if (auto il = parseInitList(ctx))
            {
                auto node = std::make_unique<Node_ExprTerm>(ctx.spanFrom(start));
                node->form = Node_ExprTerm::Form::InitListForm;
                node->initList = std::move(il);
                return node;
            }
        }

        const uint32_t savedPos = ctx.save();
        auto type = parseType(ctx);
        if (type && ctx.check("="))
        {
            ctx.advance();
            if (auto il = parseInitList(ctx))
            {
                auto node = std::make_unique<Node_ExprTerm>(ctx.spanFrom(start));
                node->form = Node_ExprTerm::Form::InitListForm;
                node->initType = std::move(type);
                node->initList = std::move(il);
                return node;
            }
        }

        ctx.rewindTo(savedPos);

        auto node = std::make_unique<Node_ExprTerm>();
        while (!ctx.isEnd() && isExprPreOp(ctx.peekText()))
        {
            node->preOps.push_back(ctx.consumeView());
        }

        node->exprValue = parseExprValue(ctx);
        if (!node->exprValue)
        {
            if (node->preOps.empty()) return nullptr;

            ctx.rewindTo(start);
            return nullptr;
        }

        while (!ctx.isEnd())
        {
            auto post = parseExprPostOp(ctx);
            if (!post) break;

            node->postOps.push_back(std::move(post));
        }

        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** EXPRVALUE ::= 'void' | CONSTRUCTORCALL | FUNCCALL | VARACCESS | CAST | LITERAL | '(' ASSIGN ')' | LAMBDA
    std::unique_ptr<NodeBase> parseExprValue(ParserContext& ctx)
    {
        if (ctx.check("void"))
        {
            const uint32_t start = ctx.save();
            ctx.advance();
            auto node = std::make_unique<Node_Literal>(ctx.spanFrom(start));
            return node;
        }

        if (ctx.check("("))
        {
            ctx.advance();
            auto inner = parseAssign(ctx);
            expectToken(ctx, ")");
            return inner;
        }

        if (canParseLambda(ctx)) return parseLambda(ctx);

        if (auto n = parseCast(ctx)) return n;

        if (auto n = parseLiteral(ctx)) return n;

        const uint32_t savedPos = ctx.save();
        if (auto n = parseConstructorCall(ctx)) return n;

        ctx.rewindTo(savedPos);

        if (auto n = parseFuncCall(ctx)) return n;

        ctx.rewindTo(savedPos);

        if (auto n = parseVarAccess(ctx)) return n;

        return nullptr;
    }

    // **BNF** CONSTRUCTORCALL ::= TYPE ARGLIST
    std::unique_ptr<Node_ConstructorCall> parseConstructorCall(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        auto type = parseType(ctx);
        if (!type) return nullptr;

        auto args = parseArgList(ctx);
        if (!args) return nullptr;

        auto node = std::make_unique<Node_ConstructorCall>(ctx.spanFrom(start));
        node->type = std::move(type);
        node->args = std::move(args);
        return node;
    }

    // **BNF** EXPRPREOP ::= '-' | '+' | '!' | '++' | '--' | '~' | '@'
    // n/a

    // **BNF** EXPRPOSTOP ::= ('.' (FUNCCALL | IDENTIFIER)) | ('[' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ']') | ARGLIST | '++' | '--'
    std::unique_ptr<Node_ExprPostOp> parseExprPostOp(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        auto node = std::make_unique<Node_ExprPostOp>();

        if (ctx.check("."))
        {
            ctx.advance();
            node->opKind = Node_ExprPostOp::Kind::Member;
            if (auto fc = parseFuncCall(ctx))
                node->dotAccess = std::move(fc);
            else if (ctx.checkKind(TokenKind::Name))
                node->dotIdentifier = ctx.consumeView();
            else
                return nullptr;
        }
        else if (ctx.check("["))
        {
            ctx.advance();
            node->opKind = Node_ExprPostOp::Kind::Subscript;
            bool first = true;
            while (!ctx.isEnd() && !ctx.check("]"))
            {
                if (!first)
                {
                    if (!ctx.check(",")) break;

                    ctx.advance();
                }

                first = false;

                Node_ExprPostOp::IndexArg arg;
                if (ctx.checkKind(TokenKind::Name) && ctx.check(":", 1))
                {
                    arg.name = ctx.consumeView();
                    ctx.advance();
                }

                arg.value = parseAssign(ctx);
                node->indexArgs.push_back(std::move(arg));
            }

            expectToken(ctx, "]");
        }
        else if (auto al = parseArgList(ctx))
        {
            node->opKind = Node_ExprPostOp::Kind::Call;
            node->callArgs = std::move(al);
        }
        else if (ctx.check("++"))
        {
            node->opKind = Node_ExprPostOp::Kind::Increment;
            ctx.advance();
        }
        else if (ctx.check("--"))
        {
            node->opKind = Node_ExprPostOp::Kind::Decrement;
            ctx.advance();
        }
        else
            return nullptr;

        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** CAST ::= 'cast' '<' TYPE '>' '(' ASSIGN ')'
    std::unique_ptr<Node_Cast> parseCast(ParserContext& ctx)
    {
        if (!ctx.check("cast")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        if (!expectToken(ctx, "<")) return nullptr;

        auto type = parseType(ctx);
        expectToken(ctx, ">");
        expectToken(ctx, "(");
        auto expr = parseAssign(ctx);
        expectToken(ctx, ")");

        auto node = std::make_unique<Node_Cast>(ctx.spanFrom(start));
        node->type = std::move(type);
        node->expr = std::move(expr);
        return node;
    }

    // **BNF** LAMBDA ::= 'function' '(' [LAMBDAPARAM {',' LAMBDAPARAM}] ')' STATBLOCK
    std::unique_ptr<Node_Lambda> parseLambda(ParserContext& ctx)
    {
        if (!ctx.check("function")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        expectToken(ctx, "(");
        auto node = std::make_unique<Node_Lambda>();

        bool first = true;
        while (!ctx.isEnd() && !ctx.check(")"))
        {
            if (!first)
            {
                if (!ctx.check(",")) break;

                ctx.advance();
            }

            first = false;

            auto p = parseLambdaParam(ctx);
            if (!p) break;

            node->params.push_back(std::move(p));
        }

        expectToken(ctx, ")");
        node->body = parseStatBlock(ctx);
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** LAMBDAPARAM ::= [TYPE TYPEMODIFIER] [IDENTIFIER]
    std::unique_ptr<Node_LambdaParam> parseLambdaParam(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        auto type = parseType(ctx);
        auto mod = parseTypeModifier(ctx);

        TokenView ident;
        if (ctx.checkKind(TokenKind::Name)) ident = ctx.consumeView();

        auto node = std::make_unique<Node_LambdaParam>(ctx.spanFrom(start));
        node->type = std::move(type);
        node->typeModifier = mod;
        node->identifier = ident;
        return node;
    }

    // **BNF** LITERAL ::= NUMBER | STRING | BITS | 'true' | 'false' | 'null'
    std::unique_ptr<Node_Literal> parseLiteral(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        if (ctx.checkKind(TokenKind::Number) || ctx.checkKind(TokenKind::String) ||
            ctx.checkKind(TokenKind::Bits) || ctx.check("true") ||

            ctx.check("false") || ctx.check("null"))
        {
            auto token = ctx.consumeView();
            auto node = std::make_unique<Node_Literal>(ctx.spanFrom(start));
            node->token = token;
            return node;
        }
        return nullptr;
    }

    // **BNF** FUNCCALL ::= SCOPE IDENTIFIER ['<' TYPE {',' TYPE} '>'] ARGLIST
    std::unique_ptr<Node_FuncCall> parseFuncCall(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        auto scope = parseScope(ctx);
        if (!ctx.checkKind(TokenKind::Name)) return nullptr;

        auto ident = ctx.consumeView();

        std::vector<std::unique_ptr<Node_Type>> templateArgs;
        parseTemplateTypeArgs(ctx, templateArgs);

        auto args = parseArgList(ctx);
        if (!args) return nullptr;

        auto node = std::make_unique<Node_FuncCall>(ctx.spanFrom(start));
        node->scope = std::move(scope);
        node->identifier = ident;
        node->templateArgs = std::move(templateArgs);
        node->args = std::move(args);
        return node;
    }

    // **BNF** VARACCESS ::= SCOPE IDENTIFIER
    std::unique_ptr<Node_VarAccess> parseVarAccess(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        auto scope = parseScope(ctx);
        if (!ctx.checkKind(TokenKind::Name)) return nullptr;

        auto ident = ctx.consumeView();

        auto node = std::make_unique<Node_VarAccess>(ctx.spanFrom(start));
        node->scope = std::move(scope);
        node->identifier = ident;
        return node;
    }

    // **BNF** ARGLIST ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'
    std::unique_ptr<Node_ArgList> parseArgList(ParserContext& ctx)
    {
        if (!ctx.check("(")) return nullptr;

        const uint32_t start = ctx.save();
        ctx.advance();

        auto node = std::make_unique<Node_ArgList>();
        bool first = true;

        while (!ctx.isEnd() && !ctx.check(")"))
        {
            if (!first)
            {
                if (!ctx.check(",")) break;

                ctx.advance();
            }

            first = false;

            Node_ArgList::Arg arg;
            if (ctx.checkKind(TokenKind::Name) && ctx.check(":", 1))
            {
                arg.name = ctx.consumeView();
                ctx.advance();
            }

            arg.value = parseAssign(ctx);
            node->args.push_back(std::move(arg));
        }

        expectToken(ctx, ")");
        node->span = ctx.spanFrom(start);
        return node;
    }

    // **BNF** ASSIGN ::= CONDITION [ ASSIGNOP ASSIGN ]
    std::unique_ptr<Node_Assign> parseAssign(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        auto cond = parseCondition(ctx);
        if (!cond) return nullptr;

        if (!ctx.isEnd() && isAssignOp(ctx.peekText()))
        {
            auto op = ctx.consumeView();
            auto rhs = parseAssign(ctx);
            auto node = std::make_unique<Node_Assign>(ctx.spanFrom(start));
            node->cond = std::move(cond);
            node->op = op;
            node->rhs = std::move(rhs);
            return node;
        }

        auto node = std::make_unique<Node_Assign>(ctx.spanFrom(start));
        node->cond = std::move(cond);
        return node;
    }

    // **BNF** CONDITION ::= EXPR ['?' ASSIGN ':' ASSIGN]
    std::unique_ptr<Node_Condition> parseCondition(ParserContext& ctx)
    {
        const uint32_t start = ctx.save();
        auto expr = parseExpr(ctx);
        if (!expr) return nullptr;

        if (ctx.check("?"))
        {
            ctx.advance();
            auto thenExpr = parseAssign(ctx);
            expectToken(ctx, ":");
            auto elseExpr = parseAssign(ctx);
            auto node = std::make_unique<Node_Condition>(ctx.spanFrom(start));
            node->expr = std::move(expr);
            node->thenExpr = std::move(thenExpr);
            node->elseExpr = std::move(elseExpr);
            return node;
        }

        auto node = std::make_unique<Node_Condition>(ctx.spanFrom(start));
        node->expr = std::move(expr);
        return node;
    }
} // namespace

namespace light_angel
{
    ParseResult Parse(str_view source)
    {
        ParserContext ctx(source);
        auto script = parseScript(ctx);
        return ParseResult{std::move(script), ctx.errorCount};
    }
} // namespace light_angel
