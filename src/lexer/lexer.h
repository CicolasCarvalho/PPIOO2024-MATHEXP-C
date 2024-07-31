#ifndef PPIOO2023_MATHEXP_C_LEXER_H
#define PPIOO2023_MATHEXP_C_LEXER_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../utils.h"
#include "../defs.h"

typedef struct {
    uint8_t type;
    int64_t value;
} Token;

typedef struct {
    Token *tokens;
    uint32_t size;
    uint32_t capacity;
} TokenExp;

typedef TokenExp RPNExp;

TokenExp *tokenize_str(char *str);
RPNExp *TokenExp_to_RPNExp(TokenExp *exp);

#endif //PPIOO2023_MATHEXP_C_LEXER_H
