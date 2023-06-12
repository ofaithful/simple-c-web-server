#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "json.h"

static void _lexerReadChar(Lexer* lexer);
static char _lexerPeek(Lexer* lexer);
static void _lexerSkipWhitespace(Lexer* lexer);
static const char* _lexerReadString(Lexer* lexer, size_t* len);
static const char* _lexerReadNumber(Lexer* lexer, size_t* len);

static uint8_t _isNumber(char ch);

static TokenType _getTokenTypeFromLiteral(const char* literal, size_t len);

Lexer* lexerCreate(const char* input) {
    size_t len = sizeof(Lexer);
    Lexer* lexer = malloc(len);
    memset(lexer, 0, len);

    lexer->input = input;
    lexer->inputLength = strlen(input);
    lexer->position = 0;
    lexer->readPosition = 0;

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
            token = createToken(TOKEN_RIGHT_BRACKET, NULL);
            break;
        case ',':
            token = createToken(TOKEN_COMMA, NULL);
            break;
        case ':':
            token = createToken(TOKEN_COLON, NULL);
            break;
        case '\0':
            token = tokenCreate(TOKEN_EOF, NULL);
            break;
    }

    if (lexer->ch == '\"') {
        size_t len = 0;
        // char* string = NULL;
        const char* string = _lexerReadString(lexer, &len); 

        token = tokenCreate(TOKEN_STRING, strndup(string, len));
        
        return token;
    } else if (_isNumber(lexer->ch)) {
        size_t len = 0;
        char* literal = NULL;
        const char* ident = _lexerReadNumber(lexer, &len);

        literal = strndup(ident, len);
        token = tokenCreate(TOKEN_NUMBER, literal);
        return token;
    }
    
    if (!token) {
        token = tokenCreate(TOKEN_ILLEGAL, NULL);
    }

    _lexerReadChar(lexer);

    return token;
}

typedef struct {
    char* key;
    struct JSON_value* value;
} JSON_object;

typedef struct {
    struct JSON_value** elements;
    int count;
} JSON_array;

typedef struct JSON_value {
    enum {
        JSON_STRING,
        JSON_NUMBER,
        JSON_BOOL,
        JSON_NULL,
        JSON_OBJECT,
        JSON_ARRAY
    } type;
    union {
        char* string;
        double number;
        int boolean;
        JSON_object object;
        JSON_array array;
    } data;
} JSON_value;

int is_numeric(char *chr) {
    int is_digit = isdigit(*chr);
    int is_numeric = isdigit(*chr) || *chr == '.' || *chr == 'e' || *chr == 'E' || *chr == '+' || *chr == '-';
    return is_digit || is_numeric;
}

void process_json_object(const char* object) {
    
}

void lex(const char* json, Token *array) {
    char buffer[2000];
    int i = 0;
    while (*json != '\0') {
        // skip leading whitespace
        // printf("CURRENT CHAR: %c\n", *json);
        while (*json && isspace(*json))
            json++;
        if (*json == '{') {
            json++; 
            array[i] = (Token){TOKEN_LEFT_BRACE, NULL};
            i++;
        }
        if (*json == '}') {
            json++; 
            array[i] = (Token){TOKEN_RIGHT_BRACE, NULL};
            i++;
        }
        if (*json == '[') {
            json++; 
            array[i] = (Token){TOKEN_LEFT_BRACKET, NULL};
            i++;
        }
        if (*json == ']') {
            json++; 
            array[i] = (Token){TOKEN_RIGHT_BRACKET, NULL};
            i++;
        }
        if (*json == ',') {
            json++; 
            array[i] = (Token){TOKEN_COMMA, NULL};
            i++;
        }
        if (*json == ':') {
            json++; 
            array[i] = (Token){TOKEN_COLON, NULL};
            i++;
        }

        // check string
        if (*json == '\"') {
            json++;

            char *start = buffer;
            while (*json && *json != '\"' && start < buffer + sizeof(buffer) - 1) {
                if (*json == '\\') {
                    json++;
                }
                *start++ = *json++;
                // printf("char: %c ", *json);
            }
            *start = '\0';

            if (*json != '\"') {
                array[i] = (Token){TOKEN_UNDEFINED, NULL};
                i++;
            }
            // printf("json state: %c\n", &json);
            json++; // skip closing quote
            array[i] = (Token){TOKEN_STRING, strdup(buffer)};
            i++;
            // printf("string: %s\n", buffer);
        }
        // printf("buffer p: %p\n", buffer);
        // printf("json state: %c\n", *json);
        // printf("buffer state: %s\n", buffer);
        // check for number tokens
        if (isdigit(*json) || *json == '-') {
            char *start = buffer;
            // int isNumeric = isdigit(*json) || *json == '.' || *json == 'e' || *json == 'E' || *json == '+' || *json == '-';
            // printf("isNumeric: %d\n", isNumeric);
            // printf("buffer p(isdigit check): %p\n", buffer);
            // printf("is json true:%d\n", *json);
            while (is_numeric((char *) json)) { 
                // printf("is char[%c] numeric? %d\n", *json, is_numeric(json));
                *start++ = *json++;
                // printf("START:%s\n", start);
            }
            *start = '\0';
            // printf("start string: %s\n", start);
            //TODO: parse number
            array[i] = (Token){TOKEN_NUMBER, strdup(buffer)};
            i++;
        }

        while (*json && isspace(*json))
            json++;

        // check for boolean and null
        // printf("current json: %s\n", json);
        if (strncmp(json, "true", 4) == 0) {
            // printf("true\n");
            json += 4;
            array[i] = (Token){TOKEN_JSON_TRUE, NULL};
            i++;
        }
        if (strncmp(json, "false", 5) == 0) {
            // printf("false\n");
            json += 5;
            array[i] = (Token){TOKEN_JSON_FALSE, NULL};
            i++;
        }
        if (strncmp(json, "null", 4) == 0) {
            // printf("null\n");
            json += 4;
            array[i] = (Token){TOKEN_JSON_NULL, NULL};
            i++;
        }

        // array[i] = (Token){TOKEN_UNDEFINED, NULL};  
        // i++;
        // json++;
    }
}




int main() {
    const char *testString = "{\"protocol\":\"HTTP\",\"version\":1.1,\"methods\":[\"GET\",\"PUT\",\"POST\",\"PATCH\",\"DELETE\"],\"headers\":{\"content-type\":\"application/json\",\"content-length\":125,\"host\":\"localhost:8080\"},\"trueValue\":true,\"falseValue\":false}";
    int length = strlen(testString);

    printf("length of the string is: %d\n", length);
    Token tokens[length];

    lex(testString, tokens);


    for (int i = 0; i < length; i++) {
        printf("TOKEN: %s(%d), value: %s\n", getTokenName(tokens[i].type), tokens[i].type, tokens[i].value);
    }
    return 0;
}
