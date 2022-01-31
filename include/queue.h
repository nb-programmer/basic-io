#ifndef _QUEUE_H
#define _QUEUE_H

typedef struct _q_node {
    void *data;
    struct _q_node *next;
} QueueNode;

typedef struct {
    QueueNode *front, *rear;
} Queue;

QueueNode *queue_create_node();
void queue_delete_node(QueueNode *node);
void queue_clear(Queue *queue);
void queue_enqueue(Queue *queue, QueueNode *node);
QueueNode *queue_dequeue(Queue *queue);

#endif