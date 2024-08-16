#ifndef PPIOO2023_MATHEXP_C_EVAL_H
#define PPIOO2023_MATHEXP_C_EVAL_H

#include <stdint.h>

#include "../parser/parser.h"

int64_t TokenTree_eval(TreeNode *tree, bool is_benchmark);

#endif //PPIOO2023_MATHEXP_C_EVAL_H
