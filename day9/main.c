#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_LEN 64

struct game_t {
    int players_count;
    int marbles_count;
    struct player_t *players;
    struct marble_t *current_marble;
};

struct player_t {
    long long int score;
};

struct marble_t {
    int value;
    struct marble_t *next;
    struct marble_t *previous;
};

struct game_t parse_game_details(char buffer[], size_t buff_len);
long long int play(struct game_t game);
void release_marble_resources(struct marble_t *current_marble);
void release_player_resources(struct player_t *players);
struct marble_t *create_marble(struct marble_t *after_this, int value);
struct marble_t *remove_marble(struct marble_t *after_this);

int main(int argc, char *argv[])
{
    char buffer[BUFF_LEN];
    if(fgets(buffer, BUFF_LEN, stdin) == NULL) {
        fprintf(stderr, "Unexpected input.\n");
        exit(1);
    }

    struct game_t game = parse_game_details(buffer, BUFF_LEN);

    long long int winning_score = play(game);
    fprintf(stdout, "What is the winning Elf's score? %lld\n", winning_score);

    game.marbles_count = game.marbles_count * 100;
    winning_score = play(game);
    fprintf(stdout, "What would the new winning Elf's score be if the number of the last marble were 100 times larger? %lld\n", winning_score);

    return 0;
}

struct game_t parse_game_details(char buffer[], size_t buff_len)
{
    struct game_t game = {.marbles_count = 0, .players = NULL, .players_count = 0};
    char *eos = NULL;

    char *start_index;
    char *token;

    start_index = buffer;
    token = memchr(start_index, ' ', start_index + buff_len - buffer);
    *token = 0;

    game.players_count = (int)strtol(start_index, &eos, 10);

    start_index = token + 31;
    token = memchr(start_index, ' ', start_index + buff_len - buffer);
    *token = 0;

    game.marbles_count = (int)strtol(start_index, &eos, 10);

    return game;
}

long long int play(struct game_t game)
{
    struct player_t *players = (struct player_t *)malloc(sizeof(struct player_t) * game.players_count);
    if(players == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < game.players_count; i++) {
        players[i].score = 0;
    }

    game.players = players;

    struct marble_t *new_marble = (struct marble_t *)malloc(sizeof(struct marble_t));
    if(new_marble == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    new_marble->value = 0;
    new_marble->next = new_marble;
    new_marble->previous = new_marble;

    game.current_marble = new_marble;

    int marble_index = 1;
    int player_index = 0;
    while(marble_index < game.marbles_count) {
        if(marble_index % 23 == 0) {
            game.players[player_index].score += marble_index;

            for(int i = 0; i < 7; i++) {
                game.current_marble = game.current_marble->previous;
            }

            game.players[player_index].score += game.current_marble->value;
            game.current_marble = remove_marble(game.current_marble);
        } else {
            game.current_marble = create_marble(game.current_marble->next, marble_index);
        }

        marble_index++;
        player_index = (player_index + 1) % game.players_count;
    }

    long long int highest_score = 0;
    for(int i = 0; i < game.players_count; i++) {
        if(game.players[i].score > highest_score) {
            highest_score = game.players[i].score;
        }
    }

    release_marble_resources(game.current_marble);
    release_player_resources(game.players);

    return highest_score;
}

void release_marble_resources(struct marble_t *current_marble)
{
    current_marble->previous->next = NULL;

    while(current_marble != NULL) {
        struct marble_t *tmp = current_marble;
        current_marble = current_marble->next;
        free(tmp);
    }
}

void release_player_resources(struct player_t *players)
{
    free(players);
}

struct marble_t *create_marble(struct marble_t *after_this, int value)
{
    struct marble_t *new_marble = (struct marble_t *)malloc(sizeof(struct marble_t));
    if(new_marble == NULL) {
        perror("Fatal error: Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    after_this->next->previous = new_marble;
    new_marble->next = after_this->next;
    new_marble->previous = after_this;
    after_this->next = new_marble;
    new_marble->value = value;

    return new_marble;
}

struct marble_t *remove_marble(struct marble_t *this)
{
    struct marble_t *tmp = this->next;

    this->previous->next = this->next;
    this->next->previous = this->previous;

    free(this);

    return tmp;
}