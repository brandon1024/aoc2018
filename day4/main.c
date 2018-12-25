#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MOST_MIN_ASLEEP_STRATEGY 0
#define MOST_FREQ_ASLEEP_SAME_MIN_STRATEGY 1

#define BUFF_LEN 64

struct instant_t {
    unsigned int year;
    unsigned int month: 4;
    unsigned int day: 5;
    unsigned int hour: 5;
    unsigned int min: 6;
};

struct entry_t {
    unsigned int guard_id;
    struct instant_t instant;
    unsigned int shift_begin: 1;
    unsigned int sleep: 1;
    unsigned int wake: 1;
};

struct list_node_t {
    struct entry_t data;
    struct list_node_t *next;
};

struct list_t {
    struct list_node_t *head;
    int len;
};

struct guard_info_t {
    unsigned int guard_id;
    unsigned int sleep_mins[60];
    unsigned int candidate_min: 6;
};

struct entry_t build_entry_from_str(char *buffer, size_t buff_len);
int insert_entry_into_list_sorted(struct list_t *list, struct list_node_t *node);
int entry_cmp(struct entry_t entry_a, struct entry_t entry_b);
struct guard_info_t determine_candidate_guard(struct list_t *entry_list, int strategy);
struct guard_info_t most_frequently_asleep_guard(struct guard_info_t *guard_info, int guard_info_len);
struct guard_info_t most_frequently_asleep_same_min_guard(struct guard_info_t *guard_info, int guard_info_len);
struct guard_info_t *find_guard_info(struct guard_info_t *guard_info, int guard_info_len, int guard_id);
struct guard_info_t *update_guard_info(struct guard_info_t *guard_info, struct entry_t asleep, struct entry_t awake);
int instant_cmp(struct instant_t instant_a, struct instant_t instant_b);

int main(int argc, char *argv[])
{
    char *buffer = (char *)malloc(sizeof(char) * BUFF_LEN);
    if(buffer == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    struct list_t entry_list;
    struct list_node_t *node;
    entry_list.head = NULL;
    entry_list.len = 0;
    while((fgets(buffer, BUFF_LEN, stdin)) != NULL) {
        struct entry_t entry = build_entry_from_str(buffer, BUFF_LEN);

        node = (struct list_node_t *)malloc(sizeof(struct list_node_t));
        if(node == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }

        node->data = entry;
        node->next = NULL;
        insert_entry_into_list_sorted(&entry_list, node);
    }

    struct guard_info_t candidate_guard = determine_candidate_guard(&entry_list, MOST_MIN_ASLEEP_STRATEGY);
    fprintf(stdout, "MOST_MIN_ASLEEP_STRATEGY\n");
    fprintf(stdout, "What is the ID of the guard you chose multiplied by the minute you chose? %d (%d * %d)\n",
            (candidate_guard.guard_id * candidate_guard.candidate_min),
            candidate_guard.guard_id,
            candidate_guard.candidate_min);

    candidate_guard = determine_candidate_guard(&entry_list, MOST_FREQ_ASLEEP_SAME_MIN_STRATEGY);
    fprintf(stdout, "MOST_FREQ_ASLEEP_SAME_MIN_STRATEGY\n");
    fprintf(stdout, "What is the ID of the guard you chose multiplied by the minute you chose? %d (%d * %d)\n",
            (candidate_guard.guard_id * candidate_guard.candidate_min),
            candidate_guard.guard_id,
            candidate_guard.candidate_min);

    free(buffer);
    node = entry_list.head;
    while(node != NULL) {
        struct list_node_t *tmp = node;
        node = node->next;
        free(tmp);
    }

    return 0;
}

struct entry_t build_entry_from_str(char *buffer, size_t buff_len)
{
    struct entry_t entry;
    char *start_index;
    char *token;

    start_index = memchr(buffer, '[', buff_len) + 1;
    token = memchr(buffer, '-', buff_len);
    *token = 0;
    entry.instant.year = (unsigned int)strtol(start_index, &start_index, 10);

    start_index = token + 1;
    token = memchr(buffer, '-', buff_len);
    *token = 0;
    entry.instant.month = (unsigned int)strtol(start_index, &start_index, 10);

    start_index = token + 1;
    token = memchr(buffer, ' ', buff_len);
    *token = 0;
    entry.instant.day = (unsigned int)strtol(start_index, &start_index, 10);

    start_index = token + 1;
    token = memchr(buffer, ':', buff_len);
    *token = 0;
    entry.instant.hour = (unsigned int)strtol(start_index, &start_index, 10);

    start_index = token + 1;
    token = memchr(buffer, ']', buff_len);
    *token = 0;
    entry.instant.min = (unsigned int)strtol(start_index, &start_index, 10);

    entry.guard_id = 0;
    entry.shift_begin = 0;
    entry.sleep = 0;
    entry.wake = 0;

    token++;
    *token = 0;
    start_index = token + 1;
    if((strlen(start_index)+1) >= 7 && !strncmp(start_index, "Guard #", 7)) {
        start_index = memchr(buffer, '#', buff_len) + 1;
        token = memchr(buffer, ' ', buff_len);
        *token = 0;
        entry.guard_id = (unsigned int)strtol(start_index, &start_index, 10);

        entry.shift_begin = 1;
    } else if((strlen(start_index)+1) >= 12 && !strncmp(start_index, "falls asleep", 12)) {
        entry.sleep = 1;
    } else if((strlen(start_index)+1) >= 8 && !strncmp(start_index, "wakes up", 8)) {
        entry.wake = 1;
    }

    return entry;
}

int insert_entry_into_list_sorted(struct list_t *list, struct list_node_t *node)
{
    struct list_node_t *next = list->head;
    list->len = list->len + 1;

    if(next == NULL) {
        list->head = node;
        return list->len;
    }

    struct entry_t insert_entry = node->data;
    struct entry_t next_entry = next->data;
    if(entry_cmp(insert_entry, next_entry) < 0) {
        list->head = node;
        node->next = next;
        return list->len;
    }

    while(next->next != NULL) {
        next_entry = next->next->data;

        if(entry_cmp(insert_entry, next_entry) < 0) {
            node->next = next->next;
            next->next = node;
            return list->len;
        }

        next = next->next;
    }

    next->next = node;
    return list->len;
}

struct guard_info_t determine_candidate_guard(struct list_t *entry_list, int strategy)
{
    int guard_info_len = BUFF_LEN;
    struct guard_info_t *guard_info = (struct guard_info_t *)malloc(sizeof(struct guard_info_t) * guard_info_len);
    if(guard_info == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    int guard_info_index = 0;
    struct list_node_t *node = entry_list->head;
    struct guard_info_t *guard = NULL;
    while(node != NULL) {
        struct entry_t entry = node->data;
        if(entry.wake) {
            fprintf(stderr, "Unexpected error: Out of order entry list\n");
            exit(1);
        }

        if(entry.sleep && guard == NULL) {
            fprintf(stderr, "Unexpected error: Out of order entry list\n");
            exit(1);
        }

        if(entry.shift_begin) {
            guard = find_guard_info(guard_info, guard_info_index, entry.guard_id);
            if(guard == NULL) {
                struct guard_info_t new_guard_info;
                new_guard_info.guard_id = entry.guard_id;
                memset(new_guard_info.sleep_mins, 0, sizeof(int) * 60);

                if(guard_info_index >= guard_info_len) {
                    guard_info_len = guard_info_len + BUFF_LEN;
                    guard_info = (struct guard_info_t *)realloc(guard_info, sizeof(struct guard_info_t) * guard_info_len);
                }

                guard_info[guard_info_index] = new_guard_info;
                guard_info_index++;

                guard = guard_info + guard_info_index;
            }
        } else if(entry.sleep) {
            struct list_node_t *sleep_node = node;
            node = node->next;

            if(node == NULL) {
                fprintf(stderr, "Unexpected error: Unexpected end of entry list\n");
                exit(1);
            }

            if(!node->data.wake) {
                fprintf(stderr, "Unexpected error: Out of order entry list\n");
                exit(1);
            }

            update_guard_info(guard, sleep_node->data, node->data);
        }

        node = node->next;
    }

    struct guard_info_t candidate_guard;
    if(strategy == MOST_MIN_ASLEEP_STRATEGY) {
        candidate_guard = most_frequently_asleep_guard(guard_info, guard_info_index);
    } else {
        candidate_guard = most_frequently_asleep_same_min_guard(guard_info, guard_info_index);
    }

    free(guard_info);

    return candidate_guard;
}

struct guard_info_t most_frequently_asleep_guard(struct guard_info_t *guard_info, int guard_info_len)
{
    struct guard_info_t *guard = NULL;
    for(int i = 0; i < guard_info_len; i++) {
        if(guard == NULL) {
            guard = guard_info + i;
        } else {
            int best_guard_mins_tot = 0;
            int curr_guard_mins_tot = 0;

            for(int j = 0; j < 60; j++) {
                best_guard_mins_tot += guard->sleep_mins[j];
                curr_guard_mins_tot += guard_info[i].sleep_mins[j];
            }

            if(curr_guard_mins_tot > best_guard_mins_tot) {
                guard = guard_info + i;
            }
        }
    }

    if(guard == NULL) {
        fprintf(stderr, "Unexpected error: Could not determine best guard\n");
        exit(1);
    }

    struct guard_info_t candidate_guard;
    candidate_guard.guard_id = guard->guard_id;
    memcpy(candidate_guard.sleep_mins, guard->sleep_mins, 60 * sizeof(int));

    unsigned int likely_min_index = 0;
    unsigned int duplicate_min_index = 0;
    for(unsigned int i = 1; i < 60; i++) {
        if(candidate_guard.sleep_mins[i] > candidate_guard.sleep_mins[likely_min_index]) {
            likely_min_index = i;
            duplicate_min_index = 0;
        } else if(candidate_guard.sleep_mins[i] == candidate_guard.sleep_mins[likely_min_index]) {
            duplicate_min_index = 1;
        }
    }

    if(duplicate_min_index) {
        fprintf(stderr, "Unexpected error: Could not determine best guard\n");
        exit(1);
    }

    candidate_guard.candidate_min = likely_min_index;

    return candidate_guard;
}

struct guard_info_t most_frequently_asleep_same_min_guard(struct guard_info_t *guard_info, int guard_info_len)
{
    struct guard_info_t **leading_sleep_freqs = (struct guard_info_t **)malloc(sizeof(struct guard_info_t *) * 60);
    if(leading_sleep_freqs == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < 60; i++) {
        leading_sleep_freqs[i] = NULL;
    }

    for(int guard_info_index = 0; guard_info_index < guard_info_len; guard_info_index++) {
        struct guard_info_t *current_guard = guard_info + guard_info_index;

        for(int i = 0; i < 60; i++) {
            if(leading_sleep_freqs[i] == NULL || current_guard->sleep_mins[i] > leading_sleep_freqs[i]->sleep_mins[i]) {
                leading_sleep_freqs[i] = current_guard;
            }
        }
    }

    struct guard_info_t *current_best_guard = NULL;
    unsigned int current_best_min = 0;
    for(unsigned int i = 0; i < 60; i++) {
        if(current_best_guard == NULL || leading_sleep_freqs[i]->sleep_mins[i] > current_best_guard->sleep_mins[current_best_min]) {
            current_best_guard = leading_sleep_freqs[i];
            current_best_min = i;
        }
    }

    if(current_best_guard == NULL) {
        fprintf(stderr, "Unexpected error: Could not determine best guard\n");
        exit(1);
    }

    struct guard_info_t candidate_guard;
    candidate_guard.guard_id = current_best_guard->guard_id;
    memcpy(candidate_guard.sleep_mins, current_best_guard->sleep_mins, 60 * sizeof(int));
    candidate_guard.candidate_min = current_best_min;

    free(leading_sleep_freqs);

    return candidate_guard;
}

struct guard_info_t *find_guard_info(struct guard_info_t *guard_info, int guard_info_len, int guard_id)
{
    for(int i = 0; i < guard_info_len; i++) {
        if(guard_info[i].guard_id == guard_id) {
            return guard_info + i;
        }
    }

    return NULL;
}

struct guard_info_t *update_guard_info(struct guard_info_t *guard_info, struct entry_t asleep, struct entry_t awake)
{
    const int month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    struct instant_t current = asleep.instant;
    while(instant_cmp(current, awake.instant) < 0) {
        guard_info->sleep_mins[current.min] = guard_info->sleep_mins[current.min] + 1;

        if(current.min < 59) {
            current.min = current.min + 1;
            continue;
        }

        current.min = 0;
        if(current.hour < 23) {
            current.hour = current.hour + 1;
            continue;
        }

        current.hour = 0;
        if(current.day < month_days[current.month - 1]) {
            current.day = current.day + 1;
            continue;
        }

        current.day = 1;
        if(current.month < 12) {
            current.month = current.month + 1;
            continue;
        }

        current.month = 1;
        current.year = current.year + 1;
    }

    return guard_info;
}

int entry_cmp(struct entry_t entry_a, struct entry_t entry_b)
{
    return instant_cmp(entry_a.instant, entry_b.instant);
}

int instant_cmp(struct instant_t instant_a, struct instant_t instant_b)
{
    if(instant_a.year > instant_b.year) {
        return 1;
    } else if(instant_a.year < instant_b.year) {
        return -1;
    }

    if(instant_a.month > instant_b.month) {
        return 1;
    } else if(instant_a.month < instant_b.month) {
        return -1;
    }

    if(instant_a.day > instant_b.day) {
        return 1;
    } else if(instant_a.day < instant_b.day) {
        return -1;
    }

    if(instant_a.hour > instant_b.hour) {
        return 1;
    } else if(instant_a.hour < instant_b.hour) {
        return -1;
    }

    if(instant_a.min > instant_b.min) {
        return 1;
    } else if(instant_a.min < instant_b.min) {
        return -1;
    }

    return 0;
}