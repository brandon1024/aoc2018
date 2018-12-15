#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BUFF_LEN 16

struct coord_t {
    int x;
    int y;
};

struct list_node_t {
    struct coord_t data;
    struct list_node_t *next;
    int locked: 1;
};

struct voronoi_point_t {
    struct coord_t coord;
    struct list_node_t *area_coords;
    int point_area_size;
    unsigned int inf: 1;
    unsigned int enclosed: 1;
};

struct coord_t build_coord_from_input(char buffer[], size_t buff_len);
int determine_largest_area(struct coord_t coords[], int coords_len);
int compute_point_areas(struct voronoi_point_t points[], int points_len);
int grow_cell(struct voronoi_point_t points[], int points_len, struct voronoi_point_t *focus, struct list_node_t **contentious_cells, struct coord_t coord);
int determine_global_boundaries(struct voronoi_point_t points[], int points_len, int *lower_x, int *upper_x, int *lower_y, int *upper_y);
int is_coord_outside_boundaries(struct coord_t coord, int lower_x, int upper_x, int lower_y, int upper_y);
int is_coord_in_list(struct list_node_t *head, struct coord_t coord);
void mark_cells_as_locked(struct voronoi_point_t *point);

int main(int argc, char *argv[])
{
    freopen("test.in","r",stdin);

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

    fprintf(stdout, "What is the size of the largest area that isn't infinite? %d\n", largest_area);

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
    struct voronoi_point_t *points = (struct voronoi_point_t *)malloc(sizeof(struct voronoi_point_t) * coords_len);
    if(points == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    for(int i = 0; i < coords_len; i++) {
        struct voronoi_point_t point_info = {.coord = coords[i], .area_coords = NULL, .point_area_size = 0, .inf = 0, .enclosed = 0};

        points[i] = point_info;
    }

    compute_point_areas(points, coords_len);

    int largest_area = -1;
    for(int i = 0; i < coords_len; i++) {
        if(points[i].inf) {
            continue;
        }

        if(points[i].point_area_size > largest_area) {
            largest_area = points[i].point_area_size;
        }
    }

    free(points);

    return largest_area;
}

int compute_point_areas(struct voronoi_point_t points[], int points_len)
{
    int lower_x = 0;
    int upper_x = 0;
    int lower_y = 0;
    int upper_y = 0;
    determine_global_boundaries(points, points_len, &lower_x, &upper_x, &lower_y, &upper_y);

    struct list_node_t *contentious_cells = NULL;

    int still_growing = 1;
    int tessellation_radius = 1;
    while(still_growing) {
        still_growing = 0;

        for(int i = 0; i < points_len; i++) {
            if(points[i].enclosed) {
                continue;
            }

            for(int y_offset = -tessellation_radius; y_offset <= tessellation_radius; y_offset++) {
                struct coord_t coord = {
                    .x = points[i].coord.x + (tessellation_radius - abs(y_offset)),
                    .y = points[i].coord.y + y_offset
                };

                if(!is_coord_outside_boundaries(coord, lower_x, upper_x, lower_y, upper_y)) {
                    if(grow_cell(points, points_len, points + i, &contentious_cells, coord)) {
                        still_growing = 1;
                    }
                } else {
                    points[i].inf = 1;
                }

                if(coord.x == points[i].coord.x) {
                    continue;
                }

                coord.x = points[i].coord.x - (tessellation_radius - abs(y_offset));

                if(!is_coord_outside_boundaries(coord, lower_x, upper_x, lower_y, upper_y)) {
                    if(grow_cell(points, points_len, points + i, &contentious_cells, coord)) {
                        still_growing = 1;
                    }
                } else {
                    points[i].inf = 1;
                }
            }
        }

        //mark all cells as locked
        for(int i = 0; i < points_len; i++) {
            mark_cells_as_locked(points + i);
        }

        tessellation_radius++;
    }

    for(int i = 0; i < points_len; i++) {
        int area = 0;
        struct list_node_t *next = points[i].area_coords;
        while(next != NULL) {
            area++;

            struct list_node_t *tmp = next->next;
            free(next);
            next = tmp;
        }

        points[i].point_area_size = area;
        points[i].area_coords = NULL;
    }

    while(contentious_cells != NULL) {
        struct list_node_t *tmp = contentious_cells->next;
        free(contentious_cells);
        contentious_cells = tmp;
    }

    return 1;
}

int grow_cell(struct voronoi_point_t points[], int points_len, struct voronoi_point_t *focus, struct list_node_t **contentious_cells, struct coord_t coord)
{
    //if coordinate is in contention, skip
    if(is_coord_in_list(*contentious_cells, coord)) {
        return 0;
    }

    //Try to find coord in other voronoi point areas
    struct voronoi_point_t *found = NULL;
    for(int j = 0; j < points_len; j++) {
        if(focus != points + j && is_coord_in_list(points[j].area_coords, coord)) {
            found = points + j;
            break;
        }
    }

    //if coord not found in other areas, mark as belonging to current voronoi cell
    if(found != NULL) {
        //remove cell from points[j] if unlocked
        struct list_node_t *next = found->area_coords;
        if(next->data.x == coord.x && next->data.y == coord.y) {
            if(next->locked) {
                return 0;
            }

            found->area_coords = next->next;
            free(next);
        } else {
            //free middle node
            while(next->next != NULL) {
                struct list_node_t *tmp = next->next;
                if(tmp->data.x == coord.x && tmp->data.y == coord.y) {
                    if(tmp->locked) {
                        return 0;
                    }

                    next->next = tmp->next;
                    free(tmp);
                    break;
                }

                next = tmp;
            }
        }

        struct list_node_t *node = (struct list_node_t *)malloc(sizeof(struct list_node_t));
        if(node == NULL) {
            fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
            exit(1);
        }

        node->data = coord;
        node->next = *contentious_cells;
        *contentious_cells = node;

        return 0;
    }

    struct list_node_t *node = (struct list_node_t *)malloc(sizeof(struct list_node_t));
    if(node == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    node->data = coord;
    node->locked = 0;
    node->next = focus->area_coords;
    focus->area_coords = node;

    return 1;
}

int determine_global_boundaries(struct voronoi_point_t points[], int points_len, int *lower_x, int *upper_x, int *lower_y, int *upper_y)
{
    if(points_len == 0) {
        return 0;
    }

    *lower_x = points[0].coord.x;
    *upper_x = points[0].coord.x;
    *lower_y = points[0].coord.y;
    *upper_y = points[0].coord.y;

    for(int i = 0; i < points_len; i++) {
        struct voronoi_point_t point = points[i];

        if(point.coord.x < *lower_x) {
            *lower_x = point.coord.x;
        }

        if(point.coord.x > *upper_x) {
            *upper_x = point.coord.x;
        }

        if(point.coord.y < *lower_y) {
            *lower_y = point.coord.y;
        }

        if(point.coord.y > *upper_y) {
            *upper_y = point.coord.y;
        }
    }

    return 1;
}

int is_coord_outside_boundaries(struct coord_t coord, int lower_x, int upper_x, int lower_y, int upper_y)
{
    if(coord.x < lower_x || coord.x > upper_x) {
        return 1;
    }

    if(coord.y < lower_y || coord.y > upper_y) {
        return 1;
    }

    return 0;
}

int is_coord_in_list(struct list_node_t *head, struct coord_t coord)
{
    struct list_node_t *next = head;
    while(next != NULL) {
        if(next->data.x == coord.x && next->data.y == coord.y) {
            return 1;
        }

        next = next->next;
    }

    return 0;
}

void mark_cells_as_locked(struct voronoi_point_t *point)
{
    struct list_node_t *next = point->area_coords;
    while(next != NULL) {
        next->locked = 1;
        next = next->next;
    }
}