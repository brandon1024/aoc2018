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
char *determine_step_completion_order(struct simple_dep_t *steps, int steps_len, char *buffer, int buffer_len);

int main(int argc, char *argv[])
{
    char buffer[BUFF_LEN];

    int steps_len = BUFF_LEN;
    struct simple_dep_t *steps = (struct simple_dep_t *)malloc(sizeof(struct simple_dep_t) * steps_len);
    if(steps == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    int steps_index = 0;
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

    char *order = (char *)malloc(sizeof(char) * steps_len);
    if(order == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    memset(order, 0, steps_len);
    order = determine_step_completion_order(steps, steps_len, order, steps_len);
    fprintf(stdout, "In what order should the steps in your instructions be completed? %.*s", steps_len, order);

    free(steps);

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

char *determine_step_completion_order(struct simple_dep_t *steps, int steps_len, char *buffer, int buffer_len)
{
    int nodes_len = BUFF_LEN;
    int node_index = 0;

    struct dep_graph_node_t *nodes = (struct dep_graph_node_t *)malloc(sizeof(struct dep_graph_node_t) * nodes_len);
    if(nodes == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    for(int i = 0; i < steps_len; i++) {
        struct dep_graph_node_t *node = NULL;
        for(int j = 0; j < node_index; j++) {
            if(nodes[j].id == steps[i].id) {
                node = nodes + j;
                break;
            }
        }

        if(node == NULL) {
            if(node_index >= nodes_len) {
                nodes_len += BUFF_LEN;
                nodes = (struct dep_graph_node_t *)realloc(nodes, sizeof(struct dep_graph_node_t) * nodes_len);
                if(nodes == NULL) {
                    fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
                    exit(1);
                }
            }

            struct dep_graph_node_t new_node = {.id = steps[i].id, .dependencies = NULL, .complete = 0};
            nodes[node_index] = new_node;
            node = nodes + node_index;
            node_index++;
        }

        struct dep_graph_node_t *dep = NULL;
        for(int j = 0; j < node_index; j++) {
            if(nodes[j].id == steps[i].depends) {
                dep = nodes + j;
                break;
            }
        }

        if(dep == NULL) {
            if(node_index >= nodes_len) {
                nodes_len += BUFF_LEN;
                nodes = (struct dep_graph_node_t *)realloc(nodes, sizeof(struct dep_graph_node_t) * nodes_len);
                if(nodes == NULL) {
                    fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
                    exit(1);
                }
            }

            struct dep_graph_node_t new_node = {.id = steps[i].depends, .dependencies = NULL, .complete = 0};
            nodes[node_index] = new_node;
            dep = nodes + node_index;
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
    nodes = (struct dep_graph_node_t *)realloc(nodes, sizeof(struct dep_graph_node_t) * nodes_len);
    if(nodes == NULL) {
        fprintf(stderr, "Fatal error: Cannot allocate memory.\n");
        exit(1);
    }

    for(int i = 0; i < nodes_len; i++) {
        struct dep_list_node_t *dep_node = nodes[i].dependencies;
        while(dep_node != NULL) {
            struct dep_list_node_t *tmp = dep_node;
            dep_node = dep_node->next;
            free(tmp);
        }
    }

    free(nodes);

    return NULL;
}