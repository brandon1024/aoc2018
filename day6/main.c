#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BUFF_LEN 16

struct coord_t {
    int x;
    int y;
};

struct vector_t {
    struct coord_t point_a;
    struct coord_t point_b;
};

struct list_node_t {
    struct coord_t data;
    struct list_node_t *next;
};

struct point_info_t {
    struct coord_t coord;
    struct list_node_t *area_coords;
    unsigned int point_area_size;
};

struct coord_t build_coord_from_input(char *buffer, size_t buff_len);
int determine_largest_area(struct coord_t coords[], int coords_len);
int compute_point_areas(struct point_info_t point_infos[], int point_info_len);
int is_area_inf(struct point_info_t *point);
int coord_belong_to_point_area(struct point_info_t *point, struct coord_t coord);
int falls_within_manhattan_distance(struct vector_t vector, struct coord_t coord);
double direct_distance_between_points(struct coord_t point_a, struct coord_t point_b);
struct point_info_t **sort_point_infos_by_closest_distance(struct point_info_t src[], int len, struct point_info_t *dest[], int skip_index);

int main(int argc, char *argv[])
{
    char buffer[BUFF_LEN];

    int coords_len = BUFF_LEN * 2;
    int coords_index = 0;
    struct coord_t *coords = (struct coord_t *)malloc(sizeof(struct coord_t) * coords_len);
    if(coords == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    while((fgets(buffer, BUFF_LEN, stdin)) != NULL) {
        if(coords_index >= coords_len) {
            coords_len = coords_len + (BUFF_LEN * 2);
            coords = (struct coord_t *)realloc(coords, sizeof(struct coord_t) * coords_len);
            if(coords == NULL) {
                fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
                exit(1);
            }
        }

        coords[coords_index] = build_coord_from_input(buffer, BUFF_LEN);
        coords_index++;
    }

    int largest_area = determine_largest_area(coords, coords_index);
    if(largest_area == -1) {
        fprintf(stderr, "Unexpected error: could not determine largest area.\n");
        exit(1);
    }

    free(coords);

    return 0;
}

struct coord_t build_coord_from_input(char buffer[], size_t buff_len)
{
    char *lf = memchr(buffer, '\n', buff_len);
    if(lf != NULL) {
        *lf = 0;
    }

    char *eos = memchr(buffer, 0, buff_len);
    if(eos == NULL) {
        fprintf(stderr, "Unexpected input: %32s\n", buffer);
        exit(1);
    }

    char *start_index = buffer;
    char *separator = memchr(buffer, ',', buff_len);
    if(separator == NULL) {
        fprintf(stderr, "Unexpected input: %32s\n", buffer);
        exit(1);
    }

    *separator = 0;

    int x = (int)strtol(start_index, &start_index, 10);
    start_index = separator + 2;
    int y = (int)strtol(start_index, &start_index, 10);

    struct coord_t coord = {.x = x, .y = y};

    return coord;
}

int determine_largest_area(struct coord_t coords[], int coords_len)
{
    struct point_info_t *point_infos = (struct point_info_t *)malloc(sizeof(struct point_info_t) * coords_len);
    if(point_infos == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    for(int i = 0; i < coords_len; i++) {
        struct point_info_t point_info = {.coord = coords[i], .area_coords = NULL, .point_area_size = 0};

        point_infos[i] = point_info;
    }

    compute_point_areas(point_infos, coords_len);

    int largest_area = -1;
    for(int i = 0; i < coords_len; i++) {
        if(is_area_inf(point_infos + i)) {
            continue;
        }

        if(point_infos[i].point_area_size > largest_area) {
            largest_area = point_infos[i].point_area_size;
        }
    }

    free(point_infos);

    return largest_area;
}

int compute_point_areas(struct point_info_t point_infos[], int point_info_len)
{
    struct point_info_t **sorted_point_infos = (struct point_info_t **)malloc(sizeof(struct point_info_t *) * point_info_len - 1);
    if(sorted_point_infos == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    for(int point_info_index = 0; point_info_index < point_info_len; point_info_index++) {
        sorted_point_infos = sort_point_infos_by_closest_distance(point_infos, point_info_len, sorted_point_infos, point_info_index);

        for(int i = 0; i < point_info_len - 1; i++) {
            struct vector_t vector = {.point_a = point_infos[point_info_index].coord, .point_b = sorted_point_infos[i]->coord};

            const int delta_x = vector.point_b.x - vector.point_a.x;
            const int delta_y = vector.point_b.y - vector.point_a.y;
            const int mid = ((abs(delta_x) + abs(delta_y)) - 1) / 2;

            for(int offset_x = 0; offset_x <= mid && offset_x < abs(delta_x); offset_x++) {
                const int x = (delta_x < 0) ? vector.point_a.x - offset_x : vector.point_a.x + offset_x;

                for(int offset_y = 0; (offset_x + offset_y) <= mid  && offset_y < abs(delta_y); offset_y++) {
                    const int y = (delta_y < 0) ? vector.point_a.y - offset_y : vector.point_a.y + offset_y;

                    if(x == vector.point_a.x && y == vector.point_a.y) {
                        continue;
                    }

                    struct coord_t coord = {.x = x, .y = y};

                }
            }
        }
    }

    for(int point_info_index = 0; point_info_index < point_info_len; point_info_index++) {
        struct list_node_t *node = point_infos->area_coords;
        while(node != NULL) {
            struct list_node_t *tmp = node->next;
            free(node);
            node->next = NULL;

            node = tmp;
        }
    }

    return 1;
}

int is_area_inf(struct point_info_t *point)
{
    struct coord_t coord;

    coord.y = point->coord.y;
    coord.x = point->coord.x - 1;
    if(!coord_belong_to_point_area(point, coord)) {
        return 1;
    }

    coord.x = point->coord.x + 1;
    if(!coord_belong_to_point_area(point, coord)) {
        return 1;
    }

    coord.x = point->coord.x;
    coord.y = point->coord.y - 1;
    if(!coord_belong_to_point_area(point, coord)) {
        return 1;
    }

    coord.y = point->coord.y + 1;
    if(!coord_belong_to_point_area(point, coord)) {
        return 1;
    }

    return 0;
}

int coord_belong_to_point_area(struct point_info_t *point, struct coord_t coord)
{
    struct list_node_t *node = point->area_coords;
    while(node != NULL) {
        struct coord_t area_coord = node->data;

        if(area_coord.x == coord.x && area_coord.y == coord.y) {
            return 1;
        }

        node = node->next;
    }

    return 0;
}

int falls_within_manhattan_distance(struct vector_t vector, struct coord_t coord)
{
    const int delta_x = vector.point_b.x - vector.point_a.x;
    const int delta_y = vector.point_b.y - vector.point_a.y;
    const int mid = ((abs(delta_x) + abs(delta_y)) - 1) / 2;

    if((delta_x <= 0) && ((coord.x > vector.point_a.x) || (coord.x < (vector.point_a.x + delta_x)))) {
        return 0;
    }

    if((delta_x >= 0) && ((coord.x < vector.point_a.x) || (coord.x > (vector.point_a.x + delta_x)))) {
        return 0;
    }

    if((delta_y <= 0) && ((coord.y > vector.point_a.y) || (coord.y < (vector.point_a.y + delta_y)))) {
        return 0;
    }

    if((delta_y >= 0) && ((coord.y < vector.point_a.y) || (coord.y > (vector.point_a.y + delta_y)))) {
        return 0;
    }

    int i;
    if(delta_x < 0) {
        i = vector.point_a.x - coord.x;
    } else {
        i = coord.x - vector.point_a.x;
    }

    if(delta_y < 0) {
        i += vector.point_a.y - coord.y;
    } else {
        i += coord.y - vector.point_a.y;
    }

    if(i <= mid) {
        return 1;
    }

    return 0;
}

double direct_distance_between_points(struct coord_t point_a, struct coord_t point_b)
{
    return sqrt(pow(point_a.x - point_b.x, 2.0) + pow(point_a.y - point_b.y, 2.0));
}

struct point_info_t **sort_point_infos_by_closest_distance(struct point_info_t src[], int len, struct point_info_t *dest[], int skip_index)
{
    int dest_index = 0;

    struct point_info_t *best = NULL;
    while(dest_index < len - 1) {
        struct point_info_t *current_best = NULL;

        for(int i = 0; i < len; i++) {
            if(i == skip_index) {
                continue;
            }

            if(best == NULL) {
                if(current_best == NULL) {
                    current_best = src + i;
                    continue;
                }

                double dist_to_curr_best = direct_distance_between_points(src[skip_index].coord, current_best->coord);
                double dist_to_i = direct_distance_between_points(src[skip_index].coord, src[i].coord);

                if(dist_to_i < dist_to_curr_best) {
                    current_best = src + i;
                }
            } else {
                double dist_to_i = direct_distance_between_points(src[skip_index].coord, src[i].coord);
                double dist_to_best = direct_distance_between_points(src[skip_index].coord, best->coord);

                if(current_best == NULL) {
                    if(dist_to_i >= dist_to_best && best != (src + i)) {
                        current_best = src + i;
                    }

                    continue;
                }

                double dist_to_curr_best = direct_distance_between_points(src[skip_index].coord, current_best->coord);
                if(dist_to_i > dist_to_best && dist_to_i < dist_to_curr_best) {
                    current_best = src + i;
                }
            }
        }

        dest[dest_index] = current_best;
        dest_index++;
        best = current_best;
    }

    return dest;
}