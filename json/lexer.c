#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"

struct SLexer {
    const char* input;
    size_t inputLength;
    size_t position;
    size_t readPosition;
    char ch;
};

static void _lexerReadChar(Lexer* lexer);
static char _lexerPeek(Lexer* lexer);
static void _lexerSkipWhitespace(Lexer* lexer);

static const char* _lexerReadString(Lexer* lexer, size_t* len);
static const char* _lexerReadNumber(Lexer* lexer, size_t* len);
static const char* _lexerReadIdentifier(Lexer* lexer, size_t* len);

static uint8_t _isNumber(char ch);
static uint8_t _isLetter(char ch);

static TokenType _getTokenTypeFromLiteral(const char* literal, size_t len);

Lexer* lexerCreate(const char* input) {
    size_t len = sizeof(Lexer);
    Lexer* lexer = malloc(len);
    memset(lexer, 0, len);

    lexer->input = input;
    lexer->inputLength = strlen(input);
    lexer->position = 0;
    lexer->readPosition = 0;
    lexer->ch = input[0];

    _lexerReadChar(lexer);

    return lexer;
}

Token* lexerNext(Lexer* lexer) {
    Token* token = NULL;

    _lexerSkipWhitespace(lexer);

    switch(lexer->ch) {
        case '{':
            token = tokenCreate(TOKEN_LEFT_BRACE, NULL);
            break;
        case '}':
            token = tokenCreate(TOKEN_RIGHT_BRACE, NULL);
            break;
        case '[':
            token = tokenCreate(TOKEN_LEFT_BRACKET, NULL);
            break;
        case ']':
            token = tokenCreate(TOKEN_RIGHT_BRACKET, NULL);
            break;
        case ',':
            token = tokenCreate(TOKEN_COMMA, NULL);
            break;
        case ':':
            token = tokenCreate(TOKEN_COLON, NULL);
            break;
        case '\0':
            token = tokenCreate(TOKEN_EOF, NULL);
            break;
    }

    // check for string
    if (lexer->ch == '\"') {
        _lexerReadChar(lexer);
        size_t len = 0;
        const char* strp = _lexerReadString(lexer, &len); 
        char str[len + 1];
        strncpy(str, strp, len);
        str[len] = '\0';
        token = tokenCreate(TOKEN_STRING, strdup(str));
       _lexerReadChar(lexer); 
        return token;
    } else if (_isNumber(lexer->ch)) {
        size_t len = 0;
        const char* ident = _lexerReadNumber(lexer, &len);
        char number[len + 1];
        strncpy(number, ident, len);
        number[len] = '\0';
        token = tokenCreate(TOKEN_NUMBER, strdup(number));
        return token;
    } else if (_isLetter(lexer->ch)) {
        size_t len = 0;
        const char* identifier = _lexerReadIdentifier(lexer, &len);

        TokenType type = _getTokenTypeFromLiteral(identifier, len);
        return tokenCreate(type, NULL);
    }

    if (!token) {
        token = tokenCreate(TOKEN_ILLEGAL, NULL);
    }

    _lexerReadChar(lexer);

    return token;
}

void lexerCleanup(Lexer** lexer) {
    if (*lexer) {
        free(*lexer);
    }

    *lexer = NULL;
}

Token *tokenCreate(TokenType type, char* literal) {
    size_t len = sizeof(Token);
    Token* token = malloc(len);
    memset(token, 0, len);

    token->literal = literal;
    token->type = type;

    return token;
}

TokensArray parseTokens(const char* json) {
    Token** tokensArray = malloc(sizeof(Token));
    size_t size = sizeof(Token);

    Lexer* lexer = lexerCreate(json);

    int i = 0;
    while ((tokensArray[i] = lexerNext(lexer)) && tokensArray[i]->type != TOKEN_EOF) {
        size_t newLength = size + sizeof(Token);
        tokensArray = realloc(tokensArray, size);
        size = newLength;
        i++;
    }

    int tokensLength = size / sizeof(Token);
    
    TokensArray returnValue = {tokensArray, tokensLength};

    return returnValue;
}

void parsedTokensCleanup(TokensArray tokens) {
    for (int i = 0; i < tokens.length; i++) {
        if (tokens.tokens[i]) {
            tokenCleanup(&tokens.tokens[i]);
        }
    }
}

void tokenCleanup(Token** token) {
    if (*token && (*token)->literal) {
        free((*token)->literal);
    }

    if (*token) {
        free(*token);
    }

    *token = NULL;
}

static void _lexerReadChar(Lexer* lexer) {
    if (lexer->readPosition >= lexer->inputLength) {
        lexer->ch = '\0';
    } else {
        lexer->ch = lexer->input[lexer->readPosition];
    }

    lexer->position = lexer->readPosition;
    lexer->readPosition++;
}

static char _lexerPeek(Lexer* lexer) {
    if (lexer->readPosition >= lexer->inputLength) {
        return '\0';
    } else {
        return lexer->input[lexer->readPosition];
    }
}

static void _lexerSkipWhitespace(Lexer* lexer) {
    while (lexer->ch == ' ' || lexer->ch == '\t' || lexer->ch == '\n' || lexer->ch == '\r') {
        _lexerReadChar(lexer);
    }
}

static const char* _lexerReadString(Lexer* lexer, size_t* len) {
    char* result = NULL;
    size_t position = lexer->position;

    while (lexer->ch && lexer->ch != '\"') {
        _lexerReadChar(lexer);
    }

    if (len) {
        *len = lexer->position - position;
    }
    return lexer->input + position;
}

static const char* _lexerReadNumber(Lexer* lexer, size_t* len) {
    char* result = NULL;
    size_t position = lexer->position;

    while (_isNumber(lexer->ch)) {
        _lexerReadChar(lexer);
    }

    if (len) {
        *len = lexer->position - position;
    }

    return lexer->input + position;
}

static const char* _lexerReadIdentifier(Lexer* lexer, size_t* len) {
    size_t position = lexer->position;

    while (_isLetter(lexer->ch)) {
        _lexerReadChar(lexer);
    }

    if (len) {
        *len = lexer->position - position;
    }

    return lexer->input + position;
}

static TokenType _getTokenTypeFromLiteral(const char* literal, size_t len) {
    if (strncmp(literal, "true", len) == 0) {
        return TOKEN_JSON_TRUE;
    } else if (strncmp(literal, "false", len) == 0) {
        return TOKEN_JSON_FALSE;
    } else if (strncmp(literal, "null", len) == 0) {
        return TOKEN_JSON_NULL;
    }
}

static uint8_t _isNumber(char ch) {
    int is_digit = isdigit(ch);
    int is_numeric = ch == '.' || ch == 'e' || ch == 'E' || ch == '+' || ch == '-';
    return is_digit || is_numeric;
}

static uint8_t _isLetter(char ch) {
    return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_';
}



// void process_json_object(const char* object) {
    
// }


// int main() {
//     const char *testString = "{\"protocol\":\"HTTP\",\"version\":1.1,\"methods\":[\"GET\",\"PUT\",\"POST\",\"PATCH\",\"DELETE\"],\"headers\":{\"content-type\":\"application/json\",\"content-length\":125,\"host\":\"localhost:8080\"},\"nullValue\": null,\"trueValue\":true,\"falseValue\":false}";
//     int length = strlen(testString);

//     Lexer* lexer = lexerCreate(testString);
//     Token* token = NULL;

//     int i = 0;
//     while ((token = lexerNext(lexer)) && token->type != TOKEN_EOF) {
//         printf("Token[%d].type: %d.\nToken[%d].literal: %s\n\n", i, token->type, i, token->literal);
//         // printf("position: %d\n", lexer->position);
//         tokenCleanup(&token);
//         i++;
//     }
//     // printf("from 207 position: %s\n", &testString[202]);

//     lexerCleanup(&lexer);
//     return 0;
// }
