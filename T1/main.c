#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#define FIXED_SIZE 20
#define SIM_TIME 1E3

// PROCESS

typedef struct process{
	char name[257];
	char status[9];
	int pid;
	int priority;
	int arrival;
	int event_length;
	int counter;
	int selected;
	int blocked;
	int added_waiting;
	int first_exec;
	int current_exec;
	int finished;
	int* events;
}Process;

Process* new_process(int pid, int priority, int arrival, int event_length, int* events, char* name){
	Process* al = malloc(sizeof(Process));
	al -> pid = pid;
	al -> priority = priority;
	al -> arrival = arrival;
	al -> event_length = event_length;
	al -> events = events;
	al -> counter = 0;
	al -> selected = 0;
	al -> blocked = 0;
	al -> added_waiting = 0;
	al -> current_exec = 0;
	al -> finished = -1;
	al -> first_exec = -1;
	strcpy(al -> name, name);
	strcpy(al -> status, "UNBORN");
	return al;
}

int get_diserved_quantum(Process* process, int quantum){
	// !!! IMPORTANTE !!!
	// hay que revisar aca pq los floats /ints se comportan medio raro
	// !!! END IMPORTANTE !!!
	float x = (float) process -> priority;
	float y;
	if (x <= 32){
		y = (x/62) + (15/31);
	}
	else {
		y = x/32;
	}
	// printf(" y = %f DESERVED_QUANTUM %i\n", y, (int)(y*(float)quantum));
	return (int)(y*(float)quantum);
}

void print_statistics(Process* process){
	printf("Proceso %i : %s\n", process -> pid, process -> name);
	printf("\tElegido para usar la CPU: %i veces\n", process -> selected);
	printf("\tBloqueado: %i veces\n", process -> blocked);
	if (process -> finished == -1){
		printf("\tTurnaround time: --Proceso no terminado--\n");
	}
	else {
		printf("\tTurnaround time: %i\n", process -> finished - process -> arrival);
	}
	if (process -> first_exec == -1){
		printf("\tResponse time: --Proceso no alcanzó a ejecutarse--\n");
	}
	else {
		printf("\tResponse time: %i\n", process -> first_exec - process -> arrival);
	}
	printf("\tWaiting time: %i\n", process -> added_waiting);
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
	// libera los nodos, destruyendo tambien los procesos
	if (node -> next == NULL){
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
	if (!isQEmpty(queue)){
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
	// printf("Proceso %i ha entrado a la cola\n", process -> pid);
}

Process* Dequeue(Queue* queue){
	Process* dequeued_process = queue -> head -> process;
	if (queue -> head == queue -> rear){
		free(queue -> head);
		queue -> head = NULL;
		queue -> rear = NULL;
	}
	else {
		Node* n = queue -> head;
		queue -> head = queue -> head -> next;
		free(n);
	}
	queue -> length -= 1;
	return dequeued_process;
}

Process* Dequeue_index(Queue* queue, int index){
	if (index == 0){
		return Dequeue(queue);
	}
	Node* parent = NULL;
	Node* selected_node = queue -> head;
	Node* child = selected_node -> next;
	Process* dequeued_process;
	int i = 0;
	while(i < index){
		parent = selected_node;
		selected_node = child;
		child =  child -> next;
		i++;
	}
	if (!child){
		parent -> next = NULL;
		queue -> rear = parent;
	}
	else{
		parent -> next = selected_node -> next;
	}
	dequeued_process = selected_node -> process;
	selected_node -> next = NULL;
	free(selected_node);
	queue -> length -= 1;
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

// GlOBALES
    // Defini variables globales para poder acceder a ellas desde el handler
int total_processes;
Process** processes_read;
// END GLOABLES


// SIGNAL HANDLER
void handler(int foo){
    printf("Se interrumpió el scheduler.\n");
    for (int i=0;i<total_processes;i++){
        print_statistics(processes_read[i]);
        free_process(processes_read[i]);
    }
    exit(0);
}
// END SIGNAL HANDLER

int main(int argc, char *argv[]){
	signal(SIGINT, handler);
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
		int current_pid = 0;
		int current_size = FIXED_SIZE;
		processes_read = malloc(sizeof(Process*)*current_size);
		// Load processes
		FILE* fr = fopen(argv[2], "r");
		char* name = malloc(sizeof(char)*257);
		int priority, arrival, event_length;
		while (fscanf(fr, "%s %i %i %i", name, &priority, &arrival, &event_length) == 4){
			int* events = malloc(sizeof(int)*(event_length*2-1));
			for(int i=0; i<(2*event_length-1); i++){
				fscanf(fr, "%i", &events[i]);
			}

			// si se supera limite max, copiamos array en uno de doble tamano
			if (current_pid == current_size){
				int new_size = 2*current_size;
				Process** new_processes_read = malloc(sizeof(Process*)*new_size);
				for(int i=0;i<current_size;i++){
					new_processes_read[i] = processes_read[i];
				}
				free(processes_read);
				processes_read = new_processes_read;
				current_size = new_size;
			}

			Process* process = new_process(current_pid, priority, arrival, event_length*2-1, events, name);
			processes_read[current_pid] = process;
			current_pid += 1;
		}
		free(name);



		total_processes = current_pid;

		// for(int j=0;j<total_processes;j++){
		// 	printf("AGH :%i >", processes_read[j]->priority);
		// 	get_diserved_quantum(processes_read[j], quantum);
		// }
		// int nada;
		// scanf("%i",&nada);


		int current_time = 0;
		int done = 0;
		int finished_processes;
		Queue* ready_queue = queue_init();
		Process* is_running = NULL;
		while(!done){
			// sleep para que el ctrl+c se pueda usar
			usleep(SIM_TIME);
			// revisamos si llega un proceso
			for(int i=0;i<total_processes;i++){
				if (current_time == processes_read[i] -> arrival){
					Enqueue(processes_read[i], ready_queue);
					printf("Proceso %i ha sido creado y ha entrado a la cola ready con estado READY  | t = %i\n", processes_read[i] -> pid, current_time);
				}
			}
			// revisamos si algun proceso dejo de esperar
			for(int i=0;i<total_processes;i++){
				if(strcmp(processes_read[i] -> status, "WAITING") == 0){
					// los procesos que estan waiting
					if (processes_read[i] -> events[processes_read[i] -> counter] == 0){
						processes_read[i] -> counter += 1;
						Enqueue(processes_read[i], ready_queue);
						strcpy(processes_read[i] -> status, "READY");
						printf("Proceso %i ha dejado de esperar y ha entrado a la cola ready (WAITING -> READY)| t = %i\n", processes_read[i] -> pid, current_time);
					}
				}
			}

			// revisamos si proceso que esta ejecutando termino de ejecutar
			if (is_running != NULL){
				// hay alguien ejecutando
				if (is_running -> events[is_running -> counter] == 0){
					// termino de ejecutar su burst
					if (is_running -> counter == ((is_running -> event_length)-1)){
						// termino ultima ejecucion
						is_running -> finished = current_time;
						strcpy(is_running -> status, "FINISHED");
						printf("Proceso %i ha finalizado su ejecucion y se ha destruido | t = %i\n", is_running -> pid, current_time);
						is_running = NULL;
					}
					else{
						is_running -> blocked += 1;
						is_running -> counter += 1;
						is_running -> current_exec = 0;
						strcpy(is_running -> status, "WAITING");
						printf("Proceso %i se ha bloqueado (RUNNING -> WAITING) | t = %i\n", is_running -> pid, current_time);
						is_running = NULL;
					}
				} else if (strcmp(argv[1],"roundrobin") == 0){
					// revisamos si hay que quitar al proceso de la cpu en rr
					if (is_running -> current_exec == get_diserved_quantum(is_running, quantum)){
						is_running -> current_exec = 0;
						// !!! IMPORTANTE !!!
						// aca hay que confirmar que cuando el scheduler se apropia de la cpu no es considerado un "bloqueo"
						// is_running -> blocked += 1;
						// !!! END IMPORTANTE !!!
						strcpy(is_running -> status, "READY");
						Enqueue(is_running, ready_queue);
						printf("Proceso %i ha agotado el quantum y vuelve a cola ready (RUNNING -> READY) | t = %i\n", is_running -> pid, current_time);
						is_running = NULL;
					}
				}
			}

			// revisamos si hay que asignar nuevo proceso a CPU
			if (is_running == NULL){
				// no hay ningun proceso corriendo
				if(isQEmpty(ready_queue)){
					// no hay procesos esperando la cpu
				}
				else{
					// hay procesos en cola ready
					// seleccionamos segun tipo de scheduler
					if (strcmp(argv[1],"fcfs") == 0 || strcmp(argv[1],"roundrobin") == 0){
						is_running = Dequeue(ready_queue);
					} else if (strcmp(argv[1],"random") == 0){
						int index_selected = rand() % ready_queue -> length;
						is_running = Dequeue_index(ready_queue, index_selected);
					}
					else {
						printf("\t<scheduler> debe ser 'fcfs', 'roundrobin' o 'random'\n");
						return 1;
					}
					is_running -> selected += 1;
					if (is_running -> first_exec == -1){
						is_running -> first_exec = current_time;
					}
					strcpy(is_running -> status, "RUNNING");
					printf("Scheduler ha asignado la CPU al proceso %i (READY -> RUNNING)| t = %i\n", is_running -> pid, current_time);
					int total_bursts = (is_running -> event_length + 1)/2;
					int completed_bursts = is_running -> counter/2;
					printf(
						"El proceso ya ha ejecutado %i intervalos de tiempo completos, y le faltan %i incluyendo el que acaba de comenzar\n",
						completed_bursts, total_bursts - completed_bursts);
				}
			}

			// hacemos avanzar el tiempo
			for(int i=0;i<total_processes;i++){
				if (strcmp(processes_read[i] -> status, "WAITING") == 0){
					// !!! IMPORTANTE !!!
					// aca hay que confirmar si waiting time incluye el tiempo en estado WAITING
					processes_read[i] -> added_waiting += 1;
					/// !!! END IMPORTANTE !!!
					processes_read[i] -> events[processes_read[i] -> counter] -= 1;
				} else if (strcmp(processes_read[i] -> status, "READY") == 0){
					processes_read[i] -> added_waiting += 1;
				} else if (strcmp(processes_read[i] -> status, "RUNNING") == 0){
					processes_read[i] -> events[processes_read[i] -> counter] -= 1;
					if (strcmp(argv[1],"random") == 0){
						processes_read[i] -> current_exec += 1;
					}
				}
			}

			//chequiamos si ya se destruyeron todos los procesos
			finished_processes = 0;
			done = 1;
			for(int i=0;i<total_processes;i++){
				if (!(strcmp(processes_read[i] -> status, "FINISHED") == 0)){
					done = 0;
				}
				else {
					finished_processes++;
				}
			}
			// avanzamos el tiempo
			current_time += 1;
		}

		printf("Se ha terminado la simulación, con %i procesos terminados y tiempo %i\n", finished_processes, current_time-1);
		printf("A continuación, las estadísticas:\n");
		for (int i=0;i<total_processes;i++){
			print_statistics(processes_read[i]);
			free_process(processes_read[i]);
		}
		free(processes_read);
		free_queue(ready_queue);
	}
}
