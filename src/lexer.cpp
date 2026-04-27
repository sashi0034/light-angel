#include "lexer.h"

namespace
{
    using namespace light_angel;

    bool isDigit(char_t c)
    {
        return c >= '0' && c <= '9';
    }

    bool isHexChar(char_t c)
    {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    bool isAlpha(char_t c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    bool isAlnum(char_t c)
    {
        return isAlpha(c) || isDigit(c);
    }

    bool isHighByte(char_t c)
    {
        return static_cast<unsigned char>(c) >= 0x80;
    }

    bool isIdentChar(char_t c)
    {
        return isAlnum(c) || isHighByte(c);
    }

    LexicalToken makeToken(TokenKind kind, uint32_t start, const LexerState& lex)
    {
        return LexicalToken{kind, TokenView{SourceSpan{start, lex.offset() - start}}};
    }

    // Returns true if a comment was consumed. Comments are skipped entirely.
    bool trySkipComment(LexerState& lex)
    {
        if (lex.startsWith("//"))
        {
            lex.advance(2);
            while (!lex.isEnd() && !lex.isNextLineBreak())
                lex.advance();
            return true;
        }
        if (lex.startsWith("/*"))
        {
            lex.advance(2);
            while (!lex.isEnd())
            {
                if (lex.startsWith("*/"))
                {
                    lex.advance(2);
                    break;
                }
                lex.advance();
            }
            return true;
        }
        return false;
    }

    // NUMBER ::= decimal | 0b binary | 0o octal | 0d decimal | 0x hex | floating-point
    bool tryLexNumber(LexerState& lex, LexicalToken& out)
    {
        const char_t c = lex.peek();
        if (!isDigit(c) && !(c == '.' && isDigit(lex.peekAt(1))))
            return false;

        const uint32_t start = lex.offset();

        if (c == '0')
        {
            const char_t prefix = lex.peekAt(1);
            if (prefix == 'b' || prefix == 'B')
            {
                lex.advance(2);
                while (!lex.isEnd() && (lex.peek() == '0' || lex.peek() == '1'))
                    lex.advance();
                out = makeToken(TokenKind::Number, start, lex);
                return true;
            }
            if (prefix == 'o' || prefix == 'O')
            {
                lex.advance(2);
                while (!lex.isEnd() && lex.peek() >= '0' && lex.peek() <= '7')
                    lex.advance();
                out = makeToken(TokenKind::Number, start, lex);
                return true;
            }
            if (prefix == 'd' || prefix == 'D')
            {
                lex.advance(2);
                while (!lex.isEnd() && isDigit(lex.peek()))
                    lex.advance();
                out = makeToken(TokenKind::Number, start, lex);
                return true;
            }
            if (prefix == 'x' || prefix == 'X')
            {
                lex.advance(2);
                while (!lex.isEnd() && isHexChar(lex.peek()))
                    lex.advance();
                out = makeToken(TokenKind::Number, start, lex);
                return true;
            }
        }

        // Consume integer part
        while (!lex.isEnd() && isDigit(lex.peek()))
            lex.advance();

        bool isFloat = false;

        // Fractional part: either N.M or .M (leading dot)
        if (lex.peek() == '.' && isDigit(lex.peekAt(1)))
        {
            isFloat = true;
            lex.advance(); // consume '.'
            while (!lex.isEnd() && isDigit(lex.peek()))
                lex.advance();
        }

        // Exponent: e/E optionally followed by +/-, then digits
        const char_t e = lex.peek();
        if (e == 'e' || e == 'E')
        {
            const char_t sign = lex.peekAt(1);
            if (isDigit(sign))
            {
                isFloat = true;
                lex.advance(2);
                while (!lex.isEnd() && isDigit(lex.peek()))
                    lex.advance();
            }
            else if ((sign == '+' || sign == '-') && isDigit(lex.peekAt(2)))
            {
                isFloat = true;
                lex.advance(3);
                while (!lex.isEnd() && isDigit(lex.peek()))
                    lex.advance();
            }
        }

        // Float suffix f/F
        if (isFloat && (lex.peek() == 'f' || lex.peek() == 'F'))
            lex.advance();

        out = makeToken(TokenKind::Number, start, lex);
        return true;
    }

    // STRING ::= '...' | "..." | """..."""
    bool tryLexString(LexerState& lex, LexicalToken& out)
    {
        const char_t c = lex.peek();
        if (c != '"' && c != '\'')
            return false;

        const uint32_t start = lex.offset();

        if (c == '"' && lex.startsWith("\"\"\""))
        {
            // Heredoc: triple-quoted, allows newlines
            lex.advance(3);
            for (;;)
            {
                if (lex.isEnd())
                    break;
                if (lex.startsWith("\"\"\""))
                {
                    lex.advance(3);
                    break;
                }
                lex.advance();
            }
        }
        else
        {
            const char_t quote = c;
            lex.advance(); // opening quote
            bool escaping = false;
            for (;;)
            {
                if (lex.isEnd() || lex.isNextLineBreak())
                    break;
                if (!escaping && lex.peek() == quote)
                {
                    lex.advance();
                    break;
                }
                escaping = (lex.peek() == '\\') && !escaping;
                lex.advance();
            }
        }

        out = makeToken(TokenKind::String, start, lex);
        return true;
    }

    // PUNCTUATOR: greedy longest-match
    bool tryLexPunctuator(LexerState& lex, LexicalToken& out)
    {
        const uint32_t start = lex.offset();
        uint32_t len = 0;

        switch (lex.peek())
        {
        case '!':
            if (lex.startsWith("!is") && !isIdentChar(lex.peekAt(3)))
                len = 3; // !is operator
            else if (lex.peekAt(1) == '=')
                len = 2; // !=
            else
                len = 1; // !
            break;
        case '#':
            len = 1;
            break;
        case '%':
            len = (lex.peekAt(1) == '=') ? 2 : 1; // %= or %
            break;
        case '&':
            if (lex.peekAt(1) == '=')
                len = 2; // &=
            else if (lex.peekAt(1) == '&')
                len = 2; // &&
            else
                len = 1; // &
            break;
        case '(':
        case ')':
        case ',':
        case ';':
        case '?':
        case '@':
        case '[':
        case ']':
        case '{':
        case '}':
        case '~':
            len = 1;
            break;
        case '*':
            if (lex.startsWith("**="))
                len = 3; // **=
            else if (lex.peekAt(1) == '*')
                len = 2; // **
            else if (lex.peekAt(1) == '=')
                len = 2; // *=
            else
                len = 1; // *
            break;
        case '+':
            if (lex.peekAt(1) == '=')
                len = 2; // +=
            else if (lex.peekAt(1) == '+')
                len = 2; // ++
            else
                len = 1; // +
            break;
        case '-':
            if (lex.peekAt(1) == '=')
                len = 2; // -=
            else if (lex.peekAt(1) == '-')
                len = 2; // --
            else
                len = 1; // -
            break;
        case '.':
            if (lex.startsWith("..."))
                len = 3; // ...
            else
                len = 1; // .
            break;
        case '/':
            len = (lex.peekAt(1) == '=') ? 2 : 1; // /= or /
            break;
        case ':':
            len = (lex.peekAt(1) == ':') ? 2 : 1; // :: or :
            break;
        case '<':
            if (lex.startsWith("<<="))
                len = 3; // <<=
            else if (lex.peekAt(1) == '=')
                len = 2; // <=
            else if (lex.peekAt(1) == '<')
                len = 2; // <<
            else
                len = 1; // <
            break;
        case '=':
            len = (lex.peekAt(1) == '=') ? 2 : 1; // == or =
            break;
        case '>':
            if (lex.startsWith(">>>="))
                len = 4; // >>>=
            else if (lex.startsWith(">>="))
                len = 3; // >>=
            else if (lex.startsWith(">>>"))
                len = 3; // >>>
            else if (lex.peekAt(1) == '=')
                len = 2; // >=
            else if (lex.peekAt(1) == '>')
                len = 2; // >>
            else
                len = 1; // >
            break;
        case '^':
            if (lex.peekAt(1) == '=')
                len = 2; // ^=
            else if (lex.peekAt(1) == '^')
                len = 2; // ^^
            else
                len = 1; // ^
            break;
        case '|':
            if (lex.peekAt(1) == '=')
                len = 2; // |=
            else if (lex.peekAt(1) == '|')
                len = 2; // ||
            else
                len = 1; // |
            break;
        default:
            break;
        }

        if (len == 0)
            return false;

        lex.advance(len);
        out = makeToken(TokenKind::Punctuator, start, lex);
        return true;
    }

    // NAME ::= identifier or keyword (starts with alpha/_/high-byte, continues with alnum/high-byte)
    bool tryLexName(LexerState& lex, LexicalToken& out)
    {
        const char_t c = lex.peek();
        if (!isAlpha(c) && !isHighByte(c))
            return false;

        const uint32_t start = lex.offset();
        while (!lex.isEnd() && isIdentChar(lex.peek()))
            lex.advance();

        out = makeToken(TokenKind::Name, start, lex);
        return true;
    }
} // namespace

namespace light_angel
{
    LexicalToken Lex(LexerState& lex)
    {
        for (;;)
        {
            if (lex.isEnd())
                return LexicalToken{TokenKind::EndOfFile, TokenView{SourceSpan{lex.offset(), 0}}};

            if (lex.isNextWhitespace())
            {
                lex.advance();
                continue;
            }

            if (trySkipComment(lex))
                continue;

            LexicalToken token;
            if (tryLexNumber(lex, token)) return token;
            if (tryLexString(lex, token)) return token;
            if (tryLexPunctuator(lex, token)) return token;
            if (tryLexName(lex, token)) return token;

            // Unknown character — skip
            lex.advance(); // TODO: report error
        }
    }
} // namespace light_angel
