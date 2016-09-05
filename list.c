
#include "list.h"

#include <assert.h>
#include <stdio.h>

void list_init(list_node_t *node)
{
    assert (node != NULL);

    node->prev = node;
    node->next = node;
}

void list_insert(list_node_t *node, list_node_t *head)
{
    assert (node != NULL);
    assert (head != NULL);

    node->next = head->next;
    node->prev = head;
    head->next->prev = node;
    head->next = node;
}
