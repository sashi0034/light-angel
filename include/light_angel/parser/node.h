#pragma once

#include <cassert>
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

    // 'override' | 'final' | 'explicit' | 'property' | 'delete' | 'nodiscard'
    struct FunctionAttribute
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

    // ['&' ['in' | 'out' | 'inout'] ['+'] ['if_handle_then_const']]
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

    using Node = NodeBase;

    template <class T>
    const T& AsNode(const Node& node)
    {
        assert(node.kind == T::Kind);
        return static_cast<const T&>(node);
    }

    // **BNF** SCRIPT ::= {IMPORT | ENUM | TYPEDEF | CLASS | INTERFACE | FUNCDEF | VIRTUALPROP | VAR | FUNC | NAMESPACE | USING | ';'}
    struct Node_Script final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Script;

        explicit Node_Script(SourceSpan span = {})
            : NodeBase(Kind, span)
        {
        }

        std::vector<std::unique_ptr<NodeBase>> children;
    };

    // **BNF** NAMESPACE ::= 'namespace' IDENTIFIER {'::' IDENTIFIER} '{' SCRIPT '}'
    struct Node_Namespace final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Namespace;

        explicit Node_Namespace(SourceSpan span = {})
            : NodeBase(Kind, span)
        {
        }

        std::vector<TokenView> identifiers;
        std::unique_ptr<Node_Script> script;
    };

    // **BNF** USING ::= 'using' 'namespace' IDENTIFIER {'::' IDENTIFIER} ';'
    struct Node_Using final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Using;

        explicit Node_Using(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::vector<TokenView> identifiers;
    };

    // **BNF** ENUM ::= {'shared' | 'external'} 'enum' IDENTIFIER [ ':' ('int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64') ] (';' | ('{' IDENTIFIER ['=' EXPR] {',' IDENTIFIER ['=' EXPR]} '}'))
    struct Node_Enum final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Enum;

        struct Declaration
        {
            TokenView name;
            std::unique_ptr<Node_Expr> expr; // null = no value
        };

        explicit Node_Enum(SourceSpan span = {}) : NodeBase(Kind, span) {}

        EntityAttribute attr;
        TokenView identifier;
        TokenView baseType; // empty = not specified
        bool isForwardDecl = false;
        std::vector<Declaration> values;
    };

    // **BNF** CLASS ::= ['mixin'] {'shared' | 'abstract' | 'final' | 'external'} 'class' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | FUNC | VAR | FUNCDEF} '}'))
    struct Node_Class final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Class;

        struct BaseSpecifier
        {
            std::unique_ptr<Node_Scope> scope;
            TokenView identifier;
        };

        explicit Node_Class(SourceSpan span = {}) : NodeBase(Kind, span) {}

        EntityAttribute attr;
        TokenView identifier;
        bool isForwardDecl = false;
        std::vector<BaseSpecifier> bases;
        std::vector<std::unique_ptr<NodeBase>> members;
    };

    // **BNF** TYPEDEF ::= 'typedef' PRIMITIVETYPE IDENTIFIER ';'
    struct Node_TypeDef final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::TypeDef;

        explicit Node_TypeDef(SourceSpan span = {}) : NodeBase(Kind, span) {}

        TokenView primitiveType;
        TokenView identifier;
    };

    // **BNF** FUNC ::= {'shared' | 'external'} ['private' | 'protected'] [((TYPE ['&']) | '~')] IDENTIFIER ['<' TYPE {',' TYPE} '>'] PARAMLIST [LISTPATTERN] ['const'] FUNCATTR (';' | STATBLOCK)
    struct Node_Func final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Func;

        explicit Node_Func(SourceSpan span = {}) : NodeBase(Kind, span) {}

        EntityAttribute attr;
        std::optional<AccessModifier> access;
        bool isDestructor = false;
        std::unique_ptr<Node_Type> returnType; // null = constructor or destructor
        bool isReturnRef = false;
        TokenView identifier;
        std::vector<std::unique_ptr<Node_Type>> templateParams;
        std::unique_ptr<Node_ParamList> params;
        std::unique_ptr<Node_ListPattern> listPattern;
        bool isConst = false;
        FunctionAttribute funcAttr;
        std::unique_ptr<Node_StatBlock> body; // null = ';'
    };

    // **BNF** FUNCATTR ::= {'override' | 'final' | 'explicit' | 'property' | 'delete' | 'nodiscard'}
    // n/a

    // **BNF** LISTPATTERN ::= '{' LISTENTRY {',' LISTENTRY} '}'
    struct Node_ListPattern final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::ListPattern;

        explicit Node_ListPattern(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::vector<std::unique_ptr<Node_ListEntry>> entries;
    };

    // **BNF** LISTENTRY ::= (('repeat' | 'repeat_same') (('{' LISTENTRY '}') | TYPE)) | (TYPE {',' TYPE})
    struct Node_ListEntry final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::ListEntry;

        enum class EntryKind
        {
            RepeatEntry, // 'repeat' | 'repeat_same' '{' LISTENTRY '}'
            RepeatType, // 'repeat' | 'repeat_same' TYPE
            TypeList, // TYPE {',' TYPE}
        };

        explicit Node_ListEntry(SourceSpan span = {}) : NodeBase(Kind, span) {}

        EntryKind entryKind = EntryKind::TypeList;
        bool isRepeatSame = false;
        std::unique_ptr<Node_ListEntry> entry; // EntryKind::RepeatEntry
        std::vector<std::unique_ptr<Node_Type>> types; // EntryKind::RepeatType / EntryKind::TypeList
    };

    // **BNF** INTERFACE ::= {'external' | 'shared'} 'interface' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | INTERFACEMETHOD} '}'))
    struct Node_Interface final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Interface;

        struct BaseSpecifier
        {
            std::unique_ptr<Node_Scope> scope;
            TokenView identifier;
        };

        explicit Node_Interface(SourceSpan span = {}) : NodeBase(Kind, span) {}

        EntityAttribute attr;
        TokenView identifier;
        bool isForwardDecl = false;
        std::vector<BaseSpecifier> bases;
        std::vector<std::unique_ptr<NodeBase>> members;
    };

    // **BNF** VAR ::= ['private' | 'protected'] TYPE IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST] {',' IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST]} ';'
    struct Node_Var final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Var;

        struct Declaration
        {
            TokenView identifier;
            std::unique_ptr<NodeBase> init; // Node_InitList | Node_Assign | Node_ArgList | null
        };

        explicit Node_Var(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::optional<AccessModifier> access;
        std::unique_ptr<Node_Type> type;
        std::vector<Declaration> decls;
    };

    // **BNF** IMPORT ::= 'import' TYPE ['&'] IDENTIFIER PARAMLIST FUNCATTR 'from' STRING ';'
    struct Node_Import final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Import;

        explicit Node_Import(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Type> type;
        bool isRef = false;
        TokenView identifier;
        std::unique_ptr<Node_ParamList> params;
        FunctionAttribute funcAttr;
        TokenView fromString;
    };

    // **BNF** FUNCDEF ::= {'external' | 'shared'} 'funcdef' TYPE ['&'] IDENTIFIER PARAMLIST ';'
    struct Node_FuncDef final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::FuncDef;

        explicit Node_FuncDef(SourceSpan span = {}) : NodeBase(Kind, span) {}

        EntityAttribute attr;
        std::unique_ptr<Node_Type> returnType;
        bool isRef = false;
        TokenView identifier;
        std::unique_ptr<Node_ParamList> params;
    };

    // **BNF** VIRTUALPROP ::= ['private' | 'protected'] TYPE ['&'] IDENTIFIER '{' {('get' | 'set') ['const'] FUNCATTR (STATBLOCK | ';')} '}'
    struct Node_VirtualProp final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::VirtualProp;

        struct Accessor
        {
            bool isConst = false;
            FunctionAttribute attr;
            std::unique_ptr<Node_StatBlock> body; // null = ';'
        };

        explicit Node_VirtualProp(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::optional<AccessModifier> access;
        std::unique_ptr<Node_Type> type;
        bool isRef = false;
        TokenView identifier;
        Accessor getter;
        Accessor setter;
    };

    // **BNF** INTERFACEMETHOD ::= TYPE ['&'] IDENTIFIER PARAMLIST ['const'] FUNCATTR ';'
    struct Node_InterfaceMethod final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::InterfaceMethod;

        explicit Node_InterfaceMethod(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Type> type;
        bool isRef = false;
        TokenView identifier;
        std::unique_ptr<Node_ParamList> params;
        bool isConst = false;
        FunctionAttribute funcAttr;
    };

    // **BNF** STATBLOCK ::= '{' {VAR | STATEMENT | USING} '}'
    struct Node_StatBlock final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::StatBlock;

        explicit Node_StatBlock(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::vector<std::unique_ptr<NodeBase>> statements;
    };

    // **BNF** PARAMLIST ::= '(' ['void' | (PARAMETER {',' PARAMETER})] ')'
    struct Node_ParamList final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::ParamList;

        explicit Node_ParamList(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::vector<std::unique_ptr<Node_Parameter>> params;
    };

    // **BNF** PARAMETER ::= TYPE TYPEMODIFIER [IDENTIFIER] ['...' | ('=' (EXPR | 'void'))]
    struct Node_Parameter final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Parameter;

        explicit Node_Parameter(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Type> type;
        TypeModifier typeModifier;
        TokenView identifier;
        bool isVariadic = false;
        bool isOptionalOutput = false; // true if '= void' is used, indicating an optional output parameter
        std::unique_ptr<Node_Expr> defaultExpr; // null = no default value
    };

    // **BNF** TYPEMODIFIER ::= ['&' ['in' | 'out' | 'inout'] ['+'] ['if_handle_then_const']]
    // n/a

    // **BNF** TYPE ::= ['const'] SCOPE DATATYPE ['<' TYPE {',' TYPE} '>'] { ('[' ']') | ('@' ['const']) }
    struct Node_Type final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Type;

        enum class Postfix
        {
            Array, // '[' ']'
            Handle, // '@'
            ConstHandle // '@' 'const'
        };

        explicit Node_Type(SourceSpan span = {}) : NodeBase(Kind, span) {}

        bool isConst = false;
        std::unique_ptr<Node_Scope> scope;
        std::unique_ptr<Node_DataType> dataType;
        std::vector<std::unique_ptr<Node_Type>> templateArgs;
        std::vector<Postfix> postfixes;
    };

    // **BNF** INITLIST ::= '{' [ASSIGN | INITLIST] {',' [ASSIGN | INITLIST]} '}'
    struct Node_InitList final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::InitList;

        explicit Node_InitList(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::vector<std::unique_ptr<NodeBase>> items; // Node_Assign | Node_InitList
    };

    // **BNF** SCOPE ::= ['::'] {IDENTIFIER '::'} [IDENTIFIER ['<' TYPE {',' TYPE} '>'] '::']
    struct Node_Scope final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Scope;

        explicit Node_Scope(SourceSpan span = {}) : NodeBase(Kind, span) {}

        bool isGlobal = false;
        std::vector<TokenView> identifiers; // includes the one before '::' if present
        std::vector<std::unique_ptr<Node_Type>> templateArgs; // for the last identifier
    };

    // **BNF** DATATYPE ::= (IDENTIFIER | PRIMITIVETYPE | '?' | 'auto')
    struct Node_DataType final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::DataType;

        explicit Node_DataType(SourceSpan span = {}) : NodeBase(Kind, span) {}

        TokenView token;
    };

    // **BNF** PRIMITIVETYPE ::= 'void' | 'int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64' | 'float' | 'double' | 'bool'
    // n/a

    // **BNF** STATEMENT ::= (IF | FOR | FOREACH | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)
    struct Node_Statement final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Statement;

        explicit Node_Statement(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<NodeBase> child;
    };

    // **BNF** SWITCH ::= 'switch' '(' ASSIGN ')' '{' {CASE} '}'
    struct Node_Switch final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Switch;

        explicit Node_Switch(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Assign> expr;
        std::vector<std::unique_ptr<Node_Case>> cases;
    };

    // **BNF** BREAK ::= 'break' ';'
    struct Node_Break final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Break;

        explicit Node_Break(SourceSpan span = {}) : NodeBase(Kind, span) {}
    };

    // **BNF** FOR ::= 'for' '(' (VAR | EXPRSTAT) EXPRSTAT [ASSIGN {',' ASSIGN}] ')' STATEMENT
    struct Node_For final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::For;

        explicit Node_For(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<NodeBase> initializer; // Node_Var or Node_ExprStat
        std::unique_ptr<Node_ExprStat> condition;
        std::vector<std::unique_ptr<Node_Assign>> increments;
        std::unique_ptr<NodeBase> body;
    };

    // **BNF** FOREACH ::= 'foreach' '(' TYPE IDENTIFIER {',' TYPE IDENTIFIER} ':' ASSIGN ')' STATEMENT
    struct Node_ForEach final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::ForEach;

        struct VarDecl
        {
            std::unique_ptr<Node_Type> type;
            TokenView identifier;
        };

        explicit Node_ForEach(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::vector<VarDecl> vars;
        std::unique_ptr<Node_Assign> range;
        std::unique_ptr<NodeBase> body;
    };

    // **BNF** WHILE ::= 'while' '(' ASSIGN ')' STATEMENT
    struct Node_While final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::While;

        explicit Node_While(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Assign> condition;
        std::unique_ptr<NodeBase> body;
    };

    // **BNF** DOWHILE ::= 'do' STATEMENT 'while' '(' ASSIGN ')' ';'
    struct Node_DoWhile final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::DoWhile;

        explicit Node_DoWhile(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<NodeBase> body;
        std::unique_ptr<Node_Assign> condition;
    };

    // **BNF** IF ::= 'if' '(' ASSIGN ')' STATEMENT ['else' STATEMENT]
    struct Node_If final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::If;

        explicit Node_If(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Assign> condition;
        std::unique_ptr<NodeBase> thenBranch;
        std::unique_ptr<NodeBase> elseBranch; // null = no else
    };

    // **BNF** CONTINUE ::= 'continue' ';'
    struct Node_Continue final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Continue;

        explicit Node_Continue(SourceSpan span = {}) : NodeBase(Kind, span) {}
    };

    // **BNF** EXPRSTAT ::= [ASSIGN] ';'
    struct Node_ExprStat final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::ExprStat;

        explicit Node_ExprStat(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Assign> assign; // null = empty statement
    };

    // **BNF** TRY ::= 'try' STATBLOCK 'catch' STATBLOCK
    struct Node_Try final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Try;

        explicit Node_Try(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_StatBlock> tryBlock;
        std::unique_ptr<Node_StatBlock> catchBlock;
    };

    // **BNF** RETURN ::= 'return' [ASSIGN] ';'
    struct Node_Return final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Return;

        explicit Node_Return(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Assign> value; // null = void return
    };

    // **BNF** CASE ::= (('case' EXPR) | 'default') ':' {STATEMENT}
    struct Node_Case final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Case;

        explicit Node_Case(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Expr> expr; // null = default
        std::vector<std::unique_ptr<NodeBase>> statements;
    };

    // **BNF** EXPR ::= EXPRTERM {EXPROP EXPRTERM}
    struct Node_Expr final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Expr;

        struct OpTerm
        {
            TokenView op; // MATHOP | COMPOP | LOGICOP | BITOP
            std::unique_ptr<Node_ExprTerm> term;
        };

        explicit Node_Expr(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_ExprTerm> first;
        std::vector<OpTerm> rest;
    };

    // **BNF** EXPRTERM ::= ([TYPE '='] INITLIST) | ({EXPRPREOP} EXPRVALUE {EXPRPOSTOP})
    struct Node_ExprTerm final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::ExprTerm;

        enum class Form
        {
            InitListForm, // [TYPE '='] INITLIST
            ExprValueForm, // {EXPRPREOP} EXPRVALUE {EXPRPOSTOP}
        };

        explicit Node_ExprTerm(SourceSpan span = {}) : NodeBase(Kind, span) {}

        Form form = Form::ExprValueForm;

        std::unique_ptr<Node_Type> initType; // null = bare INITLIST
        std::unique_ptr<Node_InitList> initList;

        std::vector<TokenView> preOps; // '-' | '+' | '!' | '++' | '--' | '~' | '@'
        std::unique_ptr<NodeBase> exprValue;
        std::vector<std::unique_ptr<Node_ExprPostOp>> postOps;
    };

    // **BNF** EXPRVALUE ::= CONSTRUCTORCALL | FUNCCALL | VARACCESS | CAST | LITERAL | '(' ASSIGN ')' | LAMBDA
    struct Node_ExprValue final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::ExprValue;

        explicit Node_ExprValue(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<NodeBase> value;
    };

    // **BNF** CONSTRUCTORCALL ::= TYPE ARGLIST
    struct Node_ConstructorCall final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::ConstructorCall;

        explicit Node_ConstructorCall(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Type> type;
        std::unique_ptr<Node_ArgList> args;
    };

    // **BNF** EXPRPREOP ::= '-' | '+' | '!' | '++' | '--' | '~' | '@'
    // n/a

    // **BNF** EXPRPOSTOP ::= ('.' (FUNCCALL | IDENTIFIER)) | ('[' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ']') | ARGLIST | '++' | '--'
    struct Node_ExprPostOp final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::ExprPostOp;

        enum class OpKind
        {
            Member, // '.' (FUNCCALL | IDENTIFIER)
            Subscript, // '[' ... ']'
            Call, // ARGLIST
            Increment, // '++'
            Decrement, // '--'
        };

        struct SubscriptArg
        {
            TokenView name; // empty = positional argument
            std::unique_ptr<Node_Assign> value;
        };

        explicit Node_ExprPostOp(SourceSpan span = {}) : NodeBase(Kind, span) {}

        OpKind opKind = OpKind::Member;
        std::unique_ptr<NodeBase> memberAccess;
        TokenView dotIdentifier;
        std::vector<SubscriptArg> subscriptArgs; // OpKind::Subscript
        std::unique_ptr<Node_ArgList> callArgs; // OpKind::Call
    };

    // **BNF** CAST ::= 'cast' '<' TYPE '>' '(' ASSIGN ')'
    struct Node_Cast final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Cast;

        explicit Node_Cast(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Type> type;
        std::unique_ptr<Node_Assign> expr;
    };

    // **BNF** LAMBDA ::= 'function' '(' [LAMBDAPARAM {',' LAMBDAPARAM}] ')' STATBLOCK
    struct Node_Lambda final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Lambda;

        explicit Node_Lambda(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::vector<std::unique_ptr<Node_LambdaParam>> params;
        std::unique_ptr<Node_StatBlock> body;
    };

    // **BNF** LAMBDAPARAM ::= [TYPE TYPEMODIFIER] [IDENTIFIER]
    struct Node_LambdaParam final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::LambdaParam;

        explicit Node_LambdaParam(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Type> type; // null = type omitted
        TypeModifier typeModifier;
        TokenView identifier;
    };

    // **BNF** LITERAL ::= NUMBER | STRING | BITS | 'true' | 'false' | 'null' | 'void'
    struct Node_Literal final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Literal;

        explicit Node_Literal(SourceSpan span = {}) : NodeBase(Kind, span) {}

        TokenView token;
    };

    // **BNF** FUNCCALL ::= SCOPE IDENTIFIER ['<' TYPE {',' TYPE} '>'] ARGLIST
    struct Node_FuncCall final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::FuncCall;

        explicit Node_FuncCall(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Scope> scope;
        TokenView identifier;
        std::vector<std::unique_ptr<Node_Type>> templateArgs;
        std::unique_ptr<Node_ArgList> args;
    };

    // **BNF** VARACCESS ::= SCOPE IDENTIFIER
    struct Node_VarAccess final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::VarAccess;

        explicit Node_VarAccess(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Scope> scope;
        TokenView identifier;
    };

    // **BNF** ARGLIST ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'
    struct Node_ArgList final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::ArgList;

        struct Arg
        {
            TokenView name; // empty = positional argument
            std::unique_ptr<Node_Assign> value;
        };

        explicit Node_ArgList(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::vector<Arg> args;
    };

    // **BNF** ASSIGN ::= CONDITION [ ASSIGNOP ASSIGN ]
    struct Node_Assign final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Assign;

        explicit Node_Assign(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Condition> condition;
        TokenView op; // empty = no operator
        std::unique_ptr<Node_Assign> rhs; // null = no rhs
    };

    // **BNF** CONDITION ::= EXPR ['?' ASSIGN ':' ASSIGN]
    struct Node_Condition final : NodeBase
    {
        static constexpr NodeKind Kind = NodeKind::Condition;

        explicit Node_Condition(SourceSpan span = {}) : NodeBase(Kind, span) {}

        std::unique_ptr<Node_Expr> expr;
        std::unique_ptr<Node_Assign> thenExpr; // null = no ternary
        std::unique_ptr<Node_Assign> elseExpr;
    };

} // namespace light_angel
