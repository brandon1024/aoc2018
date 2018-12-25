#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_LEN 8
#define CELL_DIM 300
#define INTERFACE_DIM 3

struct coord_t {
    int x;
    int y;
};

struct power_info_t {
    struct coord_t coord;
    int largest_power;
    int dim;
};

int **build_power_values_table(int dim, int grid_serial_num);
int determine_power_value(struct coord_t coord, int grid_serial_number);
int **build_summed_area_table(int *power_values[], int dim);
void release_table_resources(int *table[], int dim);
struct power_info_t determine_largest_total_power_interface(int *sat[], int dim, int interface_dim);

int main(int argc, char *argv[])
{
    char buffer[BUFF_LEN];
    if(fgets(buffer, BUFF_LEN, stdin) == NULL) {
        fprintf(stderr, "Unexpected error: cannot read puzzle input from stdin.\n");
        exit(1);
    }

    char *eos = memchr(buffer, 0, BUFF_LEN);
    if(eos == NULL) {
        fprintf(stderr, "Unexpected error: input exceeds buffer size.\n");
        exit(1);
    }

    int grid_serial_num = (int)strtol(buffer, &eos, 10);
    int **power_values = build_power_values_table(CELL_DIM, grid_serial_num);
    int **sat = build_summed_area_table(power_values, CELL_DIM);

    struct power_info_t largest_pow_int = determine_largest_total_power_interface(sat, CELL_DIM, INTERFACE_DIM);
    fprintf(stdout, "What is the X,Y coordinate of the top-left fuel cell of the 3x3 square with the largest total power? %d,%d\n", largest_pow_int.coord.x, largest_pow_int.coord.y);

    for(int i = 1; i <= CELL_DIM; i++) {
        struct power_info_t tmp = determine_largest_total_power_interface(sat, CELL_DIM, i);
        if(tmp.largest_power > largest_pow_int.largest_power) {
            largest_pow_int = tmp;
        }
    }

    fprintf(stdout, "What is the X,Y,size identifier of the square with the largest total power? %d,%d,%d\n", largest_pow_int.coord.x, largest_pow_int.coord.y, largest_pow_int.dim);

    release_table_resources(power_values, CELL_DIM);
    release_table_resources(sat, CELL_DIM);

    return 0;
}

int **build_power_values_table(int dim, int grid_serial_num)
{
    int **power_values = (int **)malloc(sizeof(int *) * dim);
    if(power_values == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < CELL_DIM; i++) {
        power_values[i] = (int *)malloc(sizeof(int) * dim);
        if(power_values[i] == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }

        for(int j = 0; j < CELL_DIM; j++) {
            struct coord_t this = {.x = j, .y = i};
            power_values[i][j] = determine_power_value(this, grid_serial_num);
        }
    }

    return power_values;
}

int determine_power_value(struct coord_t coord, int grid_serial_number)
{
    coord.x += 1;
    coord.y += 1;

    int rack_id = coord.x + 10;

    return (rack_id * coord.y + grid_serial_number) * rack_id / 100 % 10 - 5;
}

int **build_summed_area_table(int *power_values[], int dim) {
    int **sat = (int **) malloc(sizeof(int *) * dim);
    if (sat == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < CELL_DIM; i++) {
        sat[i] = (int *) malloc(sizeof(int) * dim);
        if (sat[i] == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < dim; i++) {
        sat[0][i] = power_values[0][i];
    }

    for (int i = 1; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            sat[i][j] = sat[i-1][j] + power_values[i][j];
        }
    }

    for (int i = 0; i < dim; i++) {
        for (int j = 1; j < dim; j++) {
            sat[i][j] += sat[i][j-1];
        }
    }

    return sat;
}

void release_table_resources(int *table[], int dim)
{
    for(int i = 0; i < dim; i++) {
        free(table[i]);
    }

    free(table);
}

struct power_info_t determine_largest_total_power_interface(int *sat[], int dim, int interface_dim)
{
    struct coord_t best = {.x = 0, .y = 0};
    int best_val = 0;
    int initialized = 0;

    for(int y = 0; y <= dim - interface_dim; y++) {
        for(int x = 0; x <= dim -interface_dim; x++) {
            int total = sat[y + interface_dim - 1][x + interface_dim - 1];

            if(y > 0) {
                total -= sat[y - 1][x + interface_dim - 1];
            }

            if(x > 0) {
                total -= sat[y + interface_dim - 1][x - 1];
            }

            if(y > 0 && x > 0) {
                total += sat[y - 1][x - 1];
            }

            if(!initialized || total > best_val) {
                best.x = x + 1;
                best.y = y + 1;
                best_val = total;
                initialized = 1;
            }
        }
    }

    struct power_info_t power_info = {.coord = best, .largest_power = best_val, .dim = interface_dim};
    return power_info;
}