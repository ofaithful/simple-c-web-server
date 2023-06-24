#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

JSON_object* parseObject(TokensArray* tokens, int start);
JSON_array* parseArray(TokensArray* tokens, int start);

void jsonObjectCleanup(JSON_object* object);
void jsonArrayCleanup(JSON_array* array);

static SeparatorsData _getSeparatorsData(TokensArray* tArray, int startPosition, int endPosition);
static void _separatorsDataCleanup(SeparatorsData* data);

static int _findEnclosingTokenPosition(TokensArray* tokens, int startPosition);
static JSON_value _parseValue(Token* token);


JSON_array* parseArray(TokensArray* tokens, int start) {
    if (tokens->tokens[start]->type != TOKEN_LEFT_BRACKET) {
        printf("Invalid char at position: %d\n", start);
        return NULL;
    }

    int endPosition = _findEnclosingTokenPosition(tokens, start);
    if (endPosition < 0) {
        return NULL;
    }

    SeparatorsData sData = _getSeparatorsData(tokens, start, endPosition);

    JSON_array* array = (JSON_array*) malloc(sizeof(JSON_array));
    array->length = sData.length;
    array->elements = (JSON_value*) malloc(sData.length * sizeof(JSON_value)); 

    for (int i = start, j = 0; j < sData.length; i = sData.positions[j++]) {
        if (tokens->tokens[i + 1]->type == TOKEN_LEFT_BRACKET) {
            JSON_value value;
            value.type = JSON_ARRAY;
            value.data.array = parseArray(tokens, i + 1);
            array->elements[j] = value;
        } else if (tokens->tokens[i + 2]->type == TOKEN_LEFT_BRACE) {
            JSON_value value;
            value.type = JSON_OBJECT;
            value.data.object = parseObject(tokens, i + 1);
            array->elements[j] = value;
        } else {
            array->elements[j] = _parseValue(tokens->tokens[i + 1]);
        }
    }

    _separatorsDataCleanup(&sData);
    return array;
}

void jsonArrayCleanup(JSON_array* array) {
    for (int i = 0; i < array->length; i++) {
        switch (array->elements[i].type) {
            case JSON_STRING:
                free(array->elements[i].data.string);
            break;
            case JSON_ARRAY:
                jsonArrayCleanup(array->elements[i].data.array);
            break;
            case JSON_OBJECT:
                jsonObjectCleanup(array->elements[i].data.object);
            break;
        }
    }

    free(array->elements);
    free(array);
    array = NULL;
}

JSON_object* parseObject(TokensArray* tokens, int start) {
    if (tokens->tokens[start]->type != TOKEN_LEFT_BRACE) {
        printf("Invalid char at position %d.\n", start);
        return NULL;
    }

    int endPosition = _findEnclosingTokenPosition(tokens, start);
    if (endPosition < 0) {
        return NULL;
    }

    SeparatorsData sData = _getSeparatorsData(tokens, start, endPosition);

    JSON_object* object = (JSON_object*) malloc(sizeof(JSON_object));

    object->length = sData.length;
    object->properties = (ObjectProperty**) malloc(sData.length * sizeof(ObjectProperty**));

    for (int i = start, j = 0, p = 0; j < sData.length; i = sData.positions[j++]) {
        // comma is at i position
        // key is at i + 1 position
        // colon is at i + 2 position
        // value is at i + 3 position

        ObjectProperty* prop = (ObjectProperty*) malloc(sizeof(ObjectProperty));
        prop->key = strdup(tokens->tokens[i + 1]->literal);

        if (tokens->tokens[i + 3]->type == TOKEN_LEFT_BRACE) {
            JSON_value value;
            value.type = JSON_OBJECT;
            value.data.object = parseObject(tokens, i + 3);
            prop->value = value;
        } else if (tokens->tokens[i + 3]->type == TOKEN_LEFT_BRACKET) {
            JSON_value value;
            value.type = JSON_ARRAY;
            value.data.array = parseArray(tokens, i + 3);
            prop->value = value;
        } else {
            prop->value = _parseValue(tokens->tokens[i + 3]);
        }

        object->properties[p++] = prop;
    }

    _separatorsDataCleanup(&sData);
    return object;

}

void jsonObjectCleanup(JSON_object* object) {
    for (int i = 0; i < object->length; i++) {
        switch (object->properties[i]->value.type) {
            case JSON_STRING:
                free(object->properties[i]->value.data.string);
            break;
            case JSON_ARRAY:
                jsonArrayCleanup(object->properties[i]->value.data.array);
            break;
            case JSON_OBJECT:
                jsonObjectCleanup(object->properties[i]->value.data.object);
            break;
        }
        free(object->properties[i]->key);
    }

    free(object->properties);
    free(object);
    object = NULL;
}

static SeparatorsData _getSeparatorsData(TokensArray* tArray, int startPosition, int endPosition) {
    SeparatorsData data;
    data.length = 0;
    data.positions = (int*) malloc(sizeof(int));

    int leftBraceCount = 0;
    int leftBracketCount = 0;

    for (int i = startPosition + 1, j = 0; i < endPosition - 1; i++) {
        switch (tArray->tokens[i]->type) {
            case TOKEN_LEFT_BRACE:
                leftBraceCount++;
                break;
            case TOKEN_LEFT_BRACKET:
                leftBracketCount++;
                break;
            case TOKEN_RIGHT_BRACE:
                leftBraceCount--;
                break;
            case TOKEN_RIGHT_BRACKET:
                leftBracketCount--;
                break;
        }

        if (leftBraceCount > 0 || leftBracketCount > 0) continue;

        if (tArray->tokens[i]->type == TOKEN_COMMA) {

            int* temp = (int*) realloc(data.positions, (data.length + 1) * sizeof(int));
            if (temp == NULL) {
                printf("memory reallocation failed\n");
            } else {
                temp[j++] = i;
                data.positions = temp;
                data.length++;
            }
        }
    }

    return data;
}

static void _separatorsDataCleanup(SeparatorsData* data) {
    if (data->positions) {
        free(data->positions);
    }
    data->positions = NULL;
}

static int _findEnclosingTokenPosition(TokensArray* tokens, int startPosition) {
    TokenType enclosingToken;
    TokenType token = tokens->tokens[startPosition]->type;
    if (token == TOKEN_LEFT_BRACE) {
        enclosingToken = TOKEN_RIGHT_BRACE;
    } else if (token == TOKEN_LEFT_BRACKET) {
        enclosingToken = TOKEN_RIGHT_BRACKET;
    } else {
        printf("Token(type=%d) has no enclosing pair.\n", token);
        return -1;
    }

    int leftBraceCount = 0;
    int leftBracketCount = 0;
    int i = startPosition + 1;

    while (tokens->tokens[i]->type != TOKEN_EOF) {
        if (tokens->tokens[i]->type == enclosingToken && leftBraceCount == 0 && leftBracketCount == 0) {
            break;
        }

        if (tokens->tokens[i]->type == TOKEN_LEFT_BRACE) {
            leftBraceCount += 1;
        } else if (tokens->tokens[i]->type == TOKEN_LEFT_BRACKET) {
            leftBracketCount += 1;
        } else if (tokens->tokens[i]->type == TOKEN_RIGHT_BRACE && leftBraceCount > 0) {
            leftBraceCount -= 1;
        } else if (tokens->tokens[i]->type == TOKEN_RIGHT_BRACKET && leftBracketCount > 0) {
            leftBracketCount -= 1; 
        }

        i++;
    }

    if (tokens->tokens[i]->type != enclosingToken) {
        printf("enclosing token wasn't found\n");
        return -1;
    }

    return i;
}

static JSON_value _parseValue(Token* token) {
    JSON_value value;

    switch (token->type) {
        case TOKEN_STRING:
            value.type = JSON_STRING;
            value.data.string = strdup(token->literal);
        break;
        case TOKEN_NUMBER:
            value.type = JSON_NUMBER;
            value.data.number = atof(token->literal);
        break;
        case TOKEN_JSON_TRUE:
            value.type = JSON_BOOLEAN;
            value.data.boolean = 1;
        break;
        case TOKEN_JSON_FALSE:
            value.type = JSON_BOOLEAN;
            value.data.boolean = 0;
        break;
        case TOKEN_JSON_NULL:
            value.type = JSON_NULL;
            value.data.null = NULL;
        break;
        default:
            printf("Value cannot be parsed: %d\n", token->type);
    }

    return value;
}

int main() {
    const char *testString = "{\"protocol\":\"HTTP\",\"version\":1.1,\"methods\":[\"GET\",\"PUT\",\"POST\",\"PATCH\",\"DELETE\"],\"headers\":{\"content-type\":\"application/json\",\"content-length\":125,\"host\":\"localhost:8080\"},\"nullValue\": null,\"trueValue\":true,\"falseValue\":false}";

    TokensArray tokens = parseTokens(testString);

    JSON_object* object = parseObject(&tokens, 0);

    parsedTokensCleanup(tokens);

    jsonObjectCleanup(object);

    return 0;
}
