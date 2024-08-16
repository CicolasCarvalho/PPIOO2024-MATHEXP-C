#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "eval/eval.h"

#ifdef WIN32

#include <windows.h>
double get_time()
{
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return (double)t.QuadPart/(double)f.QuadPart;
}

#else

#include <sys/time.h>
#include <sys/resource.h>

double get_time()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec*1e-6;
}

#endif


char *shift_arg(int *argc, char ***argv);

int main(int argc, char **argv) {
    shift_arg(&argc, &argv);

    if (argc == 0) {
        LOG("Esperado um argumento contendo a expressão\n./main <expressão> [-b <numero>]\n");
        return 0;
    }
    char *str = shift_arg(&argc, &argv);

    bool is_benchmark = false;
    uint32_t bench_size = 1;
    if (argc > 0) {
        char *buffer;
        buffer = shift_arg(&argc, &argv);

        is_benchmark = strcmp(buffer, "-b") == 0;
        
        if (argc == 0) {
            RAISE("Esperado número de execuçẽos"); 
        }

        buffer = shift_arg(&argc, &argv);
        bench_size = atoi(buffer);
    }
    
    if (is_benchmark) {
        LOG("rodando %i testes:\n", bench_size);
    }

    double before = get_time();

    while (bench_size--) {
        if (!is_benchmark) {
            PRINT("Tokenização de: '%s'", str); 
        }
        TokenExp *tokens = tokenize_str(str, is_benchmark);

        if (!is_benchmark) {
            PRINT("Geração da árvore de: '%s'", str);
        }
        TreeNode *tree = TokenTree_generate(tokens, is_benchmark);

        if (!is_benchmark) {
            PRINT("Avaliação de: '%s'", str);
        }
        int64_t result = TokenTree_eval(tree, is_benchmark);
        
        if (!is_benchmark) {
            PRINT("Resultado: %li\n", result);
        }    
        free(tokens->tokens);
        free(tokens);
        
        free(tree->left);
        free(tree->right);
        free(tree);
    }

    LOG("tempo decorrido: %lfs\n", get_time() - before);

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