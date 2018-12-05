#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Advent of Code 2018: Day 1
 *
 * All inputs taken from stdin.
 * */

#define BUFF_LEN 32

int main(int argc, char *argv[])
{
    long int freq = 0;
    char buffer[BUFF_LEN];

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

        long int offset = strtol(buffer, &eos, 10);
        freq = freq + offset;
    }

    fprintf(stdout, "Resulting Frequency: %ld\n", freq);

    return 0;
}