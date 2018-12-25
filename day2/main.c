#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFF_LEN 32

unsigned char id_multiples(char *id, int occr);
char *find_common_chars(char **ids, int len, char *buffer, int buffer_len);
char *find_common_chars_ids(char *id_a, char *id_b, char *buffer, int buffer_len);

int main(int argc, char *argv[])
{
    int ids_index = 0;
    int ids_len = BUFF_LEN;

    char **ids = (char **)malloc(sizeof(char *) * ids_len);
    if(ids == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for(int i = ids_index; i < ids_len; i++) {
        ids[i] = NULL;
    }

    char *buffer = (char *)malloc(sizeof(char) * BUFF_LEN);
    if(buffer == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    int freq2 = 0;
    int freq3 = 0;
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

        if(id_multiples(buffer, 2)) {
            freq2++;
        }

        if(id_multiples(buffer, 3)) {
            freq3++;
        }

        if(ids_index >= ids_len) {
            ids_len = ids_len + BUFF_LEN;
            ids = (char **)realloc(ids, sizeof(char *) * ids_len);
            if(ids == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }

            for(int i = ids_index; i < ids_len; i++) {
                ids[i] = NULL;
            }
        }

        char *str = (char *)malloc(sizeof(char) * strlen(buffer));
        if(str == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }

        strcpy(str, buffer);
        ids[ids_index] = str;
        ids_index++;
    }

    buffer = find_common_chars(ids, ids_len, buffer, BUFF_LEN);
    if(buffer == NULL) {
        fprintf(stderr, "Fatal error: Unexpected error occurred while finding matching characters.\n");
        exit(1);
    }

    fprintf(stdout, "Resulting Checksum: %d (%d * %d)\n", freq2 * freq3, freq2, freq3);
    fprintf(stdout, "Common Characters: %s\n", buffer);

    for(ids_index = 0; ids_index < ids_len; ids_index++) {
        if(ids[ids_index] != NULL) {
            free(ids[ids_index]);
        }
    }

    free(ids);
    free(buffer);

    return 0;
}

unsigned char id_multiples(char *id, int occr)
{
    size_t len = strlen(id);

    for(char *ptr_a = id; ptr_a < (id + len); ptr_a++) {
        int count = 1;

        for(char *ptr_b = id; ptr_b < (id + len); ptr_b++) {
            if(ptr_a == ptr_b) {
                continue;
            }

            if(*ptr_b == *ptr_a) {
                count++;
            }
        }


        if(count == occr) {
            return 1;
        }
    }

    return 0;
}

char *find_common_chars(char **ids, int len, char *buffer, int buffer_len)
{
    for(int ids_a = 0; ids_a < len; ids_a++) {
        if(ids[ids_a] == NULL) {
            continue;
        }

        for(int ids_b = 0; ids_b < len; ids_b++) {
            if(ids[ids_b] == NULL) {
                continue;
            }

            if(ids_a == ids_b) {
                continue;
            }

            buffer = find_common_chars_ids(ids[ids_a], ids[ids_b], buffer, buffer_len);
            if(buffer == NULL) {
                return NULL;
            }

            if(strlen(buffer) == (strlen(ids[ids_a]) - 1)) {
                return buffer;
            }
        }
    }

    return NULL;
}

char *find_common_chars_ids(char *id_a, char *id_b, char *buffer, int buffer_len)
{
    size_t len = strlen(id_a);

    if(len != strlen(id_b) || len > buffer_len) {
        return NULL;
    }

    memset(buffer, 0, sizeof(char) * buffer_len);

    int buff_index = 0;
    for(int i = 0; i < len; i++) {
        if(id_a[i] == id_b[i]) {
            buffer[buff_index] = id_a[i];
            buff_index++;
        }
    }

    return buffer;
}