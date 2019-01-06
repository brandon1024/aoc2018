#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#define BUFF_LEN 32

struct operation_t {
    int opcode;
    int in_a;
    int in_b;
    int out;
};

struct sample_t {
    struct operation_t op;
    int before[4];
    int after[4];
};

struct input_t {
    struct sample_t *samples;
    struct operation_t *program;
    size_t samples_len;
    size_t program_len;
};

struct input_t build_from_input(FILE *fd);
int build_sample_from_input(char buffer[], size_t buff_len, int regs[]);
int build_op_from_input(char buffer[], size_t buff_len, struct operation_t *op);
int find_similar_behaviours(struct sample_t *samples, size_t samples_len, int **likely_codes);
int execute_program(struct operation_t *program, size_t program_len, int **likely_codes);

void addr(const int *a, const int *b, int *c);
void addi(const int *a, int b, int *c);
void mulr(const int *a, const int *b, int *c);
void muli(const int *a, int b, int *c);
void banr(const int *a, const int *b, int *c);
void bani(const int *a, int b, int *c);
void borr(const int *a, const int *b, int *c);
void bori(const int *a, int b, int *c);
void setr(const int *a, int *c);
void seti(int a, int *c);
void gtir(int a, const int *b, int *c);
void gtri(const int *a, int b, int *c);
void gtrr(const int *a, const int *b, int *c);
void eqir(int a, const int *b, int *c);
void eqri(const int *a, int b, int *c);
void eqrr(const int *a, const int *b, int *c);

int main(int argc, char *argv[])
{
    struct input_t input = build_from_input(stdin);

    int **counts = (int **)malloc(sizeof(int *) * 16);
    if(counts == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for(size_t i = 0; i < 16; i++) {
        counts[i] = (int *)calloc(16, sizeof(int));
        if(counts[i] == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }
    }

    int count = find_similar_behaviours(input.samples, input.samples_len, counts);
    fprintf(stdout, "How many samples in your puzzle input behave like three or more opcodes? %d\n", count);

    int reg_val = execute_program(input.program, input.program_len, counts);
    fprintf(stdout, "What value is contained in register 0 after executing the test program? %d\n", reg_val);

    for(size_t i = 0; i < 16; i++)
        free(counts[i]);
    free(counts);

    free(input.samples);
    free(input.program);

    return 0;
}

struct input_t build_from_input(FILE *fd)
{
    char buff[BUFF_LEN];
    struct input_t input = {.samples = NULL, .samples_len = 0, .program = NULL, .program_len = 0};

    size_t samples_index = 0;
    size_t program_index = 0;
    while(fgets(buff, BUFF_LEN, fd) != NULL) {
        char *eos = memchr(buff, 0, BUFF_LEN);
        if(eos == NULL) {
            fprintf(stderr, "Unexpected error: input exceeds buffer size.\n");
            exit(EXIT_FAILURE);
        }

        char *lf = memchr(buff, '\n', eos - buff);
        if(lf != NULL) {
            *lf = 0;
            eos = lf;
        }

        if((eos - buff) == 0) {
            continue;
        }

        if(memchr(buff, ':', eos - buff) != NULL) {
            int ret = 0;
            struct sample_t sample = {
                    .before = {0},
                    .after = {0},
                    .op = {
                            .opcode = 0,
                            .in_a = 0,
                            .in_b = 0,
                            .out = 0
                    }
            };

            ret = build_sample_from_input(buff, BUFF_LEN, sample.before);
            if(ret == 1) {
                fprintf(stderr, "Unexpected error: invalid input: %.*s.\n", BUFF_LEN, buff);
                exit(EXIT_FAILURE);
            }

            if(fgets(buff, BUFF_LEN, fd) == NULL) {
                fprintf(stderr, "Unexpected error: unexpected end of input.\n");
                exit(EXIT_FAILURE);
            }

            ret = build_op_from_input(buff, BUFF_LEN, &sample.op);
            if(ret == 1) {
                fprintf(stderr, "Unexpected error: invalid input: %.*s.\n", BUFF_LEN, buff);
                exit(EXIT_FAILURE);
            }

            if(fgets(buff, BUFF_LEN, fd) == NULL) {
                fprintf(stderr, "Unexpected error: unexpected end of input.\n");
                exit(EXIT_FAILURE);
            }

            ret = build_sample_from_input(buff, BUFF_LEN, sample.after);
            if(ret == 1) {
                fprintf(stderr, "Unexpected error: invalid input: %.*s.\n", BUFF_LEN, buff);
                exit(EXIT_FAILURE);
            }

            if(samples_index >= input.samples_len) {
                input.samples_len = (input.samples_len < 16) ? 16 : input.samples_len * 2;
                input.samples = (struct sample_t *)realloc(input.samples, input.samples_len * sizeof(struct sample_t));
                if(input.samples == NULL) {
                    perror("Fatal error: Cannot allocate memory.\n");
                    exit(EXIT_FAILURE);
                }
            }

            input.samples[samples_index++] = sample;
        } else {
            struct operation_t op = {.opcode = 0, .in_a = 0, .in_b = 0, .out = 0};
            int ret = 0;

            ret = build_op_from_input(buff, BUFF_LEN, &op);
            if(ret == 1) {
                fprintf(stderr, "Unexpected error: invalid input: %.*s.\n", BUFF_LEN, buff);
                exit(EXIT_FAILURE);
            }

            if(program_index >= input.program_len) {
                input.program_len = (input.program_len < 16) ? 16 : input.program_len * 2;
                input.program = (struct operation_t *)realloc(input.program, input.program_len * sizeof(struct operation_t));
                if(input.program == NULL) {
                    perror("Fatal error: Cannot allocate memory.\n");
                    exit(EXIT_FAILURE);
                }
            }

            input.program[program_index++] = op;
        }
    }

    input.samples_len = samples_index;
    input.samples = (struct sample_t *)realloc(input.samples, input.samples_len * sizeof(struct sample_t));
    if(input.samples == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    input.program_len = program_index;
    input.program = (struct operation_t *)realloc(input.program, input.program_len * sizeof(struct operation_t));
    if(input.program == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    return input;
}

int build_sample_from_input(char buffer[], size_t buff_len, int regs[])
{
    char buff_cpy[buff_len];
    memcpy(buff_cpy, buffer, buff_len);

    char *eos = memchr(buff_cpy, 0, buff_len);
    if(eos == NULL) {
        return 1;
    }

    char *lf = memchr(buff_cpy, '\n', eos - buff_cpy);
    if(lf != NULL) {
        *lf = 0;
        eos = lf;
    }

    char *start = buff_cpy;
    char *token = memchr(start, '[', eos - start);
    if(token == NULL) {
        return 1;
    }

    start = token + 1;
    token = memchr(start, ',', eos - start);
    if(token == NULL) {
        return 1;
    }

    *token = 0;
    regs[0] = (int)strtol(start, &token, 10);

    start = token + 2;
    token = memchr(start, ',', eos - start);
    if(token == NULL) {
        return 1;
    }

    *token = 0;
    regs[1] = (int)strtol(start, &token, 10);

    start = token + 2;
    token = memchr(start, ',', eos - start);
    if(token == NULL) {
        return 1;
    }

    *token = 0;
    regs[2] = (int)strtol(start, &token, 10);

    start = token + 2;
    token = memchr(start, ']', eos - start);
    if(token == NULL) {
        return 1;
    }

    *token = 0;
    regs[3] = (int)strtol(start, &token, 10);

    return 0;
}

int build_op_from_input(char buffer[], size_t buff_len, struct operation_t *op)
{
    char buff_cpy[buff_len];
    memcpy(buff_cpy, buffer, buff_len);

    char *eos = memchr(buff_cpy, 0, buff_len);
    if(eos == NULL) {
        return 1;
    }

    char *lf = memchr(buff_cpy, '\n', eos - buff_cpy);
    if(lf != NULL) {
        *lf = 0;
        eos = lf;
    }

    char *start = buff_cpy;
    char *token = memchr(start, ' ', eos - start);
    if(token == NULL) {
        return 1;
    }

    *token = 0;
    op->opcode = (int)strtol(start, &token, 10);

    start = token + 1;
    token = memchr(start, ' ', eos - start);
    if(token == NULL) {
        return 1;
    }

    *token = 0;
    op->in_a = (int)strtol(start, &token, 10);

    start = token + 1;
    token = memchr(start, ' ', eos - start);
    if(token == NULL) {
        return 1;
    }

    *token = 0;
    op->in_b = (int)strtol(start, &token, 10);

    start = token + 1;
    op->out = (int)strtol(start, &token, 10);

    if(op->opcode > 15 || op->in_a > 3 || op->in_b > 3 || op->out > 3) {
        return 1;
    }

    return 0;
}

int find_similar_behaviours(struct sample_t *samples, size_t samples_len, int **likely_codes)
{
    int count = 0;

    for(size_t index = 0; index < samples_len; index++) {
        int curr = 0;

        int registers[4];
        for(int opcode = 0; opcode < 16; opcode++) {
            memcpy(registers, samples[index].before, sizeof(int) * 4);

            struct operation_t op = samples[index].op;
            switch(opcode) {
                case 0:
                    addr(registers + op.in_a, registers + op.in_b, registers + op.out);
                    break;
                case 1:
                    addi(registers + op.in_a, op.in_b, registers + op.out);
                    break;
                case 2:
                    mulr(registers + op.in_a, registers + op.in_b, registers + op.out);
                    break;
                case 3:
                    muli(registers + op.in_a, op.in_b, registers + op.out);
                    break;
                case 4:
                    banr(registers + op.in_a, registers + op.in_b, registers + op.out);
                    break;
                case 5:
                    bani(registers + op.in_a, op.in_b, registers + op.out);
                    break;
                case 6:
                    borr(registers + op.in_a, registers + op.in_b, registers + op.out);
                    break;
                case 7:
                    bori(registers + op.in_a, op.in_b, registers + op.out);
                    break;
                case 8:
                    setr(registers + op.in_a, registers + op.out);
                    break;
                case 9:
                    seti(op.in_a, registers + op.out);
                    break;
                case 10:
                    gtir(op.in_a, registers + op.in_b, registers + op.out);
                    break;
                case 11:
                    gtri(registers + op.in_a, op.in_b, registers + op.out);
                    break;
                case 12:
                    gtrr(registers + op.in_a, registers + op.in_b, registers + op.out);
                    break;
                case 13:
                    eqir(op.in_a, registers + op.in_b, registers + op.out);
                    break;
                case 14:
                    eqri(registers + op.in_a, op.in_b, registers + op.out);
                    break;
                default:
                    eqrr(registers + op.in_a, registers + op.in_b, registers + op.out);
                    break;
            }

            if(!memcmp(registers, samples[index].after, sizeof(int) * 4)) {
                curr++;
                likely_codes[op.opcode][opcode]++;
            }
        }

        if(curr >= 3) {
            count++;
        }
    }

    return count;
}

int execute_program(struct operation_t *program, size_t program_len, int **likely_codes)
{
    int done = 0;
    do {
        done = 0;

        int tot = 0;
        for(size_t i = 0; i < 16; i++) {
            int count = 0;
            size_t col = 0;

            for(size_t j = 0; j < 16; j++) {
                if(likely_codes[i][j] > 0) {
                    count++;
                    col = j;
                }
            }

            if(count == 0) {
                fprintf(stderr, "Unexpected error: nondeterministic sample.\n");
                exit(EXIT_FAILURE);
            }

            if(count == 1) {
                for(size_t k = 0; k < 16; k++) {
                    if(k == i)
                        continue;

                    likely_codes[k][col] = 0;
                }

                done++;
            } else {
                tot++;
            }
        }

        if(tot == 16) {
            fprintf(stderr, "Unexpected error: nondeterministic sample.\n");
            exit(EXIT_FAILURE);
        }
    } while(done < 16);

    int codes[16] = {0};
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 16; j++) {
            if(likely_codes[i][j] > 0) {
                codes[j] = i;
            }
        }
    }


    int registers[4] = {0};
    for(size_t pc = 0; pc < program_len; pc++) {
        struct operation_t op = program[pc];

        if(op.opcode == codes[0])
            addr(registers + op.in_a, registers + op.in_b, registers + op.out);
        else if(op.opcode == codes[1])
            addi(registers + op.in_a, op.in_b, registers + op.out);
        else if(op.opcode == codes[2])
            mulr(registers + op.in_a, registers + op.in_b, registers + op.out);
        else if(op.opcode == codes[3])
            muli(registers + op.in_a, op.in_b, registers + op.out);
        else if(op.opcode == codes[4])
            banr(registers + op.in_a, registers + op.in_b, registers + op.out);
        else if(op.opcode == codes[5])
            bani(registers + op.in_a, op.in_b, registers + op.out);
        else if(op.opcode == codes[6])
            borr(registers + op.in_a, registers + op.in_b, registers + op.out);
        else if(op.opcode == codes[7])
            bori(registers + op.in_a, op.in_b, registers + op.out);
        else if(op.opcode == codes[8])
            setr(registers + op.in_a, registers + op.out);
        else if(op.opcode == codes[9])
            seti(op.in_a, registers + op.out);
        else if(op.opcode == codes[10])
            gtir(op.in_a, registers + op.in_b, registers + op.out);
        else if(op.opcode == codes[11])
            gtri(registers + op.in_a, op.in_b, registers + op.out);
        else if(op.opcode == codes[12])
            gtrr(registers + op.in_a, registers + op.in_b, registers + op.out);
        else if(op.opcode == codes[13])
            eqir(op.in_a, registers + op.in_b, registers + op.out);
        else if(op.opcode == codes[14])
            eqri(registers + op.in_a, op.in_b, registers + op.out);
        else
            eqrr(registers + op.in_a, registers + op.in_b, registers + op.out);
    }

    return registers[0];
}

void addr(const int *a, const int *b, int *c)
{
    *c = *a + *b;
}

void addi(const int *a, int b, int *c)
{
    *c = *a + b;
}

void mulr(const int *a, const int *b, int *c)
{
    *c = *a * *b;
}

void muli(const int *a, int b, int *c)
{
    *c = *a * b;
}

void banr(const int *a, const int *b, int *c)
{
    *c = *a & *b;
}

void bani(const int *a, int b, int *c)
{
    *c = *a & b;
}

void borr(const int *a, const int *b, int *c)
{
    *c = *a | *b;
}

void bori(const int *a, int b, int *c)
{
    *c = *a | b;
}

void setr(const int *a, int *c)
{
    *c = *a;
}

void seti(int a, int *c)
{
    *c = a;
}

void gtir(int a, const int *b, int *c)
{
    *c = (a > *b);
}

void gtri(const int *a, int b, int *c)
{
    *c = (*a > b);
}

void gtrr(const int *a, const int *b, int *c)
{
    *c = (*a > *b);
}

void eqir(int a, const int *b, int *c)
{
    *c = (a == *b);
}

void eqri(const int *a, int b, int *c)
{
    *c = (*a == b);
}

void eqrr(const int *a, const int *b, int *c)
{
    *c = (*a == *b);
}
