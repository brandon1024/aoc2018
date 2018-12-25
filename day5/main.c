#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_LEN 64

char *react_polymer(const char *polymer_begin, char *polymer_safe, long int polymer_len);
int reduce_polymer(char *polymer, long int polymer_len);
int check_unit_similarity(const char *unit_a, const char *unit_b);
void replace_all_char_from_str(char *str, long int str_len, char c, char p);

int main(int argc, char *argv[])
{
    char buffer[BUFF_LEN];

    long int polymer_len = BUFF_LEN;
    char *polymer = (char *)malloc(sizeof(char) * BUFF_LEN);
    if(polymer == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    memset(polymer, 0, sizeof(char) * BUFF_LEN);

    long int polymer_index = 0;
    size_t chars_read = 0;
    while(chars_read = fread(buffer, sizeof(char), BUFF_LEN, stdin), chars_read != 0) {
        if((polymer_index + chars_read) > polymer_len) {
            polymer_len = polymer_len + (2 * BUFF_LEN);

            polymer = (char *)realloc(polymer, sizeof(char) * polymer_len);
            if(polymer == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }

            memset(polymer + polymer_index, 0, polymer_len - polymer_index);
        }

        strncpy(polymer + polymer_index, buffer, chars_read);
        polymer_index = polymer_index + chars_read;
    }

    polymer_len = polymer_index;
    polymer = (char *)realloc(polymer, sizeof(char) * polymer_len);
    if(polymer == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    char *polymer_cpy = (char *)malloc(sizeof(char) * polymer_len);
    if(polymer_cpy == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    strncpy(polymer_cpy, polymer, polymer_len);

    char *polymer_pivot = polymer_cpy;
    while(polymer_pivot != NULL) {
        polymer_pivot = react_polymer(polymer_cpy, polymer_pivot, polymer_len);
    }

    reduce_polymer(polymer_cpy, polymer_len);

    size_t best_len = strlen(polymer_cpy);
    fprintf(stdout, "How many units remain after fully reacting the polymer you scanned? %zu\n", best_len);

    for(char c = 65; c < 91; c++) {
        strncpy(polymer_cpy, polymer, polymer_len);
        replace_all_char_from_str(polymer_cpy, polymer_len, c, 0);

        polymer_pivot = polymer_cpy;
        while(polymer_pivot != NULL) {
            polymer_pivot = react_polymer(polymer_cpy, polymer_pivot, polymer_len);
        }

        reduce_polymer(polymer_cpy, polymer_len);

        size_t curr_len = strlen(polymer_cpy);
        if(curr_len < best_len) {
            best_len = curr_len;
        }
    }

    fprintf(stdout, "What is the length of the shortest polymer you can produce by removing all units of exactly one type and fully reacting the result? %zu\n", best_len);

    free(polymer);
    free(polymer_cpy);

    return 0;
}

char *react_polymer(const char *polymer_begin, char *polymer_safe, long int polymer_len)
{
    long int safe_len = polymer_len - (polymer_safe - polymer_begin);

    for(long int index = 0; index < safe_len; index++) {
        char *unit_a = polymer_safe + index;
        if(*unit_a == 0) {
            continue;
        }

        if((index + 1) >= safe_len) {
            return NULL;
        }

        char *unit_b = unit_a + 1;
        while(unit_b < (polymer_safe + safe_len) && *unit_b == 0) {
            unit_b = unit_b + 1;
        }

        if(check_unit_similarity(unit_a, unit_b)) {
            *unit_a = 0;
            *unit_b = 0;

            char *closest_non_null_unit = unit_a;
            while(closest_non_null_unit >= polymer_begin) {
                if(*closest_non_null_unit != 0) {
                    return closest_non_null_unit;
                }

                closest_non_null_unit = closest_non_null_unit - 1;
            }

            closest_non_null_unit = unit_b;
            while(closest_non_null_unit < (polymer_begin + polymer_len)) {
                if(*closest_non_null_unit != 0) {
                    return closest_non_null_unit;
                }

                closest_non_null_unit = closest_non_null_unit + 1;
            }

            return NULL;
        }
    }

    return NULL;
}

int reduce_polymer(char *polymer, long int polymer_len)
{
    char *pivot = polymer;
    char *forward_pivot = polymer + 1;

    while(pivot < (polymer + polymer_len) && forward_pivot < (polymer + polymer_len)) {
        if(*pivot == 0) {
            while(*forward_pivot == 0) {
                forward_pivot++;

                if(forward_pivot >= (polymer + polymer_len)) {
                    break;
                }
            }

            if(forward_pivot >= (polymer + polymer_len)) {
                break;
            }

            *pivot = *forward_pivot;
            *forward_pivot = 0;
            forward_pivot++;
        }

        pivot++;
    }

    return 1;
}

int check_unit_similarity(const char *unit_a, const char *unit_b)
{
    if((*unit_a < 65 || *unit_a > 90) && (*unit_a < 97 || *unit_a > 122)) {
        return 0;
    }

    if((*unit_b < 65 || *unit_b > 90) && (*unit_b < 97 || *unit_b > 122)) {
        return 0;
    }

    if((*unit_a >= 65 && *unit_a <= 90) && ((*unit_a + 32) == *unit_b)) {
        return 1;
    }

    if((*unit_b >= 65 && *unit_b <= 90) && (*unit_a == (*unit_b + 32))) {
        return 1;
    }

    return 0;
}

void replace_all_char_from_str(char *str, long int str_len, char c, char p)
{
    if((c >= 65 && c <= 90)) {
        c = c + (char)32;
    }

    for(long int index = 0; index < str_len; index++) {
        char curr_char = *(str + index);

        if((curr_char >= 65 && curr_char <= 90)) {
            curr_char = curr_char + (char)32;
        }

        if(curr_char == c) {
            *(str + index) = p;
        }
    }
}