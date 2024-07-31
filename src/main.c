#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "eval/eval.h"

char *shift_arg(int *argc, char ***argv);

int main(int argc, char **argv) {
    shift_arg(&argc, &argv);
    char *str = shift_arg(&argc, &argv);

    PRINT("Tokenização de: '%s'", str);
    TokenExp *tokens = tokenize_str(str);

    PRINT("Geração da árvore de: '%s'", str);
    TreeNode *tree = TokenTree_generate(tokens);

    PRINT("Avaliação de: '%s'", str);
    int64_t result = TokenTree_eval(tree);

    PRINT("Resultado: %li\n", result);
    return 0;
}

char *shift_arg(int *argc, char ***argv) {
    if (*argc == 0) {
        RAISE("Não há argumentos a serem pegos");
    }

    char *str = **argv;
    (*argv)++;
    (*argc)--;

    return str;
}