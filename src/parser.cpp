#include "light_angel/parser.h"

#include "parser_context.h"

namespace
{
    // forward declarations
    std::unique_ptr<light_angel::ParserContext> parseScript(light_angel::ParserContext& ctx, char storKeyword = 0);

    // -----------------------------------------------

    // **BNF** SCRIPT ::= {IMPORT | ENUM | TYPEDEF | CLASS | INTERFACE | FUNCDEF | VIRTUALPROP | VAR | FUNC | NAMESPACE | USING | ';'}
    // TODO

    // **BNF** NAMESPACE ::= 'namespace' IDENTIFIER {'::' IDENTIFIER} '{' SCRIPT '}'
    // TODO

    // **BNF** USING ::= 'using' 'namespace' IDENTIFIER {'::' IDENTIFIER} ';'
    // TODO

    // **BNF** ENUM ::= {'shared' | 'external'} 'enum' IDENTIFIER [ ':' ('int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64') ] (';' | ('{' IDENTIFIER ['=' EXPR] {',' IDENTIFIER ['=' EXPR]} '}'))
    // TODO

    // **BNF** CLASS ::= ['mixin'] {'shared' | 'abstract' | 'final' | 'external'} 'class' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | FUNC | VAR | FUNCDEF} '}'))
    // TODO

    // **BNF** TYPEDEF ::= 'typedef' PRIMITIVETYPE IDENTIFIER ';'
    // TODO

    // **BNF** FUNC ::= {'shared' | 'external'} ['private' | 'protected'] [((TYPE ['&']) | '~')] IDENTIFIER ['<' TYPE {',' TYPE} '>'] PARAMLIST [LISTPATTERN] ['const'] FUNCATTR (';' | STATBLOCK)
    // TODO

    // **BNF** LISTPATTERN ::= '{' LISTENTRY {',' LISTENTRY} '}'
    // TODO

    // **BNF** LISTENTRY ::= (('repeat' | 'repeat_same') (('{' LISTENTRY '}') | TYPE)) | (TYPE {',' TYPE})
    // TODO

    // **BNF** INTERFACE ::= {'external' | 'shared'} 'interface' IDENTIFIER (';' | ([':' SCOPE IDENTIFIER {',' SCOPE IDENTIFIER}] '{' {VIRTUALPROP | INTERFACEMETHOD} '}'))
    // TODO

    // **BNF** VAR ::= ['private' | 'protected'] TYPE IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST] {',' IDENTIFIER [( '=' (INITLIST | ASSIGN)) | ARGLIST]} ';'
    // TODO

    // **BNF** IMPORT ::= 'import' TYPE ['&'] IDENTIFIER PARAMLIST FUNCATTR 'from' STRING ';'
    // TODO

    // **BNF** FUNCDEF ::= {'external' | 'shared'} 'funcdef' TYPE ['&'] IDENTIFIER PARAMLIST ';'
    // TODO

    // **BNF** VIRTUALPROP ::= ['private' | 'protected'] TYPE ['&'] IDENTIFIER '{' {('get' | 'set') ['const'] FUNCATTR (STATBLOCK | ';')} '}'
    // TODO

    // **BNF** INTERFACEMETHOD ::= TYPE ['&'] IDENTIFIER PARAMLIST ['const'] FUNCATTR ';'
    // TODO

    // **BNF** STATBLOCK ::= '{' {VAR | STATEMENT | USING} '}'
    // TODO

    // **BNF** PARAMLIST ::= '(' ['void' | (PARAMETER {',' PARAMETER})] ')'
    // TODO

    // **BNF** PARAMETER ::= TYPE TYPEMODIFIER [IDENTIFIER] ['...' | ('=' (EXPR | 'void'))]
    // TODO

    // **BNF** TYPEMODIFIER ::= ['&' ['in' | 'out' | 'inout'] ['+'] ['if_handle_then_const']]
    // TODO

    // **BNF** TYPE ::= ['const'] SCOPE DATATYPE ['<' TYPE {',' TYPE} '>'] { ('[' ']') | ('@' ['const']) }
    // TODO

    // **BNF** INITLIST ::= '{' [ASSIGN | INITLIST] {',' [ASSIGN | INITLIST]} '}'
    // TODO

    // **BNF** SCOPE ::= ['::'] {IDENTIFIER '::'} [IDENTIFIER ['<' TYPE {',' TYPE} '>'] '::']
    // TODO

    // **BNF** DATATYPE ::= (IDENTIFIER | PRIMITIVETYPE | '?' | 'auto')
    // TODO

    // **BNF** PRIMITIVETYPE ::= 'void' | 'int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64' | 'float' | 'double' | 'bool'
    // TODO

    // **BNF** FUNCATTR ::= {'override' | 'final' | 'explicit' | 'property' | 'delete' | 'nodiscard'}
    // TODO

    // **BNF** STATEMENT ::= (IF | FOR | FOREACH | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)
    // TODO

    // **BNF** SWITCH ::= 'switch' '(' ASSIGN ')' '{' {CASE} '}'
    // TODO

    // **BNF** BREAK ::= 'break' ';'
    // TODO

    // **BNF** FOR ::= 'for' '(' (VAR | EXPRSTAT) EXPRSTAT [ASSIGN {',' ASSIGN}] ')' STATEMENT
    // TODO

    // **BNF** FOREACH ::= 'foreach' '(' TYPE IDENTIFIER {',' TYPE IDENTIFIER} ':' ASSIGN ')' STATEMENT
    // TODO

    // **BNF** WHILE ::= 'while' '(' ASSIGN ')' STATEMENT
    // TODO

    // **BNF** DOWHILE ::= 'do' STATEMENT 'while' '(' ASSIGN ')' ';'
    // TODO

    // **BNF** IF ::= 'if' '(' ASSIGN ')' STATEMENT ['else' STATEMENT]
    // TODO

    // **BNF** CONTINUE ::= 'continue' ';'
    // TODO

    // **BNF** EXPRSTAT ::= [ASSIGN] ';'
    // TODO

    // **BNF** TRY ::= 'try' STATBLOCK 'catch' STATBLOCK
    // TODO

    // **BNF** RETURN ::= 'return' [ASSIGN] ';'
    // TODO

    // **BNF** CASE ::= (('case' EXPR) | 'default') ':' {STATEMENT}
    // TODO

    // **BNF** EXPR ::= EXPRTERM {EXPROP EXPRTERM}
    // TODO

    // **BNF** EXPRTERM ::= ([TYPE '='] INITLIST) | ({EXPRPREOP} EXPRVALUE {EXPRPOSTOP})
    // TODO

    // **BNF** EXPRVALUE ::= 'void' | CONSTRUCTORCALL | FUNCCALL | VARACCESS | CAST | LITERAL | '(' ASSIGN ')' | LAMBDA
    // TODO

    // **BNF** CONSTRUCTORCALL ::= TYPE ARGLIST
    // TODO

    // **BNF** EXPRPREOP ::= '-' | '+' | '!' | '++' | '--' | '~' | '@'
    // TODO

    // **BNF** EXPRPOSTOP ::= ('.' (FUNCCALL | IDENTIFIER)) | ('[' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ']') | ARGLIST | '++' | '--'
    // TODO

    // **BNF** CAST ::= 'cast' '<' TYPE '>' '(' ASSIGN ')'
    // TODO

    // **BNF** LAMBDA ::= 'function' '(' [LAMBDAPARAM {',' LAMBDAPARAM}] ')' STATBLOCK
    // TODO

    // **BNF** LAMBDAPARAM ::= [TYPE TYPEMODIFIER] [IDENTIFIER]
    // TODO

    // **BNF** LITERAL ::= NUMBER | STRING | BITS | 'true' | 'false' | 'null'
    // TODO

    // **BNF** FUNCCALL ::= SCOPE IDENTIFIER ['<' TYPE {',' TYPE} '>'] ARGLIST
    // TODO

    // **BNF** VARACCESS ::= SCOPE IDENTIFIER
    // TODO

    // **BNF** ARGLIST ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'
    // TODO

    // **BNF** ASSIGN ::= CONDITION [ ASSIGNOP ASSIGN ]
    // TODO

    // **BNF** CONDITION ::= EXPR ['?' ASSIGN ':' ASSIGN]
    // TODO

    // **BNF** EXPROP ::= MATHOP | COMPOP | LOGICOP | BITOP
    // TODO

    // **BNF** BITOP ::= '&' | '|' | '^' | '<<' | '>>' | '>>>'
    // TODO

    // **BNF** MATHOP ::= '+' | '-' | '*' | '/' | '%' | '**'
    // TODO

    // **BNF** COMPOP ::= '==' | '!=' | '<' | '<=' | '>' | '>=' | 'is' | '!is'
    // TODO

    // **BNF** LOGICOP ::= '&&' | '||' | '^^' | 'and' | 'or' | 'xor'
    // TODO

    // **BNF** ASSIGNOP ::= '=' | '+=' | '-=' | '*=' | '/=' | '|=' | '&=' | '^=' | '%=' | '**=' | '<<=' | '>>=' | '>>>='
    // TODO

    // **BNF** IDENTIFIER ::= single token: starts with letter or '_', can include any letter and digit, same as in C++
    // TODO

    // **BNF** NUMBER ::= single token: includes integers and real numbers, same as C++
    // TODO

    // **BNF** STRING ::= single token: single quoted `'`, double quoted `"`, or heredoc multi-line string `"""`
    // TODO

    // **BNF** BITS ::= single token: binary 0b or 0B, octal 0o or 0O, decimal 0d or 0D, hexadecimal 0x or 0X
    // TODO

    // **BNF** COMMENT ::= single token: starts with '//' and ends with new line or starts with '/*' and ends with '*/'
    // TODO

    // **BNF** WHITESPACE ::= single token: spaces, tab, carriage return, line feed, and UTF8 byte-order-mark
    // TODO
} // namespace
