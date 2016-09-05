#ifndef LIST_H
#define LIST_H

typedef struct list_node_s {
    char s_name[256];

    struct list_node_s *prev;
    struct list_node_s *next;
} list_node_t;

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

void list_init(list_node_t *node);
void list_insert(list_node_t *node, list_node_t *head);

#endif // LIST_H