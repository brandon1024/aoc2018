#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FABRIC_DIM 1000
#define BUFF_LEN 64

struct entry_t {
    unsigned int guard_id;
    unsigned int year;
    unsigned int month: 4;
    unsigned int day: 5;
    unsigned int hour: 5;
    unsigned int min: 6;
    unsigned int shift_begin: 1;
    unsigned int sleep: 1;
    unsigned int wake: 1;
};

struct entry_t build_entry_from_str(char *buffer, size_t buff_len);

int main(int argc, char *argv[])
{
    char buffer[BUFF_LEN] = "[1518-12-31 23:59] Guard #26 begins shift";
    struct entry_t entry = build_entry_from_str(buffer, BUFF_LEN);
    printf("%d %d %d %d %d %d %d %d %d\n", entry.year, entry.month, entry.day, entry.hour, entry.min, entry.guard_id, entry.shift_begin, entry.sleep, entry.wake);

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
    entry.year = (unsigned int)strtol(start_index, &start_index, 10);

    start_index = token + 1;
    token = memchr(buffer, '-', buff_len);
    *token = 0;
    entry.month = (unsigned int)strtol(start_index, &start_index, 10);

    start_index = token + 1;
    token = memchr(buffer, ' ', buff_len);
    *token = 0;
    entry.day = (unsigned int)strtol(start_index, &start_index, 10);

    start_index = token + 1;
    token = memchr(buffer, ':', buff_len);
    *token = 0;
    entry.hour = (unsigned int)strtol(start_index, &start_index, 10);

    start_index = token + 1;
    token = memchr(buffer, ']', buff_len);
    *token = 0;
    entry.min = (unsigned int)strtol(start_index, &start_index, 10);

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