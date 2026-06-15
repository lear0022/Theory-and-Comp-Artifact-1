#include "lexer.h"
#include <string.h>
#include <stdio.h>

static const char *KEYWORDS[] = {
    "char", "do", "else", "for",
    "if", "int", "return", "while"
};
static const int KEYWORD_COUNT = 8;

typedef enum {
    CC_LETTER, CC_DIGIT, CC_OP,
    CC_DELIM,  CC_SPACE, CC_OTHER
} CharClass;

static CharClass classify(char c) {
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c == '_')) 
        return CC_LETTER;

    if (c >= '0' && c <= '9')    
       return CC_DIGIT;
    if (c == '+' || c == '-' ||
        c == '*' || c == '/' ||
        c == '=' || c == '<' ||
        c == '>' || c == '!' ||
        c == '&' || c == '|')   
        return CC_OP;

    if (c == ';' || c == ',' ||
        c == '(' || c == ')' ||
        c == '{' || c == '}' ||
        c == '[' || c == ']')    
        return CC_DELIM;

    if (c == ' '  || c == '\t' ||
        c == '\r' || c == '\n') 
        return CC_SPACE;
        
        return CC_OTHER;
}

void lexer_init(Lexer *lx, const char *source) {
    lx->src = source;
    lx->start = source;
    lx->line = 1;
    lx->col = 1;
    lx->error_count = 0;
}

static char advance(Lexer *lx) {
    char c = *lx->src;
    lx->src++;
    if (c == '\n') { lx->line++; lx->col = 1; }
    else lx->col++;
    return c;
}

static char peek(const Lexer *lx) {
    return *lx->src;
}

Token next_token(Lexer *lx) {
    Token tok;
    State state = S_START;

    while (1) {
        char c = peek(lx);

        switch (state) {

        case S_START:
            if (c == '\0') {
                tok.type = TOK_EOF;
                tok.line = lx->line;
                tok.col = lx->col;
                tok.lexeme[0] = '\0';
                return tok;
            }

            advance(lx);
            lx->start = lx->src - 1;
            switch (classify(c)) {
            case CC_SPACE: state = S_START;  break;
            case CC_LETTER: state = S_IDENT;  break;
            case CC_DIGIT: state = S_NUMBER; break;
            case CC_OP: state = S_OP; break;
            case CC_DELIM: state = S_DELIM; break;
            default: state = S_ERROR; break;
            }
            break;

        case S_IDENT:
            if (classify(peek(lx)) == CC_LETTER ||
                classify(peek(lx)) == CC_DIGIT) {
                advance(lx);
            } else {
                state = S_ACCEPT_IDENT;
            }
            break;

        case S_NUMBER:
            if (classify(peek(lx)) == CC_DIGIT) {
                advance(lx);
            } else {
                state = S_ACCEPT_NUM;
            }
            break;

        case S_OP: {
            char prev = *(lx->src - 1);
            char next = peek(lx);
            if ((prev == '=' && next == '=') ||
                (prev == '!' && next == '=') ||
                (prev == '<' && next == '=') ||
                (prev == '>' && next == '=') ||
                (prev == '&' && next == '&') ||
                (prev == '|' && next == '|')) {
                advance(lx);
                state = S_OP_MULTI;
            } else {
                state = S_ACCEPT_OP;
            }
            break;
        }

        case S_OP_MULTI:
            state = S_ACCEPT_OP;
            break;

        case S_DELIM:
            state = S_ACCEPT_DELIM;
            break;

        case S_ACCEPT_IDENT: {
            int len = (int)(lx->src - lx->start);
            if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
            memcpy(tok.lexeme, lx->start, len);
            tok.lexeme[len] = '\0';
            tok.line = lx->line;
            tok.col  = lx->col - len;
            int lo = 0, hi = KEYWORD_COUNT - 1, found = 0;
            while (lo <= hi) {
                int mid = (lo + hi) / 2;
                int cmp = strcmp(tok.lexeme, KEYWORDS[mid]);
                if (cmp == 0) { found = 1; break; }
                else if (cmp <  0) hi = mid - 1;
                else  lo = mid + 1;
            }
            tok.type = found ? TOK_KEYWORD : TOK_IDENT;
            return tok;
        }

        case S_ACCEPT_NUM: {
            int len = (int)(lx->src - lx->start);
            if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
            memcpy(tok.lexeme, lx->start, len);
            tok.lexeme[len] = '\0';
            tok.type = TOK_NUMBER;
            tok.line = lx->line;
            tok.col  = lx->col - len;
            return tok;
        }

        case S_ACCEPT_OP:
        case S_ACCEPT_DELIM: {
            int len = (int)(lx->src - lx->start);
            if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
            memcpy(tok.lexeme, lx->start, len);
            tok.lexeme[len] = '\0';
            tok.type = (state == S_ACCEPT_OP) ? TOK_OPERATOR : TOK_DELIMITER;
            tok.line = lx->line;
            tok.col  = lx->col - len;
            return tok;
        }

        case S_ERROR: {
            if (lx->error_count < MAX_ERRORS) {
                snprintf(lx->errors[lx->error_count],
                         MAX_LEXEME_LEN,
                         "Line %d col %d: unexpected '%c'",
                         lx->line, lx->col - 1,
                         *(lx->src - 1));
                lx->error_count++;
            }
            while (peek(lx) != '\0' &&
                   peek(lx) != ';'  &&
                   peek(lx) != '\n' &&
                   peek(lx) != ' ') {
                advance(lx);
            }
            tok.type = TOK_ERROR;
            tok.line = lx->line;
            tok.col  = lx->col;
            int len  = (int)(lx->src - lx->start);
            if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
            memcpy(tok.lexeme, lx->start, len);
            tok.lexeme[len] = '\0';
            state = S_START;
            return tok;
        }

        }
    }
}

const char *token_type_str(TokenType t) {
    switch (t) {
    case TOK_KEYWORD: return "KEYWORD";
    case TOK_IDENT: return "IDENTIFIER";
    case TOK_NUMBER: return "NUMBER";
    case TOK_OPERATOR: return "OPERATOR";
    case TOK_DELIMITER: return "DELIMITER";
    case TOK_EOF: return "EOF";
    default: return "ERROR";
    }
}