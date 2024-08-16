#include "parser.h"

//-declarations---------------------------------------------------------------------------------------------------------

static TreeNode *TokenTree_from_RPNExp(RPNExp *exp);

static TreeNode *TreeNode_new(Token token);
static TreeNode *TreeNode_build(RPNExp *exp, int32_t *i);

//-function-------------------------------------------------------------------------------------------------------------

TreeNode *TokenTree_generate(TokenExp *exp, bool is_benchmark) {
    RPNExp *tokens = TokenExp_to_RPNExp(exp);

    if (!is_benchmark) {
        PRINT("RPN da expressão:");
        TokenExp_print(tokens);
    }
    
    TreeNode *token_tree = TokenTree_from_RPNExp(tokens);

    if (!is_benchmark) {
        START_LOG("TokenTree");
        TreeNode_print(token_tree, 0);
        END_LOG("TokenTree");
    }

    free(tokens->tokens);
    free(tokens);

    return token_tree;
}

//-static---------------------------------------------------------------------------------------------------------------

static TreeNode *TreeNode_new(Token token) {
    TreeNode *tree_node = malloc(sizeof(TreeNode));

    tree_node->token = token;
    tree_node->left = NULL;
    tree_node->right = NULL;

    return tree_node;
}

void TreeNode_print(TreeNode *node, uint32_t level) {
    for (uint32_t i = 1; i < level; i++) { LOG(" | "); }
    if (level > 0) { LOG(" |-"); }

    if (node == NULL) {
        LOG("(\033[0;90mNULL\033[0m)\n");
        return;
    }

    if (node->token.type == LITERAL) {
        LOG("(%li)\n", node->token.value);
    }
    else {
        LOG("(%c)\n", (int32_t)node->token.value);
    }

    TreeNode_print(node->left, level + 1);
    TreeNode_print(node->right, level + 1);
}

static TreeNode *TokenTree_from_RPNExp(RPNExp *exp) {
    int32_t i = (int32_t)exp->size - 1;
    TreeNode *root = TreeNode_build(exp, &i);

    return root;
}

static TreeNode *TreeNode_build(RPNExp *exp, int32_t *i) {
    if (*i < 0) return NULL;

    Token actual_token = exp->tokens[*i];
    TreeNode *node = TreeNode_new(actual_token);

    if (actual_token.type == LITERAL) {
        return node;
    }

    int32_t right = (*i) - 1;
    node->right = TreeNode_build(exp, &right);

    if (actual_token.type & UNARY_FLAG) {
        *i = right;
        return node;
    }

    int32_t left = right - 1;
    node->left = TreeNode_build(exp, &left);
    *i = left;

    if (node->left == NULL || node->right == NULL) {
        RAISE("Erro de sintaxe");
    }

    return node;
}

