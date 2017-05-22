#include "filesystem.c"

Dir* current_dir;

int main(int argc, char** argv)
{
    // Dir* otro_dir1 = init_dir("hola", 1);
    // Dir* otro_dir2 = init_dir("chao", 2);
    // Dir* otro_dir1a = init_dir("wena", 3);

    // // test methods
    // insert_dir(current_dir, otro_dir1);
    // insert_dir(current_dir, otro_dir2);
    // insert_dir(otro_dir1, otro_dir1a);
    // char b[10] = "hola/wena";

    // // b debería estar en current dir (se guarda en a el puntero)
    // Dir* a = has_subdir(current_dir, b);
    // printf("Quedé en: %s\n", a -> name);
    // char d[8] = "hola/uu";
    // Dir* c = has_subdir(current_dir, d);
    // // d no deberia estar en current dir (se guarda en c el NULL)
    // if (c == NULL) printf("NULL\n");

    int acum_flags = 0;

    char* seed = NULL;
    char* actions = NULL;
    char* disk = NULL;

    if(argc == 2)
    {
        // int steps = atoi(argv[3]);
    }
    else if (argc >= 4 && argc <=6)
    {
        char* flags = argv[2];
        if (flags[0] != '-')
        {
            printf("Modo de uso: %s <steps> -sa(u)d(opcionales) <seed> <action.txt> <disk>\n", argv[0]);
            printf("\t steps : Cantidad de acciones a simular.\n");
            printf("\t flags saud \n");
            return 1;
        }

        for (int i = 1; i < strlen(flags); ++i)
        {
            char current_flag = flags[i];
            if (current_flag == 's')
            {
                acum_flags |= SEED;
            }
            else if (current_flag == 'a')
            {
                acum_flags |= ACTIONS;
            }
            else if (current_flag == 'u')
            {
                acum_flags |= UNTILL;
            }
            else if (current_flag == 'd')
            {
                acum_flags |= DISK;
            }
            else
            {
                printf("Modo de uso: %s <steps> -sa(u)d(opcionales) <seed> <action.txt> <disk>\n", argv[0]);
                printf("\t steps : Cantidad de acciones a simular.\n");
                printf("\t flags sa(u)d : Indica si \n");
                return 1;
            }
        }


        if (argc == 4)
        {
            seed = argv[3];
        }
        else if (argc == 5)
        {
            seed = argv[3];
            actions = argv[4];
        }
        else
        {
            seed = argv[3];
            actions = argv[4];
            disk = argv[5];
        }


        // if someone gives untill flag but not action, it'll disable it
        if ( (acum_flags & UNTILL) != 0 && (acum_flags & ACTIONS) == 0)
        {
            acum_flags &= ~UNTILL;
        }
        int format_error = 0;
        if ((acum_flags & DISK) != 0 && disk == NULL) format_error = 1;
        if ((acum_flags & ACTIONS) != 0 && actions == NULL) format_error = 1;
        if (format_error)
        {
            printf("Modo de uso: %s <steps> -sa(u)d(opcionales) <seed> <action.txt> <disk>\n", argv[0]);
            printf("\t steps : Cantidad de acciones a simular.\n");
            printf("\t flags saud\n");
            return 1;
        }
        // Now its absolutely certain that if a flag is on, the corresponding
        // variable is set.
    }
    else
    {
        printf("Modo de uso: %s <steps> -sa(u)d(opcionales) <seed> <action.txt> <disk>\n", argv[0]);
        printf("\t steps : Cantidad de acciones a simular.\n");
        printf("\t flags saud\n");
        return 1;
    }

    int steps = atoi(argv[1]);

    FILE *accesses = fopen("accesos.txt", "w");
    int current_line_accesses = 0;

    uint32_t* simdisk = (uint32_t*) calloc(DISK_SIZE, sizeof(uint32_t));
    if (acum_flags & DISK)
    {
        FILE* read_disk = fopen(disk, "rb");
        // fseek(read_disk, 0, SEEK_SET);
        fread(simdisk, 4, sizeof(simdisk),read_disk);
        fclose(read_disk);
        // usamos 
    }
    else
    {
        for (int block = 0; block < DISK_SIZE; ++block)
        {
            simdisk[block] |= FREE_BLOCK;
        }
    }

    current_dir = init_dir("base_dir", 0);
    current_dir -> absolute_path = malloc(sizeof(char)*strlen("base_dir"));
    strcpy(current_dir -> absolute_path, "base_dir");
    simdisk[0] &= (~FREE_BLOCK); // set free to 0
    simdisk[0] |= IS_DIRECTORY; // set is directory to 1
    uint32_t metadata = get_metadata(simdisk[0]);
    fprintf(accesses, "%s.txt\n", current_dir->absolute_path);
    simdisk[0] = ((current_line_accesses << 8) | metadata);

    current_line_accesses++;        









    int executed_instructions = 0;

    if (acum_flags & ACTIONS)
    {
        FILE* action_file = fopen(actions,"r");
        if(action_file == NULL)
        {
           perror("Error opening file");
           return(-1);
        }

        // Expecting instructions with < 200 chars
        char key[10];
        char buff[200];
        char buff2[200];
        char buff3[200];

        char buff_whole_line[1024];

        // while (fgets (buff, 200, action_file) != NULL)
        // int executed_instructions = 0;
        // while (executed_instructions < steps && fscanf(action_file, "%s\n", key) == 1)
        // while (fscanf(action_file, "%s\n", key) == 1)
        while (fscanf(action_file, "%s\n", key) == 1)
        {
            if (strcmp(key, "cd") == 0)
            {
                // fscanf(action_file, "%s\n", buff);
                fgets(buff_whole_line, 1024, action_file);
                sscanf(buff_whole_line, "%s", buff);

                Dir* new_directory;

                if (strcmp(buff,".") == 0)
                {  
                    new_directory = current_dir;
                }
                else if (strcmp(buff,"..") == 0)
                {
                    new_directory = current_dir->parent_dir;
                }
                else
                {
                    new_directory = has_subdir(current_dir, buff);
                }


                if (new_directory)
                {
                    current_dir = new_directory;
                    printf("Has movido a la nueva ruta absoluta %s\n", current_dir->absolute_path);
                }
                else
                {
                    printf("La ruta relativa %s no existe. Instruccion ignorada. Directorio actual: %s\n", buff, current_dir->absolute_path);
                    continue;
                }
            }
            else if (strcmp(key, "mkdir") == 0)
            {
                // fscanf(action_file, "%s\n", buff);
                fgets(buff_whole_line, 1024, action_file);
                sscanf(buff_whole_line, "%s\n", buff);                

                char* new_dir_name = strrchr(buff, '/');

                char* parent_directory_path;
                Dir* parent_directory;

                if (new_dir_name)
                {
                    new_dir_name+=1;
                    parent_directory_path = malloc(sizeof(char)*(new_dir_name-buff-1));
                    strncpy(parent_directory_path, buff, new_dir_name-buff-1);
                    parent_directory = has_subdir(current_dir, parent_directory_path);
                }
                else
                {
                    new_dir_name = buff;
                    parent_directory_path = current_dir->absolute_path; 
                    parent_directory = current_dir;
                }

                if (parent_directory)
                {
                    if (is_in(new_dir_name, parent_directory) == -1)
                    {
                        uint32_t* free_block = find_free_block(simdisk);
                        if (free_block){
                            // manage file system info
                            uint32_t block_index = free_block-simdisk;
                            Dir* new_dir = init_dir(new_dir_name, block_index);
                            insert_dir(parent_directory, new_dir);

                            // manage block info
                            *free_block &= (~FREE_BLOCK); // toggle free_to 0
                            *free_block |= IS_DIRECTORY; // toggle  is directory to 1
                            uint32_t metadata = get_metadata(*free_block);

                            // write access
                            fprintf(accesses, "%s.txt\n", new_dir->absolute_path);
                            *free_block = ((current_line_accesses << 8) | metadata);

                            current_line_accesses++;

                            // log 
                            printf("Has creado el directorio en la ruta absoluta %s\n", new_dir->absolute_path);
                        }
                        else
                        {
                            printf("El disco esta lleno. Instruccion ignorada.\n");
                            continue;
                        }



                    }
                    else
                    {
                        printf("La ruta relativa %s ya contiene un directorio llamado %s. Instruccion ignorada.\n", parent_directory_path, new_dir_name);
                        continue;
                    }
                }
                else
                {
                    printf("La ruta relativa %s no existe. Instruccion ignorada.\n", parent_directory_path);
                    continue;
                }
            }
            else if (strcmp(key, "mkfile") == 0)
            {
                // fscanf(action_file, "%s\n", buff);
                fgets(buff_whole_line, 1024, action_file);
                sscanf(buff_whole_line, "%s\n", buff);

                char* new_file_name = strrchr(buff, '/');

                char* parent_directory_path;
                Dir* parent_directory;

                if (new_file_name)
                {
                    new_file_name+=1;
                    parent_directory_path = malloc(sizeof(char)*(new_file_name-buff-1));
                    strncpy(parent_directory_path, buff, new_file_name-buff-1);
                    parent_directory = has_subdir(current_dir, parent_directory_path);
                }
                else
                {
                    new_file_name = buff;
                    parent_directory_path = current_dir->absolute_path; 
                    parent_directory = current_dir;
                }

                if (parent_directory)
                {

                    if (file_is_in(new_file_name, parent_directory) == -1)
                    {
                        uint32_t* free_block = find_free_block(simdisk);
                        if (free_block){
                            uint32_t block_index = free_block-simdisk;
                            File* new_file = init_file(new_file_name, block_index);
                            insert_file(parent_directory, new_file);
                            *free_block &= (~FREE_BLOCK); // toggle free_to 0
                            *free_block |= IS_CONTENT; // toggle  is content to 1
                            uint32_t metadata = get_metadata(*free_block);
                            *free_block = ((ENDOFFILE << 8) | metadata);

                            printf("Has creado el archivo %s dentro del directorio de ruta absoluta %s\n", new_file->name, parent_directory->absolute_path);

                        }
                        else
                        {
                            printf("El disco esta lleno. Instruccion ignorada.\n");
                            continue;
                        }
                    }
                    else
                    {
                        printf("La ruta relativa %s ya contiene un archivo llamado %s. Instruccion ignorada.\n", parent_directory_path, new_file_name);
                        continue;
                    }                    
                }
                else
                {
                    printf("La ruta relativa %s no existe. Instruccion ignorada.\n", parent_directory_path);
                    continue;
                }
            }
            else if (strcmp(key, "mv") == 0)
            {
                // sscanf(buff_whole_line, "%s %s", buff, buff2);
                fgets(buff_whole_line, 1024, action_file);
                
                // if (fscanf(action_file, "%s %s\n", buff, buff2) != 2)
                if (sscanf(buff_whole_line, "%s %s", buff, buff2) != 2)
                {
                    printf("Debes ingresar tanto la ruta relativa del archivo a mover como el nuevo destino. Instruccion  %s ignorada.\n", buff_whole_line);
                    continue;
                }

                Dir* destination_parent_dir = has_subdir(current_dir, buff2);

                if (!(destination_parent_dir))
                {
                    printf("La ruta relativa de destino %s no existe. Instruccion ignorada.\n", buff2);
                    continue;
                }
                else
                {
                    char* sth_to_move_name = strrchr(buff, '/');

                    char* parent_directory_path;
                    Dir* parent_directory;

                    if (sth_to_move_name)
                    {
                        // it wasnt directly under the current dir
                        sth_to_move_name+=1;
                        parent_directory_path = malloc(sizeof(char)*(sth_to_move_name-buff-1));
                        strncpy(parent_directory_path, buff, sth_to_move_name-buff-1);
                        parent_directory = has_subdir(current_dir, parent_directory_path);
                    }
                    else
                    {
                        sth_to_move_name = buff;
                        // strcpy(sth_to_move_name, buff);
                        parent_directory_path = current_dir->absolute_path; 
                        parent_directory = current_dir;
                    }

                    if (parent_directory)
                    {
                        // check if sth is file
                        if (file_is_in(sth_to_move_name, parent_directory) != -1)
                        {

                            File* file_to_move = parent_directory -> files[file_is_in(sth_to_move_name, parent_directory)];
                            remove_file(parent_directory, file_to_move);
                            insert_file(destination_parent_dir, file_to_move);

                            printf("Se ha movido el archivo %s a la ruta %s\n", buff, buff2);
                        }
                        else
                        {
                            // now check if it was dir
                            if (is_in(sth_to_move_name, parent_directory) != -1)
                            {
                                Dir* dir_to_move = parent_directory -> subdirs[is_in(sth_to_move_name, parent_directory)];

                                // remove_dir(parent_directory, dir_to_move);
                                // delete_release_and_destroy(dir_to_move, simdisk);

                                remove_dir(parent_directory, dir_to_move);
                                insert_dir(destination_parent_dir, dir_to_move);

                                // Correct access info and 
                                fprintf(accesses, "%s.txt\n", dir_to_move->absolute_path);
                                simdisk[dir_to_move -> mem_dir] = ((current_line_accesses << 8) | get_metadata(simdisk[dir_to_move -> mem_dir]));
                                current_line_accesses++;                    

                                for (int subd_index = 0; subd_index < dir_to_move -> n_subdirs; ++subd_index)
                                {
                                    update_absolute_path(dir_to_move -> subdirs[subd_index],
                                                        dir_to_move -> absolute_path,
                                                        simdisk,
                                                        &current_line_accesses,
                                                        accesses);
                                }
                                printf("Se ha movido el directorio %s a la ruta %s\n", buff, buff2);


                            }
                            else
                            {
                                printf("La ruta relativa %s no contiene ni directorio ni archivo llamado %s. Instruccion ignorada.\n", parent_directory_path, sth_to_move_name);
                                continue;
                            }                        
                        }                    
                    }
                    else
                    {
                        printf("La ruta relativa %s no existe. Instruccion ignorada.\n", parent_directory_path);
                        continue;
                    }
                }
            }
            else if (strcmp(key, "rm") == 0)
            {
                // fscanf(action_file, "%s\n", buff);
                fgets(buff_whole_line, 1024, action_file);
                sscanf(buff_whole_line, "%s", buff);

                char* sth_to_rm_name = strrchr(buff, '/');

                char* parent_directory_path;
                Dir* parent_directory;

                if (sth_to_rm_name)
                {
                    sth_to_rm_name+=1;
                    parent_directory_path = malloc(sizeof(char)*(sth_to_rm_name-buff-1));
                    strncpy(parent_directory_path, buff, sth_to_rm_name-buff-1);
                    parent_directory = has_subdir(current_dir, parent_directory_path);
                }
                else
                {
                    sth_to_rm_name = buff;
                    parent_directory_path = current_dir->absolute_path; 
                    parent_directory = current_dir;
                }

                if (parent_directory)
                {
                    // check if sth is file
                    if (file_is_in(sth_to_rm_name, parent_directory) != -1)
                    {
                        File* file_to_rm = parent_directory -> files[file_is_in(sth_to_rm_name, parent_directory)];
                        remove_file(parent_directory, file_to_rm);
                        release_block_trail(file_to_rm, simdisk);
                        destroy_file(file_to_rm);
                        
                        printf("Has eliminado el archivo %s dentro del directorio de ruta absoluta %s\n", sth_to_rm_name, parent_directory->absolute_path);
                    }
                    else
                    {
                        // now check if it was dir
                        if (is_in(sth_to_rm_name, parent_directory) != -1)
                        {
                            Dir* dir_to_rm = parent_directory -> subdirs[is_in(sth_to_rm_name, parent_directory)];
                            remove_dir(parent_directory, dir_to_rm);
                            delete_release_and_destroy_dir(dir_to_rm, simdisk);
                            printf("Has eliminado el directorio %s (y sus subdirectorios) dentro del directorio de ruta absoluta %s\n", sth_to_rm_name, parent_directory->absolute_path);
                        }
                        else
                        {
                            printf("La ruta relativa %s no contiene ni directorio ni archivo llamado %s. Instruccion ignorada.\n", parent_directory_path, sth_to_rm_name);
                            continue;
                        }                        
                    }                    
                }
                else
                {
                    printf("La ruta relativa %s no existe. Instruccion ignorada.\n", parent_directory_path);
                    continue;
                }
            }
            else if (strcmp(key, "ad") == 0)
            {
                fgets(buff_whole_line, 1024, action_file);
                int num_scans = sscanf(buff_whole_line, "%s %s %s\n", buff, buff2, buff3);
                // int num_scans = fscanf(action_file, "%s %s %s\n", buff, buff2, buff3);


                if (num_scans == 2 || num_scans == 3){


                    char* file_to_w_name = strrchr(buff, '/');

                    char* parent_directory_path;
                    Dir* parent_directory;

                    if (file_to_w_name)
                    {
                        file_to_w_name+=1;
                        parent_directory_path = malloc(sizeof(char)*(file_to_w_name-buff-1));
                        strncpy(parent_directory_path, buff, file_to_w_name-buff-1);
                        parent_directory = has_subdir(current_dir, parent_directory_path);
                    }
                    else
                    {
                        file_to_w_name = buff;
                        parent_directory_path = current_dir->absolute_path; 
                        parent_directory = current_dir;
                    }

                    if (parent_directory)
                    {
                        // get _ file now
                        if (file_is_in(file_to_w_name, parent_directory) != -1)
                        {
                            File* file_to_write = parent_directory -> files[file_is_in(file_to_w_name, parent_directory)];
                            int write_offset = file_to_write -> memory_used ; // "last position + 1 (first empty byte)"
                            if (num_scans == 3)
                            {
                                write_offset = atoi(buff3);
                            }
                            
                            int barrier = 4096;
                            while (file_to_write -> memory_used > barrier)
                            {
                                barrier+= 4096;
                            }

                            int final_w_pos = write_offset + atoi(buff2);
                            if (final_w_pos > barrier)
                            {
                                int additional_blocks = 0;
                                int temp_start = final_w_pos-1;
                                while (temp_start > barrier)
                                {
                                    additional_blocks++;
                                    temp_start -= 4096;
                                }
                                int broke = 0;
                                for (int iter = 0; iter < additional_blocks; ++iter)
                                {
                                    // find empty block, asign it fixing recursive last block,

                                    uint32_t* free_block = find_free_block(simdisk);
                                    if (free_block){
                                        uint32_t last_block_index = recursive_get_last_block_index(simdisk, file_to_write->mem_dir);
                                        uint32_t meta = get_metadata(simdisk[last_block_index]);
                                        simdisk[last_block_index] = (((free_block - simdisk) << 8) | meta);

                                        *free_block &= (~FREE_BLOCK); // toggle free_to 0
                                        *free_block |= IS_CONTENT; // toggle  is content to 1
                                        uint32_t metadata = get_metadata(*free_block);
                                        *free_block = ((ENDOFFILE << 8) | metadata);
                                    }
                                    else
                                    {
                                        printf("El disco esta lleno. Escritura detenida.\n");
                                        broke = 1;
                                        break;
                                    }
                                }

                                if (broke)
                                {
                                    printf("Se ha llevado a cabo la escritura parcialmente pues se acabo el espacio en disco.\n");
                                    continue;  
                                }
                                printf("Se han escrito %d bytes en archivo %s de directorio %s\n", atoi(buff2), file_to_write->name, parent_directory_path);
                            }

                            file_to_write -> memory_used = final_w_pos-1;

                        }
                        else
                        {
                            printf("La ruta relativa %s no contiene un archivo llamado %s. Instruccion ignorada.\n", parent_directory_path, file_to_w_name);
                            continue;
                        }                 
                    }
                    else 
                    {
                        printf("La ruta relativa %s no existe. Instruccion ignorada.\n", parent_directory_path); 
                        continue;                       
                    }
                }
                else
                {
                    printf("Debes ingresar la ruta relativa al archivo y la cantidad de bytes a agregar, \
                        seguido opcionalmente de la posicion en el archivo que deseas comenzar la escritura. Instruccion  %s ignorada.\n", buff_whole_line);
                    continue;
                }
            }
            else if (strcmp(key, "rd") == 0)
            {
                fgets(buff_whole_line, 1024, action_file);
                // int num_scans = fscanf(action_file, "%s %s %s\n", buff, buff2, buff3);
                int num_scans = sscanf(buff_whole_line, "%s %s %s\n", buff, buff2, buff3);

                if (num_scans == 2 || num_scans == 3){


                    char* file_to_w_name = strrchr(buff, '/');

                    char* parent_directory_path;
                    Dir* parent_directory;

                    if (file_to_w_name)
                    {
                        file_to_w_name+=1;
                        parent_directory_path = malloc(sizeof(char)*(file_to_w_name-buff-1));
                        strncpy(parent_directory_path, buff, file_to_w_name-buff-1);
                        parent_directory = has_subdir(current_dir, parent_directory_path);
                    }
                    else
                    {
                        file_to_w_name = buff;
                        parent_directory_path = current_dir->absolute_path; 
                        parent_directory = current_dir;
                    }

                    if (parent_directory)
                    {
                        // get _ file now
                        if (file_is_in(file_to_w_name, parent_directory) != -1)
                        {
                            File* file_to_write = parent_directory -> files[file_is_in(file_to_w_name, parent_directory)];
                            int write_starting_index = file_to_write -> memory_used - atoi(buff2) ; // "last position + 1 (first empty byte)"
                            if (num_scans == 3)
                            {
                                write_starting_index = atoi(buff3);
                            }
                            
                            int remaining_lines_till_full;
                            int block_of_starting_w = find_number_of_block_of_line(write_starting_index, &remaining_lines_till_full);

                            if (atoi(buff2) > remaining_lines_till_full)
                            {
                                // first reach block of starting
                                uint32_t block_pointer_index = file_to_write -> mem_dir;
                                for (int i = 0; i < block_of_starting_w; ++i)
                                {
                                    block_pointer_index = get_pointer(simdisk[block_pointer_index]);
                                }

                                // handle border case if deletion starts on first block
                                if (block_pointer_index == 0)
                                {
                                    // el bloque cero siempre contendra los primeros 50B, por lo que 
                                    // para cada 4096 dps de los primeros remaining se borraran bloques
                                    int additional_deletion = atoi(buff2) - remaining_lines_till_full;
                                    // for every 4096, delete a block
                                    int num_blocks = 0;
                                    int temp_c = additional_deletion;
                                    while (temp_c > 4096)
                                    {
                                        num_blocks++;
                                        temp_c -= 4096;
                                    }

                                    for (int iter = 0; iter < num_blocks; ++iter)
                                    {
                                        uint32_t ptr_to_next = get_pointer(simdisk[block_pointer_index]);
                                        uint32_t point_now_at = get_pointer(simdisk[ptr_to_next]);
                                        simdisk[ptr_to_next] = FREE_BLOCK;
                                        simdisk[block_pointer_index] = (point_now_at << 8) | get_metadata(simdisk[block_pointer_index]);
                                    }
                                }
                                else
                                {
                                    // at least one block must be freed
                                    if (write_starting_index % 4096 == 0){
                                        // for 
                                        int num_blocks = 0;
                                        int temp_c = atoi(buff2);
                                        while (temp_c > 4096)
                                        {
                                            num_blocks++;
                                            temp_c -= 4096;
                                        }

                                        // since the deletions here include deleting the starting block,
                                        // find the reference to the previous one before deleting
                                        uint32_t block_pointer_index = file_to_write -> mem_dir;
                                        for (int i = 0; i < block_of_starting_w-1; ++i)
                                        {
                                            block_pointer_index = get_pointer(simdisk[block_pointer_index]);
                                        }

                                        for (int iter = 0; iter < num_blocks; ++iter)
                                        {
                                            uint32_t ptr_to_next = get_pointer(simdisk[block_pointer_index]);
                                            uint32_t point_now_at = get_pointer(simdisk[ptr_to_next]);
                                            simdisk[ptr_to_next] = FREE_BLOCK;
                                            simdisk[block_pointer_index] = (point_now_at << 8) | get_metadata(simdisk[block_pointer_index]);
                                        }
                                    }
                                    else
                                    {
                                        // for every 4096 after the remaining lines, delete a block, starting from next one
                                        int additional_deletion = atoi(buff2) - remaining_lines_till_full;
                                        // for every 4096, delete a block
                                        int num_blocks = 0;
                                        int temp_c = additional_deletion;
                                        while (temp_c > 4096)
                                        {
                                            num_blocks++;
                                            temp_c -= 4096;
                                        }

                                        for (int iter = 0; iter < num_blocks; ++iter)
                                        {
                                            uint32_t ptr_to_next = get_pointer(simdisk[block_pointer_index]);
                                            uint32_t point_now_at = get_pointer(simdisk[ptr_to_next]);
                                            simdisk[ptr_to_next] = FREE_BLOCK;
                                            simdisk[block_pointer_index] = (point_now_at << 8) | get_metadata(simdisk[block_pointer_index]);
                                        }

                                    }
                                    
                                }
                            }

                            if (file_to_write -> memory_used - atoi(buff2) < 50)
                            {
                                file_to_write -> memory_used = 50;
                            }
                            else
                            {
                                file_to_write -> memory_used -= atoi(buff2); 
                            }

                        }
                        else
                        {
                            printf("La ruta relativa %s no contiene un archivo llamado %s. Instruccion ignorada.\n", parent_directory_path, file_to_w_name);
                            continue;
                        }                 
                    }
                    else 
                    {
                        printf("La ruta relativa %s no existe. Instruccion ignorada.\n", parent_directory_path); 
                        continue;                       
                    }
                }
                else
                {
                    printf("Debes ingresar la ruta relativa al archivo y la cantidad de bytes a eliminar, \
                        seguido opcionalmente de la posicion en el archivo que deseas comenzar la eliminacion. Instruccion ignorada.\n");
                    continue;
                }
            }
            else
            {
                printf("%s is not a valid instruction. Ignored.\n", key);
            }

            if (acum_flags & UNTILL)
            {
                executed_instructions++;
                if (executed_instructions == steps)
                {
                    break;
                }
            }

        }

        fclose(action_file);
    }

    if (executed_instructions < steps){
        // SI NO HUBO UNTILL, ESTO OCURRIRA SI O SI
        // SI HUBO UNTILL, EXECUTED INSTRUCTIONS NO SERA 0, Y DEPENDERA DE CUANTAS SE EJECUTARON


        // generate steps instructions and execute em


        // EJECUTAR STEPS
    }


    fclose(accesses);

    // GENERAR DIRECTORIOS Y .TXTS RESPECTIVOS RECURSIVAMENTE Y AÑADIR RUTAS A ESOS .TXTS EN ACCESOS.TXT Y GENERAR BITMAP


}