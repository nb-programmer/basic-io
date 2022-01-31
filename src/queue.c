
#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

QueueNode *queue_create_node() {
    //Allocate a new node
	QueueNode *new_node = (QueueNode *)malloc(sizeof(QueueNode));

	//See if we actually got the new node
	if (new_node == NULL)
		return NULL;

	new_node->data = NULL;
	new_node->next = NULL;

	return new_node;
}
void queue_delete_node(QueueNode *node) {
    if (node != NULL)
        free(node);
}

void queue_clear(Queue *queue) {
    QueueNode *ptr;
    while ((ptr = queue_dequeue(queue)) != NULL)
        queue_delete_node(ptr);
}

void queue_enqueue(Queue *queue, QueueNode *node) {
    //Tried to insert a nullptr, which may destroy the queue's structure. Just exit without inserting
	if (node == NULL) return;

    //If queue is empty
	if (queue->rear == NULL)
		//Make both front & rear point to this node
		queue->rear = queue->front = node;
	//Non-empty queue
	else {
		//Add a new node at the end of the queue, which is pointed by rear
		queue->rear->next = node;
		//Make rear now point to the new last node
		queue->rear = node;
	}
}

QueueNode *queue_dequeue(Queue *queue) {
    //If the queue is empty, no nodes to remove, so return NULL
	if (queue->front == NULL) return NULL;

	//Non-empty queue path

	//Get the front node element
	QueueNode *front_node = queue->front;

	//Front->next may be null if this is the sole node in the queue
	//in that case, front will be null and queue would be considered empty
	queue->front = queue->front->next;

	//Check if it is NULL, then we can also set rear to null instead of pointing
	//to the now-dequeued node
	if (queue->front==NULL)
		queue->rear = NULL;

	//Return the node
	return front_node;
}