#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_LEN 128
#define GENERATIONS 20
#define EQ_TOLERANCE 50
#define EQ_MAX 10000
#define EXTENDED_GENERATIONS 50000000000

struct list_t {
    struct list_node_t *head;
    struct list_node_t *tail;
};

struct list_node_t {
    struct list_node_t *next;
    struct list_node_t *prev;
    long int id;
};

struct claim_t {
    unsigned int next_gen_plant: 1;
    unsigned int ll: 1;
    unsigned int l: 1;
    unsigned int c: 1;
    unsigned int r: 1;
    unsigned int rr: 1;
};

struct list_t build_initial_state_from_input(char buffer[], size_t len);
void release_state_resources(struct list_t state_list);
struct claim_t build_claim_from_input(char buffer[], size_t len);
int is_valid_claim(char buffer[], size_t len);
struct list_t advance_n_generations(struct list_t initial_state, struct claim_t claims[], int claims_len, int generations);
struct list_t advance_generation(struct list_t initial_state_list, struct claim_t claims[], int claims_len);
struct list_t duplicate_list(struct list_t list);
int claim_matches_list(struct claim_t *claim, struct list_node_t *closest_node, long int pos);
int sum_state_values(struct list_t list);
long long int find_equilibrium_state_sum(struct list_t initial_state_list, struct claim_t claims[], int claims_len, long int generations);

int main(int argc, char *argv[])
{
    char buffer[BUFF_LEN];
    if(fgets(buffer, BUFF_LEN, stdin) == NULL) {
        fprintf(stderr, "Unexpected input: end of input or error occurred\n");
        exit(1);
    }

    struct list_t initial_state_list = build_initial_state_from_input(buffer, BUFF_LEN);

    if(fgets(buffer, BUFF_LEN, stdin) == NULL || buffer[0] != '\n') {
        fprintf(stderr, "Unexpected input: end of input or error occurred\n");
        exit(1);
    }

    int claims_len = BUFF_LEN;
    int claims_index = 0;
    struct claim_t *claims = (struct claim_t *)malloc(sizeof(struct claim_t) * claims_len);
    if(claims == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    while(fgets(buffer, BUFF_LEN, stdin) != NULL) {
        if(claims_index >= claims_len) {
            claims_len += BUFF_LEN;
            claims = (struct claim_t *)realloc(claims, sizeof(struct claim_t) * claims_len);
            if(claims == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }
        }

        claims[claims_index] = build_claim_from_input(buffer, BUFF_LEN);
        claims_index++;
    }

    claims_len = claims_index;
    claims = (struct claim_t *)realloc(claims, sizeof(struct claim_t) * claims_len);
    if(claims == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    struct list_t advanced_state_list = advance_n_generations(initial_state_list, claims, claims_len, GENERATIONS);
    long long int sum = sum_state_values(advanced_state_list);
    fprintf(stdout, "After 20 generations, what is the sum of the numbers of all pots which contain a plant? %lld\n", sum);

    sum = find_equilibrium_state_sum(initial_state_list, claims, claims_len, EXTENDED_GENERATIONS);
    fprintf(stdout, "After fifty billion (50000000000) generations, what is the sum of the numbers of all pots which contain a plant? %lld\n", sum);

    free(claims);
    release_state_resources(initial_state_list);
    release_state_resources(advanced_state_list);

    return 0;
}

struct list_t build_initial_state_from_input(char buffer[], size_t len)
{
    struct list_t list = {.head = NULL, .tail = NULL};

    char *current_index = NULL;
    char *start_index = NULL;
    char *eos = NULL;
    char *token = NULL;

    eos = memchr(buffer, 0, len);
    if(eos == NULL) {
        fprintf(stderr, "Unexpected input: invalid initial state\n");
        exit(1);
    }

    token = memchr(buffer, '\n', eos-buffer);
    if(token != NULL) {
        *token = 0;
        eos = token;
    }

    token = memchr(buffer, ':', eos-buffer);
    if(token == NULL || (token + 2) > eos) {
        fprintf(stderr, "Unexpected input: invalid initial state\n");
        exit(1);
    }

    start_index = token + 2;
    current_index = start_index;
    while(current_index < eos) {
        if(*current_index != '.' && *current_index != '#') {
            fprintf(stderr, "Unexpected input: invalid initial state\n");
            exit(1);
        }

        if(*current_index == '#') {
            struct list_node_t *new_node = (struct list_node_t *)malloc(sizeof(struct list_node_t));
            if(new_node == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }

            if(list.tail == NULL) {
                list.head = new_node;
                list.tail = new_node;

                new_node->next = NULL;
                new_node->prev = NULL;
            } else {
                list.tail->next = new_node;
                new_node->prev = list.tail;
                new_node->next = NULL;
                list.tail = new_node;
            }

            new_node->id = current_index - start_index;
        }

        current_index++;
    }

    return list;
}

void release_state_resources(struct list_t state_list)
{
    struct list_node_t *current = state_list.head;
    while(current != NULL) {
        struct list_node_t *tmp = current;
        current = current->next;
        free(tmp);
    }
}

struct claim_t build_claim_from_input(char buffer[], size_t len)
{
    struct claim_t claim = {.ll = 0, .l = 0, .c = 0, .r = 0, .rr = 0, .next_gen_plant = 0};

    if(!is_valid_claim(buffer, len)) {
        fprintf(stderr, "Unexpected input: invalid claim\n");
        exit(1);
    }

    claim.ll = buffer[0] == '.' ? 0 : 1;
    claim.l = buffer[1] == '.' ? 0 : 1;
    claim.c = buffer[2] == '.' ? 0 : 1;
    claim.r = buffer[3] == '.' ? 0 : 1;
    claim.rr = buffer[4] == '.' ? 0 : 1;
    claim.next_gen_plant = buffer[9] == '.' ? 0 : 1;

    return claim;
}

int is_valid_claim(char buffer[], size_t len)
{
    char *start_index = buffer;
    char *eos = NULL;
    char *token = NULL;

    eos = memchr(buffer, 0, len);
    if(eos == NULL) {
        return 0;
    }

    token = memchr(buffer, '\n', len);
    if(token != NULL) {
        eos = token;
    }

    if((eos - start_index) != 10) {
        return 0;
    }

    for(int i = 0; i < 10; i++) {
        if((i < 5 || i > 8) && (buffer[i] != '.' && buffer[i] != '#')) {
            return 0;
        }
    }

    return 1;
}

struct list_t advance_n_generations(struct list_t initial_state_list, struct claim_t claims[], int claims_len, int generations)
{
    struct list_t advanced_generation_list = duplicate_list(initial_state_list);

    for(int generation = 0; generation < generations; generation++) {
        struct list_t gen_i = advance_generation(advanced_generation_list, claims, claims_len);

        release_state_resources(advanced_generation_list);
        advanced_generation_list = gen_i;
    }

    return advanced_generation_list;
}

struct list_t advance_generation(struct list_t initial_state_list, struct claim_t claims[], int claims_len)
{
    if(initial_state_list.head == NULL) {
        return initial_state_list;
    }

    struct list_t new_list = {.head = NULL, .tail = NULL};

    struct list_node_t *closest_relative_node = initial_state_list.head;
    for(long int pos = initial_state_list.head->id - 3; pos <= initial_state_list.tail->id + 3; pos++) {
        while(closest_relative_node->next != NULL && closest_relative_node->id < pos) {
            closest_relative_node = closest_relative_node->next;
        }

        for(struct claim_t *claim = claims; claim < claims + claims_len; claim++) {
            if(!claim_matches_list(claim, closest_relative_node, pos)) {
                continue;
            }

            if(claim->next_gen_plant) {
                struct list_node_t *new_node = (struct list_node_t *)malloc(sizeof(struct list_node_t));
                if(new_node == NULL) {
                    perror("Fatal error: Cannot allocate memory.\n");
                    exit(EXIT_FAILURE);
                }

                if(new_list.head == NULL) {
                    new_node->id = pos;
                    new_node->next = NULL;
                    new_node->prev = NULL;

                    new_list.head = new_node;
                    new_list.tail = new_node;
                } else {
                    new_node->id = pos;
                    new_node->prev = new_list.tail;
                    new_node->next = NULL;

                    new_list.tail->next = new_node;
                    new_list.tail = new_node;
                }
            }
        }
    }

    return new_list;
}

struct list_t duplicate_list(struct list_t list)
{
    struct list_t new_list = {.head = NULL, .tail = NULL};

    struct list_node_t *current = list.head;
    while(current != NULL) {
        struct list_node_t *new_node = (struct list_node_t *)malloc(sizeof(struct list_node_t));
        if(new_node == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }

        new_node->next = NULL;
        new_node->prev = NULL;
        new_node->id = current->id;

        if(new_list.head == NULL) {
            new_list.head = new_node;
            new_list.tail = new_node;
        } else {
            new_node->prev = new_list.tail;
            new_list.tail->next = new_node;
            new_list.tail = new_node;
        }

        current = current->next;
    }

    return new_list;
}

int claim_matches_list(struct claim_t *claim, struct list_node_t *closest_node, long int pos)
{
    struct list_node_t *c = NULL;
    struct list_node_t *l = NULL;
    struct list_node_t *ll = NULL;
    struct list_node_t *r = NULL;
    struct list_node_t *rr = NULL;

    struct list_node_t *current = closest_node;
    while(current->next != NULL && current->id <= pos + 3) {
        current = current->next;
    }

    while(current != NULL && current->id >= pos - 3) {
        if(current->id == pos + 2)
            rr = current;
        else if(current->id == pos + 1)
            r = current;
        else if(current->id == pos)
            c = current;
        else if(current->id == pos - 1)
            l = current;
        else if(current->id == pos - 2)
            ll = current;

        current = current->prev;
    }

    if((rr == NULL) == claim->rr)
        return 0;

    if((r == NULL) == claim->r)
        return 0;

    if((c == NULL) == claim->c)
        return 0;

    if((l == NULL) == claim->l)
        return 0;

    if((ll == NULL) == claim->ll)
        return 0;

    return 1;
}

int sum_state_values(struct list_t list)
{
    int sum = 0;
    struct list_node_t *current = list.head;
    while(current != NULL) {
        sum += current->id;
        current = current->next;
    }

    return sum;
}

long long int find_equilibrium_state_sum(struct list_t initial_state_list, struct claim_t claims[], int claims_len, long int generations)
{
    struct list_t advanced_generation_list = duplicate_list(initial_state_list);

    int diff = 0;
    int count = 0;
    int iterations = 0;
    while(count < EQ_TOLERANCE) {
        struct list_t gen_i = advance_generation(advanced_generation_list, claims, claims_len);

        int current_diff = sum_state_values(gen_i) - sum_state_values(advanced_generation_list);
        if(current_diff == diff) {
            count++;
        } else {
            count = 0;
            diff = current_diff;
        }

        release_state_resources(advanced_generation_list);
        advanced_generation_list = gen_i;

        iterations++;

        if(iterations > EQ_MAX) {
            fprintf(stderr, "Limit reached: did not reach steady state equilibrium after %d iterations.\n", EQ_MAX);
            exit(1);
        }
    }

    long long int sum = sum_state_values(advanced_generation_list) + (diff * (generations - iterations));

    release_state_resources(advanced_generation_list);

    return sum;
}