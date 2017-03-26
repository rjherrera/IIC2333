#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// PROCESS

typedef struct process{
	char name[257];
	int pid;
	int priority;
	int arrival;
	int event_length;
	int* events;
}Process;

Process* new_process(int pid, int priority, int arrival, int event_length, int* events, char* name){
	Process* al = malloc(sizeof(Process));
	al -> pid = pid;
	al -> priority = priority;
	al -> arrival = arrival;
	al -> event_length = event_length;
	al -> events = events;
	strcpy(al -> name, name);
	return al;
}

void free_process(Process* process){
	free(process -> events);
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

void free_linked_nodes(Node* node){
	if (node -> next == NULL){
		// free_process(node -> process);
		// printf("Liberado el node c/ process %i\n",node->process->pid);
		free_process(node -> process);
		free(node);
	}
	else {
		free_linked_nodes(node -> next);
		node -> next = NULL;
		free_linked_nodes(node);
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
	int length;
}Queue;

Queue* queue_init(){
	Queue* al = malloc(sizeof(Queue));
	al -> head = NULL;
	al -> rear = NULL;
	al -> length = 0;
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
	queue -> length += 1;
	printf("Proceso %i ha entrado a la cola\n", process -> pid);
}

Process* Dequeue(Queue* queue){
	Process* dequeued_process = queue -> head -> process;
	if (queue -> head == queue -> rear){
		queue -> head = NULL;
		queue -> rear = NULL;
	}
	else {
		Node* n = queue -> head;
		queue -> head = queue -> head -> next;
		free(n);
	}
	queue -> length -= 1;
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
		printf("Queue status:\n");
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
	if ((argc != 3) && (argc != 4)){
		printf("Modo de uso: %s <scheduler> <file> <quantum>\n", argv[0]);
		printf("\t<scheduler> es puede ser 'fcfs', 'roundrobin' o 'random'\n");
		printf("\t<file> es el nombre del archivo con el detalle de los procesos\n");
		printf("\t<quantum> es opcional, es 3 por defecto y corresponde al quantum con scheduler roundrobin\n");
		return 1;
	}
	else {
		// Set variables
		int quantum;
		if (argc == 4){
			sscanf(argv[3], "%i", &quantum);
		}
		else {
			quantum = 3;
		}

		// Process array
		// por ahora en una queue pero dps en otra edd yo creo
		Queue* processed_read = queue_init();
		// Load processes
		int current_pid = 0;
		FILE* fr = fopen(argv[2], "r");
		char* name = malloc(sizeof(char)*257);
		int priority, arrival, event_length;
		while (fscanf(fr, "%s %i %i %i", name, &priority, &arrival, &event_length) == 4){
			int* events = malloc(sizeof(int)*(event_length*2-1));
			for(int i=0; i<(2*event_length-1); i++){
				fscanf(fr, "%i", &events[i]);
			}
			Process* process = new_process(current_pid, priority, arrival, event_length, events, name);
			Enqueue(process, processed_read);
			current_pid += 1;
		}
		free(name);
		// if (feof(fr)){
		// 	printf("File ended\n");
		// }
		// else {
		// 	printf("Some error \n");
		// }
		// printf("%s %i %i %i\n", name, priority, arrival, event_length);
		// Check Expected Scheduler
		if (strcmp(argv[1],"fcfs") == 0){
			Queue* queue = queue_init();

		} else if (strcmp(argv[1],"roundrobin") == 0){

		} else if (strcmp(argv[1],"random") == 0){

		}
		else {
			printf("\t<scheduler> debe ser 'fcfs', 'roundrobin' o 'random'\n");
			return 1;
		}

	}
}
