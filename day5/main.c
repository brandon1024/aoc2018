#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_LEN 64

char *react_polymer(const char *polymer_begin, char *polymer_safe, long int polymer_len);
int reduce_polymer(char *polymer, long int polymer_len);
int check_unit_similarity(const char *unit_a, const char *unit_b);

int main(int argc, char *argv[])
{
    char buffer[BUFF_LEN];

    long int polymer_len = BUFF_LEN;
    char *polymer = (char *)malloc(sizeof(char) * BUFF_LEN);
    if(polymer == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    memset(polymer, 0, sizeof(char) * BUFF_LEN);

    long int polymer_index = 0;
    size_t chars_read = 0;
    while(chars_read = fread(buffer, sizeof(char), BUFF_LEN, stdin), chars_read != 0) {
        //resize if necessary
        if((polymer_index + chars_read) > polymer_len) {
            polymer_len = polymer_len + (2 * BUFF_LEN);

            polymer = (char *)realloc(polymer, sizeof(char) * polymer_len);
            memset(polymer + polymer_index, 0, polymer_len - polymer_index);
        }

        strncpy(polymer + polymer_index, buffer, chars_read);
        polymer_index = polymer_index + chars_read;
    }

    polymer_len = polymer_index;
    char *polymer_pivot = polymer;
    while(polymer_pivot != NULL) {
        polymer_pivot = react_polymer(polymer, polymer_pivot, polymer_len);
    }

    reduce_polymer(polymer, polymer_len);

    fprintf(stdout, "How many units remain after fully reacting the polymer you scanned? %zu\n", strlen(polymer));

    free(polymer);

    return 0;
}

char *react_polymer(const char *polymer_begin, char *polymer_safe, long int polymer_len)
{
    long int safe_len = polymer_len - (polymer_safe - polymer_begin);

    for(int index = 0; index < safe_len; index++) {
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
    //Verify that unit_a is a-zA-Z
    if((*unit_a < 65 || *unit_a > 90) && (*unit_a < 97 || *unit_a > 122)) {
        return 0;
    }

    //Verify that unit_b is a-zA-Z
    if((*unit_b < 65 || *unit_b > 90) && (*unit_b < 97 || *unit_b > 122)) {
        return 0;
    }

    //Return true if unit_a is lowercase and unit_b is uppercase, and both are same letter
    if((*unit_a >= 65 && *unit_a <= 90) && ((*unit_a + 32) == *unit_b)) {
        return 1;
    }

    //Return true if unit_b is lowercase and unit_a is uppercase, and both are same letter
    if((*unit_b >= 65 && *unit_b <= 90) && (*unit_a == (*unit_b + 32))) {
        return 1;
    }

    return 0;
}