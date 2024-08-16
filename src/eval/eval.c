#include "eval.h"

//-declarations---------------------------------------------------------------------------------------------------------

static void eval_step(TreeNode *tree);
static int64_t eval_node(TreeNode *tree);

static void print_exp(TreeNode *tree);

//-functions------------------------------------------------------------------------------------------------------------

int64_t TokenTree_eval(TreeNode *tree, bool is_benchmark) {
    if (tree == NULL) {
        RAISE("TokenTree is NULL");
    }

    while (tree->token.type != LITERAL) {
        eval_step(tree);

        if (!is_benchmark) {
            START_LOG("TreeExpression");
            print_exp(tree);
            OUTPUT("\n");
            END_LOG("TreeExpression");

            START_LOG("EvalStep");
            TreeNode_print(tree, 0);
            END_LOG("EvalStep");
        }
    }

    return tree->token.value;
}

//-static---------------------------------------------------------------------------------------------------------------

static void eval_step(TreeNode *tree) {
    TreeNode *left_tree = tree->left;
    TreeNode *right_tree = tree->right;

    if (tree->token.type == LITERAL) {
        return;
    }

    if (right_tree->token.type == LITERAL && tree->token.type & UNARY_FLAG) {
        int64_t result = eval_node(tree);

        tree->token.type = LITERAL;
        tree->token.value = result;

        free(tree->right);

        tree->right = NULL;
        return;
    } else if (tree->token.type & UNARY_FLAG) {
        eval_step(right_tree);
        return;
    }

    if (left_tree == NULL || right_tree == NULL) {
        PRINT("%c", (int32_t)tree->token.value);
        RAISE("LeftTree | RightTree is NULL");
    }

    if (left_tree->token.type == LITERAL && right_tree->token.type == LITERAL) {
        int64_t result = eval_node(tree);

        tree->token.type = LITERAL;
        tree->token.value = result;

        free(tree->left);
        free(tree->right);

        tree->left = NULL;
        tree->right = NULL;
    } else if (left_tree->token.type != LITERAL) {
        eval_step(left_tree);
    } else if (right_tree->token.type != LITERAL) {
        eval_step(right_tree);
    }
}

int64_t eval_node(TreeNode *tree) {
    if (tree == NULL) {
        RAISE("TokenTree is NULL");
    }

    TreeNode *left_node = tree->left;
    TreeNode *right_node = tree->right;
    int64_t result;

    if (tree->token.type == LITERAL) {
        return tree->token.value;
    }

    if (right_node->token.type == LITERAL && tree->token.type & UNARY_FLAG) {
        return (tree->token.type & SUB ? -1: 1) * right_node->token.value;
    }

    if (left_node == NULL) {
        RAISE("TreeNode não tem 2 literais como operandos");
    }

    int64_t left_value = left_node->token.value;
    int64_t right_value = right_node->token.value;

    switch (tree->token.type) {
        case SUM:
            result = left_value + right_value;
            break;
        case SUB:
            result = left_value - right_value;
            break;
        case MUL:
            result = left_value * right_value;
            break;
        case DIV:
            result = left_value / right_value;
            break;
        default:
            RAISE("Invalid Token type");
    }

    return result;
}

void print_exp(TreeNode *tree) {
    if (tree->left != NULL) {
        print_exp(tree->left);
    }

    if (tree->token.type == LITERAL) {
        OUTPUT("%li ", tree->token.value);
    } else if (tree->token.type & UNARY_FLAG) {
        OUTPUT("%c", (int32_t)tree->token.value);
    } else {
        OUTPUT("%c ", (int32_t)tree->token.value);
    }

    if (tree->right != NULL) {
        print_exp(tree->right);
    }
}
