#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

JSON_object* parseObject(TokensArray* tokens, int start);
JSON_array* parseArray(TokensArray* tokens, int start);

Parser* parserCreate(Lexer* lexer) {
    size_t len = sizeof(Parser);
    Parser* parser = malloc(len);
    if (parser == NULL) {
        return NULL;
    }
    memset(parser, 0, len);

    parser->current = NULL;
    parser->peek = NULL;

    return parser;
} 


void parserCleanup(Parser* parser);

SeparatorsData getSeparatorsData(TokensArray* tArray, int startPosition, int endPosition) {
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

void separatorsDataCleanup(SeparatorsData* data) {
    if (data->positions) {
        free(data->positions);
    }
}

int findEnclosingTokenPosition(TokensArray* tokens, int startPosition) {
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

// void parseValue(TokensArray* tokens, int index, int isArray) {
//     int indexOfValue = isArray ? index : index + 2;
//     switch (tokens->tokens[index]->type) {
//         case TOKEN_NUMBER:
//         case TOKEN_STRING:
//         case TOKEN_JSON_TRUE:
//         case TOKEN_JSON_FALSE:
//         case TOKEN_NULL:
//         case TOKEN_LEFT_BRACE:
//         case TOKEN_LEFT_BRACKET:
//         default: {
//             printf("Unsupported value\n");
//             return NULL;
//         }
//     }
// }

// void parseKeyValuePairs(int* start, int end, TokensArray* tokens, JSON_value* object) {
//     for (; *start < end; *start++) {
//         // key is at start position, colon is at start + 1 position, value is at start + 2 position
//         // different for arrays
//
//         ObjectProperty* property = malloc(sizeof(ObjectProperty));
//         property->key = &tokens->tokens[*start]->literal;
//
//         switch (tokens->tokens[*start]->type) {
//             case TOKEN_NUMBER:
//                 property->type = JSON_NUMBER;
//                 property->data.number = &tokens->tokens[*start + 2]->literal;
//                 break;
//             case TOKEN_STRING:
//                 property->type = JSON_STRING;
//                 property->data.string = tokens->tokens[*start + 2]->literal;
//                 break;
//             case TOKEN_JSON_TRUE:
//                 property->type = JSON_BOOLEAN;
//                 proprety->data.boolean = 1;
//                 break;
//             case TOKEN_JSON_FALSE:
//                 property->type = JSON_BOOLEAN;
//                 proprety->data.boolean = 0;
//                 break;
//             case TOKEN_JSON_NULL:
//                 property->type = JSON_NULL;
//                 proprety->data.null = NULL;
//                 break;
//             case TOKEN_LEFT_BRACE: {
//                 property->type = JSON_OBJECT;
//                 proprety->data.object = parseJSON(tokens, *start);
//                 break;
//             }
//             case TOKEN_LEFT_BRACKET: {
//                 proprety->type = JSON_ARRAY;
//                 proprety->data.array = (JSON_array*) malloc(sizeof(JSON_array));
//                 int endPosition = findEnclosingTokenPosition(tokens, tokens->tokens[*start]->type, *start);
//                 if (endPosition < 0) {
//                     return NULL;
//                 }
//                 
//                 SeparatorsData sData = getSeparatorsData(tokens, *start + 1, endPosition - 1);
//
//                 parseArray(tokens, *start + 1, endPosition - 1, array);
//                 break;
//                 // return parseArray(proprety->data.array, tokens, i, k);
//             }
//             default: {
//                 printf("Unsupported value\n");
//                 return NULL;
//             }
//         }
//     }
// }


// void parseArray(TokensArray* tokens, int startIndex, int endIndex, JSON_array* array) {
//
// }

// JSON_value* parseJSON(TokensArray* tokens, int startPosition) {
//     int endPosition = findEnclosingTokenPosition(tokens, tokens->tokens[startPosition]->type, startPosition);
//     if (endPosition < 0) {
//         return NULL;
//     }
//     
//     SeparatorsData sData = getSeparatorsData(tokens, startPosition + 1, endPosition - 1);
//
//     JSON_value* value = (JSON_value*) malloc(sizeof(JSON_value));
//
//     value->type = tokens->tokens[startPosition]->type == TOKEN_LEFT_BRACKET ? JSON_ARRAY : JSON_OBJECT;
//     if (value->type == JSON_ARRAY) {
//         value->data.array = (JSON_array*) malloc(sizeof(JSON_array));
//     } else {
//         value->data.object = (JSON_object*) malloc(sizeof(JSON_object));
//     }
//
//     for (int j = 0, i = startPosition + 1; j <= sData.length; j++) {
//         int endsAt = j < sData.length ? sData.positions[j] : endPosition - 1;
//         parseKeyValuePairs(&i, endsAt, tokens, value->type == TOKEN_ARRAY ? value->data.array : value->data.object);
//         i = sData.positions[j] + 1;
//     }
//
//     for (int i = startPosition, j = 0; i < endPosition; i++, ) {
//
//     }
//
//     separatorsDataCleanup(sData);
//
//     return object;
// }

JSON_value parseValue(Token* token) {
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

JSON_array* parseArray(TokensArray* tokens, int start) {
    if (tokens->tokens[start]->type != TOKEN_LEFT_BRACKET) {
        printf("Invalid char at position: %d\n", start);
        return NULL;
    }

    int endPosition = findEnclosingTokenPosition(tokens, start);
    if (endPosition < 0) {
        return NULL;
    }

    SeparatorsData sData = getSeparatorsData(tokens, start, endPosition);

    JSON_array* array = (JSON_array*) malloc(sizeof(JSON_array*));
    array->length = sData.length;

    JSON_value elements[sData.length];
    array->elements = elements; 

    for (int i = start, j = 0; j < sData.length; j++, i = sData.positions[j]) {
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
            JSON_value v = parseValue(tokens->tokens[i + 1]);
            array->elements[j] = parseValue(tokens->tokens[i + 1]);
        }
    }

    separatorsDataCleanup(&sData);
    return array;
}

JSON_object* parseObject(TokensArray* tokens, int start) {
    if (tokens->tokens[start]->type != TOKEN_LEFT_BRACE) {
        printf("Invalid char at position %d.\n", start);
        return NULL;
    }

    int endPosition = findEnclosingTokenPosition(tokens, start);
    if (endPosition < 0) {
        return NULL;
    }

    SeparatorsData sData = getSeparatorsData(tokens, start, endPosition);

    JSON_object* object = (JSON_object*) malloc(sizeof(JSON_object));

    object->length = sData.length;
    object->properties = (ObjectProperty**) malloc(sData.length * sizeof(ObjectProperty**));

    for (int i = start, j = 0, p = 0; j < sData.length; j++, i = sData.positions[j]) {
        // commas is at i position
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
            // printf("prop->value->type\n");
            // printf("after another parseArray");
        } else {
            // printf("key: %s | value: %d\n", tokens->tokens[i + 1]->literal, tokens->tokens[i + 3]->type);
            prop->value = parseValue(tokens->tokens[i + 3]);
        }
        object->properties[p++] = prop;

        // int endsAt = j < sData.length ? sData.positions[j] : endPosition - 1;
        // parseKeyValuePairs(i, )
    }

    return object;

}

// parse json(TokensArray* tokens, unsigned int startPosition) {
//      // find enclosing token for token at start position
//      int endPosition = findEnclosingTokenPosition();
//
//      create value to attach
//
//      // get separators for range(start position|end position);
//      // for each group separated by the separator run loop
//      // loop should also run from last separator position to the end position
//
//
// }

void jsonCleanup(JSON_object** parsedJson) {
    if (parsedJson) {
        free(parsedJson);
    }
}

int main() {
    const char *testString = "{\"protocol\":\"HTTP\",\"version\":1.1,\"methods\":[\"GET\",\"PUT\",\"POST\",\"PATCH\",\"DELETE\"],\"headers\":{\"content-type\":\"application/json\",\"content-length\":125,\"host\":\"localhost:8080\"},\"nullValue\": null,\"trueValue\":true,\"falseValue\":false}";

    TokensArray tokens = parseTokens(testString);
    // for (int i = 0; i < tokens.length; i++) {
    //     printf("token[%d]: %d \n", i, tokens.tokens[i]->type);
    // }

    JSON_object* object = parseObject(&tokens, 0);

    // printf("object->length: %d\n", object->length);
    for (int i = 0; i < object->length; i++) {
        printf("key: %s | value type: %d\n", object->properties[i]->key, object->properties[i]->value.type);
    }

    printf("values of array: \n");
    printf("key: %s, value type: %d\n", object->properties[1]->key, object->properties[1]->value);
    for (int i = 0; i < object->properties[1]->value.data.array->length; i++) {
        printf("%d ", object->properties[1]->value.data.array->length);
        printf("%d ", object->properties[1]->value.data.array->elements[i].type);
    }
    // for (int i = 0; i < tokens.length; i++) {
    //     printf("token[%d] type = %d | value = %s\n", i, tokens.tokens[i]->type, tokens.tokens[i]->literal);
    // }

    parsedTokensCleanup(tokens);
    free(object);

    return 0;
}
