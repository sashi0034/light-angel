#include "sir_builder_context.h"

#include <utility>

namespace light_angel
{
    SirSymbolId SirBuilderContext::lookupName(const std::string& name)
    {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
        {
            auto found = it->names.find(name);
            if (found != it->names.end()) return found->second;
        }

        return InvalidSymbol;
    }

    std::string SirBuilderContext::spanText(SourceSpan span) const
    {
        if (span.offset >= source.size()) return {};

        size_t end = static_cast<size_t>(span.offset) + static_cast<size_t>(span.length);
        if (end > source.size()) end = source.size();

        return std::string(source.substr(span.offset, end - span.offset));
    }

    std::string SirBuilderContext::getTokenText(const TokenView& tok) const
    {
        return spanText(tok.span());
    }

    SirExprId SirBuilderContext::makeErrorExpr(SourceSpan span)
    {
        SirExpr e;
        e.span = span;
        e.type = module.errorType;
        e.data = SirErrorExpr{};
        return module.addExpr(std::move(e));
    }

    SirStatementId SirBuilderContext::makeErrorStatement(SourceSpan span)
    {
        SirStatement s;
        s.span = span;
        s.data = SirErrorStatement{};
        return module.addStatement(std::move(s));
    }

    SirExprId SirBuilderContext::makeImplicitCastIfNeeded(SirExprId src, SirTypeId target, SourceSpan span)
    {
        // TODO: full implicit conversion rules. For now only wrap when types differ
        // and neither is the error type. The cast leaves overload semantics to a
        // later pass.
        if (src == InvalidExpr || target == InvalidType) return src;

        SirTypeId srcType = module.exprs[src].type;
        if (srcType == target) return src;

        if (srcType == module.errorType || target == module.errorType) return src;

        SirExpr cast;
        cast.span = span;
        cast.type = target;
        cast.data = SirImplicitCastExpr{src};
        return module.addExpr(std::move(cast));
    }

    void SirBuilderContext::reportDiagnostic(SourceSpan span, std::string msg)
    {
        Diagnostic d;
        d.severity = Diagnostic::Severity::Error;
        d.span = span;
        d.message = std::move(msg);
        diagnostics.push_back(std::move(d));
    }

    void SirBuilderContext::pushScope()
    {
        scopes.emplace_back();
    }

    void SirBuilderContext::popScope()
    {
        scopes.pop_back();
    }

    void SirBuilderContext::declareInScope(const std::string& name, SirSymbolId id)
    {
        if (scopes.empty()) return;

        scopes.back().names[name] = id;
    }

    void SirBuilderContext::initBuiltinTypes()
    {
        auto addSimple = [&](SirTypeKind kind, const char* name)
        {
            SirTypeInfo t;
            t.kind = kind;
            t.name = name;
            return module.addType(std::move(t));
        };
        auto addPrimitive = [&](PrimitiveTypeKind pk, const char* name)
        {
            SirTypeInfo t;
            t.kind = SirTypeKind::Primitive;
            t.name = name;
            SirTypeInfo::primitive_data pd;
            pd.primitiveKind = pk;
            t.data = pd;
            return module.addType(std::move(t));
        };
        module.errorType = addSimple(SirTypeKind::Error, "<error>");
        module.voidType = addSimple(SirTypeKind::Void, "void");
        module.boolType = addPrimitive(PrimitiveTypeKind::Bool, "bool");
        module.intType = addPrimitive(PrimitiveTypeKind::Int, "int");
        module.int8Type = addPrimitive(PrimitiveTypeKind::Int8, "int8");
        module.int16Type = addPrimitive(PrimitiveTypeKind::Int16, "int16");
        module.int32Type = addPrimitive(PrimitiveTypeKind::Int32, "int32");
        module.int64Type = addPrimitive(PrimitiveTypeKind::Int64, "int64");
        module.uintType = addPrimitive(PrimitiveTypeKind::UInt, "uint");
        module.uint8Type = addPrimitive(PrimitiveTypeKind::UInt8, "uint8");
        module.uint16Type = addPrimitive(PrimitiveTypeKind::UInt16, "uint16");
        module.uint32Type = addPrimitive(PrimitiveTypeKind::UInt32, "uint32");
        module.uint64Type = addPrimitive(PrimitiveTypeKind::UInt64, "uint64");
        module.floatType = addPrimitive(PrimitiveTypeKind::Float, "float");
        module.doubleType = addPrimitive(PrimitiveTypeKind::Double, "double");
        module.stringType = addSimple(SirTypeKind::NativeType, "string");
        module.nullType = addSimple(SirTypeKind::Null, "null_t");
    }

    SirTypeId SirBuilderContext::getBuiltinType(str_view keyword) const
    {
        if (keyword == "void") return module.voidType;

        if (keyword == "bool") return module.boolType;

        if (keyword == "int") return module.intType;
        if (keyword == "int8") return module.int8Type;
        if (keyword == "int16") return module.int16Type;
        if (keyword == "int32") return module.int32Type;
        if (keyword == "int64") return module.int64Type;

        if (keyword == "uint") return module.uintType;
        if (keyword == "uint8") return module.uint8Type;
        if (keyword == "uint16") return module.uint16Type;
        if (keyword == "uint32") return module.uint32Type;
        if (keyword == "uint64") return module.uint64Type;

        if (keyword == "float") return module.floatType;

        if (keyword == "double") return module.doubleType;

        if (keyword == "string") return module.stringType;

        return InvalidType;
    }
} // namespace light_angel
