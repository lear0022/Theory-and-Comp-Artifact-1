#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Error: could not open '%s'\n", argv[1]);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *source = (char *)malloc(size + 1);
    if (!source) {
        fprintf(stderr, "Error: out of memory\n");
        fclose(fp);
        return 1;
    }
   size_t bytes_read = fread(source, 1, size, fp);
    source[bytes_read] = '\0';
    source[size] = '\0';
    fclose(fp);

    FILE *out = fopen("tokens.out", "w");
    if (!out) {
        fprintf(stderr, "Error: could not create tokens.out\n");
        free(source);
        return 1;
    }

    Lexer lx;
    lexer_init(&lx, source);

    printf("%-20s %-14s %s\n", "LEXEME", "TYPE", "POSITION");
    printf("%-20s %-14s %s\n", "------", "----", "--------");
    fprintf(out, "%-20s %-14s %s\n", "LEXEME", "TYPE", "POSITION");
    fprintf(out, "%-20s %-14s %s\n", "------", "----", "--------");

    Token tok;
    do {
        tok = next_token(&lx);
        printf("%-20s %-14s line %d col %d\n",
               tok.lexeme, token_type_str(tok.type),
               tok.line, tok.col);
        fprintf(out, "%-20s %-14s line %d col %d\n",
                tok.lexeme, token_type_str(tok.type),
                tok.line, tok.col);
    } while (tok.type != TOK_EOF);

    if (lx.error_count > 0) {
        printf("\n--- Lexer Errors ---\n");
        fprintf(out, "\n--- Lexer Errors ---\n");
        int i;
        for (i = 0; i < lx.error_count; i++) {
            printf("%s\n", lx.errors[i]);
            fprintf(out, "%s\n", lx.errors[i]);
        }
    }

    fclose(out);
    free(source);
    return 0;
}