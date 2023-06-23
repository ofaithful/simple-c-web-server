#ifndef _PARSER_H_
#define _PARSER_H_

#include "lexer.h"

typedef struct {
    int length;
    int* positions;
} SeparatorsData;

typedef enum {
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOLEAN,
    JSON_NULL,
    JSON_OBJECT,
    JSON_ARRAY
} JSON_valueType;

typedef struct JSON_value {
    JSON_valueType type;
    union {
        char* string;
        double number;
        int boolean;
        int* null;
        struct JSON_object* object;
        struct JSON_array* array;
    } data;
} JSON_value;

typedef struct ObjectProperty {
    char* key;
    struct JSON_value value;
} ObjectProperty;

typedef struct JSON_object {
    int length;
    ObjectProperty** properties;
} JSON_object;

typedef struct JSON_array {
    int length;
    JSON_value* elements;
} JSON_array;

typedef struct {
    size_t length;
    size_t cap;
    char** value;
} ParserErrors;

typedef struct {
//    Lexer* lexer;
//    ParserErrors* errors;
    Token* current;
    Token* peek;
} Parser;

Parser* parserCreate();
JSON_object* parseJSON(TokensArray* tokens);

char** parserErrors(Parser* parser);

#endif // _PARSER_H_
