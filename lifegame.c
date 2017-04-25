#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// #include <sys/types.h>
// #include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <semaphore.h>

sem_t* sem;

void set_flattened_grid(int* grid, int height, int width, int row, int col, int frame, int value){
	grid[height*width*frame + width*row + col] = value;
}

int get_flattened_grid(int* grid, int height, int width, int row, int col, int frame){
	return grid[height*width*frame + width*row + col];
}

int count_alive_neighbours(int* grid, int height, int width, int row, int col, int frame){
	int count = 0;

	if (row -1 < 0);
	else {
		if (get_flattened_grid(grid, height, width, row-1, col, frame)==1)count++;
	}
	if (row == height-1);
	else {
		if (get_flattened_grid(grid, height, width, row+1, col, frame)==1)count++;
	}
	if (col-1<0);
	else {
		if (get_flattened_grid(grid, height, width, row, col-1, frame)==1)count++;
	}
	if (col==width-1);
	else {
		if (get_flattened_grid(grid, height, width, row, col+1, frame)==1)count++;
	}
	if (row -1 < 0 || col -1 < 0);
	else {
		if (get_flattened_grid(grid, height, width, row-1, col-1, frame)==1)count++;
	}
	if (row -1 < 0 || col == width-1);
	else {
		if (get_flattened_grid(grid, height, width, row-1, col+1, frame)==1)count++;
	}
	if (row == height-1 || col -1 < 0);
	else {
		if (get_flattened_grid(grid, height, width, row+1, col-1, frame)==1)count++;
	}
	if (row == height-1 || col == width-1);
	else {
		if (get_flattened_grid(grid, height, width, row+1, col+1, frame)==1)count++;
	}
	return count;
}

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		printf("Modo de uso: %s <input.txt> <output.txt>\n", argv[0]);
		printf("\t input.txt : Nombre de archivo con definicion del problema.\n");
		printf("\t output.txt : Nombre de archivo donde se escribira la respuesta.\n");
		return 1;
	}

	FILE* fr = fopen(argv[1], "r");
	int iter, height, width, living_init;
	fscanf(fr, "%i %i %i %i", &iter, &height, &width, &living_init);

	// ASIGN AND ATTACH SHARED GRID
	key_t key;
	int shmid;
	int* flattened_grid;
	int memory_needed = width*height*2*sizeof(int);
	key = 100;
	shmid = shmget(key, memory_needed, 0644 | IPC_CREAT);
	flattened_grid = (int*) shmat(shmid, 0, 0);
	if (flattened_grid == (int *)(-1)) perror("shmat");

	// ASIGN AND ATTACH current_iter
	key_t key2;
	int shmid2;
	int* current_iter;
	key2 = 1000;
	shmid2 = shmget(key2, sizeof(int), 0644 | IPC_CREAT);
	current_iter = (int*) shmat(shmid2, 0, 0);
	*current_iter = 0;
	if (current_iter == (int *)(-1)) perror("shmat");

	// ASIGN AND ATTACH global_advance
	key_t key4;
	int shmid4;
	int* global_advance;
	key4 = 1500;
	shmid4 = shmget(key4, sizeof(int), 0644 | IPC_CREAT);
	global_advance = (int*) shmat(shmid4, 0, 0);
	*global_advance = 0;
	if (global_advance == (int *)(-1)) perror("shmat");

	// SET LIVING CELLS
	int i,j;
	for (int live_cell=0;live_cell<living_init;live_cell++){
		fscanf(fr, "%i %i", &i, &j);
		set_flattened_grid(flattened_grid, height, width, i, j, 0, 1);
	}

	fclose(fr);

	int cpus = sysconf(_SC_NPROCESSORS_ONLN);
	// if theres less rows & cols than cpus, use less cpus
	// to give one for each row/col
	while (height<cpus && width<cpus){
		cpus--;
	}

	// ASIGN AND ATTACH each_process_ready (array where bit signifies if process is ready )
	key_t key3;
	int shmid3;
	int* each_process_ready;
	key3 = 2000;
	shmid3 = shmget(key3, sizeof(int)*cpus, 0644 | IPC_CREAT);
	each_process_ready = (int*) shmat(shmid3, 0, 0);
	for (int i=0;i<cpus;i++){
		each_process_ready[i] = 1;
	}
	if (each_process_ready == (int *)(-1)) perror("shmat");

	int col_per_cpu = width/cpus; // amount of cols per cpu
	int row_per_cpu = height/cpus; // amount of rows per cpu

	int divide_horizontally = 1;
	int search_width = row_per_cpu;

	// Divide grid vertically or horizontally depending on which one
	// has the most number of rows & cols respectively
	if (row_per_cpu < col_per_cpu){
		search_width = col_per_cpu;
		divide_horizontally = 0;
	}
	// if amount of rows/col divid
	// if (search_width == 0){
	// 	search_width = 1;
	// }

	// ASSIGN AND ATTACH SEMAPHORE
	sem = sem_open("/semaf", O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 1);

	int parent_id = getpid();
	int index_assigned;
	int identity;
	pid_t pid;
    for(i=0;i<cpus;i++)
    {
        pid = fork();
        if(pid == -1) 
        {
            perror("Err");
            exit(EXIT_FAILURE);
        }
        if(pid == 0){
        	identity = i;
        	index_assigned = i*search_width;
        	break;
        }
    }

    if(getpid()!=parent_id){
	    sem = sem_open("/semaf", 0, 0644, 0);

    	if (identity == cpus-1){
    		if (divide_horizontally){
    			search_width = height - index_assigned;
    		}
    		else {
    			search_width = width - index_assigned;
    		}
    	}
		// printf("PROCESS (%d) with parent (%d): will check rows [%i-%i]\n", getpid(), getppid(), index_assigned, index_assigned+search_width);
    }

    int personal_iter = 0;
    while(*current_iter < iter || getpid()==parent_id){

    	if (getpid() == parent_id){
    		int finished_current_check = 0;
    		while (personal_iter == *current_iter && !finished_current_check){
	    		sem_wait(sem);
	    		int all_done = 1;
	    		for (int i=0;i<cpus;i++){
	    			if (each_process_ready[i] == 1) all_done = 0;
	    		}
	    		if (all_done){
	    			// printf("PARENT process %i was one who noticed every1 ready at the end of iter %i == %i \n", getpid(), *current_iter, personal_iter);
	    			*current_iter += 1;
	    			*global_advance = 0;
		    		for (int i=0;i<cpus;i++){
		    			each_process_ready[i] = 1;
		    		}
		    		finished_current_check = 1;
				   //  printf("PRINT MATRIX end of iter %i\n", personal_iter);
				   //  for(int row=0;row<height;row++){
				   //  	for (int col = 0; col < width; ++col)
				   //  	{
							// printf("%i " , get_flattened_grid(flattened_grid, height, width, row, col, (personal_iter+1)%2));
				   //  	}
				   //  	printf("\n");
				   //  }
				   //  printf("END MATRIX\n");		    		
	    		}
	    		sem_post(sem);
    		}
    		if (finished_current_check) personal_iter++;
    		if (*current_iter >= iter) break;
    	}
    	else {

    		while(personal_iter>*current_iter);

    		// printf("Process %i waiting to check if can advance\n", getpid());
    		// Once you're on the corresponding iter
    		sem_wait(sem);
    		int advance = 1;
    		for (int i=0;i<cpus;i++){
    			if (each_process_ready[i] == 0) advance = 0;
    		}
    		if (advance){
    			each_process_ready[identity] = 0;
	    		// printf("Process %i was first to adv on iter %i == %i \n", getpid(), *current_iter, personal_iter);
    		}
    		sem_post(sem);
    		while(!advance && !*global_advance);
    		// // process was first or was allowed to advance by first one
    		// if (!advance){
	    	// 	printf("Process %i was allowed to adv on iter %i == %i \n", getpid(), *current_iter, personal_iter);
    		// }

	    	int frame = *current_iter%2;
	    	int next_frame = (*current_iter+1)%2;
			if (divide_horizontally){
		    	for(int offset=0; offset<search_width; offset++){
		    		for (int col=0;col<width;col++){
		    			int alive = get_flattened_grid(flattened_grid, height, width, index_assigned+offset, col, frame);
		    			int living_neighbours = count_alive_neighbours(flattened_grid, height, width, index_assigned+offset, col, frame);
		    			if (alive && living_neighbours<2) set_flattened_grid(flattened_grid, height, width, index_assigned+offset, col, next_frame, 0);
		    			else if (alive && living_neighbours>3) set_flattened_grid(flattened_grid, height, width, index_assigned+offset, col, next_frame, 0);
		    			else if (!alive && living_neighbours==3) set_flattened_grid(flattened_grid, height, width, index_assigned+offset, col, next_frame, 1);
		    			else set_flattened_grid(flattened_grid, height, width, index_assigned+offset, col, next_frame, get_flattened_grid(flattened_grid, height, width, index_assigned+offset, col, frame));
		    		}
		    	}			
			}
			else{
		    	for(int row=0; row<height; row++){
		    		for (int offset=0;offset<search_width;offset++){
		    			int alive = get_flattened_grid(flattened_grid, height, width, row, index_assigned+offset, frame);
		    			int living_neighbours = count_alive_neighbours(flattened_grid, height, width, row, index_assigned+offset, frame);
		    			if (alive && living_neighbours<2) set_flattened_grid(flattened_grid, height, width, row, index_assigned+offset, next_frame, 0);
		    			else if (alive && living_neighbours>3) set_flattened_grid(flattened_grid, height, width, row, index_assigned+offset, next_frame, 0);
		    			else if (!alive && living_neighbours==3) set_flattened_grid(flattened_grid, height, width, row, index_assigned+offset, next_frame, 1);
		    			else set_flattened_grid(flattened_grid, height, width, row, index_assigned+offset, next_frame, get_flattened_grid(flattened_grid, height, width, row, index_assigned+offset, frame));
		    		}
		    	}	
			}

    		if (!advance){
    			sem_wait(sem);
	    		// printf("Process %i said hes all done on iter %i == %i \n", getpid(), *current_iter, personal_iter);
	    		each_process_ready[identity] = 0;
	    		personal_iter++;
	    		// printf("Process %i increase pers iter from %i to %i\n", getpid(), personal_iter-1, personal_iter);
	    		sem_post(sem);
    		}
    		if (advance){
    			sem_wait(sem);
    			// printf("Process %i allowed others to adv on iter %i == %i \n", getpid(), *current_iter, personal_iter);
    			// printf("Process %i said hes all done on iter %i == %i \n", getpid(), *current_iter, personal_iter);
	    		*global_advance = 1;
	    		personal_iter++;
	    		// printf("Process %i increase pers iter from %i to %i\n", getpid(), personal_iter-1, personal_iter);
	    		sem_post(sem);
    		}

    		if (personal_iter == iter) break;
    	}
    }

    // printf("got here %i\n", getpid());
    if (getpid()!=parent_id){
    	// printf("Process %i says goodbvye!\n", getpid());
    	exit(0);
    }

    printf("Result after %i iterations.\n", iter);
    for(int row=0;row<height;row++){
    	for (int col = 0; col < width; ++col)
    	{
			printf("%i " , get_flattened_grid(flattened_grid, height, width, row, col, (iter)%2));
    	}
    	printf("\n");
    }

	/* Detach the shared memory segment. */
	shmdt (flattened_grid);
	/* Deallocate the shared memory segment.*/
	shmctl (shmid, IPC_RMID, 0);


	return 0;
}