#pragma once

#include <memory>
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
        Statement, // list node
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
    class Node_Script;
    class Node_Namespace;
    class Node_Using;
    class Node_Enum;
    class Node_Class;
    class Node_TypeDef;
    class Node_Func;
    class Node_ListPattern;
    class Node_ListEntry;
    class Node_Interface;
    class Node_Var;
    class Node_Import;
    class Node_FuncDef;
    class Node_VirtualProp;
    class Node_InterfaceMethod;
    class Node_StatBlock;
    class Node_ParamList;
    class Node_Parameter;
    class Node_Type;
    class Node_InitList;
    class Node_Scope;
    class Node_DataType;
    class Node_Statement;
    class Node_Switch;
    class Node_Break;
    class Node_For;
    class Node_ForEach;
    class Node_While;
    class Node_DoWhile;
    class Node_If;
    class Node_Continue;
    class Node_ExprStat;
    class Node_Try;
    class Node_Return;
    class Node_Case;
    class Node_Expr;
    class Node_ExprTerm;
    class Node_ConstructorCall;
    class Node_ExprPostOp;
    class Node_Cast;
    class Node_Lambda;
    class Node_LambdaParam;
    class Node_Literal;
    class Node_FuncCall;
    class Node_VarAccess;
    class Node_ArgList;
    class Node_Assign;
    class Node_Condition;

    // -----------------------------------------------

    class NodeBase
    {
    public:
        explicit NodeBase(NodeKind kind, SourceSpan span = {})
            : m_kind(kind), m_range(span)
        {
        }

        virtual ~NodeBase() = default;

        NodeKind kind() const
        {
            return m_kind;
        }

        SourceSpan span() const
        {
            return m_range;
        }

    protected:
        NodeKind m_kind;
        SourceSpan m_range;
    };

    // **BNF** SCRIPT ::= {IMPORT | ENUM | TYPEDEF | CLASS | INTERFACE | FUNCDEF | VIRTUALPROP | VAR | FUNC | NAMESPACE | USING | ';'}
    class Node_Script final : NodeBase
    {
    public:
        explicit Node_Script(SourceSpan span = {})
            : NodeBase(NodeKind::Script, span)
        {
        }

    private:
        std::vector<std::unique_ptr<NodeBase>> m_children;
    };

    // **BNF** NAMESPACE ::= 'namespace' IDENTIFIER {'::' IDENTIFIER} '{' SCRIPT '}'
    class Node_Namespace final : NodeBase
    {
    public:
        explicit Node_Namespace(SourceSpan span = {})
            : NodeBase(NodeKind::Namespace, span)
        {
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

        const Node_Script& script() const
        {
            return *m_script;
        }

    private:
        TokenView m_identifier;
        std::unique_ptr<Node_Script> m_script;
    };

    // **BNF** USING ::= 'using' 'namespace' IDENTIFIER {'::' IDENTIFIER} ';'

    // **BNF** ENUM ::= {'shared' | 'external'} 'enum' IDENTIFIER [ ':' ('int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64') ] (';' | ('{' IDENTIFIER ['=' EXPR] {',' IDENTIFIER ['=' EXPR]} '}'))

    // **BNF** CLASS ::= ['mixin'] {'shared' | 'abstract' | 'final' | 'external'} 'class' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | FUNC | VAR | FUNCDEF} '}'))

    // **BNF** TYPEDEF ::= 'typedef' PRIMITIVETYPE IDENTIFIER ';'

    // **BNF** FUNC ::= {'shared' | 'external'} ['private' | 'protected'] [((TYPE ['&']) | '~')] IDENTIFIER ['<' TYPE {',' TYPE} '>'] PARAMLIST [LISTPATTERN] ['const'] FUNCATTR (';' | STATBLOCK)

    // **BNF** LISTPATTERN ::= '{' LISTENTRY {',' LISTENTRY} '}'

    // **BNF** LISTENTRY ::= (('repeat' | 'repeat_same') (('{' LISTENTRY '}') | TYPE)) | (TYPE {',' TYPE})

    // **BNF** INTERFACE ::= {'external' | 'shared'} 'interface' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | INTERFACEMETHOD} '}'))

    // **BNF** VAR ::= ['private' | 'protected'] TYPE IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST] {',' IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST]} ';'

    // **BNF** IMPORT ::= 'import' TYPE ['&'] IDENTIFIER PARAMLIST FUNCATTR 'from' STRING ';'

    // **BNF** FUNCDEF ::= {'external' | 'shared'} 'funcdef' TYPE ['&'] IDENTIFIER PARAMLIST ';'

    // **BNF** VIRTUALPROP ::= ['private' | 'protected'] TYPE ['&'] IDENTIFIER '{' {('get' | 'set') ['const'] FUNCATTR (STATBLOCK | ';')} '}'

    // **BNF** INTERFACEMETHOD ::= TYPE ['&'] IDENTIFIER PARAMLIST ['const'] FUNCATTR ';'

    // **BNF** STATBLOCK ::= '{' {VAR | STATEMENT | USING} '}'

    // **BNF** PARAMLIST ::= '(' ['void' | (PARAMETER {',' PARAMETER})] ')'

    // **BNF** PARAMETER ::= TYPE TYPEMODIFIER [IDENTIFIER] ['...' | ('=' (EXPR | 'void'))]

    // **BNF** TYPEMODIFIER ::= ['&' ['in' | 'out' | 'inout'] ['+'] ['if_handle_then_const']]

    // **BNF** TYPE ::= ['const'] SCOPE DATATYPE ['<' TYPE {',' TYPE} '>'] { ('[' ']') | ('@' ['const']) }

    // **BNF** INITLIST ::= '{' [ASSIGN | INITLIST] {',' [ASSIGN | INITLIST]} '}'

    // **BNF** SCOPE ::= ['::'] {IDENTIFIER '::'} [IDENTIFIER ['<' TYPE {',' TYPE} '>'] '::']

    // **BNF** DATATYPE ::= (IDENTIFIER | PRIMITIVETYPE | '?' | 'auto')

    // **BNF** PRIMITIVETYPE ::= 'void' | 'int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64' | 'float' | 'double' | 'bool'

    // **BNF** FUNCATTR ::= {'override' | 'final' | 'explicit' | 'property' | 'delete' | 'nodiscard'}

    // **BNF** STATEMENT ::= (IF | FOR | FOREACH | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)

    // **BNF** SWITCH ::= 'switch' '(' ASSIGN ')' '{' {CASE} '}'

    // **BNF** BREAK ::= 'break' ';'

    // **BNF** FOR ::= 'for' '(' (VAR | EXPRSTAT) EXPRSTAT [ASSIGN {',' ASSIGN}] ')' STATEMENT

    // **BNF** FOREACH ::= 'foreach' '(' TYPE IDENTIFIER {',' TYPE IDENTIFIER} ':' ASSIGN ')' STATEMENT

    // **BNF** WHILE ::= 'while' '(' ASSIGN ')' STATEMENT

    // **BNF** DOWHILE ::= 'do' STATEMENT 'while' '(' ASSIGN ')' ';'

    // **BNF** IF ::= 'if' '(' ASSIGN ')' STATEMENT ['else' STATEMENT]

    // **BNF** CONTINUE ::= 'continue' ';'

    // **BNF** EXPRSTAT ::= [ASSIGN] ';'

    // **BNF** TRY ::= 'try' STATBLOCK 'catch' STATBLOCK

    // **BNF** RETURN ::= 'return' [ASSIGN] ';'

    // **BNF** CASE ::= (('case' EXPR) | 'default') ':' {STATEMENT}

    // **BNF** EXPR ::= EXPRTERM {EXPROP EXPRTERM}

    // **BNF** EXPRTERM ::= ([TYPE '='] INITLIST) | ({EXPRPREOP} EXPRVALUE {EXPRPOSTOP})

    // **BNF** EXPRVALUE ::= 'void' | CONSTRUCTORCALL | FUNCCALL | VARACCESS | CAST | LITERAL | '(' ASSIGN ')' | LAMBDA

    // **BNF** CONSTRUCTORCALL ::= TYPE ARGLIST

    // **BNF** EXPRPREOP ::= '-' | '+' | '!' | '++' | '--' | '~' | '@'

    // **BNF** EXPRPOSTOP ::= ('.' (FUNCCALL | IDENTIFIER)) | ('[' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ']') | ARGLIST | '++' | '--'

    // **BNF** CAST ::= 'cast' '<' TYPE '>' '(' ASSIGN ')'

    // **BNF** LAMBDA ::= 'function' '(' [LAMBDAPARAM {',' LAMBDAPARAM}] ')' STATBLOCK

    // **BNF** LAMBDAPARAM ::= [TYPE TYPEMODIFIER] [IDENTIFIER]

    // **BNF** LITERAL ::= NUMBER | STRING | BITS | 'true' | 'false' | 'null'

    // **BNF** FUNCCALL ::= SCOPE IDENTIFIER ['<' TYPE {',' TYPE} '>'] ARGLIST

    // **BNF** VARACCESS ::= SCOPE IDENTIFIER

    // **BNF** ARGLIST ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'

    // **BNF** ASSIGN ::= CONDITION [ ASSIGNOP ASSIGN ]

    // **BNF** CONDITION ::= EXPR ['?' ASSIGN ':' ASSIGN]

} // namespace light_angel
