#include <stdio.h>
#include <stdlib.h>

// PROCESS

typedef struct process{
	int pid;
	int priority;
	int exec_time;
	int event_length;
	int* events;
	char* name;
}Process;

Process* new_process(int pid, int priority, int exec_time, int event_length, int* events, char* name){
	Process* al = malloc(sizeof(Process));
	al -> pid = pid;
	al -> priority = priority;
	al -> exec_time = exec_time;
	al -> event_length = event_length;
	al -> events = events;
	al -> name = name;
	return al;
}

void free_process(Process* process){
	// free(process -> events);
	// free(process -> name);
	free(process);
}

// NODE

typedef struct node{
	Process* process;
	struct node* next;
}Node;

Node* node_init(Process* process){
	Node* al = malloc(sizeof(Node));
	al -> process = process;
	al -> next = NULL;
	return al;
}

// int isLast(Node* node){
// 	if (node -> next == NULL){
// 		return 1;
// 	}
// 	else {
// 		return 0;
// 	}
// }

void free_linked_nodes(Node* node){
	if (node -> next == NULL){
		// free_process(node -> process);
		free_process(node -> process);
		free(node);
	}
	else {
		free_linked_nodes(node -> next);
	}
}

void set_next_node(Node* node, Node* new_node){
	node -> next = new_node;
}

Node* get_next(Node* node){
	return node -> next;
}

// END NODE

// QUEUE

typedef struct queue{
	Node* head;
	Node* rear;
}Queue;

Queue* queue_init(){
	Queue* al = malloc(sizeof(Queue));
	al -> head = NULL;
	al -> rear = NULL;
	return al;
}

int isQEmpty(Queue* queue){
	if((queue -> head == NULL) && (queue -> rear == NULL)){
		return 1;
	}
	else{
		return 0;
	}
}

void free_queue(Queue* queue){
	if (isQEmpty(queue) == 0){
		free_linked_nodes(queue -> head);
	}
	free(queue);
}

void Enqueue(Process* process, Queue* queue){
	Node* new_node = node_init(process);
	if (isQEmpty(queue)){
		queue -> head = new_node;
		queue -> rear = new_node;
	}
	else{
		set_next_node(queue -> rear, new_node);
		queue -> rear = new_node;
	}
	printf("Proceso %i ha entrado a la cola\n", process -> pid);
}

Process* Dequeue(Queue* queue){
	Process* dequeued_process = queue -> head -> process;
	if (queue -> head == queue -> rear){
		queue -> head = NULL;
		queue -> rear = NULL;
	}
	else {
		queue -> head = queue -> head -> next;
	}
	printf("Proceso %i ha salido de la cola\n", dequeued_process -> pid);
	return dequeued_process;
}

Node* first_in_queue(Queue* queue){
	return queue -> head;
}

void see_queue(Queue* queue){
	if (isQEmpty(queue)){
		printf("Queue empty\n");
	}
	else {
		Node* n = first_in_queue(queue);
		while (n != NULL){
			printf("%i  ",n -> process -> pid);
			n = get_next(n);
		}
		printf("\n");
	}
}

// END QUEUE

int main(int argc, char *argv[]){
	int a;
	char b;
	Process* p1 = new_process(1,1,1,1, &a, &b);
	Process* p2 = new_process(2,2,2,2, &a, &b);
	Process* p3 = new_process(3,3,3,3, &a, &b);
	Queue* queue = queue_init();
	see_queue(queue);
	Enqueue(p1, queue);
	//printf("%i\n", queue -> head -> process -> pid);
	see_queue(queue);
	Enqueue(p3, queue);
	see_queue(queue);
	Enqueue(p2, queue);
	see_queue(queue);
	Process* sefue = Dequeue(queue);
	printf("SALIO! %i\n", sefue->pid);
	see_queue(queue);
	Enqueue(p1, queue);
	see_queue(queue);
	Process* sefue2 = Dequeue(queue);
	printf("SALIO! %i\n", sefue2->pid);
	see_queue(queue);
	Enqueue(p3, queue);
	see_queue(queue);

	free_queue(queue);
	
}
