#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_LEN 32

#define addr(a,b,c) *(int *)(c) = *(const int *)(a) + *(const int *)(b)
#define addi(a,b,c) *(int *)(c) = *(const int *)(a) + b
#define mulr(a,b,c) *(int *)(c) = *(const int *)(a) * *(const int *)(b)
#define muli(a,b,c) *(int *)(c) = *(const int *)(a) * b
#define banr(a,b,c) *(int *)(c) = *(const int *)(a) & *(const int *)(b)
#define bani(a,b,c) *(int *)(c) = *(const int *)(a) & b
#define borr(a,b,c) *(int *)(c) = *(const int *)(a) | *(const int *)(b)
#define bori(a,b,c) *(int *)(c) = *(const int *)(a) | b
#define setr(a,c) *(int *)(c) = *(const int *)(a)
#define seti(a,c) *(int *)(c) = a
#define gtir(a,b,c) *(int *)(c) = (a > *(const int *)(b))
#define gtri(a,b,c) *(int *)(c) = (*(const int *)(a) > b)
#define gtrr(a,b,c) *(int *)(c) = (*(const int *)(a) > *(const int *)(b))
#define eqir(a,b,c) *(int *)(c) = (a == *(const int *)(b))
#define eqri(a,b,c) *(int *)(c) = (*(const int *)(a) == b)
#define eqrr(a,b,c) *(int *)(c) = (*(const int *)(a) == *(const int *)(b))

struct operation_t {
    unsigned char opcode;
    unsigned char in_a;
    unsigned char in_b;
    unsigned char out;
};

struct program_t {
    unsigned int registers[6];
    struct operation_t *instructions;
    size_t program_len;
    size_t pc_reg;
};

struct op_map_t {
    const char *instr_name;
    unsigned char code;
};

static const struct op_map_t map[] = {
    {"addr", 0},{"addi", 1},
    {"mulr", 2},{"muli", 3},
    {"banr", 4},{"bani", 5},
    {"borr", 6},{"bori", 7},
    {"setr", 8},{"seti", 9},
    {"gtir", 10},{"gtri", 11},{"gtrr", 12},
    {"eqir", 13},{"eqri", 14},{"eqrr", 15},
    NULL
};

int compile(FILE *fd, struct program_t *program);
int execute(struct program_t *program);

int main(int argc, char *argv[])
{
    freopen("input.in", "r", stdin);

    struct program_t program;
    int ret = compile(stdin, &program);
    if(ret == 1) {
        fprintf(stderr, "Fatal Error: unable to compile source.\n");
        exit(EXIT_FAILURE);
    }

    execute(&program);
    fprintf(stdout, "What value is left in register 0 when the background process halts? %d\n", program.registers[0]);

    fprintf(stdout, "Part 2 cannot be easily computed (in an efficient manner),\n"
                    "and requires a bit of reverse engineering to optimize before\n"
                    "it can be computed in a reasonable amount of time.\n");

    free(program.instructions);

    return 0;
}

int compile(FILE *fd, struct program_t *program)
{
    char buffer[BUFF_LEN];

    program->program_len = 0;
    program->instructions = NULL;
    program->pc_reg = 0;

    size_t program_index = 0;
    while(fgets(buffer, BUFF_LEN, fd) != NULL) {
        char *eos = memchr(buffer, 0, BUFF_LEN);
        if(eos == NULL) {
            return 1;
        }

        char *lf = memchr(buffer, '\n', eos - buffer);
        if(lf != NULL) {
            eos = lf;
            *eos = 0;
        }

        if(!strncmp("#ip", buffer, ((eos - buffer) < 3) ? eos - buffer : 3)) {
            char *token = memchr(buffer, ' ', eos - buffer);
            if(token == NULL) {
                return 1;
            }

            *token = 0;
            token++;
            program->pc_reg = (unsigned char)strtol(token, &token, 10);

            if(program->pc_reg >= 6)
                return 1;
        } else {
            struct operation_t op = {.opcode = 0, .in_a = 0, .in_b = 0, .out = 0};

            char *start = buffer;
            char *token = memchr(start, ' ', eos - start);
            if(token == NULL)
                return 1;

            *token = 0;

            struct op_map_t *i = (struct op_map_t *)map;
            while(i != NULL) {
                if(!strcmp(i->instr_name, buffer))
                    break;

                i++;
            }

            if(i == NULL)
                return 1;

            op.opcode = i->code;

            start = token + 1;
            token = memchr(start, ' ', eos - start);
            if(token == NULL)
                return 1;

            *token = 0;
            op.in_a = (unsigned char)strtol(start, &token, 10);

            start = token + 1;
            token = memchr(start, ' ', eos - start);
            if(token == NULL)
                return 1;

            *token = 0;
            op.in_b = (unsigned char)strtol(start, &token, 10);

            start = token + 1;
            op.out = (unsigned char)strtol(start, &token, 10);

            if(program_index >= program->program_len) {
                program->program_len += BUFF_LEN;
                program->instructions = (struct operation_t *)realloc(program->instructions, sizeof(struct operation_t) * program->program_len);
                if(program->instructions == NULL)
                    return 1;
            }

            program->instructions[program_index++] = op;
        }
    }

    program->program_len = program_index;
    program->instructions = (struct operation_t *)realloc(program->instructions, sizeof(struct operation_t) * program->program_len);
    if(program->instructions == NULL)
        return 1;

    return 0;
}

int execute(struct program_t *program)
{
    memset(program->registers, 0, sizeof(unsigned int) * 6);
    program->registers[0] = 0;

    while(program->registers[program->pc_reg] >= 0 && program->registers[program->pc_reg] < program->program_len) {
        struct operation_t op = program->instructions[program->registers[program->pc_reg]];
        switch(op.opcode) {
            case 0:
                addr(program->registers + op.in_a, program->registers + op.in_b, program->registers + op.out);
                break;
            case 1:
                addi(program->registers + op.in_a, op.in_b, program->registers + op.out);
                break;
            case 2:
                mulr(program->registers + op.in_a, program->registers + op.in_b, program->registers + op.out);
                break;
            case 3:
                muli(program->registers + op.in_a, op.in_b, program->registers + op.out);
                break;
            case 4:
                banr(program->registers + op.in_a, program->registers + op.in_b, program->registers + op.out);
                break;
            case 5:
                bani(program->registers + op.in_a, op.in_b, program->registers + op.out);
                break;
            case 6:
                borr(program->registers + op.in_a, program->registers + op.in_b, program->registers + op.out);
                break;
            case 7:
                bori(program->registers + op.in_a, op.in_b, program->registers + op.out);
                break;
            case 8:
                setr(program->registers + op.in_a, program->registers + op.out);
                break;
            case 9:
                seti(op.in_a, program->registers + op.out);
                break;
            case 10:
                gtir(op.in_a, program->registers + op.in_b, program->registers + op.out);
                break;
            case 11:
                gtri(program->registers + op.in_a, op.in_b, program->registers + op.out);
                break;
            case 12:
                gtrr(program->registers + op.in_a, program->registers + op.in_b, program->registers + op.out);
                break;
            case 13:
                eqir(op.in_a, program->registers + op.in_b, program->registers + op.out);
                break;
            case 14:
                eqri(program->registers + op.in_a, op.in_b, program->registers + op.out);
                break;
            default:
                eqrr(program->registers + op.in_a, program->registers + op.in_b, program->registers + op.out);
                break;
        }

        program->registers[program->pc_reg]++;
    }

    return 0;
}