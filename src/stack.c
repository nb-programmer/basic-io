
#include "stack.h"

#include <stdlib.h>

StackNode *stack_create_node() {
    //Allocate a new node
	StackNode *new_node = (StackNode *)malloc(sizeof(StackNode));

	//See if we actually got the new node
	if (new_node == NULL)
		return NULL;

	new_node->data = NULL;
	new_node->next = NULL;

	return new_node;
}
void stack_delete_node(StackNode *node) {
    if (node != NULL)
        free(node);
}

void stack_clear(StackNode **top) {
    StackNode *ptr;
    while ((ptr = stack_pop(top)) != NULL)
        stack_delete_node(ptr);
}

void stack_push(StackNode **top, StackNode *node) {
    if (node == NULL) return;
    node->next = *top;
    *top = node;
}

StackNode *stack_pop(StackNode **top) {
    if (*top == NULL) return NULL;
    StackNode *top_node = *top;
    *top = (*top)->next;
    return top_node;
}
