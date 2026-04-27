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
        static constexpr uint8_t Mixin = 1 << 4; // CLASS のみ

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

        const std::vector<std::unique_ptr<NodeBase>>& children() const
        {
            return m_children;
        }

        void addChild(std::unique_ptr<NodeBase> child)
        {
            m_children.push_back(std::move(child));
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

        TokenView& identifier()
        {
            return m_identifier;
        }

        const Node_Script& script() const
        {
            return *m_script;
        }

        std::unique_ptr<Node_Script>& script()
        {
            return m_script;
        }

    private:
        TokenView m_identifier;
        std::unique_ptr<Node_Script> m_script;
    };

    // **BNF** USING ::= 'using' 'namespace' IDENTIFIER {'::' IDENTIFIER} ';'
    class Node_Using final : NodeBase
    {
    public:
        explicit Node_Using(SourceSpan span = {}) : NodeBase(NodeKind::Using, span) {}

        const std::vector<TokenView>& identifiers() const
        {
            return m_identifiers;
        }

        void addIdentifier(TokenView id)
        {
            m_identifiers.push_back(id);
        }

    private:
        std::vector<TokenView> m_identifiers;
    };

    // **BNF** ENUM ::= {'shared' | 'external'} 'enum' IDENTIFIER [ ':' ('int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64') ] (';' | ('{' IDENTIFIER ['=' EXPR] {',' IDENTIFIER ['=' EXPR]} '}'))
    class Node_Enum final : NodeBase
    {
    public:
        struct EnumValue
        {
            TokenView name;
            std::unique_ptr<Node_Expr> expr; // null = 値なし
        };

        explicit Node_Enum(SourceSpan span = {}) : NodeBase(NodeKind::Enum, span) {}

        EntityAttribute& attr()
        {
            return m_attr;
        }

        EntityAttribute attr() const
        {
            return m_attr;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

        TokenView& baseType()
        {
            return m_baseType;
        } // empty = 指定なし

        const TokenView& baseType() const
        {
            return m_baseType;
        }

        bool isForwardDecl() const
        {
            return m_isForwardDecl;
        }

        void setForwardDecl(bool v)
        {
            m_isForwardDecl = v;
        }

        std::vector<EnumValue>& values()
        {
            return m_values;
        }

        const std::vector<EnumValue>& values() const
        {
            return m_values;
        }

    private:
        EntityAttribute m_attr;
        TokenView m_identifier;
        TokenView m_baseType;
        bool m_isForwardDecl = false;
        std::vector<EnumValue> m_values;
    };

    // **BNF** CLASS ::= ['mixin'] {'shared' | 'abstract' | 'final' | 'external'} 'class' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | FUNC | VAR | FUNCDEF} '}'))
    class Node_Class final : NodeBase
    {
    public:
        struct BaseClass
        {
            std::unique_ptr<Node_Scope> scope;
            TokenView identifier;
        };

        explicit Node_Class(SourceSpan span = {}) : NodeBase(NodeKind::Class, span) {}

        EntityAttribute& attr()
        {
            return m_attr;
        }

        EntityAttribute attr() const
        {
            return m_attr;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

        bool isForwardDecl() const
        {
            return m_isForwardDecl;
        }

        void setForwardDecl(bool v)
        {
            m_isForwardDecl = v;
        }

        std::vector<BaseClass>& bases()
        {
            return m_bases;
        }

        const std::vector<BaseClass>& bases() const
        {
            return m_bases;
        }

        const std::vector<std::unique_ptr<NodeBase>>& members() const
        {
            return m_members;
        }

        void addMember(std::unique_ptr<NodeBase> m)
        {
            m_members.push_back(std::move(m));
        }

    private:
        EntityAttribute m_attr;
        TokenView m_identifier;
        bool m_isForwardDecl = false;
        std::vector<BaseClass> m_bases;
        std::vector<std::unique_ptr<NodeBase>> m_members;
    };

    // **BNF** TYPEDEF ::= 'typedef' PRIMITIVETYPE IDENTIFIER ';'
    class Node_TypeDef final : NodeBase
    {
    public:
        explicit Node_TypeDef(SourceSpan span = {}) : NodeBase(NodeKind::TypeDef, span) {}

        TokenView& primitiveType()
        {
            return m_primitiveType;
        }

        const TokenView& primitiveType() const
        {
            return m_primitiveType;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

    private:
        TokenView m_primitiveType;
        TokenView m_identifier;
    };

    // **BNF** FUNC ::= {'shared' | 'external'} ['private' | 'protected'] [((TYPE ['&']) | '~')] IDENTIFIER ['<' TYPE {',' TYPE} '>'] PARAMLIST [LISTPATTERN] ['const'] FUNCATTR (';' | STATBLOCK)
    class Node_Func final : NodeBase
    {
    public:
        explicit Node_Func(SourceSpan span = {}) : NodeBase(NodeKind::Func, span) {}

        EntityAttribute& attr()
        {
            return m_attr;
        }

        EntityAttribute attr() const
        {
            return m_attr;
        }

        std::optional<AccessModifier>& access()
        {
            return m_access;
        }

        const std::optional<AccessModifier>& access() const
        {
            return m_access;
        }

        bool isDestructor() const
        {
            return m_isDestructor;
        }

        void setDestructor(bool v)
        {
            m_isDestructor = v;
        }

        std::unique_ptr<Node_Type>& returnType()
        {
            return m_returnType;
        } // null = destructor

        const std::unique_ptr<Node_Type>& returnType() const
        {
            return m_returnType;
        }

        bool isReturnRef() const
        {
            return m_isReturnRef;
        }

        void setReturnRef(bool v)
        {
            m_isReturnRef = v;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

        std::vector<std::unique_ptr<Node_Type>>& templateParams()
        {
            return m_templateParams;
        }

        const std::vector<std::unique_ptr<Node_Type>>& templateParams() const
        {
            return m_templateParams;
        }

        std::unique_ptr<Node_ParamList>& params()
        {
            return m_params;
        }

        const std::unique_ptr<Node_ParamList>& params() const
        {
            return m_params;
        }

        std::unique_ptr<Node_ListPattern>& listPattern()
        {
            return m_listPattern;
        }

        const std::unique_ptr<Node_ListPattern>& listPattern() const
        {
            return m_listPattern;
        }

        bool isConst() const
        {
            return m_isConst;
        }

        void setConst(bool v)
        {
            m_isConst = v;
        }

        FuncAttr& funcAttr()
        {
            return m_funcAttr;
        }

        FuncAttr funcAttr() const
        {
            return m_funcAttr;
        }

        std::unique_ptr<Node_StatBlock>& body()
        {
            return m_body;
        } // null = ';'

        const std::unique_ptr<Node_StatBlock>& body() const
        {
            return m_body;
        }

    private:
        EntityAttribute m_attr;
        std::optional<AccessModifier> m_access;
        bool m_isDestructor = false;
        std::unique_ptr<Node_Type> m_returnType;
        bool m_isReturnRef = false;
        TokenView m_identifier;
        std::vector<std::unique_ptr<Node_Type>> m_templateParams;
        std::unique_ptr<Node_ParamList> m_params;
        std::unique_ptr<Node_ListPattern> m_listPattern;
        bool m_isConst = false;
        FuncAttr m_funcAttr;
        std::unique_ptr<Node_StatBlock> m_body;
    };

    // **BNF** LISTPATTERN ::= '{' LISTENTRY {',' LISTENTRY} '}'
    class Node_ListPattern final : NodeBase
    {
    public:
        explicit Node_ListPattern(SourceSpan span = {}) : NodeBase(NodeKind::ListPattern, span) {}

        const std::vector<std::unique_ptr<Node_ListEntry>>& entries() const
        {
            return m_entries;
        }

        void addEntry(std::unique_ptr<Node_ListEntry> e)
        {
            m_entries.push_back(std::move(e));
        }

    private:
        std::vector<std::unique_ptr<Node_ListEntry>> m_entries;
    };

    // **BNF** LISTENTRY ::= (('repeat' | 'repeat_same') (('{' LISTENTRY '}') | TYPE)) | (TYPE {',' TYPE})
    class Node_ListEntry final : NodeBase
    {
    public:
        enum class Kind
        {
            RepeatSub, // 'repeat'|'repeat_same' '{' LISTENTRY '}'
            RepeatType, // 'repeat'|'repeat_same' TYPE
            TypeList, // TYPE {',' TYPE}
        };

        explicit Node_ListEntry(SourceSpan span = {}) : NodeBase(NodeKind::ListEntry, span) {}

        Kind& entryKind()
        {
            return m_entryKind;
        }

        Kind entryKind() const
        {
            return m_entryKind;
        }

        bool isRepeatSame() const
        {
            return m_isRepeatSame;
        }

        void setRepeatSame(bool v)
        {
            m_isRepeatSame = v;
        }

        // Kind::RepeatSub
        std::unique_ptr<Node_ListEntry>& subEntry()
        {
            return m_subEntry;
        }

        const std::unique_ptr<Node_ListEntry>& subEntry() const
        {
            return m_subEntry;
        }

        // Kind::RepeatType / Kind::TypeList
        std::vector<std::unique_ptr<Node_Type>>& types()
        {
            return m_types;
        }

        const std::vector<std::unique_ptr<Node_Type>>& types() const
        {
            return m_types;
        }

    private:
        Kind m_entryKind = Kind::TypeList;
        bool m_isRepeatSame = false;
        std::unique_ptr<Node_ListEntry> m_subEntry;
        std::vector<std::unique_ptr<Node_Type>> m_types;
    };

    // **BNF** INTERFACE ::= {'external' | 'shared'} 'interface' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | INTERFACEMETHOD} '}'))
    class Node_Interface final : NodeBase
    {
    public:
        struct BaseInterface
        {
            std::unique_ptr<Node_Scope> scope;
            TokenView identifier;
        };

        explicit Node_Interface(SourceSpan span = {}) : NodeBase(NodeKind::Interface, span) {}

        EntityAttribute& attr()
        {
            return m_attr;
        }

        EntityAttribute attr() const
        {
            return m_attr;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

        bool isForwardDecl() const
        {
            return m_isForwardDecl;
        }

        void setForwardDecl(bool v)
        {
            m_isForwardDecl = v;
        }

        std::vector<BaseInterface>& bases()
        {
            return m_bases;
        }

        const std::vector<BaseInterface>& bases() const
        {
            return m_bases;
        }

        const std::vector<std::unique_ptr<NodeBase>>& members() const
        {
            return m_members;
        }

        void addMember(std::unique_ptr<NodeBase> m)
        {
            m_members.push_back(std::move(m));
        }

    private:
        EntityAttribute m_attr;
        TokenView m_identifier;
        bool m_isForwardDecl = false;
        std::vector<BaseInterface> m_bases;
        std::vector<std::unique_ptr<NodeBase>> m_members;
    };

    // **BNF** VAR ::= ['private' | 'protected'] TYPE IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST] {',' IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST]} ';'
    class Node_Var final : NodeBase
    {
    public:
        struct VarDecl
        {
            TokenView identifier;
            std::unique_ptr<NodeBase> init; // Node_InitList | Node_Assign | Node_ArgList | null
        };

        explicit Node_Var(SourceSpan span = {}) : NodeBase(NodeKind::Var, span) {}

        std::optional<AccessModifier>& access()
        {
            return m_access;
        }

        const std::optional<AccessModifier>& access() const
        {
            return m_access;
        }

        std::unique_ptr<Node_Type>& type()
        {
            return m_type;
        }

        const std::unique_ptr<Node_Type>& type() const
        {
            return m_type;
        }

        std::vector<VarDecl>& decls()
        {
            return m_decls;
        }

        const std::vector<VarDecl>& decls() const
        {
            return m_decls;
        }

    private:
        std::optional<AccessModifier> m_access;
        std::unique_ptr<Node_Type> m_type;
        std::vector<VarDecl> m_decls;
    };

    // **BNF** IMPORT ::= 'import' TYPE ['&'] IDENTIFIER PARAMLIST FUNCATTR 'from' STRING ';'
    class Node_Import final : NodeBase
    {
    public:
        explicit Node_Import(SourceSpan span = {}) : NodeBase(NodeKind::Import, span) {}

        std::unique_ptr<Node_Type>& type()
        {
            return m_type;
        }

        const std::unique_ptr<Node_Type>& type() const
        {
            return m_type;
        }

        bool isRef() const
        {
            return m_isRef;
        }

        void setRef(bool v)
        {
            m_isRef = v;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

        std::unique_ptr<Node_ParamList>& params()
        {
            return m_params;
        }

        const std::unique_ptr<Node_ParamList>& params() const
        {
            return m_params;
        }

        FuncAttr& funcAttr()
        {
            return m_funcAttr;
        }

        FuncAttr funcAttr() const
        {
            return m_funcAttr;
        }

        TokenView& fromString()
        {
            return m_fromString;
        }

        const TokenView& fromString() const
        {
            return m_fromString;
        }

    private:
        std::unique_ptr<Node_Type> m_type;
        bool m_isRef = false;
        TokenView m_identifier;
        std::unique_ptr<Node_ParamList> m_params;
        FuncAttr m_funcAttr;
        TokenView m_fromString;
    };

    // **BNF** FUNCDEF ::= {'external' | 'shared'} 'funcdef' TYPE ['&'] IDENTIFIER PARAMLIST ';'
    class Node_FuncDef final : NodeBase
    {
    public:
        explicit Node_FuncDef(SourceSpan span = {}) : NodeBase(NodeKind::FuncDef, span) {}

        EntityAttribute& attr()
        {
            return m_attr;
        }

        EntityAttribute attr() const
        {
            return m_attr;
        }

        std::unique_ptr<Node_Type>& returnType()
        {
            return m_returnType;
        }

        const std::unique_ptr<Node_Type>& returnType() const
        {
            return m_returnType;
        }

        bool isRef() const
        {
            return m_isRef;
        }

        void setRef(bool v)
        {
            m_isRef = v;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

        std::unique_ptr<Node_ParamList>& params()
        {
            return m_params;
        }

        const std::unique_ptr<Node_ParamList>& params() const
        {
            return m_params;
        }

    private:
        EntityAttribute m_attr;
        std::unique_ptr<Node_Type> m_returnType;
        bool m_isRef = false;
        TokenView m_identifier;
        std::unique_ptr<Node_ParamList> m_params;
    };

    // **BNF** VIRTUALPROP ::= ['private' | 'protected'] TYPE ['&'] IDENTIFIER '{' {('get' | 'set') ['const'] FUNCATTR (STATBLOCK | ';')} '}'
    class Node_VirtualProp final : NodeBase
    {
    public:
        struct Accessor
        {
            bool isGet = true;
            bool isConst = false;
            FuncAttr attr;
            std::unique_ptr<Node_StatBlock> body; // null = ';'
        };

        explicit Node_VirtualProp(SourceSpan span = {}) : NodeBase(NodeKind::VirtualProp, span) {}

        std::optional<AccessModifier>& access()
        {
            return m_access;
        }

        const std::optional<AccessModifier>& access() const
        {
            return m_access;
        }

        std::unique_ptr<Node_Type>& type()
        {
            return m_type;
        }

        const std::unique_ptr<Node_Type>& type() const
        {
            return m_type;
        }

        bool isRef() const
        {
            return m_isRef;
        }

        void setRef(bool v)
        {
            m_isRef = v;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

        std::vector<Accessor>& accessors()
        {
            return m_accessors;
        }

        const std::vector<Accessor>& accessors() const
        {
            return m_accessors;
        }

    private:
        std::optional<AccessModifier> m_access;
        std::unique_ptr<Node_Type> m_type;
        bool m_isRef = false;
        TokenView m_identifier;
        std::vector<Accessor> m_accessors;
    };

    // **BNF** INTERFACEMETHOD ::= TYPE ['&'] IDENTIFIER PARAMLIST ['const'] FUNCATTR ';'
    class Node_InterfaceMethod final : NodeBase
    {
    public:
        explicit Node_InterfaceMethod(SourceSpan span = {}) : NodeBase(NodeKind::InterfaceMethod, span) {}

        std::unique_ptr<Node_Type>& type()
        {
            return m_type;
        }

        const std::unique_ptr<Node_Type>& type() const
        {
            return m_type;
        }

        bool isRef() const
        {
            return m_isRef;
        }

        void setRef(bool v)
        {
            m_isRef = v;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

        std::unique_ptr<Node_ParamList>& params()
        {
            return m_params;
        }

        const std::unique_ptr<Node_ParamList>& params() const
        {
            return m_params;
        }

        bool isConst() const
        {
            return m_isConst;
        }

        void setConst(bool v)
        {
            m_isConst = v;
        }

        FuncAttr& funcAttr()
        {
            return m_funcAttr;
        }

        FuncAttr funcAttr() const
        {
            return m_funcAttr;
        }

    private:
        std::unique_ptr<Node_Type> m_type;
        bool m_isRef = false;
        TokenView m_identifier;
        std::unique_ptr<Node_ParamList> m_params;
        bool m_isConst = false;
        FuncAttr m_funcAttr;
    };

    // **BNF** STATBLOCK ::= '{' {VAR | STATEMENT | USING} '}'
    class Node_StatBlock final : NodeBase
    {
    public:
        explicit Node_StatBlock(SourceSpan span = {}) : NodeBase(NodeKind::StatBlock, span) {}

        const std::vector<std::unique_ptr<NodeBase>>& statements() const
        {
            return m_statements;
        }

        void addStatement(std::unique_ptr<NodeBase> s)
        {
            m_statements.push_back(std::move(s));
        }

    private:
        std::vector<std::unique_ptr<NodeBase>> m_statements;
    };

    // **BNF** PARAMLIST ::= '(' ['void' | (PARAMETER {',' PARAMETER})] ')'
    class Node_ParamList final : NodeBase
    {
    public:
        explicit Node_ParamList(SourceSpan span = {}) : NodeBase(NodeKind::ParamList, span) {}

        bool isVoid() const
        {
            return m_isVoid;
        }

        void setVoid(bool v)
        {
            m_isVoid = v;
        }

        const std::vector<std::unique_ptr<Node_Parameter>>& params() const
        {
            return m_params;
        }

        void addParam(std::unique_ptr<Node_Parameter> p)
        {
            m_params.push_back(std::move(p));
        }

    private:
        bool m_isVoid = false;
        std::vector<std::unique_ptr<Node_Parameter>> m_params;
    };

    // **BNF** PARAMETER ::= TYPE TYPEMODIFIER [IDENTIFIER] ['...' | ('=' (EXPR | 'void'))]
    class Node_Parameter final : NodeBase
    {
    public:
        explicit Node_Parameter(SourceSpan span = {}) : NodeBase(NodeKind::Parameter, span) {}

        std::unique_ptr<Node_Type>& type()
        {
            return m_type;
        }

        const std::unique_ptr<Node_Type>& type() const
        {
            return m_type;
        }

        TypeModifier& typeModifier()
        {
            return m_typeModifier;
        }

        const TypeModifier& typeModifier() const
        {
            return m_typeModifier;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

        bool isVariadic() const
        {
            return m_isVariadic;
        }

        void setVariadic(bool v)
        {
            m_isVariadic = v;
        }

        // デフォルト値: '=' EXPR の場合は defaultExpr()、'=' 'void' の場合は hasVoidDefault()
        bool hasVoidDefault() const
        {
            return m_hasVoidDefault;
        }

        void setVoidDefault(bool v)
        {
            m_hasVoidDefault = v;
        }

        std::unique_ptr<Node_Expr>& defaultExpr()
        {
            return m_defaultExpr;
        }

        const std::unique_ptr<Node_Expr>& defaultExpr() const
        {
            return m_defaultExpr;
        }

    private:
        std::unique_ptr<Node_Type> m_type;
        TypeModifier m_typeModifier;
        TokenView m_identifier;
        bool m_isVariadic = false;
        bool m_hasVoidDefault = false;
        std::unique_ptr<Node_Expr> m_defaultExpr;
    };

    // **BNF** TYPE ::= ['const'] SCOPE DATATYPE ['<' TYPE {',' TYPE} '>'] { ('[' ']') | ('@' ['const']) }
    class Node_Type final : NodeBase
    {
    public:
        // ('[' ']') → Array、('@') → Handle、('@' 'const') → HandleConst
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

        bool isConst() const
        {
            return m_isConst;
        }

        void setConst(bool v)
        {
            m_isConst = v;
        }

        std::unique_ptr<Node_Scope>& scope()
        {
            return m_scope;
        }

        const std::unique_ptr<Node_Scope>& scope() const
        {
            return m_scope;
        }

        std::unique_ptr<Node_DataType>& dataType()
        {
            return m_dataType;
        }

        const std::unique_ptr<Node_DataType>& dataType() const
        {
            return m_dataType;
        }

        std::vector<std::unique_ptr<Node_Type>>& templateArgs()
        {
            return m_templateArgs;
        }

        const std::vector<std::unique_ptr<Node_Type>>& templateArgs() const
        {
            return m_templateArgs;
        }

        std::vector<Postfix>& postfixes()
        {
            return m_postfixes;
        }

        const std::vector<Postfix>& postfixes() const
        {
            return m_postfixes;
        }

    private:
        bool m_isConst = false;
        std::unique_ptr<Node_Scope> m_scope;
        std::unique_ptr<Node_DataType> m_dataType;
        std::vector<std::unique_ptr<Node_Type>> m_templateArgs;
        std::vector<Postfix> m_postfixes;
    };

    // **BNF** INITLIST ::= '{' [ASSIGN | INITLIST] {',' [ASSIGN | INITLIST]} '}'
    class Node_InitList final : NodeBase
    {
    public:
        explicit Node_InitList(SourceSpan span = {}) : NodeBase(NodeKind::InitList, span) {}

        const std::vector<std::unique_ptr<NodeBase>>& items() const
        {
            return m_items;
        }

        void addItem(std::unique_ptr<NodeBase> item)
        {
            m_items.push_back(std::move(item));
        }

    private:
        std::vector<std::unique_ptr<NodeBase>> m_items; // Node_Assign | Node_InitList
    };

    // **BNF** SCOPE ::= ['::'] {IDENTIFIER '::'} [IDENTIFIER ['<' TYPE {',' TYPE} '>'] '::']
    class Node_Scope final : NodeBase
    {
    public:
        // 各識別子セグメント。テンプレート引数は最後の qualified セグメントのみ持つ
        struct ScopePart
        {
            TokenView identifier;
            std::vector<std::unique_ptr<Node_Type>> templateArgs;
        };

        explicit Node_Scope(SourceSpan span = {}) : NodeBase(NodeKind::Scope, span) {}

        bool isGlobal() const
        {
            return m_isGlobal;
        }

        void setGlobal(bool v)
        {
            m_isGlobal = v;
        }

        std::vector<ScopePart>& parts()
        {
            return m_parts;
        }

        const std::vector<ScopePart>& parts() const
        {
            return m_parts;
        }

    private:
        bool m_isGlobal = false;
        std::vector<ScopePart> m_parts;
    };

    // **BNF** DATATYPE ::= (IDENTIFIER | PRIMITIVETYPE | '?' | 'auto')
    class Node_DataType final : NodeBase
    {
    public:
        explicit Node_DataType(SourceSpan span = {}) : NodeBase(NodeKind::DataType, span) {}

        TokenView& token()
        {
            return m_token;
        }

        const TokenView& token() const
        {
            return m_token;
        }

    private:
        TokenView m_token;
    };

    // **BNF** PRIMITIVETYPE ::= 'void' | 'int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64' | 'float' | 'double' | 'bool'
    // (PRIMITIVETYPE はトークンレベルで判別されるため、独立したノードクラスは持たない)

    // **BNF** STATEMENT ::= (IF | FOR | FOREACH | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)
    class Node_Statement final : NodeBase
    {
    public:
        explicit Node_Statement(SourceSpan span = {}) : NodeBase(NodeKind::Statement, span) {}

        const NodeBase* child() const
        {
            return m_child.get();
        }

        void setChild(std::unique_ptr<NodeBase> c)
        {
            m_child = std::move(c);
        }

    private:
        std::unique_ptr<NodeBase> m_child;
    };

    // **BNF** SWITCH ::= 'switch' '(' ASSIGN ')' '{' {CASE} '}'
    class Node_Switch final : NodeBase
    {
    public:
        explicit Node_Switch(SourceSpan span = {}) : NodeBase(NodeKind::Switch, span) {}

        std::unique_ptr<Node_Assign>& expr()
        {
            return m_expr;
        }

        const std::unique_ptr<Node_Assign>& expr() const
        {
            return m_expr;
        }

        const std::vector<std::unique_ptr<Node_Case>>& cases() const
        {
            return m_cases;
        }

        void addCase(std::unique_ptr<Node_Case> c)
        {
            m_cases.push_back(std::move(c));
        }

    private:
        std::unique_ptr<Node_Assign> m_expr;
        std::vector<std::unique_ptr<Node_Case>> m_cases;
    };

    // **BNF** BREAK ::= 'break' ';'
    class Node_Break final : NodeBase
    {
    public:
        explicit Node_Break(SourceSpan span = {}) : NodeBase(NodeKind::Break, span) {}
    };

    // **BNF** FOR ::= 'for' '(' (VAR | EXPRSTAT) EXPRSTAT [ASSIGN {',' ASSIGN}] ')' STATEMENT
    class Node_For final : NodeBase
    {
    public:
        explicit Node_For(SourceSpan span = {}) : NodeBase(NodeKind::For, span) {}

        // init: Node_Var または Node_ExprStat
        std::unique_ptr<NodeBase>& init()
        {
            return m_init;
        }

        const std::unique_ptr<NodeBase>& init() const
        {
            return m_init;
        }

        std::unique_ptr<Node_ExprStat>& cond()
        {
            return m_cond;
        }

        const std::unique_ptr<Node_ExprStat>& cond() const
        {
            return m_cond;
        }

        std::vector<std::unique_ptr<Node_Assign>>& incs()
        {
            return m_incs;
        }

        const std::vector<std::unique_ptr<Node_Assign>>& incs() const
        {
            return m_incs;
        }

        std::unique_ptr<NodeBase>& body()
        {
            return m_body;
        }

        const std::unique_ptr<NodeBase>& body() const
        {
            return m_body;
        }

    private:
        std::unique_ptr<NodeBase> m_init;
        std::unique_ptr<Node_ExprStat> m_cond;
        std::vector<std::unique_ptr<Node_Assign>> m_incs;
        std::unique_ptr<NodeBase> m_body;
    };

    // **BNF** FOREACH ::= 'foreach' '(' TYPE IDENTIFIER {',' TYPE IDENTIFIER} ':' ASSIGN ')' STATEMENT
    class Node_ForEach final : NodeBase
    {
    public:
        struct IterVar
        {
            std::unique_ptr<Node_Type> type;
            TokenView identifier;
        };

        explicit Node_ForEach(SourceSpan span = {}) : NodeBase(NodeKind::ForEach, span) {}

        std::vector<IterVar>& vars()
        {
            return m_vars;
        }

        const std::vector<IterVar>& vars() const
        {
            return m_vars;
        }

        std::unique_ptr<Node_Assign>& range()
        {
            return m_range;
        }

        const std::unique_ptr<Node_Assign>& range() const
        {
            return m_range;
        }

        std::unique_ptr<NodeBase>& body()
        {
            return m_body;
        }

        const std::unique_ptr<NodeBase>& body() const
        {
            return m_body;
        }

    private:
        std::vector<IterVar> m_vars;
        std::unique_ptr<Node_Assign> m_range;
        std::unique_ptr<NodeBase> m_body;
    };

    // **BNF** WHILE ::= 'while' '(' ASSIGN ')' STATEMENT
    class Node_While final : NodeBase
    {
    public:
        explicit Node_While(SourceSpan span = {}) : NodeBase(NodeKind::While, span) {}

        std::unique_ptr<Node_Assign>& cond()
        {
            return m_cond;
        }

        const std::unique_ptr<Node_Assign>& cond() const
        {
            return m_cond;
        }

        std::unique_ptr<NodeBase>& body()
        {
            return m_body;
        }

        const std::unique_ptr<NodeBase>& body() const
        {
            return m_body;
        }

    private:
        std::unique_ptr<Node_Assign> m_cond;
        std::unique_ptr<NodeBase> m_body;
    };

    // **BNF** DOWHILE ::= 'do' STATEMENT 'while' '(' ASSIGN ')' ';'
    class Node_DoWhile final : NodeBase
    {
    public:
        explicit Node_DoWhile(SourceSpan span = {}) : NodeBase(NodeKind::DoWhile, span) {}

        std::unique_ptr<NodeBase>& body()
        {
            return m_body;
        }

        const std::unique_ptr<NodeBase>& body() const
        {
            return m_body;
        }

        std::unique_ptr<Node_Assign>& cond()
        {
            return m_cond;
        }

        const std::unique_ptr<Node_Assign>& cond() const
        {
            return m_cond;
        }

    private:
        std::unique_ptr<NodeBase> m_body;
        std::unique_ptr<Node_Assign> m_cond;
    };

    // **BNF** IF ::= 'if' '(' ASSIGN ')' STATEMENT ['else' STATEMENT]
    class Node_If final : NodeBase
    {
    public:
        explicit Node_If(SourceSpan span = {}) : NodeBase(NodeKind::If, span) {}

        std::unique_ptr<Node_Assign>& cond()
        {
            return m_cond;
        }

        const std::unique_ptr<Node_Assign>& cond() const
        {
            return m_cond;
        }

        std::unique_ptr<NodeBase>& thenBranch()
        {
            return m_then;
        }

        const std::unique_ptr<NodeBase>& thenBranch() const
        {
            return m_then;
        }

        std::unique_ptr<NodeBase>& elseBranch()
        {
            return m_else;
        } // null = else なし

        const std::unique_ptr<NodeBase>& elseBranch() const
        {
            return m_else;
        }

    private:
        std::unique_ptr<Node_Assign> m_cond;
        std::unique_ptr<NodeBase> m_then;
        std::unique_ptr<NodeBase> m_else;
    };

    // **BNF** CONTINUE ::= 'continue' ';'
    class Node_Continue final : NodeBase
    {
    public:
        explicit Node_Continue(SourceSpan span = {}) : NodeBase(NodeKind::Continue, span) {}
    };

    // **BNF** EXPRSTAT ::= [ASSIGN] ';'
    class Node_ExprStat final : NodeBase
    {
    public:
        explicit Node_ExprStat(SourceSpan span = {}) : NodeBase(NodeKind::ExprStat, span) {}

        std::unique_ptr<Node_Assign>& assign()
        {
            return m_assign;
        } // null = 空文

        const std::unique_ptr<Node_Assign>& assign() const
        {
            return m_assign;
        }

    private:
        std::unique_ptr<Node_Assign> m_assign;
    };

    // **BNF** TRY ::= 'try' STATBLOCK 'catch' STATBLOCK
    class Node_Try final : NodeBase
    {
    public:
        explicit Node_Try(SourceSpan span = {}) : NodeBase(NodeKind::Try, span) {}

        std::unique_ptr<Node_StatBlock>& tryBlock()
        {
            return m_try;
        }

        const std::unique_ptr<Node_StatBlock>& tryBlock() const
        {
            return m_try;
        }

        std::unique_ptr<Node_StatBlock>& catchBlock()
        {
            return m_catch;
        }

        const std::unique_ptr<Node_StatBlock>& catchBlock() const
        {
            return m_catch;
        }

    private:
        std::unique_ptr<Node_StatBlock> m_try;
        std::unique_ptr<Node_StatBlock> m_catch;
    };

    // **BNF** RETURN ::= 'return' [ASSIGN] ';'
    class Node_Return final : NodeBase
    {
    public:
        explicit Node_Return(SourceSpan span = {}) : NodeBase(NodeKind::Return, span) {}

        std::unique_ptr<Node_Assign>& value()
        {
            return m_value;
        } // null = void return

        const std::unique_ptr<Node_Assign>& value() const
        {
            return m_value;
        }

    private:
        std::unique_ptr<Node_Assign> m_value;
    };

    // **BNF** CASE ::= (('case' EXPR) | 'default') ':' {STATEMENT}
    class Node_Case final : NodeBase
    {
    public:
        explicit Node_Case(SourceSpan span = {}) : NodeBase(NodeKind::Case, span) {}

        std::unique_ptr<Node_Expr>& expr()
        {
            return m_expr;
        } // null = default

        const std::unique_ptr<Node_Expr>& expr() const
        {
            return m_expr;
        }

        const std::vector<std::unique_ptr<NodeBase>>& statements() const
        {
            return m_statements;
        }

        void addStatement(std::unique_ptr<NodeBase> s)
        {
            m_statements.push_back(std::move(s));
        }

    private:
        std::unique_ptr<Node_Expr> m_expr;
        std::vector<std::unique_ptr<NodeBase>> m_statements;
    };

    // **BNF** EXPR ::= EXPRTERM {EXPROP EXPRTERM}
    class Node_Expr final : NodeBase
    {
    public:
        struct OpTerm
        {
            TokenView op; // MATHOP | COMPOP | LOGICOP | BITOP
            std::unique_ptr<Node_ExprTerm> term;
        };

        explicit Node_Expr(SourceSpan span = {}) : NodeBase(NodeKind::Expr, span) {}

        std::unique_ptr<Node_ExprTerm>& first()
        {
            return m_first;
        }

        const std::unique_ptr<Node_ExprTerm>& first() const
        {
            return m_first;
        }

        std::vector<OpTerm>& rest()
        {
            return m_rest;
        }

        const std::vector<OpTerm>& rest() const
        {
            return m_rest;
        }

    private:
        std::unique_ptr<Node_ExprTerm> m_first;
        std::vector<OpTerm> m_rest;
    };

    // **BNF** EXPRTERM ::= ([TYPE '='] INITLIST) | ({EXPRPREOP} EXPRVALUE {EXPRPOSTOP})
    class Node_ExprTerm final : NodeBase
    {
    public:
        enum class Form
        {
            InitListForm, // [TYPE '='] INITLIST
            ExprValueForm, // {EXPRPREOP} EXPRVALUE {EXPRPOSTOP}
        };

        explicit Node_ExprTerm(SourceSpan span = {}) : NodeBase(NodeKind::ExprTerm, span) {}

        Form& form()
        {
            return m_form;
        }

        Form form() const
        {
            return m_form;
        }

        // --- InitListForm ---
        // m_initType: null の場合は TYPE なし (= bare INITLIST)
        std::unique_ptr<Node_Type>& initType()
        {
            return m_initType;
        }

        const std::unique_ptr<Node_Type>& initType() const
        {
            return m_initType;
        }

        std::unique_ptr<Node_InitList>& initList()
        {
            return m_initList;
        }

        const std::unique_ptr<Node_InitList>& initList() const
        {
            return m_initList;
        }

        // --- ExprValueForm ---
        // **BNF** EXPRPREOP ::= '-' | '+' | '!' | '++' | '--' | '~' | '@'
        std::vector<TokenView>& preOps()
        {
            return m_preOps;
        }

        const std::vector<TokenView>& preOps() const
        {
            return m_preOps;
        }

        // **BNF** EXPRVALUE ::= 'void' | CONSTRUCTORCALL | FUNCCALL | VARACCESS | CAST | LITERAL | '(' ASSIGN ')' | LAMBDA
        std::unique_ptr<NodeBase>& exprValue()
        {
            return m_exprValue;
        }

        const std::unique_ptr<NodeBase>& exprValue() const
        {
            return m_exprValue;
        }

        std::vector<std::unique_ptr<Node_ExprPostOp>>& postOps()
        {
            return m_postOps;
        }

        const std::vector<std::unique_ptr<Node_ExprPostOp>>& postOps() const
        {
            return m_postOps;
        }

    private:
        Form m_form = Form::ExprValueForm;
        // InitListForm
        std::unique_ptr<Node_Type> m_initType;
        std::unique_ptr<Node_InitList> m_initList;
        // ExprValueForm
        std::vector<TokenView> m_preOps;
        std::unique_ptr<NodeBase> m_exprValue;
        std::vector<std::unique_ptr<Node_ExprPostOp>> m_postOps;
    };

    // **BNF** CONSTRUCTORCALL ::= TYPE ARGLIST
    class Node_ConstructorCall final : NodeBase
    {
    public:
        explicit Node_ConstructorCall(SourceSpan span = {}) : NodeBase(NodeKind::ConstructorCall, span) {}

        std::unique_ptr<Node_Type>& type()
        {
            return m_type;
        }

        const std::unique_ptr<Node_Type>& type() const
        {
            return m_type;
        }

        std::unique_ptr<Node_ArgList>& args()
        {
            return m_args;
        }

        const std::unique_ptr<Node_ArgList>& args() const
        {
            return m_args;
        }

    private:
        std::unique_ptr<Node_Type> m_type;
        std::unique_ptr<Node_ArgList> m_args;
    };

    // **BNF** EXPRPREOP ::= '-' | '+' | '!' | '++' | '--' | '~' | '@'
    // (EXPRPREOP は Node_ExprTerm 内の TokenView リストとして表現)

    // **BNF** EXPRPOSTOP ::= ('.' (FUNCCALL | IDENTIFIER)) | ('[' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ']') | ARGLIST | '++' | '--'
    class Node_ExprPostOp final : NodeBase
    {
    public:
        enum class Kind
        {
            Dot, // '.' (FUNCCALL | IDENTIFIER)
            Subscript, // '[' ... ']'
            Call, // ARGLIST
            Increment, // '++'
            Decrement, // '--'
        };

        struct IndexArg
        {
            TokenView name; // empty = 位置引数
            std::unique_ptr<Node_Assign> value;
        };

        explicit Node_ExprPostOp(SourceSpan span = {}) : NodeBase(NodeKind::ExprPostOp, span) {}

        Kind& opKind()
        {
            return m_opKind;
        }

        Kind opKind() const
        {
            return m_opKind;
        }

        // Kind::Dot — FUNCCALL のときは dotAccess() に Node_FuncCall、IDENTIFIER のときは dotIdentifier()
        std::unique_ptr<NodeBase>& dotAccess()
        {
            return m_dotAccess;
        }

        const std::unique_ptr<NodeBase>& dotAccess() const
        {
            return m_dotAccess;
        }

        TokenView& dotIdentifier()
        {
            return m_dotIdentifier;
        }

        const TokenView& dotIdentifier() const
        {
            return m_dotIdentifier;
        }

        // Kind::Subscript
        std::vector<IndexArg>& indexArgs()
        {
            return m_indexArgs;
        }

        const std::vector<IndexArg>& indexArgs() const
        {
            return m_indexArgs;
        }

        // Kind::Call
        std::unique_ptr<Node_ArgList>& callArgs()
        {
            return m_callArgs;
        }

        const std::unique_ptr<Node_ArgList>& callArgs() const
        {
            return m_callArgs;
        }

    private:
        Kind m_opKind = Kind::Dot;
        std::unique_ptr<NodeBase> m_dotAccess;
        TokenView m_dotIdentifier;
        std::vector<IndexArg> m_indexArgs;
        std::unique_ptr<Node_ArgList> m_callArgs;
    };

    // **BNF** CAST ::= 'cast' '<' TYPE '>' '(' ASSIGN ')'
    class Node_Cast final : NodeBase
    {
    public:
        explicit Node_Cast(SourceSpan span = {}) : NodeBase(NodeKind::Cast, span) {}

        std::unique_ptr<Node_Type>& type()
        {
            return m_type;
        }

        const std::unique_ptr<Node_Type>& type() const
        {
            return m_type;
        }

        std::unique_ptr<Node_Assign>& expr()
        {
            return m_expr;
        }

        const std::unique_ptr<Node_Assign>& expr() const
        {
            return m_expr;
        }

    private:
        std::unique_ptr<Node_Type> m_type;
        std::unique_ptr<Node_Assign> m_expr;
    };

    // **BNF** LAMBDA ::= 'function' '(' [LAMBDAPARAM {',' LAMBDAPARAM}] ')' STATBLOCK
    class Node_Lambda final : NodeBase
    {
    public:
        explicit Node_Lambda(SourceSpan span = {}) : NodeBase(NodeKind::Lambda, span) {}

        const std::vector<std::unique_ptr<Node_LambdaParam>>& params() const
        {
            return m_params;
        }

        void addParam(std::unique_ptr<Node_LambdaParam> p)
        {
            m_params.push_back(std::move(p));
        }

        std::unique_ptr<Node_StatBlock>& body()
        {
            return m_body;
        }

        const std::unique_ptr<Node_StatBlock>& body() const
        {
            return m_body;
        }

    private:
        std::vector<std::unique_ptr<Node_LambdaParam>> m_params;
        std::unique_ptr<Node_StatBlock> m_body;
    };

    // **BNF** LAMBDAPARAM ::= [TYPE TYPEMODIFIER] [IDENTIFIER]
    class Node_LambdaParam final : NodeBase
    {
    public:
        explicit Node_LambdaParam(SourceSpan span = {}) : NodeBase(NodeKind::LambdaParam, span) {}

        std::unique_ptr<Node_Type>& type()
        {
            return m_type;
        } // null = 型省略

        const std::unique_ptr<Node_Type>& type() const
        {
            return m_type;
        }

        TypeModifier& typeModifier()
        {
            return m_typeModifier;
        }

        const TypeModifier& typeModifier() const
        {
            return m_typeModifier;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

    private:
        std::unique_ptr<Node_Type> m_type;
        TypeModifier m_typeModifier;
        TokenView m_identifier;
    };

    // **BNF** LITERAL ::= NUMBER | STRING | BITS | 'true' | 'false' | 'null'
    class Node_Literal final : NodeBase
    {
    public:
        explicit Node_Literal(SourceSpan span = {}) : NodeBase(NodeKind::Literal, span) {}

        TokenView& token()
        {
            return m_token;
        }

        const TokenView& token() const
        {
            return m_token;
        }

    private:
        TokenView m_token;
    };

    // **BNF** FUNCCALL ::= SCOPE IDENTIFIER ['<' TYPE {',' TYPE} '>'] ARGLIST
    class Node_FuncCall final : NodeBase
    {
    public:
        explicit Node_FuncCall(SourceSpan span = {}) : NodeBase(NodeKind::FuncCall, span) {}

        std::unique_ptr<Node_Scope>& scope()
        {
            return m_scope;
        }

        const std::unique_ptr<Node_Scope>& scope() const
        {
            return m_scope;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

        std::vector<std::unique_ptr<Node_Type>>& templateArgs()
        {
            return m_templateArgs;
        }

        const std::vector<std::unique_ptr<Node_Type>>& templateArgs() const
        {
            return m_templateArgs;
        }

        std::unique_ptr<Node_ArgList>& args()
        {
            return m_args;
        }

        const std::unique_ptr<Node_ArgList>& args() const
        {
            return m_args;
        }

    private:
        std::unique_ptr<Node_Scope> m_scope;
        TokenView m_identifier;
        std::vector<std::unique_ptr<Node_Type>> m_templateArgs;
        std::unique_ptr<Node_ArgList> m_args;
    };

    // **BNF** VARACCESS ::= SCOPE IDENTIFIER
    class Node_VarAccess final : NodeBase
    {
    public:
        explicit Node_VarAccess(SourceSpan span = {}) : NodeBase(NodeKind::VarAccess, span) {}

        std::unique_ptr<Node_Scope>& scope()
        {
            return m_scope;
        }

        const std::unique_ptr<Node_Scope>& scope() const
        {
            return m_scope;
        }

        TokenView& identifier()
        {
            return m_identifier;
        }

        const TokenView& identifier() const
        {
            return m_identifier;
        }

    private:
        std::unique_ptr<Node_Scope> m_scope;
        TokenView m_identifier;
    };

    // **BNF** ARGLIST ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'
    class Node_ArgList final : NodeBase
    {
    public:
        struct Arg
        {
            TokenView name; // empty = 位置引数
            std::unique_ptr<Node_Assign> value;
        };

        explicit Node_ArgList(SourceSpan span = {}) : NodeBase(NodeKind::ArgList, span) {}

        std::vector<Arg>& args()
        {
            return m_args;
        }

        const std::vector<Arg>& args() const
        {
            return m_args;
        }

    private:
        std::vector<Arg> m_args;
    };

    // **BNF** ASSIGN ::= CONDITION [ ASSIGNOP ASSIGN ]
    class Node_Assign final : NodeBase
    {
    public:
        explicit Node_Assign(SourceSpan span = {}) : NodeBase(NodeKind::Assign, span) {}

        std::unique_ptr<Node_Condition>& cond()
        {
            return m_cond;
        }

        const std::unique_ptr<Node_Condition>& cond() const
        {
            return m_cond;
        }

        // **BNF** ASSIGNOP ::= '=' | '+=' | '-=' | '*=' | '/=' | '|=' | '&=' | '^=' | '%=' | '**=' | '<<=' | '>>=' | '>>>='
        TokenView& op()
        {
            return m_op;
        } // empty = op なし

        const TokenView& op() const
        {
            return m_op;
        }

        std::unique_ptr<Node_Assign>& rhs()
        {
            return m_rhs;
        } // null = rhs なし

        const std::unique_ptr<Node_Assign>& rhs() const
        {
            return m_rhs;
        }

    private:
        std::unique_ptr<Node_Condition> m_cond;
        TokenView m_op;
        std::unique_ptr<Node_Assign> m_rhs;
    };

    // **BNF** CONDITION ::= EXPR ['?' ASSIGN ':' ASSIGN]
    class Node_Condition final : NodeBase
    {
    public:
        explicit Node_Condition(SourceSpan span = {}) : NodeBase(NodeKind::Condition, span) {}

        std::unique_ptr<Node_Expr>& expr()
        {
            return m_expr;
        }

        const std::unique_ptr<Node_Expr>& expr() const
        {
            return m_expr;
        }

        std::unique_ptr<Node_Assign>& thenExpr()
        {
            return m_then;
        } // null = 三項なし

        const std::unique_ptr<Node_Assign>& thenExpr() const
        {
            return m_then;
        }

        std::unique_ptr<Node_Assign>& elseExpr()
        {
            return m_else;
        }

        const std::unique_ptr<Node_Assign>& elseExpr() const
        {
            return m_else;
        }

    private:
        std::unique_ptr<Node_Expr> m_expr;
        std::unique_ptr<Node_Assign> m_then;
        std::unique_ptr<Node_Assign> m_else;
    };

} // namespace light_angel
