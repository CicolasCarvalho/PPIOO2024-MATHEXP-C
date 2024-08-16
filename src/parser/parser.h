#ifndef PPIOO2023_MATHEXP_C_PARSER_H
#define PPIOO2023_MATHEXP_C_PARSER_H

#include <stdlib.h>
#include <stdint.h>

#include "../lexer/lexer.h"
#include "../utils.h"
#include "../defs.h"

typedef struct TreeNode TreeNode;

struct TreeNode {
    Token token;
    TreeNode *left;
    TreeNode *right;
};

TreeNode *TokenTree_generate(TokenExp *tokenExp, bool is_benchmark);
void TreeNode_print(TreeNode *node, uint32_t level);


#endif //PPIOO2023_MATHEXP_C_PARSER_H
