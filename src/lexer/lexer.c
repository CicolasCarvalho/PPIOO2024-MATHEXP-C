#include "lexer.h"

#define FLUSH_ALL (Token){ \
    .type = NULL_OPS,       \
    .value = 0,            \
}

#define FLUSH_PAREN (Token){ \
    .type = PAREN_OPEN,       \
    .value = 0,            \
}

//-definition-----------------------------------------------------------------------------------------------------------

typedef struct TokenStack TokenStack;

struct TokenStack {
    Token token;
    TokenStack *next;
};

typedef struct TokenQueue TokenQueue;

struct TokenQueue {
    Token token;
    TokenQueue *next;
};

//-declarations---------------------------------------------------------------------------------------------------------

static TokenExp *TokenExp_new(void);
static void TokenExp_push(TokenExp *exp, uint8_t type, int64_t value);
static void TokenExp_print(TokenExp *exp);

static bool is_number(char c);
static void shift_str_token(char **str, char dst[MAX_TOKEN_SIZE], uint8_t last_token);

static int8_t precedence_test(TokenStack *stack, Token token);
static void flush_stack_to_queue(TokenStack **stack, TokenQueue **queue, Token delim);
static void flush_queue_to_exp(TokenQueue **queue, TokenExp *exp);

static TokenStack *TokenStack_new(Token token);
static void TokenStack_push(TokenStack **stack, TokenStack *next_stack);
static Token TokenStack_pop(TokenStack **stack);
static void TokenStack_print(TokenStack *stack);
static Token *TokenStack_peek(TokenStack *stack);

static TokenQueue *TokenQueue_new(Token token);
static void TokenQueue_enqueue(TokenQueue **queue, TokenQueue *next_queue);
static Token TokenQueue_dequeue(TokenQueue **queue);
static void TokenQueue_print(TokenQueue *queue);
static void TokenQueue_to_str(TokenQueue *queue, char *buffer);

//-functions------------------------------------------------------------------------------------------------------------

TokenExp *tokenize_str(char *str) {
    TokenExp *token_exp = TokenExp_new();

    char buffer[MAX_TOKEN_SIZE];
    uint8_t last_token = 0;

    do {
        shift_str_token(&str, buffer, last_token);
        PRINT("'%s' - '%s'", buffer, str);

        if (*buffer == '+' && buffer[1] == '\0') {
            TokenExp_push(token_exp, SUM, '+');
            last_token = SUM;
        } else if (*buffer == '-' && buffer[1] == '\0') {
            TokenExp_push(token_exp, SUB, '-');
            last_token = SUB;
        } else if (*buffer == '*') {
            TokenExp_push(token_exp, MUL, '*');
            last_token = MUL;
        } else if (*buffer == '/') {
            TokenExp_push(token_exp, DIV, '/');
            last_token = DIV;
        } else if (*buffer == '(') {
            TokenExp_push(token_exp, PAREN_OPEN, '(');
            last_token = PAREN_OPEN;
        } else if (*buffer == ')') {
            TokenExp_push(token_exp, PAREN_CLOSE, ')');
            last_token = PAREN_CLOSE;
        } else if (*buffer != '\0') {
            int64_t num = atoi(buffer);
            TokenExp_push(token_exp, LITERAL, num);
            last_token = LITERAL;
        }
    } while (*buffer != '\0');

    TokenExp_print(token_exp);

    return token_exp;
}

RPNExp *TokenExp_to_RPNExp(TokenExp *exp) {
    TokenStack *operators_stack = NULL;
    TokenQueue *output_queue = NULL;

    uint32_t str_size = MAX_TOKEN_SIZE * exp->size;
    char buffer[str_size];

    for (uint32_t i = 0; i < exp->size; ++i) {
        Token token = exp->tokens[i];

        // PRINT("i: %i", i);
        // TokenStack_print(operators_stack);
        // TokenQueue_print(output_queue);

        if (token.type == LITERAL) {
            TokenQueue_enqueue(&output_queue, TokenQueue_new(token));
        } else if (token.type & OPERATOR) {
            while (operators_stack != NULL && precedence_test(operators_stack, token) >= 0) {
                Token greater_token = TokenStack_pop(&operators_stack);

                TokenQueue_enqueue(&output_queue, TokenQueue_new(greater_token));
            }

            TokenStack_push(&operators_stack, TokenStack_new(token));
        } else if (token.type == PAREN_OPEN) {
            TokenStack_push(&operators_stack, TokenStack_new(token));
        } else if (token.type == PAREN_CLOSE) {
            flush_stack_to_queue(&operators_stack, &output_queue, FLUSH_PAREN);
        }
    }
    TokenStack_print(operators_stack);
    TokenQueue_print(output_queue);

    flush_stack_to_queue(&operators_stack, &output_queue, FLUSH_ALL);

    TokenQueue_to_str(output_queue, buffer);
    PRINT("RPN da expressão:");
    PRINT("%s (%lu bytes)", buffer, strlen(buffer));

    RPNExp *tokens = TokenExp_new();
    flush_queue_to_exp(&output_queue, tokens);

    TokenExp_print(tokens);
    return tokens;
}

//-static---------------------------------------------------------------------------------------------------------------

static TokenExp *TokenExp_new(void) {
    TokenExp *token_exp = malloc(sizeof(TokenExp));

    token_exp->tokens = malloc(sizeof(Token) * CHUNK_SIZE);
    token_exp->size = 0;
    token_exp->capacity = CHUNK_SIZE;

    return token_exp;
}

static void TokenExp_push(TokenExp *exp, uint8_t type, int64_t value) {
    if (exp == NULL) {
        RAISE("NULL TokenExp");
    }

    if (exp->size + 1 > exp->capacity) {
        void *tmp_tokens = realloc(exp->tokens, sizeof(Token) * exp->capacity * 2);
        exp->tokens = tmp_tokens;
        exp->capacity *= 2;
    }

    exp->tokens[exp->size] = (Token){.type = type, .value = value};
    exp->size++;
}

static void TokenExp_print(TokenExp *exp) {
    if (exp == NULL) {
        RAISE("NULL TokenExp");
    }

    START_LOG("TokenExpression");

    for (uint32_t i = 0; i < exp->size; ++i) {
        switch (exp->tokens[i].type) {
            case SUM:
            case SUB:
            case MUL:
            case DIV:
            case PAREN_OPEN:
            case PAREN_CLOSE:
                LOG("%c", (int32_t)exp->tokens[i].value);
                break;
            case LITERAL:
                LOG("%li", exp->tokens[i].value);
                break;
            default:
                RAISE("UNKNOWN TOKEN: [%i] %li", i, exp->tokens[i].value);
        }

        if (i < exp->size - 1) { LOG(" | "); }
    }

    END_LOG("TokenExpression");
}

static bool is_number(char c) {
    return (c >= 48 && c <= 57);
}

static void shift_str_token(char **str, char dst[MAX_TOKEN_SIZE], uint8_t last_token) {
    dst[0] = '\0';
    uint8_t len = 0;
    // trim dos espaços iniciais
    while ((**str) == ' ') (*str)++;

    while ((**str) != '\0' && (**str) != ' ') {
        if (len > MAX_TOKEN_SIZE) {
            RAISE("buffer size overflow");
        }

        char c = (**str);

        if (len > 0 && (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')')) {
            break;
        }

        dst[len++] = c;
        dst[len] = '\0';
        (*str)++;

        if (c == '(' || c == ')') break;

        // && (last_token & (LITERAL | PAREN_CLOSE))
        if (
            c == '*'   ||
            c == '/'
        ) {
            if (last_token & OPERATOR) {
                RAISE("Erro de sintaxe");
            }
            break;
        }
    }
}

static int8_t precedence_test(TokenStack *stack, Token token) {
    Token *peek = TokenStack_peek(stack);

    if (peek != NULL && !(peek->type & (OPERATOR | PAREN_OPEN))) {
        RAISE("Invalid Token Type on OperatorStack");
    }

    int8_t token_priority = -1;
    if (token.type & (SUM | SUB)) token_priority = 0;
    if (token.type & (MUL | DIV)) token_priority = 1;

    if (peek == NULL) return (int8_t)(0 - token_priority);

    int8_t peek_priority = -1;
    if (peek->type & (SUM | SUB)) peek_priority = 0;
    if (peek->type & (MUL | DIV)) peek_priority = 1;

    // PRINT("precedence: %i", peek_priority - token_priority);

    return (int8_t)(peek_priority - token_priority);
}

static void flush_stack_to_queue(TokenStack **stack, TokenQueue **queue, Token delim) {
    Token *peek = TokenStack_peek(*stack);

    while ((*stack) != NULL && peek->type != delim.type) {
        Token top = TokenStack_pop(stack);

        TokenQueue_enqueue(queue, TokenQueue_new(top));

        peek = TokenStack_peek(*stack);
    }

    if ((*stack) != NULL) TokenStack_pop(stack);
}

static void TokenQueue_to_str(TokenQueue *queue, char *buffer) {
    TokenQueue *actual = queue;
    buffer[0] = '\0';

    while (actual != NULL) {
        Token token = actual->token;

        switch (token.type) {
            case SUM:
            case SUB:
            case MUL:
            case DIV:
            case PAREN_OPEN:
            case PAREN_CLOSE:
                sprintf(buffer + strlen(buffer), "%c", (int32_t)token.value);
                break;
            case LITERAL:
                sprintf(buffer + strlen(buffer), "%li", token.value);
                break;
            default:
            RAISE("UNKNOWN TOKEN: %li", token.value);
        }

        actual = actual->next;
    }
}

static void flush_queue_to_exp(TokenQueue **queue, TokenExp *exp) {
    while (*queue != NULL) {
        Token token = TokenQueue_dequeue(queue);

        TokenExp_push(exp, token.type, token.value);
    }
}

//-token-stack-functions------------------------------------------------------------------------------------------------

static TokenStack *TokenStack_new(Token token) {
    TokenStack *token_stack = malloc(sizeof(TokenStack));

    token_stack->token = token;
    token_stack->next = NULL;

    return token_stack;
}

static void TokenStack_push(TokenStack **stack, TokenStack *next_stack) {
    if ((*stack) == NULL) {
        (*stack) = next_stack;
        return;
    }

    TokenStack **actual_ptr = stack;
    TokenStack *next;

    while ((*actual_ptr)->next != NULL) {
        next = (*actual_ptr)->next;
        actual_ptr = &next;
    }

    (*actual_ptr)->next = next_stack;
}

static Token TokenStack_pop(TokenStack **stack) {
    if ((*stack) == NULL) {
        RAISE("NULL TokenStack on pop");
    }

    TokenStack **actual_ptr = stack;

    while ((*actual_ptr)->next != NULL) {
        actual_ptr = &((*actual_ptr)->next);
    }

    Token token = (*actual_ptr)->token;
    free(*actual_ptr);
    *actual_ptr = NULL;
    return token;
}

static void TokenStack_print(TokenStack *stack) {
    TokenStack *actual = stack;

    START_LOG("TokenStack");

    while (actual != NULL) {
        LOG("(t: %i, v: %li) -> ", actual->token.type, actual->token.value);
        actual = actual->next;
    }

    LOG("NULL");
    END_LOG("TokenStack");
}

static Token *TokenStack_peek(TokenStack *stack) {
    if (stack == NULL) return NULL;

    TokenStack *actual_ptr = stack;
    while (actual_ptr->next != NULL) {
        actual_ptr = actual_ptr->next;
    }

    Token *token = &actual_ptr->token;
    return token;
}

//-token-queue-functions------------------------------------------------------------------------------------------------

static TokenQueue *TokenQueue_new(Token token) {
    TokenQueue *token_queue = malloc(sizeof(TokenQueue));

    token_queue->token = token;
    token_queue->next = NULL;

    return token_queue;
}

static void TokenQueue_enqueue(TokenQueue **queue, TokenQueue *next_queue) {
    if ((*queue) == NULL) {
        (*queue) = next_queue;
        return;
    }

    TokenQueue **actual_ptr = queue;
    TokenQueue *next;

    while ((*actual_ptr)->next != NULL) {
        next = (*actual_ptr)->next;
        actual_ptr = &next;
    }

    (*actual_ptr)->next = next_queue;
}

static Token TokenQueue_dequeue(TokenQueue **queue) {
    if ((*queue) == NULL) {
        RAISE("NULL TokenQueue on dequeue");
    }

    TokenQueue *tmp_queue = (*queue);
    Token token = tmp_queue->token;
    (*queue) = (*queue)->next;

    free(tmp_queue);
    return token;
}

static void TokenQueue_print(TokenQueue *queue) {
    TokenQueue *actual = queue;

    START_LOG("TokenQueue");

    while (actual != NULL) {
        LOG("(t: %i, v: %li) -> ", actual->token.type, actual->token.value);
        actual = actual->next;
    }

    LOG("NULL");
    END_LOG("TokenQueue");
}

