#include "parser.h"

//-declarations---------------------------------------------------------------------------------------------------------

static TreeNode *TokenTree_from_RPNExp(RPNExp *exp);

static TreeNode *TreeNode_new(Token token);
static TreeNode *TreeNode_build(RPNExp *exp, int32_t *i);

//-function-------------------------------------------------------------------------------------------------------------

TreeNode *TokenTree_generate(TokenExp *exp) {
    RPNExp *tokens = TokenExp_to_RPNExp(exp);
    TreeNode *token_tree = TokenTree_from_RPNExp(tokens);

    START_LOG("TokenTree");
    TreeNode_print(token_tree, 0);
    END_LOG("TokenTree");

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

    PRINT("(%i, %i, %li)", *i, node->token.type, node->token.value);
    if (actual_token.type == LITERAL) {
        return node;
    }

    int32_t right = (*i) - 1;
    node->right = TreeNode_build(exp, &right);

    int32_t left = right - 1;
    node->left = TreeNode_build(exp, &left);

    *i = left;

    if (node->left == NULL && actual_token.type & (SUB | SUM)) {
        return node;
    }

    if (node->left == NULL || node->right == NULL) {
        RAISE("Erro de sintaxe");
    }

    return node;
}

