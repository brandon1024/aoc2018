#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_LEN 64
#define OFFSET 0

struct moving_point_t {
    long int x;
    long int y;
    int velocity_x;
    int velocity_y;
};

struct moving_point_t build_point_from_input(char buffer[], size_t buff_len);
void print_message(struct moving_point_t points[], int points_len);
long int find_smallest_point_distribution(const struct moving_point_t points[], int points_len);
struct moving_point_t *advance_points_n_seconds(struct moving_point_t points[], int points_len, long int seconds);

int main(int argc, char *argv[])
{
    int points_len = BUFF_LEN;
    int points_index = 0;
    struct moving_point_t *points = (struct moving_point_t *)malloc(sizeof(struct moving_point_t) * points_len);
    if(points == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFF_LEN];
    while(fgets(buffer, BUFF_LEN, stdin) != NULL) {
        if(points_index >= points_len) {
            points_len += BUFF_LEN;
            points = (struct moving_point_t *)realloc(points, sizeof(struct moving_point_t) * points_len);
            if(points == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }
        }

        points[points_index] = build_point_from_input(buffer, BUFF_LEN);
        points_index++;
    }

    points_len = points_index;
    points = (struct moving_point_t *)realloc(points, sizeof(struct moving_point_t) * points_len);
    if(points == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    long int smallest_distribution_time = find_smallest_point_distribution(points, points_len);
    advance_points_n_seconds(points, points_len, smallest_distribution_time + OFFSET);

    fprintf(stdout, "What message will eventually appear in the sky?\n");
    print_message(points, points_len);

    fprintf(stdout, "Exactly how many seconds would they have needed to wait for that message to appear? %ld\n", smallest_distribution_time);

    free(points);

    return 0;
}

struct moving_point_t build_point_from_input(char buffer[], size_t buff_len)
{
    struct moving_point_t point = {.y = 0, .x = 0, .velocity_x = 0, .velocity_y = 0};
    char *lf, *eos, *start_index, *token;

    lf = memchr(buffer, '\n', buff_len);
    if(lf != NULL) {
        *lf = 0;
    }

    eos = memchr(buffer, 0, buff_len);
    if(eos == NULL) {
        fprintf(stderr, "Unexpected input: %32s\n", buffer);
        exit(1);
    }

    start_index = memchr(buffer, '<', buff_len) + 1;
    if(*start_index == ' ') {
        start_index++;
    }
    token = memchr(start_index, ',', start_index + buff_len - buffer);
    *token = 0;
    point.x = strtol(start_index, &eos, 10);

    start_index = token + 2;
    if(*start_index == ' ') {
        start_index++;
    }
    token = memchr(start_index, '>', start_index + buff_len - buffer);
    *token = 0;
    point.y = strtol(start_index, &eos, 10);

    start_index = memchr(start_index, '<', start_index + buff_len - buffer) + 1;
    if(*start_index == ' ') {
        start_index++;
    }
    token = memchr(start_index, ',', start_index + buff_len - buffer);
    *token = 0;
    point.velocity_x = (int)strtol(start_index, &eos, 10);

    start_index = token + 2;
    if(*start_index == ' ') {
        start_index++;
    }
    token = memchr(start_index, '>', start_index + buff_len - buffer);
    *token = 0;
    point.velocity_y = (int)strtol(start_index, &eos, 10);

    return point;
}

void print_message(struct moving_point_t points[], int points_len)
{
    long int lower_x = points[0].x;
    long int upper_x = points[0].x;
    long int lower_y = points[0].y;
    long int upper_y = points[0].y;
    for(int i = 0; i < points_len; i++) {
        if(points[i].x < lower_x) {
            lower_x = points[i].x;
        }

        if(points[i].x > upper_x) {
            upper_x = points[i].x;
        }

        if(points[i].y < lower_y) {
            lower_y = points[i].y;
        }

        if(points[i].y > upper_y) {
            upper_y = points[i].y;
        }
    }

    for(long int y = lower_y - 1; y <= upper_y + 1; y++) {
        for(long int x = lower_x - 1; x <= upper_x + 1; x++) {
            int found = 0;
            for(int i = 0; i < points_len; i++) {
                if(points[i].x == x && points[i].y == y) {
                    found = 1;
                    break;
                }
            }

            if(found) {
                fprintf(stdout, "▓");
            } else {
                fprintf(stdout, "░");
            }
        }

        fprintf(stdout, "\n");
    }
}

long int find_smallest_point_distribution(const struct moving_point_t points[], int points_len)
{
    struct moving_point_t *mutable_points = (struct moving_point_t *)malloc(sizeof(struct moving_point_t) * points_len);
    if(mutable_points == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(mutable_points, points, sizeof(struct moving_point_t) * points_len);

    long int record_seconds = 0;
    long int record_delta_x = 500000;
    long int record_delta_y = 500000;

    long int seconds = 0;
    long int current_delta_x = 0;
    long int current_delta_y = 0;
    while(current_delta_x < 500000 && current_delta_y < 500000) {
        seconds++;
        advance_points_n_seconds(mutable_points, points_len, 1);

        long int lower_x = mutable_points[0].x;
        long int upper_x = mutable_points[0].x;
        long int lower_y = mutable_points[0].y;
        long int upper_y = mutable_points[0].y;
        for(int i = 0; i < points_len; i++) {
            if(mutable_points[i].x < lower_x) {
                lower_x = mutable_points[i].x;
            }

            if(mutable_points[i].x > upper_x) {
                upper_x = mutable_points[i].x;
            }

            if(mutable_points[i].y < lower_y) {
                lower_y = mutable_points[i].y;
            }

            if(mutable_points[i].y > upper_y) {
                upper_y = mutable_points[i].y;
            }
        }

        current_delta_x = upper_x - lower_x;
        current_delta_y = upper_y - lower_y;

        if(current_delta_x < record_delta_x && current_delta_y < record_delta_y) {
            record_delta_x = current_delta_x;
            record_delta_y = current_delta_y;
            record_seconds = seconds;
        }
    }

    return record_seconds;
}

struct moving_point_t *advance_points_n_seconds(struct moving_point_t points[], int points_len, long int seconds)
{
    for(int i = 0; i < points_len; i++) {
        points[i].x = points[i].x + (points[i].velocity_x * seconds);
        points[i].y = points[i].y + (points[i].velocity_y * seconds);
    }

    return points;
}