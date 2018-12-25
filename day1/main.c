#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_LEN 32

int determine_frequency_reached_twice(int *offsets, int offset_len);
int exists_in_frequencies_encountered(const int *freqs_encountered, int freqs_encountered_len, int freq);

int main(int argc, char *argv[])
{
    int freq = 0;
    char buffer[BUFF_LEN];

    int offsets_len = BUFF_LEN;
    int offsets_index = 0;
    int *offsets = (int *)malloc(sizeof(int) * offsets_len);
    if(offsets == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

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

        int offset = (int)strtol(buffer, &eos, 10);
        freq = freq + offset;

        if(offsets_index >= offsets_len) {
            offsets_len = offsets_len + BUFF_LEN;
            offsets = (int *)realloc(offsets, sizeof(int) * offsets_len);
            if(offsets == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }
        }

        offsets[offsets_index] = offset;
        offsets_index++;
    }

    fprintf(stdout, "Resulting Frequency: %d\n", freq);

    freq = determine_frequency_reached_twice(offsets, offsets_index);
    fprintf(stdout, "What is the first frequency your device reaches twice: %d\n", freq);

    free(offsets);

    return 0;
}

int determine_frequency_reached_twice(int *offsets, int offset_len)
{
    int curr_freq = 0;
    int freqs_encountered_index = 0;
    int freqs_encountered_len = BUFF_LEN;
    int *freqs_encountered = (int *)malloc(sizeof(int) * freqs_encountered_len);
    if(freqs_encountered == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    freqs_encountered[freqs_encountered_index] = curr_freq;
    freqs_encountered_index++;

    int offset_index = 0;
    int found = 0;
    while(!found) {
        curr_freq = curr_freq + offsets[offset_index];
        offset_index = (offset_index + 1) % offset_len;

        if(exists_in_frequencies_encountered(freqs_encountered, freqs_encountered_index, curr_freq)) {
            found = 1;
        } else {
            if(freqs_encountered_index >= freqs_encountered_len) {
                freqs_encountered_len = freqs_encountered_len + BUFF_LEN;
                freqs_encountered = (int *)realloc(freqs_encountered, sizeof(int) * freqs_encountered_len);
                if(freqs_encountered == NULL) {
                    perror("Fatal error: Cannot allocate memory.\n");
                    exit(EXIT_FAILURE);
                }
            }

            freqs_encountered[freqs_encountered_index] = curr_freq;
            freqs_encountered_index++;
        }
    }

    free(freqs_encountered);

    return curr_freq;
}

int exists_in_frequencies_encountered(const int *freqs_encountered, int freqs_encountered_len, int freq)
{
    for(int i = 0; i < freqs_encountered_len; i++) {
        if(freqs_encountered[i] == freq) {
            return 1;
        }
    }

    return 0;
}