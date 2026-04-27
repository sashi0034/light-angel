#pragma once
#include <cstdint>
#include <string>

namespace light_angel
{
    struct SourceSpan
    {
        uint32_t offset;
        uint32_t length;
    };

    struct LineColumn
    {
        uint32_t line;
        uint32_t column;
    };

    class TokenView
    {
    public:
        TokenView() = default;

        TokenView(SourceSpan span);

    private:
        SourceSpan m_span{};

#if defined(LIGHT_ANGEL_DEBUG)
        std::string m_text;
#endif
    };

    enum class TokenKind : uint8_t
    {
        Name, // identifier or keyword
        Number,
        String,
        Punctuator,
        Comment,
        EndOfFile,
    };

    struct LexicalToken
    {
        TokenKind kind;
        TokenView view;
    };
} // namespace light_angel
