#ifndef _STACK_H
#define _STACK_H

typedef struct _stk_node {
    void *data;
    struct _stk_node *next;
} StackNode;

StackNode *stack_create_node();
void stack_delete_node(StackNode *node);
void stack_clear(StackNode **top);
void stack_push(StackNode **top, StackNode *node);
StackNode *stack_pop(StackNode **top);

#endif