#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define BUFF_LEN 64
#define LAST_RECIPES 10

int fill_last_n_recipe_scores(size_t recipes_count, uint8_t last_recipes[], size_t last_count);
size_t find_recipes_before_sequence(size_t sequence, size_t sequence_len);
uint8_t get_nibble(const uint8_t *numbers, size_t nibble_len, size_t nibble_index);
uint8_t set_nibble(uint8_t *numbers, size_t nibble_len, size_t nibble_index, uint8_t value);

int main(int argc, char *argv[])
{
    char buffer[BUFF_LEN];

    if(fgets(buffer, BUFF_LEN, stdin) == NULL) {
        fprintf(stderr, "Unexpected error: unable to read from stdin.\n");
        exit(EXIT_FAILURE);
    }

    char *eos = memchr(buffer, 0, BUFF_LEN);
    if(eos == NULL) {
        fprintf(stderr, "Unexpected error: input exceeds buffer size.\n");
        exit(EXIT_FAILURE);
    }

    size_t recipes = (size_t)strtol(buffer, &eos, 10);

    uint8_t *last_recipes = (uint8_t *)calloc(LAST_RECIPES, sizeof(uint8_t));
    if(last_recipes == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    if(fill_last_n_recipe_scores(recipes, last_recipes, LAST_RECIPES)) {
        fprintf(stderr, "Unexpected error: unable to compute last %d scores.\n", LAST_RECIPES);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "What are the scores of the ten recipes immediately after the number of recipes in your puzzle input? ");
    for(int index = 0; index < LAST_RECIPES; index++) {
        fprintf(stdout, "%u", last_recipes[index]);
    }

    fprintf(stdout, "\n");

    size_t recipes_before_sequence = find_recipes_before_sequence(recipes, strlen(buffer));
    fprintf(stdout, "How many recipes appear on the scoreboard to the left of the score sequence in your puzzle input? %lu\n", recipes_before_sequence);

    free(last_recipes);

    return 0;
}

int fill_last_n_recipe_scores(size_t recipes_count, uint8_t last_recipes[], size_t last_count)
{
    size_t scores_len = (recipes_count + 10) * 2;
    size_t recipes_index = 0;
    uint8_t *scores = (uint8_t *)calloc((recipes_count + 10), sizeof(uint8_t));
    if(scores == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    size_t p1i = 0, p2i = 1;

    set_nibble(scores, scores_len, recipes_index, 3);
    recipes_index++;
    set_nibble(scores, scores_len, recipes_index, 7);
    recipes_index++;

    while(recipes_index < (recipes_count + last_count)) {
        uint8_t sum = get_nibble(scores, scores_len, p1i) + get_nibble(scores, scores_len, p2i);

        if(sum < 10) {
            set_nibble(scores, scores_len, recipes_index, sum);
            recipes_index++;
        } else {
            uint8_t msd = sum / (uint8_t)10;
            uint8_t lsd = sum % (uint8_t)10;

            set_nibble(scores, scores_len, recipes_index, msd);
            recipes_index++;

            set_nibble(scores, scores_len, recipes_index, lsd);
            recipes_index++;
        }

        p1i = (p1i + 1 + get_nibble(scores, scores_len, p1i)) % recipes_index;
        p2i = (p2i + 1 + get_nibble(scores, scores_len, p2i)) % recipes_index;
    }

    for(size_t index = recipes_count; index < recipes_count + last_count; index++) {
        last_recipes[index - recipes_count] = get_nibble(scores, scores_len, index);
    }

    free(scores);

    return 0;
}

size_t find_recipes_before_sequence(size_t sequence, size_t sequence_len)
{
    uint8_t *digits = (uint8_t *)malloc(sequence_len * sizeof(uint8_t));
    if(digits == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    size_t sequence_tmp = sequence;
    for(size_t index = sequence_len; index > 0; index--) {
        digits[index - 1] = (uint8_t)(sequence_tmp % 10);
        sequence_tmp /= 10;
    }

    size_t scores_len = BUFF_LEN;
    size_t recipes_index = 0;
    uint8_t *scores = (uint8_t *)malloc(sizeof(uint8_t) * scores_len);
    if(scores == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    size_t p1i = 0, p2i = 1;

    set_nibble(scores, scores_len * 2, recipes_index, 3);
    recipes_index++;
    set_nibble(scores, scores_len * 2, recipes_index, 7);
    recipes_index++;

    size_t match_index = 0;
    while(match_index < sequence_len) {
        uint8_t sum = get_nibble(scores, scores_len * 2, p1i) + get_nibble(scores, scores_len * 2, p2i);

        if((recipes_index+1) >= (scores_len * 2)) {
            scores_len += BUFF_LEN;
            scores = (uint8_t *)realloc(scores, sizeof(uint8_t) * scores_len);
            if(scores == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }
        }

        if(sum < 10) {
            set_nibble(scores, scores_len * 2, recipes_index, sum);
            recipes_index++;

            if(sum == digits[match_index]) {
                match_index++;
            } else {
                match_index = (sum == digits[0]) ? 1 : 0;
            }
        } else {
            uint8_t msd = sum / (uint8_t)10;
            uint8_t lsd = sum % (uint8_t)10;

            set_nibble(scores, scores_len * 2, recipes_index, msd);
            recipes_index++;

            set_nibble(scores, scores_len * 2, recipes_index, lsd);
            recipes_index++;

            if(msd == digits[match_index]) {
                match_index++;
            } else {
                match_index = (msd == digits[0]) ? 1 : 0;
            }

            if(match_index == sequence_len) {
                break;
            }

            if(lsd == digits[match_index]) {
                match_index++;
            } else {
                match_index = (lsd == digits[0]) ? 1 : 0;
            }
        }

        p1i = (p1i + 1 + get_nibble(scores, scores_len * 2, p1i)) % recipes_index;
        p2i = (p2i + 1 + get_nibble(scores, scores_len * 2, p2i)) % recipes_index;
    }

    free(scores);
    free(digits);

    return recipes_index - sequence_len;
}

uint8_t get_nibble(const uint8_t *numbers, size_t nibble_len, size_t nibble_index)
{
    if(nibble_index >= nibble_len) {
        return 0;
    }

    size_t index = nibble_index / 2;

    return (nibble_index % 2 == 0) ? numbers[index] >> 4 : numbers[index] & (uint8_t)0x0f;
}

uint8_t set_nibble(uint8_t *numbers, size_t nibble_len, size_t nibble_index, uint8_t value)
{
    if(nibble_index >= nibble_len) {
        return 0;
    }

    size_t index = nibble_index / 2;

    if(nibble_index % 2 == 0) {
        value = (value << 4) & (uint8_t)0xf0;
        numbers[index] = numbers[index] & (uint8_t)0x0f;
    } else {
        value = value & (uint8_t)0x0f;
        numbers[index] = numbers[index] & (uint8_t)0xf0;
    }

    numbers[index] = numbers[index] | value;

    return 1;
}