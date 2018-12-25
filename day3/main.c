#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FABRIC_DIM 1000
#define BUFF_LEN 32

struct claim_t {
    long int claim_id;
    long int pos_x;
    long int pos_y;
    long int dim_x;
    long int dim_y;
};

struct claim_t build_claim(char *buffer, size_t buff_len);
int find_overlapping_fabric_inches(int **fabric);
long int find_non_overlapping_id(int **fabric, struct claim_t *claims, int claims_len);
int claim_overlaps(int **fabric, struct claim_t claim);

int main(int argc, char *argv[])
{
    int **fabric = (int **)malloc(sizeof(int *) * FABRIC_DIM);
    if(fabric == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < FABRIC_DIM; i++) {
        fabric[i] = (int *)malloc(sizeof(int) * FABRIC_DIM);
        if(fabric[i] == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }

        for(int j = 0; j < FABRIC_DIM; j++) {
            fabric[i][j] = 0;
        }
    }

    char *buffer = (char *)malloc(sizeof(char) * BUFF_LEN);
    if(buffer == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    memset(buffer, 0, sizeof(char) * BUFF_LEN);

    int claims_len = BUFF_LEN;
    int claims_index = 0;
    struct claim_t *claims = (struct claim_t *)malloc(sizeof(struct claim_t) * claims_len);
    if(claims == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    while((fgets(buffer, BUFF_LEN, stdin)) != NULL) {
        char *lf = memchr(buffer, '\n', BUFF_LEN);
        if(lf != NULL) {
            *lf = 0;
        }

        char *eos = memchr(buffer, 0, BUFF_LEN);
        if(eos == NULL) {
            fprintf(stderr, "Unexpected input: %32s\n", buffer);
            exit(1);
        }

        struct claim_t claim = build_claim(buffer, BUFF_LEN);
        for(long int i = claim.pos_y; (i < (claim.pos_y + claim.dim_y)) && (i < FABRIC_DIM); i++) {
            for(long int j = claim.pos_x; (j < (claim.pos_x + claim.dim_x)) && (j < FABRIC_DIM); j++) {
                fabric[i][j] = fabric[i][j] + 1;
            }
        }

        if(claims_index >= claims_len) {
            claims_len = claims_len + BUFF_LEN;
            claims = (struct claim_t *)realloc(claims, sizeof(struct claim_t) * claims_len);
            if(claims == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }
        }

        claims[claims_index] = claim;
        claims_index++;
    }

    int fabric_inches = find_overlapping_fabric_inches(fabric);
    long int non_overlapping_id = find_non_overlapping_id(fabric, claims, claims_len);

    fprintf(stdout, "How many square inches of fabric are within two or more claims? %d\n", fabric_inches);
    fprintf(stdout, "What is the ID of the only claim that doesn't overlap? %ld\n", non_overlapping_id);

    for(int i = 0; i < FABRIC_DIM; i++) {
        free(fabric[i]);
    }

    free(fabric);
    free(buffer);
    free(claims);

    return 0;
}

struct claim_t build_claim(char *buffer, size_t buff_len)
{
    struct claim_t c;
    char *start_index;
    char *token;

    start_index = memchr(buffer, '#', buff_len) + 1;
    token = memchr(buffer, ' ', buff_len);
    *token = 0;
    c.claim_id = strtol(start_index, &start_index, 10);

    start_index = token + 3;
    token = memchr(buffer, ',', buff_len);
    *token = 0;
    c.pos_y = strtol(start_index, &start_index, 10);

    start_index = token + 1;
    token = memchr(buffer, ':', buff_len);
    *token = 0;
    c.pos_x = strtol(start_index, &start_index, 10);

    start_index = token + 2;
    token = memchr(buffer, 'x', buff_len);
    *token = 0;
    c.dim_y = strtol(start_index, &start_index, 10);

    start_index = token + 1;
    c.dim_x = strtol(start_index, &start_index, 10);

    return c;
}

int find_overlapping_fabric_inches(int **fabric)
{
    int fabric_inches = 0;
    for(long int i = 0; i < FABRIC_DIM; i++) {
        for(long int j = 0; j < FABRIC_DIM; j++) {
            if(fabric[i][j] >= 2) {
                fabric_inches++;
            }
        }
    }

    return fabric_inches;
}

long int find_non_overlapping_id(int **fabric, struct claim_t *claims, int claims_len)
{
    for(int claim_index = 0; claim_index < claims_len; claim_index++) {
        struct claim_t claim = claims[claim_index];

        if(!claim_overlaps(fabric, claim)) {
            return claim.claim_id;
        }

    }

    return -1;
}

int claim_overlaps(int **fabric, struct claim_t claim)
{
    for(long int i = claim.pos_y; (i < (claim.pos_y + claim.dim_y)) && (i < FABRIC_DIM); i++) {
        for(long int j = claim.pos_x; (j < (claim.pos_x + claim.dim_x)) && (j < FABRIC_DIM); j++) {
            if(fabric[i][j] != 1) {
                return 1;
            }
        }
    }

    return 0;
}