#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_LEN 64

struct simple_dep_t {
    char id;
    char depends;
};

struct dep_graph_node_t {
    char id;
    unsigned int complete: 1;
    struct dep_list_node_t *dependencies;
};

struct dep_list_node_t {
    struct dep_graph_node_t *dependency;
    struct dep_list_node_t *next;
};

struct simple_dep_t build_dependency_node(char buffer[], size_t buff_len);
int build_dependency_graph(struct simple_dep_t *steps, int steps_len, struct dep_graph_node_t **nodes_addr[]);
void release_dependency_graph_resources(struct dep_graph_node_t *nodes[], int nodes_len);
int populate_buffer_with_order(struct dep_graph_node_t *nodes[], int nodes_len, char *buffer, int buffer_len);
struct dep_graph_node_t *find_next_node_with_satisfied_deps(struct dep_graph_node_t *nodes[], int nodes_len);
int determine_completion_length(struct dep_graph_node_t *nodes[], int nodes_len);

int main(int argc, char *argv[])
{
    int steps_len = BUFF_LEN;
    struct simple_dep_t *steps = (struct simple_dep_t *)malloc(sizeof(struct simple_dep_t) * steps_len);
    if(steps == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    int steps_index = 0;
    char buffer[BUFF_LEN];
    while(fgets(buffer, BUFF_LEN, stdin) != NULL) {
        if(steps_index >= steps_len) {
            steps_len = steps_len + BUFF_LEN;
            steps = (struct simple_dep_t *)realloc(steps, sizeof(struct simple_dep_t) * steps_len);
            if(steps == NULL) {
                fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
                exit(1);
            }
        }

        steps[steps_index] = build_dependency_node(buffer, BUFF_LEN);
        steps_index++;
    }

    steps_len = steps_index;

    struct dep_graph_node_t **nodes = NULL;
    int nodes_len = build_dependency_graph(steps, steps_len, &nodes);
    free(steps);

    char *order = (char *)malloc(sizeof(char) * steps_len);
    if(order == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    int steps_count = populate_buffer_with_order(nodes, nodes_len, order, steps_len);
    if(steps_count == 0) {
        fprintf(stderr, "Unexpected error occurred: cannot resolve dependencies.\n");
        exit(1);
    }

    fprintf(stdout, "In what order should the steps in your instructions be completed? %.*s\n", steps_count, order);
    free(order);

    int seconds_to_complete = determine_completion_length(nodes, nodes_len);
    fprintf(stdout, "With 5 workers and the 60+ second step durations described above, how long will it take to complete all of the steps? %d\n", seconds_to_complete);

    release_dependency_graph_resources(nodes, nodes_len);

    return 0;
}

struct simple_dep_t build_dependency_node(char buffer[], size_t buff_len)
{
    char *lf = memchr(buffer, '\n', buff_len);
    if(lf != NULL) {
        *lf = 0;
    }

    char *eos = memchr(buffer, 0, buff_len);
    if(eos == NULL) {
        fprintf(stderr, "Unexpected input: %32s\n", buffer);
        exit(1);
    }

    char id = 0;
    char dep = 0;
    if(sscanf(buffer, "Step %c must be finished before step %c can begin.", &dep, &id) != 2) {
        fprintf(stderr, "Unexpected input: %32s\n", buffer);
        exit(1);
    }

    struct simple_dep_t node = {.id = id, .depends = dep};

    return node;
}

int build_dependency_graph(struct simple_dep_t *steps, int steps_len, struct dep_graph_node_t **nodes_addr[])
{
    int nodes_len = BUFF_LEN;
    int node_index = 0;

    struct dep_graph_node_t **nodes = (struct dep_graph_node_t **)malloc(sizeof(struct dep_graph_node_t *) * nodes_len);
    if(nodes == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    for(int i = 0; i < steps_len; i++) {
        struct dep_graph_node_t *node = NULL;
        for(int j = 0; j < node_index; j++) {
            if(nodes[j]->id == steps[i].id) {
                node = nodes[j];
                break;
            }
        }

        if(node == NULL) {
            if(node_index >= nodes_len) {
                nodes_len += BUFF_LEN;
                nodes = (struct dep_graph_node_t **)realloc(nodes, sizeof(struct dep_graph_node_t *) * nodes_len);
                if(nodes == NULL) {
                    fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
                    exit(1);
                }
            }


            struct dep_graph_node_t *new_node = (struct dep_graph_node_t *)malloc(sizeof(struct dep_graph_node_t));
            if(new_node == NULL) {
                fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
                exit(1);
            }

            new_node->id = steps[i].id;
            new_node->dependencies = NULL;
            new_node->complete = 0;

            nodes[node_index] = new_node;
            node = new_node;
            node_index++;
        }

        struct dep_graph_node_t *dep = NULL;
        for(int j = 0; j < node_index; j++) {
            if(nodes[j]->id == steps[i].depends) {
                dep = nodes[j];
                break;
            }
        }

        if(dep == NULL) {
            if(node_index >= nodes_len) {
                nodes_len += BUFF_LEN;
                nodes = (struct dep_graph_node_t **)realloc(nodes, sizeof(struct dep_graph_node_t *) * nodes_len);
                if(nodes == NULL) {
                    fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
                    exit(1);
                }
            }

            struct dep_graph_node_t *new_node = (struct dep_graph_node_t *)malloc(sizeof(struct dep_graph_node_t));
            if(new_node == NULL) {
                fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
                exit(1);
            }

            new_node->id = steps[i].depends;
            new_node->dependencies = NULL;
            new_node->complete = 0;

            nodes[node_index] = new_node;
            dep = new_node;
            node_index++;
        }

        struct dep_list_node_t *new_list_node = (struct dep_list_node_t *)malloc(sizeof(struct dep_list_node_t));
        if(new_list_node == NULL) {
            fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
            exit(1);
        }

        new_list_node->next = node->dependencies;
        new_list_node->dependency = dep;
        node->dependencies = new_list_node;
    }

    nodes_len = node_index;
    nodes = (struct dep_graph_node_t **)realloc(nodes, sizeof(struct dep_graph_node_t *) * nodes_len);
    if(nodes == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    *nodes_addr = nodes;
    return nodes_len;
}

void release_dependency_graph_resources(struct dep_graph_node_t *nodes[], int nodes_len)
{
    for(int i = 0; i < nodes_len; i++) {
        struct dep_list_node_t *dep_node = nodes[i]->dependencies;
        while(dep_node != NULL) {
            struct dep_list_node_t *tmp = dep_node;
            dep_node = dep_node->next;
            free(tmp);
        }
    }

    free(nodes);
}

int populate_buffer_with_order(struct dep_graph_node_t *nodes[], int nodes_len, char *buffer, int buffer_len)
{
    int buff_index = 0;

    memset(buffer, 0, buffer_len);

    struct dep_graph_node_t *node = NULL;
    while((node = find_next_node_with_satisfied_deps(nodes, nodes_len)) != NULL) {
        if(buff_index >= buffer_len) {
            return 0;
        }

        buffer[buff_index] = node->id;
        node->complete = 1;
        buff_index++;
    }

    return buff_index;
}

struct dep_graph_node_t *find_next_node_with_satisfied_deps(struct dep_graph_node_t *nodes[], int nodes_len)
{
    struct dep_graph_node_t *best = NULL;
    for(int i = 0; i < nodes_len; i++) {
        struct dep_graph_node_t *current = nodes[i];

        if(current->complete) {
            continue;
        }

        int all_satisfied = 1;
        struct dep_list_node_t *dep = current->dependencies;
        while(dep != NULL) {
            if(!dep->dependency->complete) {
                all_satisfied = 0;
                break;
            }

            dep = dep->next;
        }

        if(all_satisfied && (best == NULL || current->id < best->id)) {
            best = current;
        }
    }

    return best;
}

int determine_completion_length(struct dep_graph_node_t *nodes[], int nodes_len)
{
    return 0;
}