#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#define SEED 1
#define ACTIONS 2
#define UNTILL 4
#define DISK 8

#define FREE_BLOCK 4
#define IS_DIRECTORY 8
#define IS_CONTENT 16


#define DISK_SIZE 1048576
#define FIXED_SIZE 100

typedef struct archivo
{
    char* name;
    uint32_t mem_dir;
    uint32_t memory_used;
} MyFile;

MyFile* init_file(char* name, uint32_t assigned_disk_block)
{
    MyFile* al = malloc(sizeof(MyFile));
    strcpy(al -> name, name);
    al -> mem_dir = assigned_disk_block;
    al -> memory_used = 4096;
    return al;
}

typedef struct directory
{
    char* absolute_path;
    char* name;
    int n_subdirs;
    struct directory** subdirs;
    MyFile** files;
    int n_files;
} Dir;

int file_is_in(char* file_name, Dir* dir){
    for (int i = 0; i < dir->n_files; ++i)
    {
        if (!strcmp(dir -> files[i] -> name, file_name)) return i;
    }
    return -1;
}

Dir* init_dir(char* path, char* name){
    Dir* dir = malloc(sizeof(Dir));
    dir -> absolute_path = malloc(strlen(path));
    dir -> name = malloc(strlen(name));
    strcpy(dir -> absolute_path, path);
    strcpy(dir -> name, name);
    dir -> n_subdirs = 0;
    dir -> subdirs = malloc(sizeof(Dir*) * FIXED_SIZE);
    return dir;
}

void insert_dir(Dir* dir, Dir* subdir){
    if (dir -> n_subdirs % FIXED_SIZE == 0) {
        dir -> subdirs = realloc(dir -> subdirs, (dir -> n_subdirs/FIXED_SIZE + 1) * sizeof(Dir*));
    }
    dir -> subdirs[dir -> n_subdirs] = subdir;
    dir -> n_subdirs++;
}

uint32_t get_metadata(uint32_t block)
{
    return (block & ((1 << 8)-1));
}

uint32_t get_pointer(uint32_t block)
{
    return (block & (((1 << 31)-1) << 8))>>8;
}

uint32_t remake_block(uint32_t metadata, uint32_t pointer)
{
    return (pointer << 8) | metadata;
}

char* select_random_root(int seed)
{

}

int find_free_block(uint32_t* disk)
{
    for (uint32_t block = 0; block < DISK_SIZE; ++block)
    {
        uint32_t metadata = get_metadata(disk[block]);
        if (metadata & FREE_BLOCK)
        {
            return block;
        }
    }
}

int is_in(char* dir_name, Dir* dir){
    for (int i = 0; i < dir -> n_subdirs; ++i){
        if (!strcmp(dir -> subdirs[i] -> name, dir_name)) return i;
    }
    return -1;
}

Dir* has_subdir(Dir* actual_dir, char* path){
    // Función que dado un directorio entrega si un path
    // dentro de el existe (si en root hay un usr/raimundo)
    // osea que se pueda ir a root/usr/raimundo
    // retorna NULL si no se puede, y el puntero al directorio
    // si esque si se puede (raimundo en ese caso)
    // printf("Destino final: %s\n", path);
    char* path_piece = strtok(path, "/");
    char* last;
    while (path_piece) {
        // printf("Estoy en: %s\n", actual_dir -> name);
        // printf("Quiero ir a: %s\n", path_piece);
        last = malloc(strlen(path_piece));
        strcpy(last, path_piece);
        path_piece = strtok(NULL, "/");
        int dir_index = is_in(last, actual_dir);
        // printf("Índice: %d\n", dir_index);
        // si no estaba, entonces retorno null
        if (dir_index == -1) return NULL;
        // si estaba, entonces me cambio a ese directorio y me quedo dentro
        actual_dir = actual_dir -> subdirs[dir_index];

        // if (path_piece != NULL){
        //     printf("This piece is %s\n", last);
        // }

    }
    // libero la memoria usada para el strtok
    free(last);
    // printf("Finalmente estoy en dir: %s\n", actual_dir -> name);
    return actual_dir;
}

// void generate_new_instruction(int seed)
// {
//     srand(seed);
//     char* instructions[5] = {
//         "cd"

//     };
//     int index = rand();
// }

Dir* current_dir;

int main(int argc, char** argv)
{
    // inicializacion
    current_dir = init_dir("base_dir", "base_dir");
    Dir* otro_dir1 = init_dir("base_dir/hola", "hola");
    Dir* otro_dir2 = init_dir("base_dir/chao", "chao");
    Dir* otro_dir1a = init_dir("base_dir/hola/wena", "wena");

    // test methods
    insert_dir(current_dir, otro_dir1);
    insert_dir(current_dir, otro_dir2);
    insert_dir(otro_dir1, otro_dir1a);
    char b[10] = "hola/wena";

    // b debería estar en current dir (se guarda en a el puntero)
    Dir* a = has_subdir(current_dir, b);
    printf("Quedé en: %s\n", a -> name);
    char d[8] = "hola/uu";
    Dir* c = has_subdir(current_dir, d);
    // d no deberia estar en current dir (se guarda en c el NULL)
    if (c == NULL) printf("NULL\n");

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

                Dir* new_directory = has_subdir(current_dir, buff);

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

                char* new_dir_name = strrchr(buff, '/') + 1;

                char* parent_directory_path = malloc(sizeof(char)*(new_dir_name-buff-1));
                strncpy(parent_directory_path, buff, new_dir_name-buff-1);

                Dir* parent_directory = has_subdir(current_dir, parent_directory_path);

                if (parent_directory)
                {
                    if (is_in(new_dir_name, parent_directory) == -1)
                    {
                        Dir* new_dir = init_dir(buff,new_dir_name);
                        insert_dir(parent_directory, new_dir);
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

                char* new_file_name = strrchr(buff, '/') + 1;

                char* parent_directory_path = malloc(sizeof(char)*(new_file_name-buff-1));
                strncpy(parent_directory_path, buff, new_file_name-buff-1);

                Dir* parent_directory = has_subdir(current_dir, parent_directory_path);

                if (parent_directory)
                {
                    MyFile* new_file = init_file(buff,new_file_name);
                    insert_file(parent_directory, new_file);
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


            // if (fgets (buff, 200, action_file) != NULL)
            // {
            //     printf("With this Argument %s \n",buff);
            // }
            // executed_instructions++;
        }



        fclose(action_file);

        if (acum_flags & UNTILL)
        {

            // steps_done = 0
            // for action in actions.txt
            //.   execute_action
            //    steps_done++
            // while steps_done < steps:
            //      generate_random_action(seed)
            //      execute it
        }
    }
    else
    {

        // generate steps instructions and execute em
    }




}