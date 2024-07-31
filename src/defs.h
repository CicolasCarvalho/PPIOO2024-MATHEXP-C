#ifndef PPIOO2023_MATHEXP_C_DEFS_H
#define PPIOO2023_MATHEXP_C_DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//-debug-info-----------------------------------------------------------------------------------------------------------

#define DEBUG_INFO 1

//-log-utilities--------------------------------------------------------------------------------------------------------

#if DEBUG_INFO
    #define PRINT_FILE_AND_LINE() printf("\033[0;90m%s:%i:\e[0m ", __FILE__, __LINE__)

    #define START_LOG(name) \
                PRINT_FILE_AND_LINE(); \
                printf("%s\n", name)
    #define LOG(...)                                \
                printf(__VA_ARGS__)
    #define END_LOG(name) \
                printf("\n")
                // PRINT_FILE_AND_LINE();
                // printf("'%s'\n", name)

    #define PRINT(...)                             \
                PRINT_FILE_AND_LINE(); \
                printf(__VA_ARGS__);                   \
                printf("\n")
    #define WARN(...)           \
                printf("\033[0;33mWARN:\033[0m ");    \
                PRINT(__VA_ARGS__)
    #define RAISE(...)          \
                printf("\033[0;31mERROR:\033[0m ");    \
                PRINT(__VA_ARGS__);  \
                exit(1)

    #define OUTPUT(...) \
                printf("\033[0;32m>>\033[0m ");        \
                printf(__VA_ARGS__)
#else
    #define PRINT_FILE_AND_LINE()

    #define START_LOG(name)
    #define LOG(...)
    #define END_LOG(name)

    #define PRINT(...)
    #define WARN(...)                               \
                printf("\033[0;33mWARN:\033[0m ");    \
                PRINT_FILE_AND_LINE();              \
                printf(__VA_ARGS__);                   \
                printf("\n")
    #define RAISE(...)          \
                printf("\033[0;31mERROR:\033[0m ");    \
                PRINT_FILE_AND_LINE();              \
                printf(__VA_ARGS__);                   \
                printf("\n"); \
                exit(1)

    #define OUTPUT(...) \
                printf(__VA_ARGS__)
#endif //DEBUG_INFO

//-bool-defs------------------------------------------------------------------------------------------------------------

#define bool uint8_t
#define true 1
#define false 0

//-program-config-------------------------------------------------------------------------------------------------------

#define CHUNK_SIZE 16
#define MAX_TOKEN_SIZE 32

#define NULL_OPS    ((uint8_t)0b00000000) // 0
#define LITERAL     ((uint8_t)0b00000001) // 1
#define SUM         ((uint8_t)0b00000010) // 2
#define SUB         ((uint8_t)0b00000100) // 4
#define MUL         ((uint8_t)0b00001000) // 8
#define DIV         ((uint8_t)0b00010000) // 16
#define PAREN_OPEN  ((uint8_t)0b00100000) // 32
#define PAREN_CLOSE ((uint8_t)0b01000000) // 64

#define UNARY_FLAG  ((uint8_t)0b10000000) // 128

#define OPERATOR    (SUM | SUB | MUL | DIV)

#endif //PPIOO2023_MATHEXP_C_DEFS_H
