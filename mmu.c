#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#define ERROR_CODE 4096 // works since frames < 128, pages < 256


typedef struct tlb_entry{
	unsigned int page_number;
	unsigned int frame_assigned;
	int valid;
	int last_accessed;
} TLBE;

TLBE* tlbe_init(){
	TLBE* al = malloc(sizeof(TLBE));
	al -> page_number = ERROR_CODE;
	al -> frame_assigned = ERROR_CODE;
	al -> valid = 0;
	al -> last_accessed = 0;
	return al;
}

typedef struct pt_entry{
	unsigned int frame_assigned;
	int present_bit;
	int last_accessed;
	int valid;
} PTE;

PTE* pte_init(){
	PTE* al = malloc(sizeof(PTE));
	al -> frame_assigned = ERROR_CODE;
	// present bit == 1 cuando NO esta en disco, else 0
	al -> present_bit = 0;
	al -> last_accessed = 0;
	// valid bit == 1 cuando ya se le asigno una pagina alguna vez por lo menos
	// es decir, ya se solicito una direccion de memoria virtual perteneciente a esa pagina
	al -> valid = 0;
	return al;
}

typedef struct physical_memory_word{
	unsigned char info[256];
	int being_used;
	int last_accessed;
	unsigned int used_by;
} MEMORYWORD;

MEMORYWORD* memword_init(){
	MEMORYWORD* al = malloc(sizeof(MEMORYWORD));
	// al -> info = malloc(256);
	al -> being_used = 0;
	al -> last_accessed = 0;
	al -> used_by = 0;
	return al;
}

unsigned int lookup_tlbe(TLBE** tlb, unsigned int page_requested, int current_time){
	// searches for page_number on tlbentries. If succesful, returns frame associated,
	// and if unsuccesful, returns Error code
	for (int index = 0; index < 32; ++index)
	{
		if (tlb[index]->valid && tlb[index]->page_number == page_requested){
			tlb[index]->last_accessed = current_time;
			return tlb[index]->frame_assigned;
		}
	}
	return ERROR_CODE;
}

void request_and_set_tlbe(TLBE** tlb, unsigned int page, unsigned int frame_assigned, int current_time)
{
	// Searches for an empty space on tlb,
	// if no empty space, then it searches for LRU tlbe, and sets the new tlbe
	unsigned int direction;
	int index;
	int empty = 0;
	for (index = 0; index < 32; ++index)
	{
		if (!tlb[index]->valid || tlb[index]->frame_assigned==ERROR_CODE)
		{
			empty = 1;
			direction = index;
			break;
		}
	}
	if (empty)
	{
		tlb[direction]->page_number = page;
		tlb[direction]->frame_assigned = frame_assigned;
		tlb[direction]->valid = 1;
		tlb[direction]->last_accessed = current_time;
	}
	else
	{
		int min = current_time + 10;
		for (index = 0; index < 32; ++index)
		{
			if (tlb[index]->last_accessed < min)
			{
				min = tlb[index]->last_accessed;
				direction = index;
			}
		}
		tlb[direction]->page_number = page;
		tlb[direction]->frame_assigned = frame_assigned;
		tlb[direction]->valid = 1;
		tlb[direction]->last_accessed = current_time;
	}
}

unsigned int request_frame(MEMORYWORD** physical_memory, PTE** page_table, unsigned int page_requesting, int current_time)
{
	// Searches for an empty space on physical memory and returns the direction
	// if no empty space, then it searches for LRU frame, and returns the direction
	unsigned int direction;
	int index;
	int empty = 0;
	for (index = 0; index < 128; ++index)
	{
		if (!physical_memory[index]->being_used)
		{
			empty = 1;
			direction = index;
			break;
		}
	}
	if (empty)
	{
		physical_memory[direction]->being_used = 1;
		physical_memory[direction]->last_accessed = current_time;
		physical_memory[direction]->used_by = page_requesting;
		return direction;
	}
	else
	{
		int min = current_time + 10;
		for (index = 0; index < 128; ++index)
		{
			if (physical_memory[index]->last_accessed < min)
			{
				min = physical_memory[index]->last_accessed;
				direction = index;
			}
		}
		// UPDATE PTE OF PREVIOUS PAGE
		// OJO aqui denuevo no toi seguro onda cn el swap out y swap in, pero como son puras
		// lecturas en teoria no hay q preocuparse de swappiar out
		// dejamos el frame_assigned igual y solo cambiamos el presence bit
		page_table[physical_memory[direction]->used_by]->present_bit = 0;


		physical_memory[direction]->being_used = 1;
		physical_memory[direction]->last_accessed = current_time;
		physical_memory[direction]->used_by = page_requesting;
		return direction;
	}
}


/// NOTA GENERAL

// Igual revisa el flow de lo que va pasando pq fijo se me paso algun detalle, o alguna actualizacion de tlbe o pte cuando
// se hace un reemplazo por ejemplo, onda se me podria haber pasado algun valid o presence bit o used by esas weas..
// lo he doublechecked caleta pero uno nunca sabe..



/// END NOTA GENERAL
int tlb_hits_counter = 0;
int page_faults_counter = 0;
int current_time = 0;
FILE *data_bin;


void handler(int foo){
	fclose(data_bin);
	printf("\nHit Rate: %f %%  Page-Fault: %f %%\n", ((float)(tlb_hits_counter*100))/((float)current_time), ((float)(page_faults_counter*100))/((float)current_time));
    exit(0);
}

int main(int argc, char** argv)
{
	signal(SIGINT, handler);

	data_bin = fopen("data.bin", "rb");

	TLBE** tlb = malloc(sizeof(TLBE*)*32);
	for (int tlbe = 0; tlbe < 32; ++tlbe)
	{
		tlb[tlbe] = tlbe_init();
	}

	PTE** page_table = malloc(sizeof(PTE*)*256);
	for (int pgte = 0; pgte < 256; ++pgte)
	{
		page_table[pgte] = pte_init();
	}

	MEMORYWORD** physical_memory = malloc(sizeof(MEMORYWORD*)*128);
	for (int memdir = 0; memdir < 128; ++memdir)
	{
		physical_memory[memdir] = memword_init();
	}

	unsigned int vir_mem_requested, offset, page;

	unsigned int frame_to_lookup;

	unsigned int value;

	printf("%lu \n",sizeof(vir_mem_requested));
	while(1){
		// printf("Enter Virtual Direction\n");
		int code = scanf("%u", &vir_mem_requested);
		if (code == EOF) break;
		offset = vir_mem_requested & 0x00FF;
		page = (vir_mem_requested & 0xFF00) >> 8;

		frame_to_lookup = lookup_tlbe(tlb, page, current_time);
		printf("VirMem: %d, Page:%d, Offset:%d\n", vir_mem_requested, page, offset);

		if (frame_to_lookup != ERROR_CODE)
		{
			// HIT! frame_to_lookup will have corresponding frame
			tlb_hits_counter++;
			printf("  Hit!\n");
			value = physical_memory[frame_to_lookup] -> info[offset];
		}
		else
		{
			// MISS! every outcome here must call request_and_set_tlbe
			if (page_table[page]->valid && page_table[page]->frame_assigned != ERROR_CODE)
			{
				// frame was already assigned, so copy it to tlb and then return
				if (page_table[page]->present_bit)
				{
					// it is NOT on disk
					frame_to_lookup = page_table[page]->frame_assigned;
					request_and_set_tlbe(tlb, page, page_table[page]->frame_assigned, current_time);
					printf("  Miss!\n");
					value = physical_memory[frame_to_lookup] -> info[offset];
				}
				else
				{
					// ESTO YA NO FALTA (1)

					// frame was asigned, but it WAS on DISK
					// aqui no cacho bien la parte del data.bin pero creo q habria que leerlo y
					// escribirlo en la parte info del mem_word del frame correspondiente?

					// END ESTO YA NO FALTA (1)

					// PAGE FAULT
					page_faults_counter++;
					printf("  Page Fault (GOTO DISK)\n");

					// search for a frame to assign to this page
					frame_to_lookup = request_frame(physical_memory, page_table, page, current_time);

					//
					// look for frame in databin and update value of physical memory frame
					unsigned char buffer[256];
					fseek(data_bin, page * 256, SEEK_SET);
					fread(buffer, sizeof(buffer), 1, data_bin);
					for (int i = 0; i < 256; ++i) physical_memory[frame_to_lookup] -> info[i] = buffer[i];
					value = physical_memory[frame_to_lookup] -> info[offset];

					// update PTE
					page_table[page]->frame_assigned = frame_to_lookup;
					page_table[page]->present_bit = 1;
					page_table[page]->valid = 1;
					page_table[page]->last_accessed = current_time;
					request_and_set_tlbe(tlb, page, page_table[page]->frame_assigned, current_time);

				}
			}
			else
			{
				// PAGE FAULT SIP
				page_faults_counter++;
				printf("  Page Fault (LOAD)\n");

				// search for a frame to assign to this page
				frame_to_lookup = request_frame(physical_memory, page_table, page, current_time);

				// look for frame in databin and update value of physical memory frame
				unsigned char buffer[256];
				fseek(data_bin, page * 256, SEEK_SET);
				fread(buffer, sizeof(buffer), 1, data_bin);
				for (int i = 0; i < 256; ++i) physical_memory[frame_to_lookup] -> info[i] = buffer[i];
				//printf("%u\n", physical_memory[frame_to_lookup] -> info[offset]);
				value = physical_memory[frame_to_lookup] -> info[offset];

				// update PTE
				page_table[page]->frame_assigned = frame_to_lookup;
				page_table[page]->present_bit = 1;
				page_table[page]->valid = 1;
				page_table[page]->last_accessed = current_time;
				request_and_set_tlbe(tlb, page, page_table[page]->frame_assigned, current_time);
			}
		}


		// FOLLOWED STRUCTURE: aqui ta cmo 'pseudopseudopseudo'- codigo de que es lo que hice ahi arriba

		// look in TLB
			// If successful, return frame number
			// else go to pagetable
				// if page is assigned
					// if page is NOT on disk (presence bit 1), assign on TLB (replacing TLBE with LRU if necessary) and return frame
					// else (page fault) check if theres space on physical memory
						// if theres space choose frame, assign on PTE and on TLB (replacing TLBE with LRU if necessary) and return frame
						// else (swap needed), choose with LRU which frame to liberate, replace it (set corresponding PTE to that frame, set old TLBE (if existant) to invalid, and assign the new frame on TLB ) and return frame
				// else (page hasnt been assigned yet)
					// if theres space choose frame, assign on PTE and on TLB (replacing TLBE with LRU if necessary) and return frame
					// else (swap needed), choose with LRU which frame to liberate, replace it (set corresponding PTE to that frame, set old TLBE (if existant) to invalid, and assign the new frame on TLB ) and return frame

		// END FOLLOWED STRUCTURE

		printf("  Frame:%d\n", frame_to_lookup);
		printf("  Value:%u\n", value);

		// printf("%u\n", physical_memory[frame_to_lookup] -> info);



		// ESTO YA NO FALTA, POR COMPLETO. :D (2)


		// Once you have the frame, look in corresponding frame (and offset?) aqui depende de como sea la interaccion cn el data.bin
		// pero recordar que frame_to_lookup es solo el frame number, habria q shiftiarlo ocho veces pa la izquierda y  unirlo con
		// los bits de offset. se puede hacer con los operadores bitwise de C asi como en el principio
		// nose como hacerlo con el data.bin pero algo asi como fseek("data.bin", frame_to_look_up) ??

		// END ESTO YA NO FALTA, POR COMPLETO :D (2)


		// ESTO TAMBIEN YA NO FALTA. :D (3)

		// falta la parte de imprimir el valor del byte correspondiente, (Creo que esto seria lo de reconstruir la direccion fisica final),
		// es decir lo de juntar la pagina transformada a marco + offset.

		// tambien falta arreglar este while True.. no me quedo claro del enunciado si el input era una pura secuencia onda "2 3 15 18..."
		// o si iban entregandolos uno a uno.. por ahora esta con un while True tonces el problema es que el programa nunca termina, y
		// nunca se printean los stats.. alomejor podria dejarse asi nomas y ponerle una SIGNAL ctrl-C como la de la tarea 1 pa salir del while
		// y imprimir las stats

		// END ESTO TAMBIEN YA NO FALTA. :D (3)

		current_time ++;
	}

	printf("Hit Rate: %f %%  Page-Fault: %f %%\n", ((float)(tlb_hits_counter*100))/((float)current_time), ((float)(page_faults_counter*100))/((float)current_time));


	for (int tlbe = 0; tlbe < 32; ++tlbe)
	{
		free(tlb[tlbe]);
	}
	free(tlb);

	for (int pgte = 0; pgte < 256; ++pgte)
	{
		free(page_table[pgte]);
	}
	free(page_table);

	for (int memdir = 0; memdir < 128; ++memdir)
	{
		// free(physical_memory[memdir]->info);
		free(physical_memory[memdir]);
	}
	free(physical_memory);
	fclose(data_bin);
}