#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_LEN 8

struct tree_node_header_t {
    int child_nodes;
    int metadata_entries;
    int node_value;
    unsigned int counted: 1;
};

struct tree_node_metadata_entry_t {
    int data;
    struct tree_node_metadata_entry_t *next;
};

struct tree_node_child_t {
    struct tree_node_t *child;
    struct tree_node_child_t *next;
};

struct tree_node_t {
    struct tree_node_header_t header;
    struct tree_node_t *parent;
    struct tree_node_child_t *children;
    struct tree_node_metadata_entry_t *entries;
};

struct tree_node_t *build_tree_from_stdin(FILE *stream);
struct tree_node_header_t build_header(FILE *stream);
struct tree_node_metadata_entry_t *build_metadata_list(FILE *stream, int count);
void release_tree_resources(struct tree_node_t *root);
int compute_metadata_sum_from_tree(struct tree_node_t *root);
int metadata_sum(struct tree_node_metadata_entry_t *metadata_list);
int verify_child_nodes_count_match(struct tree_node_t *node);

int main(int argc, char *argv[])
{
    struct tree_node_t *root = build_tree_from_stdin(stdin);
    int metadata_sum = compute_metadata_sum_from_tree(root);

    fprintf(stdout, "What is the sum of all metadata entries? %d\n", metadata_sum);
    fprintf(stdout, "What is the value of the root node? %d\n", root->header.node_value);

    release_tree_resources(root);

    return 0;
}

struct tree_node_t *build_tree_from_stdin(FILE *stream)
{
    struct tree_node_t *parent = NULL;
    do {
        if(parent == NULL || !verify_child_nodes_count_match(parent)) {
            struct tree_node_t *node = (struct tree_node_t *)malloc(sizeof(struct tree_node_t));
            if(node == NULL) {
                perror("Fatal error: Cannot allocate memory.\n");
                exit(EXIT_FAILURE);
            }

            node->header = build_header(stream);
            node->parent = parent;
            node->children = NULL;
            node->entries = NULL;

            if(parent != NULL) {
                struct tree_node_child_t *new_child_node = (struct tree_node_child_t *)malloc(sizeof(struct tree_node_child_t));
                if(new_child_node == NULL) {
                    perror("Fatal error: Cannot allocate memory.\n");
                    exit(EXIT_FAILURE);
                }

                new_child_node->next = parent->children;
                new_child_node->child = node;

                parent->children = new_child_node;
            }

            parent = node;
        } else {
            parent->entries = build_metadata_list(stream, parent->header.metadata_entries);

            if(parent->header.child_nodes == 0) {
                parent->header.node_value = metadata_sum(parent->entries);
            } else {
                struct tree_node_metadata_entry_t *current_entry = parent->entries;

                while(current_entry != NULL) {
                    struct tree_node_child_t *current_child = parent->children;
                    int index = parent->header.child_nodes;

                    while(current_child != NULL) {
                        if(current_entry->data == index) {
                            parent->header.node_value += current_child->child->header.node_value;
                            break;
                        }

                        current_child = current_child->next;
                        index--;
                    }

                    current_entry = current_entry->next;
                }
            }

            if(parent->parent == NULL) {
                break;
            } else {
                parent = parent->parent;
            }
        }
    } while(!feof(stream));

    return parent;
}

struct tree_node_header_t build_header(FILE *stream)
{
    struct tree_node_header_t header = {.child_nodes = 0, .metadata_entries = 0};
    char buffer[BUFF_LEN];
    char *eos = NULL;
    int items_read = 0;

    memset(buffer, 0, BUFF_LEN);
    items_read = fscanf(stream, "%s ", buffer);
    if(!items_read) {
        fprintf(stderr, "Unexpected input: cannot read number of child nodes.\n");
        exit(1);
    }
    header.child_nodes = (int)strtol(buffer, &eos, 10);

    memset(buffer, 0, BUFF_LEN);
    items_read = fscanf(stream, "%s ", buffer);
    if(!items_read) {
        fprintf(stderr, "Unexpected input: cannot read number of metadata entries.\n");
        exit(1);
    }
    header.metadata_entries = (int)strtol(buffer, &eos, 10);
    header.counted = 0;
    header.node_value = 0;

    return header;
}

struct tree_node_metadata_entry_t *build_metadata_list(FILE *stream, int count)
{
    struct tree_node_metadata_entry_t *current = NULL;
    char buffer[BUFF_LEN];
    char *eos = NULL;
    int items_read = 0;

    for(int i = 0; i < count; i++) {
        struct tree_node_metadata_entry_t *list_node = (struct tree_node_metadata_entry_t *)malloc(sizeof(struct tree_node_metadata_entry_t));
        if(list_node == NULL) {
            perror("Fatal error: Cannot allocate memory.\n");
            exit(EXIT_FAILURE);
        }

        list_node->next = current;

        memset(buffer, 0, BUFF_LEN);
        items_read = fscanf(stream, "%s ", buffer);
        if(!items_read) {
            fprintf(stderr, "Unexpected input: cannot read metadata entry.\n");
            exit(1);
        }

        list_node->data = (int)strtol(buffer, &eos, 10);

        current = list_node;
    }

    return current;
}

void release_tree_resources(struct tree_node_t *root)
{
    struct tree_node_t *current = root;

    while(current != NULL) {
        if(current->children != NULL) {
            struct tree_node_child_t *tmp = current->children;

            current->children = tmp->next;
            current = tmp->child;
            free(tmp);
        } else {
            struct tree_node_metadata_entry_t *entry = current->entries;
            while(entry != NULL) {
                struct tree_node_metadata_entry_t *tmp = entry->next;
                free(entry);
                entry = tmp;
            }

            struct tree_node_t *tmp = current;
            current = current->parent;

            free(tmp);
        }
    }
}

int compute_metadata_sum_from_tree(struct tree_node_t *root)
{
    int total = 0;

    struct tree_node_t *current = root;
    while(current != NULL) {
        struct tree_node_child_t *child = current->children;
        while(child != NULL) {
            if(!child->child->header.counted) {
                break;
            }
            child = child->next;
        }

        if(child != NULL) {
            current = child->child;
        } else {
            total += metadata_sum(current->entries);
            current->header.counted = 1;
            current = current->parent;
        }
    }

    return total;
}

int metadata_sum(struct tree_node_metadata_entry_t *metadata_list)
{
    int total = 0;

    struct tree_node_metadata_entry_t *current = metadata_list;
    while(current != NULL) {
        total += current->data;
        current = current->next;
    }

    return total;
}

int verify_child_nodes_count_match(struct tree_node_t *node)
{
    int count = 0;

    struct tree_node_child_t *current = node->children;
    while(current != NULL) {
        count++;
        current = current->next;
    }

    return count == node->header.child_nodes;
}