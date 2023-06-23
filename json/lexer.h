#ifndef _LEXER_H_
#define _LEXER_H_

typedef enum {
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_JSON_NULL,
    TOKEN_ARRAY,
    TOKEN_OBJECT,
    TOKEN_BOOLEAN,
    TOKEN_JSON_TRUE,
    TOKEN_JSON_FALSE,
    TOKEN_ILLEGAL,
    TOKEN_EOF,
} TokenType;

typedef struct SToken {
    TokenType type;
    char* literal;
} Token;

typedef struct {
    Token** tokens;
    size_t length;
} TokensArray;

typedef struct SLexer Lexer;
Lexer* lexerCreate(const char* input);

Token* lexerNext(Lexer* lexer);
void lexerCleanup(Lexer** lexer);

Token* tokenCreate(TokenType type, char* literal);
TokensArray parseTokens(const char* json);
void parsedTokensCleanup(TokensArray tokens);
void tokenCleanup(Token** token);

#endif // _LEXER_H_
