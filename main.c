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
    }
    else
    {
        for (int block = 0; block < DISK_SIZE; ++block)
        {
            simdisk[block] |= FREE_BLOCK;
        }
    }

    current_dir = init_dir("base_dir", 0);
    current_dir -> absolute_path = "base_dir/";
    simdisk[0] &= (~FREE_BLOCK); // set free to 0
    simdisk[0] |= IS_DIRECTORY; // set is directory to 1
    uint32_t metadata = get_metadata(simdisk[0]);
    fprintf(accesses, "%s\n", current_dir->absolute_path);
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

        // while (fgets (buff, 200, action_file) != NULL)
        // int executed_instructions = 0;
        // while (executed_instructions < steps && fscanf(action_file, "%s\n", key) == 1)
        while (fscanf(action_file, "%s\n", key) == 1)
        {
            if (strcmp(key, "cd") == 0)
            {
                fscanf(action_file, "%s\n", buff);


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
                }
                else
                {
                    printf("La ruta relativa %s no existe. Instruccion ignorada. Directorio actual: %s\n", buff, current_dir->absolute_path);
                }
            }
            else if (strcmp(key, "mkdir") == 0)
            {
                fscanf(action_file, "%s\n", buff);

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
                            fprintf(accesses, "%s\n", new_dir->absolute_path);
                            *free_block = ((current_line_accesses << 8) | metadata);

                            current_line_accesses++;
                        }
                        else
                        {
                            printf("El disco esta lleno. Instruccion ignorada.\n");
                        }



                    }
                    else
                    {
                        printf("La ruta relativa %s ya contiene un directorio llamado %s. Instruccion ignorada.\n", parent_directory_path, current_dir->absolute_path);
                    }
                }
                else
                {
                    printf("La ruta relativa %s no existe. Instruccion ignorada.\n", parent_directory_path);
                }

            }
            else if (strcmp(key, "mkfile") == 0)
            {
                fscanf(action_file, "%s\n", buff);

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
                        }
                        else
                        {
                            printf("El disco esta lleno. Instruccion ignorada.\n");
                        }
                    }
                    else
                    {
                        printf("La ruta relativa %s ya contiene un archivo llamado %s. Instruccion ignorada.\n", parent_directory_path, new_file_name);
                    }                    
                }
                else
                {
                    printf("La ruta relativa %s no existe. Instruccion ignorada. Directorio actual: %s\n", parent_directory_path, current_dir->absolute_path);
                }

            }
            else if (strcmp(key, "mv") == 0)
            {


            }
            else if (strcmp(key, "rm") == 0)
            {


            }
            else if (strcmp(key, "ad") == 0)
            {


            }
            else if (strcmp(key, "rd") == 0)
            {

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
            // if (fgets (buff, 200, action_file) != NULL)
            // {
            //     printf("With this Argument %s \n",buff);
            // }
            // executed_instructions++;
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