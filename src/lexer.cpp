#include "lexer.h"

namespace
{

}

namespace light_angel
{

    LexicalToken Lex(LexerState& lexer)
    {
        for (;;)
        {
            if (lexer.isEnd())
            {
                return LexicalToken{TokenKind::EndOfFile, TokenView{SourceSpan{lexer.offset(), 0}}};
            }

            if (lexer.isNextLineBreak() || lexer.isNextWhitespace())
            {
                lexer.advance();
                continue;
            }

            // TODO

            // 全 Punctuator は以下の通り
            // '*', '**', '/', '%', '+', '-', '<=', '<', '>=', '>', '(', ')', '==', '!=', '?', ':', '=', '+=', '-=', '*=', '/=', '%=', '**=', '++', '--', '&', ',', '{', '}', ';', '|', '^', '~', '<<', '>>', '>>>', '&=', '|=', '^=', '<<=', '>>=', '>>>=', '.', '...', '&&', '||', '!', '[', ']', '^^', '@', '!is', '::', '#'
        }
    }
} // namespace light_angel
