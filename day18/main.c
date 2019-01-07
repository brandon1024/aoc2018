#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM 50
#define BUFF_LEN 64
#define TOLERANCE 100

int value_after_n_minutes(unsigned char **area, int minutes, int fast_forward);
int adjacent_acres_count(unsigned char **area, size_t x, size_t y, unsigned char type);
int is_valid_point(size_t x, size_t y);
int compute_value(unsigned char **area);

int main(int argc, char *argv[])
{
    char buffer[BUFF_LEN];

    size_t area_len = DIM;
    size_t area_index = 0;
    unsigned char **area = (unsigned char **)malloc(sizeof(unsigned char *) * area_len);
    if(area == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    while(fgets(buffer, BUFF_LEN, stdin) != NULL) {
        if(area_index >= area_len) {
            fprintf(stderr, "Unexpected error: unexpected dimensions.\n");
            exit(EXIT_FAILURE);
        }

        char *eos = memchr(buffer, 0, BUFF_LEN);
        if(eos == NULL) {
            fprintf(stderr, "Unexpected error: input exceeds buffer size.\n");
            exit(EXIT_FAILURE);
        }

        char *lf = memchr(buffer, '\n', BUFF_LEN);
        if(lf != NULL) {
            eos = lf;
            *eos = 0;
        }

        if((eos - buffer) != DIM) {
            fprintf(stderr, "Unexpected error: unexpected dimensions.\n");
            exit(EXIT_FAILURE);
        }

        area[area_index] = (unsigned char *)calloc(DIM, sizeof(unsigned char));
        if(area[area_index] == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }

        for(char *c = buffer; c < eos; c++) {
            switch(*c) {
                case '.':
                    area[area_index][c - buffer] = 0;
                    break;
                case '|':
                    area[area_index][c - buffer] = 1;
                    break;
                case '#':
                    area[area_index][c - buffer] = 2;
                    break;
                default:
                    fprintf(stderr, "Unexpected error: unexpected input %.*s.\n", BUFF_LEN, buffer);
                    exit(EXIT_FAILURE);
            }
        }

        area_index++;
    }

    int value = value_after_n_minutes(area, 10, 0);
    fprintf(stdout, "What will the total resource value of the lumber collection area be after 10 minutes? %d\n", value);

    value = value_after_n_minutes(area, 1000000000, 1);
    fprintf(stdout, "What will the total resource value of the lumber collection area be after 1000000000 minutes? %d\n", value);

    for(size_t i = 0; i < DIM; i++)
        free(area[i]);
    free(area);

    return 0;
}

int value_after_n_minutes(unsigned char **area, int minutes, int fast_forward)
{
    unsigned char **area_cpy = (unsigned char **)malloc(sizeof(unsigned char *) * DIM);
    if(area_cpy == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for(size_t i = 0; i < DIM; i++) {
        area_cpy[i] = (unsigned char *)calloc(DIM, sizeof(unsigned char));
        if(area_cpy[i] == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }

        memcpy(area_cpy[i], area[i], DIM * sizeof(unsigned char));
    }

    int sample = 0;
    int sample_count = 0;
    int sample_index = 0;
    for(int minute = 0; minute < minutes; minute++) {
        unsigned char **tmp = (unsigned char **)malloc(sizeof(unsigned char *) * DIM);
        if(tmp == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }

        for(size_t i = 0; i < DIM; i++) {
            tmp[i] = (unsigned char *)calloc(DIM, sizeof(unsigned char));
            if(tmp[i] == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }

            memcpy(tmp[i], area_cpy[i], DIM * sizeof(unsigned char));
        }

        for(size_t i = 0; i < DIM; i++) {
            for(size_t j = 0; j < DIM; j++) {
                switch(area_cpy[i][j]) {
                    case 0:
                        if(adjacent_acres_count(area_cpy, j, i, 1) >= 3) {
                            tmp[i][j] = 1;
                        }
                        break;
                    case 1:
                        if(adjacent_acres_count(area_cpy, j, i, 2) >= 3) {
                            tmp[i][j] = 2;
                        }
                        break;
                    case 2:
                        if(adjacent_acres_count(area_cpy, j, i, 2) < 1 || adjacent_acres_count(area_cpy, j, i, 1) < 1) {
                            tmp[i][j] = 0;
                        }
                        break;
                    default:
                        fprintf(stderr, "Unexpected error: unexpected acre value.\n");
                        exit(EXIT_FAILURE);
                }
            }
        }

        for(size_t i = 0; i < DIM; i++) {
            free(area_cpy[i]);
            area_cpy[i] = tmp[i];
        }

        free(tmp);

        if(fast_forward) {
            int val = compute_value(area_cpy);
            if(sample == val && sample_count == 50) {
                minute += ((minutes - minute) / (minute - sample_index)) * (minute - sample_index);

                sample = val;
                sample_index = minute;
                sample_count = 0;
            } else if(sample == val) {
                sample_count++;
                sample_index = minute;
            } else if((minute - sample_index) == TOLERANCE) {
                sample = val;
                sample_index = minute;
                sample_count = 0;
            }
        }
    }

    int value = compute_value(area_cpy);

    for(size_t i = 0; i < DIM; i++)
        free(area_cpy[i]);

    free(area_cpy);

    return value;
}

int adjacent_acres_count(unsigned char **area, size_t x, size_t y, unsigned char type)
{
    int count = 0;

    if(is_valid_point(x-1, y-1) && area[y-1][x-1] == type)
        count++;

    if(is_valid_point(x, y-1) && area[y-1][x] == type)
        count++;

    if(is_valid_point(x+1, y-1) && area[y-1][x+1] == type)
        count++;

    if(is_valid_point(x-1, y) && area[y][x-1] == type)
        count++;

    if(is_valid_point(x+1, y) && area[y][x+1] == type)
        count++;

    if(is_valid_point(x-1, y+1) && area[y+1][x-1] == type)
        count++;

    if(is_valid_point(x, y+1) && area[y+1][x] == type)
        count++;

    if(is_valid_point(x+1, y+1) && area[y+1][x+1] == type)
        count++;

    return count;
}

int is_valid_point(size_t x, size_t y)
{
    return x >= 0 && x < DIM && y >= 0 && y < DIM;
}

int compute_value(unsigned char **area)
{
    int wood_areas = 0;
    int lumberyards = 0;
    for(size_t i = 0; i < DIM; i++) {
        for(size_t j = 0; j < DIM; j++) {
            if(area[i][j] == 1) {
                wood_areas++;
            } else if(area[i][j] == 2) {
                lumberyards++;
            }
        }
    }

    return wood_areas * lumberyards;
}