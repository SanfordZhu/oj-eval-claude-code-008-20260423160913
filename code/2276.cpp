#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MEMORYSIZE 65536
#define CODESIZE 65536
#define uint8 unsigned char
#define uint16 unsigned short

long c;
uint16 p = 0, q = 0;
uint8 reg[26];
uint8 mem[MEMORYSIZE];
FILE *input;

struct instruct {
    uint8 src_mode, dest_mode;
    uint16 src, dest, src_offset, dest_offset;
    uint16 line;
} code[CODESIZE];

void err(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
    fclose(input);
    exit(1);
}

void preprocess(const char *mov_code) {
    char src[16] = "", dest[16] = "";
    uint8 buf1 = 0, buf2 = 0;
    int buf3 = 0;
    char *code_copy = strdup(mov_code);
    char *line = code_copy;
    uint16 line_num = 0;

    while (*line) {
        line_num++;
        // Skip whitespace
        while (*line == ' ' || *line == '\t') line++;
        if (*line == '#' || *line == '\r' || *line == '\n') {
            while (*line && *line != '\n') line++;
            if (*line == '\n') line++;
            continue;
        }

        // Parse instruction
        char *dest_start = line;
        while (*line && *line != '<') line++;
        if (*line != '<') err("Syntax error");
        *line = '\0';
        line++;
        char *src_start = line;
        while (*line && *line != '\n' && *line != '\r' && *line != ' ' && *line != '\t') line++;

        // Copy dest and src
        strncpy(dest, dest_start, 15);
        strncpy(src, src_start, 15);

        // Parse src
        if (src[0] == '[') {
            char buf1_str[2] = "", buf2_str[2] = "";
            if (sscanf(src, "[%1[A-Z]+%1[A-Z]]", buf1_str, buf2_str) == 2) {
                code[q].src_mode = 4;
                code[q].src = buf1_str[0] - 'A';
                code[q].src_offset = buf2_str[0] - 'A';
                if (code[q].src_offset == 8 || code[q].src_offset == 14)
                    err("I/O can't be used here");
            } else if (sscanf(src, "[%1[A-Z]+%d]", buf1_str, &buf3) == 2) {
                code[q].src_mode = 3;
                code[q].src = buf1_str[0] - 'A';
                code[q].src_offset = (uint16)buf3;
            } else if (sscanf(src, "[%1[A-Z]]", buf1_str) == 1) {
                code[q].src_mode = 2;
                code[q].src = buf1_str[0] - 'A';
                code[q].src_offset = 0;
            } else if (sscanf(src, "[%d]", &buf3) == 1) {
                code[q].src_mode = 5;
                code[q].src = (uint16)buf3;
                code[q].src_offset = 0;
            } else {
                err("Syntax error in src");
            }
            if ((code[q].src == 8 || code[q].src == 14) && code[q].src_mode != 5)
                err("I/O can't be used here");
        } else if (src[0] >= 'A' && src[0] <= 'Z') {
            code[q].src_mode = 1;
            code[q].src = src[0] - 'A';
            code[q].src_offset = 0;
            if (code[q].src == 14)
                err("O can't be used as src");
        } else if (sscanf(src, "%d", &buf3) == 1) {
            code[q].src_mode = 0;
            code[q].src = (uint16)buf3;
            code[q].src_offset = 0;
        } else {
            err("Syntax error in src");
        }

        // Parse dest
        if (dest[0] == '[') {
            char buf1_str[2] = "", buf2_str[2] = "";
            if (sscanf(dest, "[%1[A-Z]+%1[A-Z]]", buf1_str, buf2_str) == 2) {
                code[q].dest_mode = 4;
                code[q].dest = buf1_str[0] - 'A';
                code[q].dest_offset = buf2_str[0] - 'A';
                if (code[q].dest_offset == 8 || code[q].dest_offset == 14)
                    err("I/O can't be used here");
            } else if (sscanf(dest, "[%1[A-Z]+%d]", buf1_str, &buf3) == 2) {
                code[q].dest_mode = 3;
                code[q].dest = buf1_str[0] - 'A';
                code[q].dest_offset = (uint16)buf3;
            } else if (sscanf(dest, "[%1[A-Z]]", buf1_str) == 1) {
                code[q].dest_mode = 2;
                code[q].dest = buf1_str[0] - 'A';
                code[q].dest_offset = 0;
            } else if (sscanf(dest, "[%d]", &buf3) == 1) {
                code[q].dest_mode = 5;
                code[q].dest = (uint16)buf3;
                code[q].dest_offset = 0;
            } else {
                err("Syntax error in dest");
            }
            if ((code[q].dest == 8 || code[q].dest == 14) && code[q].dest_mode != 5)
                err("I/O can't be used here");
        } else if (dest[0] >= 'A' && dest[0] <= 'Z') {
            code[q].dest_mode = 1;
            code[q].dest = dest[0] - 'A';
            code[q].dest_offset = 0;
            if (code[q].dest == 8)
                err("I can't be used as dest");
        } else {
            err("Syntax error in dest");
        }
        code[q].line = line_num;
        q++;

        // Skip to next line
        while (*line && *line != '\n') line++;
        if (*line == '\n') line++;
    }
    free(code_copy);
}

int main() {
    input = stdin;

    // 2276 - Hello World
    const char *mov_code =
        "A<72\n"
        "O<A\n"
        "A<101\n"
        "O<A\n"
        "A<108\n"
        "O<A\n"
        "A<108\n"
        "O<A\n"
        "A<111\n"
        "O<A\n"
        "A<32\n"
        "O<A\n"
        "A<87\n"
        "O<A\n"
        "A<111\n"
        "O<A\n"
        "A<114\n"
        "O<A\n"
        "A<108\n"
        "O<A\n"
        "A<100\n"
        "O<A\n"
        "A<33\n"
        "O<A\n"
        "Z<1\n";

    preprocess(mov_code);

    while (true) {
        for (uint16 i = 0; i < q; i++) {
            p = code[i].line;
            uint8 src = 0;
            if (code[i].src == 8 && code[i].src_mode == 1) {
                if ((c = fgetc(input)) != EOF) src = c;
            } else {
                switch (code[i].src_mode) {
                    case 0: src = code[i].src; break;
                    case 1: src = reg[code[i].src]; break;
                    case 2: src = mem[reg[code[i].src]]; break;
                    case 3: src = mem[reg[code[i].src] + code[i].src_offset]; break;
                    case 4: src = mem[reg[code[i].src] + reg[code[i].src_offset]]; break;
                    case 5: src = mem[code[i].src]; break;
                    default: err("Syntax error");
                }
            }
            if (code[i].dest == 14 && code[i].dest_mode == 1) {
                putchar(src);
            } else if (code[i].dest == 25 && code[i].dest_mode == 1) {
                if (src != 0) {
                    fclose(input);
                    exit(0);
                }
            } else {
                switch (code[i].dest_mode) {
                    case 1: reg[code[i].dest] = src; break;
                    case 2: mem[reg[code[i].dest]] = src; break;
                    case 3: mem[reg[code[i].dest] + code[i].dest_offset] = src; break;
                    case 4: mem[reg[code[i].dest] + reg[code[i].dest_offset]] = src; break;
                    case 5: mem[code[i].dest] = src; break;
                    default: err("Syntax error");
                }
            }
        }
    }

    fclose(input);
    return 0;
}
