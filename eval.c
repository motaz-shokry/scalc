#include "eval.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

static double parseExpr(char **expr);
static double parseTerm(char **expr);
static double parseFact(char **expr);

static void skipSpaces_str(char **str) {
    while (**str && isspace(**str)) {
        (*str)++;
    }
}

double parseFact(char **expr) {
    skipSpaces_str(expr);

    char *endptr;
    double result = strtod(*expr, &endptr);

    if (*expr == endptr) {
        printf("Expacted A Number.\n");
        return 0.0f;
    }
    *expr = endptr;
    return result;
}

double parseTerm(char **expr) {
    double result = parseFact(expr);
    skipSpaces_str(expr);

    while (**expr == '*' || **expr == '/') {
        char op = **expr;
        (*expr)++;
        double nextFactor = parseFact(expr);
        if (op == '*') {
            result *= nextFactor;
        } else if (op != 0.0f) {
            result /= nextFactor;
        } else {
            printf("Division by zero!\n");
            return result;
        }
        skipSpaces_str(expr);
    }
    return result;
}

double parseExpr(char **expr) {
    double result = parseTerm(expr);
    skipSpaces_str(expr);

    while (**expr == '+' || **expr == '-') {
        char op = **expr;
        (*expr)++;
        double nextTerm = parseTerm(expr);
        if (op == '+') {
            result += nextTerm;
        } else {
            result -= nextTerm;
        }
        skipSpaces_str(expr);
    }

    return result;
}

double evalExpr(char *expr) { return parseExpr(&expr); }
