#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "token.h"

namespace light_angel
{
    enum class AccessModifier
    {
        Private,
        Protected,
    };

    struct EntityAttribute
    {
        uint8_t flags = 0;

        static constexpr uint8_t Shared = 1 << 0;
        static constexpr uint8_t External = 1 << 1;
        static constexpr uint8_t Abstract = 1 << 2;
        static constexpr uint8_t Final = 1 << 3;
        static constexpr uint8_t Mixin = 1 << 4; // CLASS only

        bool has(uint8_t flag) const
        {
            return (flags & flag) != 0;
        }
    };

    // **BNF** FUNCATTR ::= {'override' | 'final' | 'explicit' | 'property' | 'delete' | 'nodiscard'}
    struct FuncAttr
    {
        uint8_t flags = 0;

        static constexpr uint8_t Override = 1 << 0;
        static constexpr uint8_t Final = 1 << 1;
        static constexpr uint8_t Explicit = 1 << 2;
        static constexpr uint8_t Property = 1 << 3;
        static constexpr uint8_t Delete = 1 << 4;
        static constexpr uint8_t Nodiscard = 1 << 5;

        bool has(uint8_t flag) const
        {
            return (flags & flag) != 0;
        }
    };

    // **BNF** TYPEMODIFIER ::= ['&' ['in' | 'out' | 'inout'] ['+'] ['if_handle_then_const']]
    enum class RefDirection
    {
        None,
        In,
        Out,
        InOut
    };

    struct TypeModifier
    {
        bool isRef = false;
        RefDirection refDir = RefDirection::None;
        bool plus = false;
        bool ifHandleThenConst = false;
    };

    enum class NodeKind : uint8_t
    {
        Script, // list node
        Namespace,
        Using,
        Enum,
        Class,
        TypeDef,
        Func,
        ListPattern,
        ListEntry,
        Interface,
        Var,
        Import,
        FuncDef,
        VirtualProp,
        InterfaceMethod,
        StatBlock,
        ParamList,
        Parameter,
        Type,
        InitList,
        Scope,
        DataType,
        Statement, // variant node
        Switch,
        Break,
        For,
        ForEach,
        While,
        DoWhile,
        If,
        Continue,
        ExprStat,
        Try,
        Return,
        Case,
        Expr,
        ExprTerm,
        ExprValue, // variant node
        ConstructorCall,
        ExprPostOp,
        Cast,
        Lambda,
        LambdaParam,
        Literal,
        FuncCall,
        VarAccess,
        ArgList,
        Assign,
        Condition,
    };

    // forward declarations
    struct Node_Script;
    struct Node_Namespace;
    struct Node_Using;
    struct Node_Enum;
    struct Node_Class;
    struct Node_TypeDef;
    struct Node_Func;
    struct Node_ListPattern;
    struct Node_ListEntry;
    struct Node_Interface;
    struct Node_Var;
    struct Node_Import;
    struct Node_FuncDef;
    struct Node_VirtualProp;
    struct Node_InterfaceMethod;
    struct Node_StatBlock;
    struct Node_ParamList;
    struct Node_Parameter;
    struct Node_Type;
    struct Node_InitList;
    struct Node_Scope;
    struct Node_DataType;
    struct Node_Statement;
    struct Node_Switch;
    struct Node_Break;
    struct Node_For;
    struct Node_ForEach;
    struct Node_While;
    struct Node_DoWhile;
    struct Node_If;
    struct Node_Continue;
    struct Node_ExprStat;
    struct Node_Try;
    struct Node_Return;
    struct Node_Case;
    struct Node_Expr;
    struct Node_ExprTerm;
    struct Node_ConstructorCall;
    struct Node_ExprPostOp;
    struct Node_Cast;
    struct Node_Lambda;
    struct Node_LambdaParam;
    struct Node_Literal;
    struct Node_FuncCall;
    struct Node_VarAccess;
    struct Node_ArgList;
    struct Node_Assign;
    struct Node_Condition;

    // -----------------------------------------------

    struct NodeBase
    {
        explicit NodeBase(NodeKind kind, SourceSpan span = {})
            : kind(kind), span(span)
        {
        }

        virtual ~NodeBase() = default;

        NodeKind kind;
        SourceSpan span;
    };

    // **BNF** SCRIPT ::= {IMPORT | ENUM | TYPEDEF | CLASS | INTERFACE | FUNCDEF | VIRTUALPROP | VAR | FUNC | NAMESPACE | USING | ';'}
    struct Node_Script final : NodeBase
    {
        explicit Node_Script(SourceSpan span = {})
            : NodeBase(NodeKind::Script, span)
        {
        }

        std::vector<std::unique_ptr<NodeBase>> children;
    };

    // **BNF** NAMESPACE ::= 'namespace' IDENTIFIER {'::' IDENTIFIER} '{' SCRIPT '}'
    struct Node_Namespace final : NodeBase
    {
        explicit Node_Namespace(SourceSpan span = {})
            : NodeBase(NodeKind::Namespace, span)
        {
        }

        std::vector<TokenView> identifiers;
        std::unique_ptr<Node_Script> script;
    };

    // **BNF** USING ::= 'using' 'namespace' IDENTIFIER {'::' IDENTIFIER} ';'
    struct Node_Using final : NodeBase
    {
        explicit Node_Using(SourceSpan span = {}) : NodeBase(NodeKind::Using, span) {}

        std::vector<TokenView> identifiers;
    };

    // **BNF** ENUM ::= {'shared' | 'external'} 'enum' IDENTIFIER [ ':' ('int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64') ] (';' | ('{' IDENTIFIER ['=' EXPR] {',' IDENTIFIER ['=' EXPR]} '}'))
    struct Node_Enum final : NodeBase
    {
        struct EnumValue
        {
            TokenView name;
            std::unique_ptr<Node_Expr> expr; // null = no value
        };

        explicit Node_Enum(SourceSpan span = {}) : NodeBase(NodeKind::Enum, span) {}

        EntityAttribute attr;
        TokenView identifier;
        TokenView baseType; // empty = not specified
        bool isForwardDecl = false;
        std::vector<EnumValue> values;
    };

    // **BNF** CLASS ::= ['mixin'] {'shared' | 'abstract' | 'final' | 'external'} 'class' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | FUNC | VAR | FUNCDEF} '}'))
    struct Node_Class final : NodeBase
    {
        struct BaseClass
        {
            std::unique_ptr<Node_Scope> scope;
            TokenView identifier;
        };

        explicit Node_Class(SourceSpan span = {}) : NodeBase(NodeKind::Class, span) {}

        EntityAttribute attr;
        TokenView identifier;
        bool isForwardDecl = false;
        std::vector<BaseClass> bases;
        std::vector<std::unique_ptr<NodeBase>> members;
    };

    // **BNF** TYPEDEF ::= 'typedef' PRIMITIVETYPE IDENTIFIER ';'
    struct Node_TypeDef final : NodeBase
    {
        explicit Node_TypeDef(SourceSpan span = {}) : NodeBase(NodeKind::TypeDef, span) {}

        TokenView primitiveType;
        TokenView identifier;
    };

    // **BNF** FUNC ::= {'shared' | 'external'} ['private' | 'protected'] [((TYPE ['&']) | '~')] IDENTIFIER ['<' TYPE {',' TYPE} '>'] PARAMLIST [LISTPATTERN] ['const'] FUNCATTR (';' | STATBLOCK)
    struct Node_Func final : NodeBase
    {
        explicit Node_Func(SourceSpan span = {}) : NodeBase(NodeKind::Func, span) {}

        EntityAttribute attr;
        std::optional<AccessModifier> access;
        bool isDestructor = false;
        std::unique_ptr<Node_Type> returnType; // null = destructor
        bool isReturnRef = false;
        TokenView identifier;
        std::vector<std::unique_ptr<Node_Type>> templateParams;
        std::unique_ptr<Node_ParamList> params;
        std::unique_ptr<Node_ListPattern> listPattern;
        bool isConst = false;
        FuncAttr funcAttr;
        std::unique_ptr<Node_StatBlock> body; // null = ';'
    };

    // **BNF** LISTPATTERN ::= '{' LISTENTRY {',' LISTENTRY} '}'
    struct Node_ListPattern final : NodeBase
    {
        explicit Node_ListPattern(SourceSpan span = {}) : NodeBase(NodeKind::ListPattern, span) {}

        std::vector<std::unique_ptr<Node_ListEntry>> entries;
    };

    // **BNF** LISTENTRY ::= (('repeat' | 'repeat_same') (('{' LISTENTRY '}') | TYPE)) | (TYPE {',' TYPE})
    struct Node_ListEntry final : NodeBase
    {
        enum class Kind
        {
            RepeatSub, // 'repeat'|'repeat_same' '{' LISTENTRY '}'
            RepeatType, // 'repeat'|'repeat_same' TYPE
            TypeList, // TYPE {',' TYPE}
        };

        explicit Node_ListEntry(SourceSpan span = {}) : NodeBase(NodeKind::ListEntry, span) {}

        Kind entryKind = Kind::TypeList;
        bool isRepeatSame = false;
        std::unique_ptr<Node_ListEntry> subEntry; // Kind::RepeatSub
        std::vector<std::unique_ptr<Node_Type>> types; // Kind::RepeatType / Kind::TypeList
    };

    // **BNF** INTERFACE ::= {'external' | 'shared'} 'interface' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | INTERFACEMETHOD} '}'))
    struct Node_Interface final : NodeBase
    {
        struct BaseInterface
        {
            std::unique_ptr<Node_Scope> scope;
            TokenView identifier;
        };

        explicit Node_Interface(SourceSpan span = {}) : NodeBase(NodeKind::Interface, span) {}

        EntityAttribute attr;
        TokenView identifier;
        bool isForwardDecl = false;
        std::vector<BaseInterface> bases;
        std::vector<std::unique_ptr<NodeBase>> members;
    };

    // **BNF** VAR ::= ['private' | 'protected'] TYPE IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST] {',' IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST]} ';'
    struct Node_Var final : NodeBase
    {
        struct VarDecl
        {
            TokenView identifier;
            std::unique_ptr<NodeBase> init; // Node_InitList | Node_Assign | Node_ArgList | null
        };

        explicit Node_Var(SourceSpan span = {}) : NodeBase(NodeKind::Var, span) {}

        std::optional<AccessModifier> access;
        std::unique_ptr<Node_Type> type;
        std::vector<VarDecl> decls;
    };

    // **BNF** IMPORT ::= 'import' TYPE ['&'] IDENTIFIER PARAMLIST FUNCATTR 'from' STRING ';'
    struct Node_Import final : NodeBase
    {
        explicit Node_Import(SourceSpan span = {}) : NodeBase(NodeKind::Import, span) {}

        std::unique_ptr<Node_Type> type;
        bool isRef = false;
        TokenView identifier;
        std::unique_ptr<Node_ParamList> params;
        FuncAttr funcAttr;
        TokenView fromString;
    };

    // **BNF** FUNCDEF ::= {'external' | 'shared'} 'funcdef' TYPE ['&'] IDENTIFIER PARAMLIST ';'
    struct Node_FuncDef final : NodeBase
    {
        explicit Node_FuncDef(SourceSpan span = {}) : NodeBase(NodeKind::FuncDef, span) {}

        EntityAttribute attr;
        std::unique_ptr<Node_Type> returnType;
        bool isRef = false;
        TokenView identifier;
        std::unique_ptr<Node_ParamList> params;
    };

    // **BNF** VIRTUALPROP ::= ['private' | 'protected'] TYPE ['&'] IDENTIFIER '{' {('get' | 'set') ['const'] FUNCATTR (STATBLOCK | ';')} '}'
    struct Node_VirtualProp final : NodeBase
    {
        struct Accessor
        {
            bool isGet = true;
            bool isConst = false;
            FuncAttr attr;
            std::unique_ptr<Node_StatBlock> body; // null = ';'
        };

        explicit Node_VirtualProp(SourceSpan span = {}) : NodeBase(NodeKind::VirtualProp, span) {}

        std::optional<AccessModifier> access;
        std::unique_ptr<Node_Type> type;
        bool isRef = false;
        TokenView identifier;
        std::vector<Accessor> accessors;
    };

    // **BNF** INTERFACEMETHOD ::= TYPE ['&'] IDENTIFIER PARAMLIST ['const'] FUNCATTR ';'
    struct Node_InterfaceMethod final : NodeBase
    {
        explicit Node_InterfaceMethod(SourceSpan span = {}) : NodeBase(NodeKind::InterfaceMethod, span) {}

        std::unique_ptr<Node_Type> type;
        bool isRef = false;
        TokenView identifier;
        std::unique_ptr<Node_ParamList> params;
        bool isConst = false;
        FuncAttr funcAttr;
    };

    // **BNF** STATBLOCK ::= '{' {VAR | STATEMENT | USING} '}'
    struct Node_StatBlock final : NodeBase
    {
        explicit Node_StatBlock(SourceSpan span = {}) : NodeBase(NodeKind::StatBlock, span) {}

        std::vector<std::unique_ptr<NodeBase>> statements;
    };

    // **BNF** PARAMLIST ::= '(' ['void' | (PARAMETER {',' PARAMETER})] ')'
    struct Node_ParamList final : NodeBase
    {
        explicit Node_ParamList(SourceSpan span = {}) : NodeBase(NodeKind::ParamList, span) {}

        bool isVoid = false;
        std::vector<std::unique_ptr<Node_Parameter>> params;
    };

    // **BNF** PARAMETER ::= TYPE TYPEMODIFIER [IDENTIFIER] ['...' | ('=' (EXPR | 'void'))]
    struct Node_Parameter final : NodeBase
    {
        explicit Node_Parameter(SourceSpan span = {}) : NodeBase(NodeKind::Parameter, span) {}

        std::unique_ptr<Node_Type> type;
        TypeModifier typeModifier;
        TokenView identifier;
        bool isVariadic = false;
        bool hasVoidDefault = false;
        std::unique_ptr<Node_Expr> defaultExpr; // null = no default value
    };

    // **BNF** TYPEMODIFIER ::= ['&' ['in' | 'out' | 'inout'] ['+'] ['if_handle_then_const']]
    // n/a

    // **BNF** TYPE ::= ['const'] SCOPE DATATYPE ['<' TYPE {',' TYPE} '>'] { ('[' ']') | ('@' ['const']) }
    struct Node_Type final : NodeBase
    {
        // '[' ']' → Array, '@' → Handle, '@' 'const' → HandleConst
        enum class PostfixKind
        {
            Array,
            Handle,
            HandleConst
        };

        struct Postfix
        {
            PostfixKind kind;
        };

        explicit Node_Type(SourceSpan span = {}) : NodeBase(NodeKind::Type, span) {}

        bool isConst = false;
        std::unique_ptr<Node_Scope> scope;
        std::unique_ptr<Node_DataType> dataType;
        std::vector<std::unique_ptr<Node_Type>> templateArgs;
        std::vector<Postfix> postfixes;
    };

    // **BNF** INITLIST ::= '{' [ASSIGN | INITLIST] {',' [ASSIGN | INITLIST]} '}'
    struct Node_InitList final : NodeBase
    {
        explicit Node_InitList(SourceSpan span = {}) : NodeBase(NodeKind::InitList, span) {}

        std::vector<std::unique_ptr<NodeBase>> items; // Node_Assign | Node_InitList
    };

    // **BNF** SCOPE ::= ['::'] {IDENTIFIER '::'} [IDENTIFIER ['<' TYPE {',' TYPE} '>'] '::']
    struct Node_Scope final : NodeBase
    {
        // Each identifier segment; only the last qualified segment may have template args
        struct ScopePart
        {
            TokenView identifier;
            std::vector<std::unique_ptr<Node_Type>> templateArgs;
        };

        explicit Node_Scope(SourceSpan span = {}) : NodeBase(NodeKind::Scope, span) {}

        bool isGlobal = false;
        std::vector<ScopePart> parts;
    };

    // **BNF** DATATYPE ::= (IDENTIFIER | PRIMITIVETYPE | '?' | 'auto')
    struct Node_DataType final : NodeBase
    {
        explicit Node_DataType(SourceSpan span = {}) : NodeBase(NodeKind::DataType, span) {}

        TokenView token;
    };

    // **BNF** PRIMITIVETYPE ::= 'void' | 'int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64' | 'float' | 'double' | 'bool'
    // n/a

    // **BNF** FUNCATTR ::= {'override' | 'final' | 'explicit' | 'property' | 'delete' | 'nodiscard'}
    // n/a

    // **BNF** STATEMENT ::= (IF | FOR | FOREACH | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)
    struct Node_Statement final : NodeBase
    {
        explicit Node_Statement(SourceSpan span = {}) : NodeBase(NodeKind::Statement, span) {}

        std::unique_ptr<NodeBase> child;
    };

    // **BNF** SWITCH ::= 'switch' '(' ASSIGN ')' '{' {CASE} '}'
    struct Node_Switch final : NodeBase
    {
        explicit Node_Switch(SourceSpan span = {}) : NodeBase(NodeKind::Switch, span) {}

        std::unique_ptr<Node_Assign> expr;
        std::vector<std::unique_ptr<Node_Case>> cases;
    };

    // **BNF** BREAK ::= 'break' ';'
    struct Node_Break final : NodeBase
    {
        explicit Node_Break(SourceSpan span = {}) : NodeBase(NodeKind::Break, span) {}
    };

    // **BNF** FOR ::= 'for' '(' (VAR | EXPRSTAT) EXPRSTAT [ASSIGN {',' ASSIGN}] ')' STATEMENT
    struct Node_For final : NodeBase
    {
        explicit Node_For(SourceSpan span = {}) : NodeBase(NodeKind::For, span) {}

        std::unique_ptr<NodeBase> init; // Node_Var or Node_ExprStat
        std::unique_ptr<Node_ExprStat> cond;
        std::vector<std::unique_ptr<Node_Assign>> incs;
        std::unique_ptr<NodeBase> body;
    };

    // **BNF** FOREACH ::= 'foreach' '(' TYPE IDENTIFIER {',' TYPE IDENTIFIER} ':' ASSIGN ')' STATEMENT
    struct Node_ForEach final : NodeBase
    {
        struct IterVar
        {
            std::unique_ptr<Node_Type> type;
            TokenView identifier;
        };

        explicit Node_ForEach(SourceSpan span = {}) : NodeBase(NodeKind::ForEach, span) {}

        std::vector<IterVar> vars;
        std::unique_ptr<Node_Assign> range;
        std::unique_ptr<NodeBase> body;
    };

    // **BNF** WHILE ::= 'while' '(' ASSIGN ')' STATEMENT
    struct Node_While final : NodeBase
    {
        explicit Node_While(SourceSpan span = {}) : NodeBase(NodeKind::While, span) {}

        std::unique_ptr<Node_Assign> cond;
        std::unique_ptr<NodeBase> body;
    };

    // **BNF** DOWHILE ::= 'do' STATEMENT 'while' '(' ASSIGN ')' ';'
    struct Node_DoWhile final : NodeBase
    {
        explicit Node_DoWhile(SourceSpan span = {}) : NodeBase(NodeKind::DoWhile, span) {}

        std::unique_ptr<NodeBase> body;
        std::unique_ptr<Node_Assign> cond;
    };

    // **BNF** IF ::= 'if' '(' ASSIGN ')' STATEMENT ['else' STATEMENT]
    struct Node_If final : NodeBase
    {
        explicit Node_If(SourceSpan span = {}) : NodeBase(NodeKind::If, span) {}

        std::unique_ptr<Node_Assign> cond;
        std::unique_ptr<NodeBase> thenBranch;
        std::unique_ptr<NodeBase> elseBranch; // null = no else
    };

    // **BNF** CONTINUE ::= 'continue' ';'
    struct Node_Continue final : NodeBase
    {
        explicit Node_Continue(SourceSpan span = {}) : NodeBase(NodeKind::Continue, span) {}
    };

    // **BNF** EXPRSTAT ::= [ASSIGN] ';'
    struct Node_ExprStat final : NodeBase
    {
        explicit Node_ExprStat(SourceSpan span = {}) : NodeBase(NodeKind::ExprStat, span) {}

        std::unique_ptr<Node_Assign> assign; // null = empty statement
    };

    // **BNF** TRY ::= 'try' STATBLOCK 'catch' STATBLOCK
    struct Node_Try final : NodeBase
    {
        explicit Node_Try(SourceSpan span = {}) : NodeBase(NodeKind::Try, span) {}

        std::unique_ptr<Node_StatBlock> tryBlock;
        std::unique_ptr<Node_StatBlock> catchBlock;
    };

    // **BNF** RETURN ::= 'return' [ASSIGN] ';'
    struct Node_Return final : NodeBase
    {
        explicit Node_Return(SourceSpan span = {}) : NodeBase(NodeKind::Return, span) {}

        std::unique_ptr<Node_Assign> value; // null = void return
    };

    // **BNF** CASE ::= (('case' EXPR) | 'default') ':' {STATEMENT}
    struct Node_Case final : NodeBase
    {
        explicit Node_Case(SourceSpan span = {}) : NodeBase(NodeKind::Case, span) {}

        std::unique_ptr<Node_Expr> expr; // null = default
        std::vector<std::unique_ptr<NodeBase>> statements;
    };

    // **BNF** EXPR ::= EXPRTERM {EXPROP EXPRTERM}
    struct Node_Expr final : NodeBase
    {
        struct OpTerm
        {
            TokenView op; // MATHOP | COMPOP | LOGICOP | BITOP
            std::unique_ptr<Node_ExprTerm> term;
        };

        explicit Node_Expr(SourceSpan span = {}) : NodeBase(NodeKind::Expr, span) {}

        std::unique_ptr<Node_ExprTerm> first;
        std::vector<OpTerm> rest;
    };

    // **BNF** EXPRTERM ::= ([TYPE '='] INITLIST) | ({EXPRPREOP} EXPRVALUE {EXPRPOSTOP})
    struct Node_ExprTerm final : NodeBase
    {
        enum class Form
        {
            InitListForm, // [TYPE '='] INITLIST
            ExprValueForm, // {EXPRPREOP} EXPRVALUE {EXPRPOSTOP}
        };

        explicit Node_ExprTerm(SourceSpan span = {}) : NodeBase(NodeKind::ExprTerm, span) {}

        Form form = Form::ExprValueForm;

        std::unique_ptr<Node_Type> initType; // null = bare INITLIST
        std::unique_ptr<Node_InitList> initList;

        std::vector<TokenView> preOps; // '-' | '+' | '!' | '++' | '--' | '~' | '@'
        std::unique_ptr<NodeBase> exprValue;
        std::vector<std::unique_ptr<Node_ExprPostOp>> postOps;
    };

    // **BNF** EXPRVALUE ::= 'void' | CONSTRUCTORCALL | FUNCCALL | VARACCESS | CAST | LITERAL | '(' ASSIGN ')' | LAMBDA
    struct Node_ExprValue final : NodeBase
    {
        explicit Node_ExprValue(SourceSpan span = {}) : NodeBase(NodeKind::ExprValue, span) {}

        std::unique_ptr<NodeBase> value;
    };

    // **BNF** CONSTRUCTORCALL ::= TYPE ARGLIST
    struct Node_ConstructorCall final : NodeBase
    {
        explicit Node_ConstructorCall(SourceSpan span = {}) : NodeBase(NodeKind::ConstructorCall, span) {}

        std::unique_ptr<Node_Type> type;
        std::unique_ptr<Node_ArgList> args;
    };

    // **BNF** EXPRPREOP ::= '-' | '+' | '!' | '++' | '--' | '~' | '@'
    // n/a

    // **BNF** EXPRPOSTOP ::= ('.' (FUNCCALL | IDENTIFIER)) | ('[' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ']') | ARGLIST | '++' | '--'
    struct Node_ExprPostOp final : NodeBase
    {
        enum class Kind
        {
            Member, // '.' (FUNCCALL | IDENTIFIER)
            Subscript, // '[' ... ']'
            Call, // ARGLIST
            Increment, // '++'
            Decrement, // '--'
        };

        struct IndexArg
        {
            TokenView name; // empty = positional argument
            std::unique_ptr<Node_Assign> value;
        };

        explicit Node_ExprPostOp(SourceSpan span = {}) : NodeBase(NodeKind::ExprPostOp, span) {}

        Kind opKind = Kind::Member;
        // Kind::Dot — dotAccess holds Node_FuncCall for FUNCCALL, dotIdentifier holds the name for IDENTIFIER
        std::unique_ptr<NodeBase> dotAccess;
        TokenView dotIdentifier;
        std::vector<IndexArg> indexArgs; // Kind::Subscript
        std::unique_ptr<Node_ArgList> callArgs; // Kind::Call
    };

    // **BNF** CAST ::= 'cast' '<' TYPE '>' '(' ASSIGN ')'
    struct Node_Cast final : NodeBase
    {
        explicit Node_Cast(SourceSpan span = {}) : NodeBase(NodeKind::Cast, span) {}

        std::unique_ptr<Node_Type> type;
        std::unique_ptr<Node_Assign> expr;
    };

    // **BNF** LAMBDA ::= 'function' '(' [LAMBDAPARAM {',' LAMBDAPARAM}] ')' STATBLOCK
    struct Node_Lambda final : NodeBase
    {
        explicit Node_Lambda(SourceSpan span = {}) : NodeBase(NodeKind::Lambda, span) {}

        std::vector<std::unique_ptr<Node_LambdaParam>> params;
        std::unique_ptr<Node_StatBlock> body;
    };

    // **BNF** LAMBDAPARAM ::= [TYPE TYPEMODIFIER] [IDENTIFIER]
    struct Node_LambdaParam final : NodeBase
    {
        explicit Node_LambdaParam(SourceSpan span = {}) : NodeBase(NodeKind::LambdaParam, span) {}

        std::unique_ptr<Node_Type> type; // null = type omitted
        TypeModifier typeModifier;
        TokenView identifier;
    };

    // **BNF** LITERAL ::= NUMBER | STRING | BITS | 'true' | 'false' | 'null'
    struct Node_Literal final : NodeBase
    {
        explicit Node_Literal(SourceSpan span = {}) : NodeBase(NodeKind::Literal, span) {}

        TokenView token;
    };

    // **BNF** FUNCCALL ::= SCOPE IDENTIFIER ['<' TYPE {',' TYPE} '>'] ARGLIST
    struct Node_FuncCall final : NodeBase
    {
        explicit Node_FuncCall(SourceSpan span = {}) : NodeBase(NodeKind::FuncCall, span) {}

        std::unique_ptr<Node_Scope> scope;
        TokenView identifier;
        std::vector<std::unique_ptr<Node_Type>> templateArgs;
        std::unique_ptr<Node_ArgList> args;
    };

    // **BNF** VARACCESS ::= SCOPE IDENTIFIER
    struct Node_VarAccess final : NodeBase
    {
        explicit Node_VarAccess(SourceSpan span = {}) : NodeBase(NodeKind::VarAccess, span) {}

        std::unique_ptr<Node_Scope> scope;
        TokenView identifier;
    };

    // **BNF** ARGLIST ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'
    struct Node_ArgList final : NodeBase
    {
        struct Arg
        {
            TokenView name; // empty = positional argument
            std::unique_ptr<Node_Assign> value;
        };

        explicit Node_ArgList(SourceSpan span = {}) : NodeBase(NodeKind::ArgList, span) {}

        std::vector<Arg> args;
    };

    // **BNF** ASSIGN ::= CONDITION [ ASSIGNOP ASSIGN ]
    struct Node_Assign final : NodeBase
    {
        explicit Node_Assign(SourceSpan span = {}) : NodeBase(NodeKind::Assign, span) {}

        std::unique_ptr<Node_Condition> cond;
        TokenView op; // empty = no operator
        std::unique_ptr<Node_Assign> rhs; // null = no rhs
    };

    // **BNF** CONDITION ::= EXPR ['?' ASSIGN ':' ASSIGN]
    struct Node_Condition final : NodeBase
    {
        explicit Node_Condition(SourceSpan span = {}) : NodeBase(NodeKind::Condition, span) {}

        std::unique_ptr<Node_Expr> expr;
        std::unique_ptr<Node_Assign> thenExpr; // null = no ternary
        std::unique_ptr<Node_Assign> elseExpr;
    };

} // namespace light_angel
