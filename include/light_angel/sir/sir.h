#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "light_angel/parser/token.h"

namespace light_angel
{
    using SirTypeId = uint32_t;
    using SirSymbolId = uint32_t;
    using SirFunctionId = uint32_t;
    using SirBlockId = uint32_t;
    using SirStatementId = uint32_t;
    using SirExprId = uint32_t;

    constexpr SirTypeId InvalidType = UINT32_MAX;
    constexpr SirSymbolId InvalidSymbol = UINT32_MAX;
    constexpr SirFunctionId InvalidFunction = UINT32_MAX;
    constexpr SirBlockId InvalidBlock = UINT32_MAX;
    constexpr SirStatementId InvalidStatement = UINT32_MAX;
    constexpr SirExprId InvalidExpr = UINT32_MAX;

    // -----------------------------------------------
    // Type info
    // -----------------------------------------------

    enum class SirTypeKind : uint8_t
    {
        Error,
        Void,
        Bool,
        Int,
        Int8,
        Int16,
        Int32,
        Int64,
        UInt,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Float,
        Double,
        Null,
        Class,
        Interface,
        Enum,
        Array, // FIXME: Remove this?
        Handle,
        Function,
        NativeType,
        ScriptType,
    };

    struct SirTypeInfo
    {
        // No payload. Used by primitive-like kinds (Error/Void/Bool/integer widths/
        // Float/Double/Null/NativeType/ScriptType).
        struct primitive_props
        {
        };

        struct class_props
        {
            // Non-empty for generic class instantiations.
            std::vector<SirTypeId> typeArgs;
        };

        struct interface_props
        {
        };

        struct enum_props
        {
        };

        struct array_props
        {
            SirTypeId elementType = InvalidType;
        };

        struct handle_props
        {
            SirTypeId elementType = InvalidType;
            bool isConstHandle = false;
        };

        struct function_props
        {
            // TODO: Use SirFunctionId instead?
            std::vector<SirTypeId> parameterTypes;
            SirTypeId returnType = InvalidType;
        };

        using Data = std::variant<
            primitive_props,
            class_props,
            interface_props,
            enum_props,
            array_props,
            handle_props,
            function_props>;

        SirTypeKind kind = SirTypeKind::Error;
        std::string name;

        SirSymbolId symbol = InvalidSymbol;

        bool isConst = false;

        Data data = primitive_props{};
    };

    // -----------------------------------------------
    // Symbols
    // -----------------------------------------------

    enum class SirSymbolKind : uint8_t
    {
        Error,
        Namespace, // FIXME?
        Type,
        Function,
        GlobalVar,
        LocalVar,
        Parameter,
        Field,
    };

    struct SirSymbol
    {
        SirSymbolKind kind = SirSymbolKind::Error;
        std::string name;
        SourceSpan span{};

        SirTypeId type = InvalidType;

        // Optional cross-references depending on kind.
        SirFunctionId function = InvalidFunction; // Function
    };

    // -----------------------------------------------
    // Functions, Global Variables
    // -----------------------------------------------

    struct SirParameter
    {
        std::string name;
        SourceSpan span{};

        SirSymbolId symbol = InvalidSymbol;
        SirTypeId type = InvalidType;

        RefDirection refDir = RefDirection::None; // "in", "out", "inout"
    };

    struct SirFunction
    {
        std::string name;
        SourceSpan span{};

        SirSymbolId symbol = InvalidSymbol;
        SirTypeId functionType = InvalidType;
        SirTypeId returnType = InvalidType;

        std::vector<SirParameter> parameters;
        SirBlockId body = InvalidBlock;

        bool isMethod = false;
        SirTypeId receiverType = InvalidType;

        bool isConstructor = false;
        bool isDestructor = false;
        bool isExternal = false;
    };

    struct SirGlobalVar
    {
        std::string name;
        SourceSpan span{};

        SirSymbolId symbol = InvalidSymbol;
        SirTypeId type = InvalidType;
        SirExprId initializer = InvalidExpr;
    };

    // -----------------------------------------------
    // Expressions
    // -----------------------------------------------

    struct SirErrorExpr
    {
    };

    enum class SirLiteralKind : uint8_t
    {
        Int,
        UInt,
        Float,
        Double,
        Bool,
        String,
        Null,
        Void,
    };

    struct SirLiteralExpr
    {
        // TODO: Use a variant?
        SirLiteralKind kind = SirLiteralKind::Int;
        int64_t intValue = 0;
        uint64_t uintValue = 0;
        double floatValue = 0.0;
        bool boolValue = false;
        std::string stringValue;
    };

    // Reference to a resolved symbol (local var, parameter, global, function name, ...).
    struct SirSymbolRefExpr
    {
        SirSymbolId symbol = InvalidSymbol;
    };

    // Plain assignment '='. Compound assigns are normalized to (target = target op value).
    struct SirAssignExpr
    {
        SirExprId target = InvalidExpr;
        SirExprId value = InvalidExpr;
    };

    enum class SirBinaryOp : uint8_t
    {
        // arithmetic
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Pow,
        // comparison
        Eq,
        Ne,
        Lt,
        Le,
        Gt,
        Ge,
        Is,
        NotIs,
        // logical
        LAnd,
        LOr,
        LXor,
        // bitwise
        BAnd,
        BOr,
        BXor,
        Shl,
        Shr,
        UShr,
    };

    struct SirBinaryExpr
    {
        SirBinaryOp op = SirBinaryOp::Add;
        SirExprId lhs = InvalidExpr;
        SirExprId rhs = InvalidExpr;

        // If non-builtin, this points to the resolved overloaded function.
        // TODO: overload resolution not yet implemented.
        SirFunctionId overloadedFunction = InvalidFunction;
    };

    enum class SirUnaryOp : uint8_t
    {
        Neg,
        Plus,
        Not,
        BitNot,
        PreInc,
        PreDec,
        PostInc,
        PostDec,
        AddrOf, // '@'
    };

    struct SirUnaryExpr
    {
        SirUnaryOp op = SirUnaryOp::Neg;
        SirExprId operand = InvalidExpr;
        SirFunctionId overloadedFunction = InvalidFunction;
    };

    struct SirCallExpr
    {
        // TODO: overload resolution may set this to InvalidFunction temporarily.
        SirFunctionId function = InvalidFunction;
        std::vector<SirExprId> args;
    };

    struct SirMethodCallExpr
    {
        SirExprId receiver = InvalidExpr;
        SirFunctionId function = InvalidFunction;
        std::vector<SirExprId> args;
    };

    struct SirMemberAccessExpr
    {
        SirExprId object = InvalidExpr;
        SirSymbolId field = InvalidSymbol;
    };

    // Implicit conversion. Target type is the SirExpr::type of the wrapping expression.
    struct SirImplicitCastExpr
    {
        SirExprId source = InvalidExpr;
    };

    using SirExprData = std::variant<
        SirErrorExpr,
        SirLiteralExpr,
        SirSymbolRefExpr,
        SirAssignExpr,
        SirBinaryExpr,
        SirUnaryExpr,
        SirCallExpr,
        SirMethodCallExpr,
        SirMemberAccessExpr,
        SirImplicitCastExpr>;

    struct SirExpr
    {
        SourceSpan span{};
        SirTypeId type = InvalidType;
        SirExprData data;
    };

    // -----------------------------------------------
    // Statements
    // -----------------------------------------------

    struct SirErrorStatement
    {
    };

    struct SirBlockStatement // TODO: Rename to SirStatementBlock? Or Remove this and just use SirBlock directly as a statement?
    {
        SirBlockId block = InvalidBlock;
    };

    struct SirVarDeclStatement
    {
        SirSymbolId symbol = InvalidSymbol;
        SirTypeId type = InvalidType;
        SirExprId initializer = InvalidExpr; // may be InvalidExpr
    };

    struct SirExprStatement
    {
        SirExprId expr = InvalidExpr;
    };

    // STATEMENT ::= (IF | FOR | FOREACH | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)

    struct SirIfStatement
    {
        SirExprId condition = InvalidExpr;
        SirBlockId thenBlock = InvalidBlock;
        SirBlockId elseBlock = InvalidBlock; // may be InvalidBlock
    };

    struct SirForStatement
    {
        SirStatementId initializer = InvalidStatement; // may be InvalidStatement
        SirExprId condition = InvalidExpr; // may be InvalidExpr
        std::vector<SirExprId> increments;
        SirBlockId body = InvalidBlock;
    };

    struct SirForEachStatement
    {
    };

    struct SirWhileStatement
    {
        SirExprId condition = InvalidExpr;
        SirBlockId body = InvalidBlock;
    };

    struct SirReturnStatement
    {
        SirExprId value = InvalidExpr; // may be InvalidExpr (void return)
    };

    struct SirBreakStatement
    {
    };

    struct SirContinueStatement
    {
    };

    struct SirDoWhileStatement
    {
        SirBlockId body = InvalidBlock;
        SirExprId condition = InvalidExpr;
    };

    struct SirSwitchStatement
    {
    };

    struct SirTryStatement
    {
    };

    using SirStatementData = std::variant<
        SirErrorStatement,
        SirBlockStatement,
        SirVarDeclStatement,
        SirExprStatement,
        SirIfStatement,
        SirForStatement,
        SirForEachStatement,
        SirWhileStatement,
        SirReturnStatement,
        SirBreakStatement,
        SirContinueStatement,
        SirDoWhileStatement,
        SirSwitchStatement,
        SirTryStatement>;

    struct SirStatement
    {
        SourceSpan span{};
        SirStatementData data;
    };

    // -----------------------------------------------
    // Block
    // -----------------------------------------------

    struct SirBlock
    {
        SourceSpan span{};
        std::vector<SirStatementId> statements;
    };

    // -----------------------------------------------
    // Module
    // -----------------------------------------------

    struct SirModule
    {
        std::vector<SirTypeInfo> types;
        std::vector<SirSymbol> symbols;
        std::vector<SirFunction> functions;
        std::vector<SirGlobalVar> globals;

        std::vector<SirBlock> blocks;
        std::vector<SirStatement> statements;
        std::vector<SirExpr> exprs;

        // Built-in type id cache. Populated by the builder up-front.
        SirTypeId errorType = InvalidType;
        SirTypeId voidType = InvalidType;
        SirTypeId boolType = InvalidType;
        SirTypeId intType = InvalidType;
        SirTypeId int8Type = InvalidType;
        SirTypeId int16Type = InvalidType;
        SirTypeId int32Type = InvalidType;
        SirTypeId int64Type = InvalidType;
        SirTypeId uintType = InvalidType;
        SirTypeId uint8Type = InvalidType;
        SirTypeId uint16Type = InvalidType;
        SirTypeId uint32Type = InvalidType;
        SirTypeId uint64Type = InvalidType;
        SirTypeId floatType = InvalidType;
        SirTypeId doubleType = InvalidType;
        SirTypeId stringType = InvalidType;
        SirTypeId nullType = InvalidType;

        SirTypeId addType(SirTypeInfo type)
        {
            types.push_back(std::move(type));
            return static_cast<SirTypeId>(types.size() - 1u);
        }

        SirSymbolId addSymbol(SirSymbol symbol)
        {
            symbols.push_back(std::move(symbol));
            return static_cast<SirSymbolId>(symbols.size() - 1u);
        }

        SirFunctionId addFunction(SirFunction function)
        {
            functions.push_back(std::move(function));
            return static_cast<SirFunctionId>(functions.size() - 1u);
        }

        SirBlockId addBlock(SirBlock block)
        {
            blocks.push_back(std::move(block));
            return static_cast<SirBlockId>(blocks.size() - 1u);
        }

        SirStatementId addStatement(SirStatement statement)
        {
            statements.push_back(std::move(statement));
            return static_cast<SirStatementId>(statements.size() - 1u);
        }

        SirExprId addExpr(SirExpr expr)
        {
            exprs.push_back(std::move(expr));
            return static_cast<SirExprId>(exprs.size() - 1u);
        }
    };

    struct SirBuildResult
    {
        SirModule module;
        std::vector<Diagnostic> diagnostics;

        bool ok() const
        {
            return diagnostics.empty();
        }
    };
} // namespace light_angel
