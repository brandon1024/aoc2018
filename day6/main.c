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
};

struct voronoi_point_t {
    struct coord_t coord;
    struct list_node_t *boundary_coords;
    struct list_node_t *candidate_boundary_coords;
    int point_area_size;
    unsigned int inf: 1;
};

struct grid_bounds_t {
    int lower_x;
    int upper_x;
    int lower_y;
    int upper_y;
};

struct coord_t build_coord_from_input(char buffer[], size_t buff_len);
int determine_largest_area(struct coord_t coords[], int coords_len);
int compute_point_areas(struct voronoi_point_t points[], int points_len, struct grid_bounds_t bounds);
int grow_area(struct grid_bounds_t bounds, struct voronoi_point_t points[], int points_len,
        struct voronoi_point_t *focus, struct list_node_t **contentious_cells);
int grow_cell(struct grid_bounds_t bounds, struct voronoi_point_t points[], int points_len,
        struct voronoi_point_t *focus, struct list_node_t **contentious_cells, struct coord_t new_boundary_coord);
struct grid_bounds_t determine_global_boundaries(struct coord_t coords[], int coords_len);
int determine_region_size(struct coord_t coords[], int coords_len);
int is_coord_outside_boundaries(struct coord_t coord, struct grid_bounds_t bounds);
struct list_node_t *is_coord_in_list(struct list_node_t *head, struct coord_t coord);

int main(int argc, char *argv[])
{
    char buffer[BUFF_LEN];

    int coords_len = BUFF_LEN * 2;
    int coords_index = 0;
    struct coord_t *coords = (struct coord_t *)malloc(sizeof(struct coord_t) * coords_len);
    if(coords == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    while((fgets(buffer, BUFF_LEN, stdin)) != NULL) {
        if(coords_index >= coords_len) {
            coords_len = coords_len + (BUFF_LEN * 2);
            coords = (struct coord_t *)realloc(coords, sizeof(struct coord_t) * coords_len);
            if(coords == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
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

    int region_size = determine_region_size(coords, coords_index);
    fprintf(stdout, "What is the size of the region containing all locations which have a total distance to all given coordinates of less than 10000? %d\n", region_size);

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
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < coords_len; i++) {
        struct list_node_t *node = (struct list_node_t *)malloc(sizeof(struct list_node_t));
        if(node == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }

        node->data = coords[i];
        node->next = NULL;

        struct voronoi_point_t point_info = {
            .coord = coords[i],
            .boundary_coords = node,
            .candidate_boundary_coords = NULL,
            .point_area_size = 1,
            .inf = 0
        };

        points[i] = point_info;
    }

    struct grid_bounds_t bounds = determine_global_boundaries(coords, coords_len);
    compute_point_areas(points, coords_len, bounds);

    int largest_area = -1;
    for(int i = 0; i < coords_len; i++) {
        if(!points[i].inf && points[i].point_area_size > largest_area) {
            largest_area = points[i].point_area_size;
        }
    }

    free(points);

    return largest_area;
}

int compute_point_areas(struct voronoi_point_t points[], int points_len, struct grid_bounds_t bounds)
{
    struct list_node_t *contentious_cells = NULL;

    int still_growing = 1;
    while(still_growing) {
        still_growing = 0;

        for(int i = 0; i < points_len; i++) {
            if(grow_area(bounds, points, points_len, points + i, &contentious_cells)) {
                still_growing = 1;
            }
        }

        for(int i = 0; i < points_len; i++) {
            struct list_node_t *next = points[i].boundary_coords;
            while(next != NULL) {
                struct list_node_t *tmp = next->next;
                free(next);
                next = tmp;
            }

            points[i].boundary_coords = points[i].candidate_boundary_coords;
            points[i].candidate_boundary_coords = NULL;
        }
    }

    for(int i = 0; i < points_len; i++) {
        struct list_node_t *next = points[i].boundary_coords;
        while(next != NULL) {
            struct list_node_t *tmp = next->next;
            free(next);
            next = tmp;
        }

        points[i].boundary_coords = NULL;
    }

    while(contentious_cells != NULL) {
        struct list_node_t *tmp = contentious_cells->next;
        free(contentious_cells);
        contentious_cells = tmp;
    }

    return 1;
}

int grow_area(struct grid_bounds_t bounds, struct voronoi_point_t points[], int points_len,
        struct voronoi_point_t *focus, struct list_node_t **contentious_cells)
{
    int still_growing = 0;

    struct list_node_t *next = focus->boundary_coords;
    while(next != NULL) {
        struct coord_t boundary_coord = next->data;
        struct coord_t focus_coord = focus->coord;

        if(boundary_coord.x <= focus_coord.x) {
            struct coord_t new_boundary_coord = {.x = boundary_coord.x - 1, .y = boundary_coord.y};
            if(grow_cell(bounds, points, points_len, focus, contentious_cells, new_boundary_coord)) {
                still_growing = 1;
            }
        }

        if(boundary_coord.x >= focus_coord.x) {
            struct coord_t new_boundary_coord = {.x = boundary_coord.x + 1, .y = boundary_coord.y};
            if(grow_cell(bounds, points, points_len, focus, contentious_cells, new_boundary_coord)) {
                still_growing = 1;
            }
        }

        if(boundary_coord.x == focus_coord.x && boundary_coord.y >= focus_coord.y) {
            struct coord_t new_boundary_coord = {.x = boundary_coord.x, .y = boundary_coord.y + 1};
            if(grow_cell(bounds, points, points_len, focus, contentious_cells, new_boundary_coord)) {
                still_growing = 1;
            }
        }

        if(boundary_coord.x == focus_coord.x && boundary_coord.y <= focus_coord.y) {
            struct coord_t new_boundary_coord = {.x = boundary_coord.x, .y = boundary_coord.y - 1};
            if(grow_cell(bounds, points, points_len, focus, contentious_cells, new_boundary_coord)) {
                still_growing = 1;
            }
        }

        next = next->next;
    }

    return still_growing;
}

int grow_cell(struct grid_bounds_t bounds, struct voronoi_point_t points[], int points_len,
        struct voronoi_point_t *focus, struct list_node_t **contentious_cells, struct coord_t new_boundary_coord)
{
    for(int i = 0; i < points_len; i++) {
        if(points + i == focus) {
            continue;
        }

        if(is_coord_in_list(points[i].boundary_coords, new_boundary_coord) != NULL) {
            return 0;
        }
    }

    if(is_coord_in_list(*contentious_cells, new_boundary_coord) != NULL) {
        return 0;
    }

    struct voronoi_point_t *parent = points;
    struct list_node_t *candidate = NULL;
    for(int i = 0; i < points_len; i++) {
        if(points + i == focus) {
            continue;
        }

        parent = points + i;
        candidate = is_coord_in_list(points[i].candidate_boundary_coords, new_boundary_coord);
        if(candidate != NULL) {
            break;
        }
    }

    if(candidate != NULL) {
        if(candidate == parent->candidate_boundary_coords) {
            parent->candidate_boundary_coords = candidate->next;
        } else {
            struct list_node_t *tmp = parent->candidate_boundary_coords;
            while(tmp->next != NULL) {
                if(tmp->next == candidate) {
                    tmp->next = candidate->next;
                    break;
                }

                tmp = tmp->next;
            }
        }

        candidate->next = *contentious_cells;
        *contentious_cells = candidate;
        parent->point_area_size = parent->point_area_size - 1;

        return 0;
    }

    if(is_coord_outside_boundaries(new_boundary_coord, bounds)) {
        focus->inf = 1;
        return 0;
    }

    struct list_node_t *new_boundary_node = (struct list_node_t *)malloc(sizeof(struct list_node_t));
    if(new_boundary_node == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    new_boundary_node->data = new_boundary_coord;
    new_boundary_node->next = focus->candidate_boundary_coords;
    focus->candidate_boundary_coords = new_boundary_node;
    focus->point_area_size = focus->point_area_size + 1;
    return 1;
}

int determine_region_size(struct coord_t coords[], int coords_len)
{
    struct grid_bounds_t bounds = determine_global_boundaries(coords, coords_len);

    int region_size = 0;
    for(int y = bounds.lower_y; y <= bounds.upper_y; y++) {
        for(int x = bounds.lower_x; x <= bounds.upper_x; x++) {
            int dist_to_other_points = 0;

            for(int i = 0; i < coords_len; i++) {
                dist_to_other_points += abs(x - coords[i].x) + abs(y - coords[i].y);
            }

            if(dist_to_other_points < 10000) {
                region_size++;
            }
        }
    }

    return region_size;
}

struct grid_bounds_t determine_global_boundaries(struct coord_t coords[], int coords_len)
{
    struct grid_bounds_t bounds = {
        .lower_x = coords[0].x,
        .upper_x = coords[0].x,
        .lower_y = coords[0].y,
        .upper_y = coords[0].y
    };

    for(int i = 0; i < coords_len; i++) {
        if(coords[i].x < bounds.lower_x) {
            bounds.lower_x = coords[i].x;
        }

        if(coords[i].x > bounds.upper_x) {
            bounds.upper_x =coords[i].x;
        }

        if(coords[i].y < bounds.lower_y) {
            bounds.lower_y = coords[i].y;
        }

        if(coords[i].y > bounds.upper_y) {
            bounds.upper_y = coords[i].y;
        }
    }

    return bounds;
}

int is_coord_outside_boundaries(struct coord_t coord, struct grid_bounds_t bounds)
{
    return coord.x < bounds.lower_x || coord.x > bounds.upper_x || coord.y < bounds.lower_y || coord.y > bounds.upper_y;
}

struct list_node_t *is_coord_in_list(struct list_node_t *head, struct coord_t coord)
{
    struct list_node_t *next = head;
    while(next != NULL) {
        if(next->data.x == coord.x && next->data.y == coord.y) {
            return next;
        }

        next = next->next;
    }

    return NULL;
}