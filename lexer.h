#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define MAX_LEXEME_LEN 128
#define MAX_ERRORS     64

typedef enum {
    TOK_KEYWORD,
    TOK_IDENT,
    TOK_NUMBER,
    TOK_OPERATOR,
    TOK_DELIMITER,
    TOK_EOF,
    TOK_ERROR
} TokenType;

typedef enum {
    S_START,
    S_IDENT,
    S_NUMBER,
    S_OP,
    S_OP_MULTI,
    S_DELIM,
    S_ACCEPT_IDENT,
    S_ACCEPT_NUM,
    S_ACCEPT_OP,
    S_ACCEPT_DELIM,
    S_ERROR
} State;

typedef struct {
    char      lexeme[MAX_LEXEME_LEN];
    TokenType type;
    int       line;
    int       col;
} Token;

typedef struct {
    const char *src;
    const char *start;
    int         line;
    int         col;
    char        errors[MAX_ERRORS][MAX_LEXEME_LEN];
    int         error_count;
} Lexer;

void        lexer_init     (Lexer *lx, const char *source);
Token       next_token     (Lexer *lx);
const char *token_type_str (TokenType t);

#endif