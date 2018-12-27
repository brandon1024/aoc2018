#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define BUFF_LEN 256
#define DIR_UP 0x1
#define DIR_DOWN 0x2
#define DIR_LEFT 0x4
#define DIR_RIGHT 0x8

struct coord_t {
    unsigned short int x;
    unsigned short int y;
};

struct cart_t {
    struct coord_t coord;
    struct track_t *track;
    unsigned char dir;
    unsigned char next_dir;
    unsigned char crashed: 1;
    unsigned char visited: 1;
};

struct track_t {
    struct coord_t tl;
    struct coord_t tr;
    struct coord_t bl;
    struct coord_t br;
    struct intersection_t **intersections;
    size_t intersections_index;
    size_t intersections_len;
    unsigned int initialized: 1;
};

struct intersection_t {
    struct coord_t coord;
    struct track_t *track_a;
    struct track_t *track_b;
};

struct system_t {
    unsigned long int tick;
    struct cart_t **carts;
    size_t carts_len;
    struct track_t **tracks;
    size_t tracks_len;
    struct intersection_t **intersections;
    size_t intersections_len;
    struct coord_t first_crash_coord;
    struct coord_t last_cart_coord;
};

struct system_t build_system_from_input(FILE *input_stream);
void release_system_resources(struct system_t system);
struct system_t determine_first_crash_coord(struct system_t system);
char *mem_search_chars(const char *buffer, const char *str, size_t buff_len);
int sort_carts_by_coord_position(struct cart_t *carts[], size_t carts_len);
int coords_equal(struct coord_t coord_1, struct coord_t coord_2);

int main(int argc, char *argv[])
{
    struct system_t system = build_system_from_input(stdin);
    system = determine_first_crash_coord(system);

    fprintf(stdout, "Where is the location (X,Y) of the first crash? %hu,%hu\n", system.first_crash_coord.x, system.first_crash_coord.y);
    fprintf(stdout, "What is the location of the last cart at the end of the first tick where it is the only cart left? %hu,%hu\n", system.last_cart_coord.x, system.last_cart_coord.y);

    release_system_resources(system);

    return 0;
}

struct system_t build_system_from_input(FILE *input_stream)
{
    struct system_t system = {.tick = 0, .carts = NULL, .tracks = NULL, .carts_len = 0, .tracks_len = 0, .intersections_len = 0};
    char buffer[BUFF_LEN];

    unsigned short int row = 0;
    size_t tracks_index = 0;
    size_t carts_index = 0;
    size_t intersections_index = 0;
    while(fgets(buffer, BUFF_LEN, input_stream) != NULL) {
        char *eos = memchr(buffer, 0, BUFF_LEN);
        if(eos == NULL) {
            fprintf(stderr, "Unexpected error: input exceeds buffer size.\n");
            exit(1);
        }

        char *token = memchr(buffer, '\n', eos-buffer);
        if(token != NULL) {
            *token = 0;
            eos = token;
        }

        char *track_start = buffer;
        while((track_start = mem_search_chars(track_start, "/\\", eos - track_start)) != NULL) {
            if(*track_start == '/') {
                char *next_track_start = memchr(track_start + 1, '/', eos - track_start - 1);
                char *track_end = memchr(track_start, '\\', eos - track_start);
                if(track_end == NULL || (next_track_start != NULL && next_track_start < track_end)) {
                    fprintf(stderr, "Unexpected error: invalid input.\n");
                    exit(1);
                }

                if(tracks_index >= system.tracks_len) {
                    system.tracks_len += BUFF_LEN;
                    system.tracks = (struct track_t **)realloc(system.tracks, sizeof(struct track_t *) * system.tracks_len);
                    if(system.tracks == NULL) {
                        perror("Fatal error: Cannot allocate memory.\n");
                        exit(EXIT_FAILURE);
                    }
                }

                struct track_t *track = (struct track_t *)malloc(sizeof(struct track_t));
                if(track == NULL) {
                    perror("Fatal error: Cannot allocate memory.\n");
                    exit(EXIT_FAILURE);
                }

                struct coord_t left = {.x = (unsigned short int)(track_start - buffer), .y = row};
                struct coord_t right = {.x = (unsigned short int)(track_end - buffer), .y = row};
                *track = (struct track_t){.tl = left, .tr = right, .intersections = NULL, .intersections_len = 0, .intersections_index = 0, .initialized = 0};

                system.tracks[tracks_index] = track;

                tracks_index++;
                track_start = track_end + 1;
            } else {
                char *next_track_start = memchr(track_start + 1, '\\', eos - track_start - 1);
                char *track_end = memchr(track_start + 1, '/', eos - track_start - 1);
                if(track_end == NULL || (next_track_start != NULL && next_track_start < track_end)) {
                    fprintf(stderr, "Unexpected error: invalid input.\n");
                    exit(1);
                }

                struct coord_t left = {.x = (unsigned short int)(track_start - buffer), .y = row};
                struct coord_t right = {.x = (unsigned short int)(track_end - buffer), .y = row};

                struct track_t *closest_track = NULL;
                for(size_t index = 0; index < tracks_index; index++) {
                    if(system.tracks[index]->initialized) {
                        continue;
                    }

                    if(system.tracks[index]->tl.x != left.x || system.tracks[index]->tr.x != right.x) {
                        continue;
                    }

                    if(closest_track != NULL && system.tracks[index]->tl.y > closest_track->tl.y) {
                        continue;
                    }

                    closest_track = system.tracks[index];
                }

                if(closest_track == NULL) {
                    fprintf(stderr, "Unexpected error: invalid input.\n");
                    exit(1);
                }

                closest_track->bl = left;
                closest_track->br = right;
                closest_track->initialized = 1;

                track_start = track_end + 1;
            }
        }

        char *cart = buffer;
        while((cart = mem_search_chars(cart, "^v<>", eos - cart)) != NULL) {
            unsigned char dir;
            switch(*cart) {
                case '^':
                    dir = DIR_UP;
                    break;
                case 'v':
                    dir = DIR_DOWN;
                    break;
                case '<':
                    dir = DIR_LEFT;
                    break;
                default:
                    dir = DIR_RIGHT;
            }

            if(carts_index >= system.carts_len) {
                system.carts_len += BUFF_LEN;
                system.carts = (struct cart_t **)realloc(system.carts, sizeof(struct cart_t *) * system.carts_len);
                if(system.carts == NULL) {
                    perror("Fatal error: Cannot allocate memory.\n");
                    exit(EXIT_FAILURE);
                }
            }

            struct cart_t *new_cart = (struct cart_t *)malloc(sizeof(struct cart_t));
            if(new_cart == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }

            system.carts[carts_index] = new_cart;
            new_cart->coord = (struct coord_t){.x = (unsigned short int)(cart - buffer), .y = row};
            new_cart->dir = dir;
            new_cart->next_dir = DIR_LEFT;
            new_cart->track = NULL;
            new_cart->crashed = 0;
            new_cart->visited = 0;

            cart++;
            carts_index++;
        }

        char *intersection = buffer;
        while((intersection = memchr(intersection, '+', eos - intersection)) != NULL) {
            if(intersections_index >= system.intersections_len) {
                system.intersections_len += BUFF_LEN;
                system.intersections = (struct intersection_t **)realloc(system.intersections, sizeof(struct intersection_t *) * system.intersections_len);
                if(system.intersections == NULL) {
                    perror("Fatal error: Cannot allocate memory.\n");
                    exit(EXIT_FAILURE);
                }
            }

            struct intersection_t *new_intersection = (struct intersection_t *)malloc(sizeof(struct intersection_t));
            if(new_intersection == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }

            system.intersections[intersections_index] = new_intersection;
            new_intersection->coord = (struct coord_t){.x = (unsigned short int) (intersection - buffer), .y = row};
            new_intersection->track_a = NULL;
            new_intersection->track_b = NULL;

            intersection++;
            intersections_index++;
        }

        if(row == USHRT_MAX) {
            fprintf(stderr, "Unexpected error: exceeded input row limit.\n");
            exit(1);
        }

        row++;
    }

    for(size_t track_index = 0; track_index < tracks_index; track_index++) {
        if(!system.tracks[track_index]->initialized) {
            fprintf(stderr, "Unexpected error: all tracks should be initialized.\n");
            exit(1);
        }
    }

    for(size_t cart_index = 0; cart_index < carts_index; cart_index++) {
        struct cart_t *cart = system.carts[cart_index];
        struct track_t *track = NULL;

        for(size_t index = 0; index < tracks_index; index++) {
            struct track_t *track_i = system.tracks[index];

            if(cart->dir == DIR_UP || cart->dir == DIR_DOWN) {
                if(track_i->tl.x != cart->coord.x && track_i->tr.x != cart->coord.x) {
                    continue;
                }

                if(cart->coord.y > track_i->tl.y && cart->coord.y < track_i->bl.y) {
                    track = track_i;
                    break;
                }
            } else {
                if(track_i->tl.y != cart->coord.y && track_i->bl.y != cart->coord.y) {
                    continue;
                }

                if(cart->coord.x > track_i->tl.x && cart->coord.x < track_i->tr.x) {
                    track = track_i;
                    break;
                }
            }
        }

        if(track == NULL) {
            fprintf(stderr, "Unexpected error: invalid input.\n");
            exit(1);
        }

        cart->track = track;
    }

    for(size_t intersection_index = 0; intersection_index < intersections_index; intersection_index++) {
        struct intersection_t *intersection = system.intersections[intersection_index];

        struct track_t *horizontal_track = NULL;
        struct track_t *vertical_track = NULL;
        for(size_t h_index = 0; h_index < tracks_index; h_index++) {
            struct track_t *tmp_h_track = system.tracks[h_index];
            if(tmp_h_track->tl.y != intersection->coord.y && tmp_h_track->bl.y != intersection->coord.y) {
                continue;
            }

            if(intersection->coord.x < tmp_h_track->tl.x || intersection->coord.x > tmp_h_track->tr.x) {
                continue;
            }

            horizontal_track = tmp_h_track;
            break;
        }

        for(size_t v_index = 0; v_index < tracks_index; v_index++) {
            struct track_t *tmp_v_track = system.tracks[v_index];
            if(tmp_v_track == horizontal_track) {
                continue;
            }

            if(tmp_v_track->tl.x != intersection->coord.x && tmp_v_track->tr.x != intersection->coord.x) {
                continue;
            }

            if(intersection->coord.y < tmp_v_track->tl.y || intersection->coord.y > tmp_v_track->bl.y) {
                continue;
            }

            vertical_track = tmp_v_track;
            break;
        }

        if(horizontal_track == NULL || vertical_track == NULL) {
            fprintf(stderr, "Unexpected error: invalid input.\n");
            exit(1);
        }

        intersection->track_a = horizontal_track;
        intersection->track_b = vertical_track;

        if(horizontal_track->intersections_index >= horizontal_track->intersections_len) {
            horizontal_track->intersections_len += BUFF_LEN;
            horizontal_track->intersections = (struct intersection_t **)realloc(horizontal_track->intersections, sizeof(struct intersection_t *) * horizontal_track->intersections_len);
            if(horizontal_track->intersections == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }
        }

        horizontal_track->intersections[horizontal_track->intersections_index] = intersection;
        horizontal_track->intersections_index++;

        if(vertical_track->intersections_index >= vertical_track->intersections_len) {
            vertical_track->intersections_len += BUFF_LEN;
            vertical_track->intersections = (struct intersection_t **)realloc(vertical_track->intersections, sizeof(struct intersection_t *) * vertical_track->intersections_len);
            if(vertical_track->intersections == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }
        }

        vertical_track->intersections[vertical_track->intersections_index] = intersection;
        vertical_track->intersections_index++;
    }

    if(carts_index % 2 == 0) {
        fprintf(stderr, "Unexpected error: expected odd number of carts.\n");
        exit(1);
    }

    system.tracks_len = tracks_index;
    system.tracks = (struct track_t **)realloc(system.tracks, sizeof(struct track_t *) * system.tracks_len);
    if(system.tracks == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    system.carts_len = carts_index;
    system.carts = (struct cart_t **)realloc(system.carts, sizeof(struct cart_t *) * system.carts_len);
    if(system.carts == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    system.intersections_len = intersections_index;
    system.intersections = (struct intersection_t **)realloc(system.intersections, sizeof(struct intersection_t *) * system.intersections_len);
    if(system.intersections == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for(size_t track_index = 0; track_index < system.tracks_len; track_index++) {
        struct track_t *track = system.tracks[track_index];
        track->intersections_len = track->intersections_index;
        track->intersections = (struct intersection_t **)realloc(track->intersections, sizeof(struct intersection_t *) * track->intersections_len);
        if(track->intersections == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }
    }

    return system;
}

void release_system_resources(struct system_t system)
{
    for(size_t index = 0; index < system.carts_len; index++) {
        free(system.carts[index]);
    }

    for(size_t index = 0; index < system.tracks_len; index++) {
        free(system.tracks[index]->intersections);
        free(system.tracks[index]);
    }

    for(size_t index = 0; index < system.intersections_len; index++) {
        free(system.intersections[index]);
    }

    free(system.carts);
    free(system.tracks);
    free(system.intersections);
}

struct system_t determine_first_crash_coord(struct system_t system)
{
    int crash_encountered = 0;
    int carts_left = 0;
    do {
        sort_carts_by_coord_position(system.carts, system.carts_len);

        do {
            struct cart_t *cart = NULL;
            for(size_t cart_index = 0; cart_index < system.carts_len; cart_index++) {
                if(!system.carts[cart_index]->visited && !system.carts[cart_index]->crashed) {
                    cart = system.carts[cart_index];
                    cart->visited = 1;
                    break;
                }
            }

            if(cart == NULL) {
                break;
            }

            if(cart->dir == DIR_UP || cart->dir == DIR_DOWN) {
                cart->dir == DIR_UP ? cart->coord.y-- : cart->coord.y++;
            } else {
                cart->dir == DIR_LEFT ? cart->coord.x-- : cart->coord.x++;
            }

            if(coords_equal(cart->coord, cart->track->tl)) {
                cart->dir = (unsigned char)(cart->dir == DIR_UP ? DIR_RIGHT : DIR_DOWN);
            } else if(coords_equal(cart->coord, cart->track->bl)) {
                cart->dir = (unsigned char)(cart->dir == DIR_DOWN ? DIR_RIGHT : DIR_UP);
            } else if(coords_equal(cart->coord, cart->track->tr)) {
                cart->dir = (unsigned char)(cart->dir == DIR_UP ? DIR_LEFT : DIR_DOWN);
            } else if(coords_equal(cart->coord, cart->track->br)) {
                cart->dir = (unsigned char)(cart->dir == DIR_DOWN ? DIR_LEFT : DIR_UP);
            } else {
                struct intersection_t *intersection = NULL;
                for(size_t intersection_index = 0; intersection_index < cart->track->intersections_len; intersection_index++) {
                    struct intersection_t *tmp = cart->track->intersections[intersection_index];

                    if(coords_equal(cart->coord, tmp->coord)) {
                        intersection = tmp;
                        break;
                    }
                }

                if(intersection != NULL) {
                    if(cart->next_dir == DIR_LEFT) {
                        switch(cart->dir) {
                            case DIR_UP:
                                cart->dir = DIR_LEFT;
                                break;
                            case DIR_DOWN:
                                cart->dir = DIR_RIGHT;
                                break;
                            case DIR_LEFT:
                                cart->dir = DIR_DOWN;
                                break;
                            default:
                                cart->dir = DIR_UP;
                                break;
                        }

                        cart->next_dir = 0;
                        cart->track = (cart->track == intersection->track_a) ? intersection->track_b : intersection->track_a;
                    } else if(cart->next_dir == DIR_RIGHT) {
                        switch(cart->dir) {
                            case DIR_UP:
                                cart->dir = DIR_RIGHT;
                                break;
                            case DIR_DOWN:
                                cart->dir = DIR_LEFT;
                                break;
                            case DIR_LEFT:
                                cart->dir = DIR_UP;
                                break;
                            default:
                                cart->dir = DIR_DOWN;
                                break;
                        }

                        cart->next_dir = DIR_LEFT;
                        cart->track = (cart->track == intersection->track_a) ? intersection->track_b : intersection->track_a;
                    } else {
                        cart->next_dir = DIR_RIGHT;
                    }
                }
            }

            for(size_t collision_check = 0; collision_check < system.carts_len; collision_check++) {
                if(system.carts[collision_check] == cart) {
                    continue;
                }

                if(!system.carts[collision_check]->crashed && coords_equal(cart->coord, system.carts[collision_check]->coord)) {
                    if(!crash_encountered) {
                        system.first_crash_coord = cart->coord;
                        crash_encountered = 1;
                    }

                    cart->crashed = 1;
                    system.carts[collision_check]->crashed = 1;
                    break;
                }
            }
        } while(1);

        system.tick++;

        carts_left = 0;
        for(size_t cart_index = 0; cart_index < system.carts_len; cart_index++) {
            system.carts[cart_index]->visited = 0;

            if(!system.carts[cart_index]->crashed) {
                system.last_cart_coord = system.carts[cart_index]->coord;
                carts_left++;
            }
        }
    } while(carts_left > 1);

    return system;
}

char *mem_search_chars(const char *buffer, const char *str, size_t buff_len)
{
    if(buff_len <= 0) {
        return NULL;
    }

    char *found = NULL;
    for(size_t i = 0; i < strlen(str); i++) {
        char *tmp = memchr(buffer, str[i], buff_len);
        if(found == NULL || (tmp != NULL && tmp < found)) {
            found = tmp;
        }
    }

    return found;
}

int sort_carts_by_coord_position(struct cart_t *carts[], size_t carts_len)
{
    if(carts == NULL) {
        return 0;
    }

    for(size_t current_index = 0; current_index < carts_len; current_index++) {
        for(size_t candidate_index = current_index + 1; candidate_index < carts_len; candidate_index++) {
            if(carts[candidate_index] == NULL) {
                continue;
            }

            if(carts[current_index] != NULL) {
                if(carts[candidate_index]->coord.y > carts[current_index]->coord.y) {
                    continue;
                }

                if(carts[candidate_index]->coord.y == carts[current_index]->coord.y && carts[candidate_index]->coord.x > carts[current_index]->coord.x) {
                    continue;
                }
            }

            struct cart_t *tmp = carts[current_index];
            carts[current_index] = carts[candidate_index];
            carts[candidate_index] = tmp;
            break;
        }
    }

    return 1;
}

int coords_equal(struct coord_t coord_1, struct coord_t coord_2)
{
    if(coord_1.x != coord_2.x) {
        return 0;
    }

    if(coord_1.y != coord_2.y) {
        return 0;
    }

    return 1;
}