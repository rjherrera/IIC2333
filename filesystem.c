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

#define ENDOFFILE 16777215

#define DISK_SIZE 1048576
#define FIXED_SIZE 100

typedef struct archivo
{
    char* name;
    uint32_t mem_dir;
    uint32_t memory_used;
} File;

File* init_file(char* name, uint32_t assigned_disk_block)
{
    File* al = malloc(sizeof(File));
    al -> name = malloc(strlen(name)*sizeof(char));
    strcpy(al -> name, name);
    al -> mem_dir = assigned_disk_block;
    al -> memory_used = 50;
    return al;
}

typedef struct directory
{
    char* absolute_path;
    char* name;
    int n_subdirs;
    struct directory** subdirs;
    File** files;
    int n_files;
    struct directory* parent_dir;
    uint32_t mem_dir;
} Dir;

Dir* init_dir(char* name, uint32_t assigned_disk_block){
    Dir* dir = malloc(sizeof(Dir));
    // dir -> absolute_path = malloc(strlen(path)*sizeof(char));
    dir -> absolute_path = NULL;
    dir -> name = malloc(strlen(name)*sizeof(char));
    // strcpy(dir -> absolute_path, path);
    strcpy(dir -> name, name);
    dir -> n_subdirs = 0;
    dir -> subdirs = malloc(sizeof(Dir*) * FIXED_SIZE);
    dir -> files = malloc(sizeof(File*) * FIXED_SIZE);
    dir -> n_files = 0;
    dir -> parent_dir = NULL;
    dir -> mem_dir = assigned_disk_block;
    return dir;
}

void set_parent(Dir* dir, Dir* parent_dir)
{
    dir -> parent_dir = parent_dir;
}

void insert_dir(Dir* dir, Dir* subdir){
    if (dir -> n_subdirs % FIXED_SIZE == 0) {
        dir -> subdirs = realloc(dir -> subdirs, (dir -> n_subdirs/FIXED_SIZE + 1) * sizeof(Dir*));
    }
    dir -> subdirs[dir -> n_subdirs] = subdir;
    // Set parent and absolute_path
    set_parent(subdir, dir);
    char abs_path[strlen(dir->absolute_path)+ strlen(subdir->name)];
    subdir -> absolute_path = malloc(sizeof(char) * (strlen(dir -> absolute_path) + strlen(subdir -> name) + 1));
    strcpy(subdir -> absolute_path, dir -> absolute_path);
    strcat(subdir -> absolute_path, subdir -> name);

    dir -> n_subdirs++;
}

void insert_file(Dir* dir, File* file){
    if (dir -> n_files % FIXED_SIZE == 0) {
        dir -> files = realloc(dir -> files, (dir -> n_files/FIXED_SIZE + 1) * sizeof(File*));
    }
    dir -> files[dir -> n_files] = file;
    dir -> n_files++;
}

uint32_t get_metadata(uint32_t block)
{
    return (block & ((1 << 8)-1));
}

uint32_t get_pointer(uint32_t block)
{
    return (block & (((1 << 31)-1) << 8))>>8;
}

// void set_metadata(uint32_t metadata, uint32_t block)
// {

// }

// void set_pointer(uint32_t pointer, uint32_t block)
// {

// }

uint32_t remake_block(uint32_t metadata, uint32_t pointer)
{
    return (pointer << 8) | metadata;
}

uint32_t* find_free_block(uint32_t* disk)
{
    for (uint32_t block = 0; block < DISK_SIZE; ++block)
    {
        uint32_t metadata = get_metadata(disk[block]);
        if (metadata & FREE_BLOCK)
        {
            return &disk[block];
        }
    }
    return NULL;
}

int is_in(char* dir_name, Dir* dir){
    for (int i = 0; i < dir -> n_subdirs; ++i){
        if (!strcmp(dir -> subdirs[i] -> name, dir_name)) return i;
    }
    return -1;
}

int file_is_in(char* file_name, Dir* dir){
    for (int i = 0; i < dir->n_files; ++i)
    {
        if (!strcmp(dir -> files[i] -> name, file_name)) return i;
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
    char* path_copy = malloc(sizeof(char)*strlen(path));
    strcpy(path_copy, path);
    char* path_piece = strtok(path_copy, "/");
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
        free(last);
    }
    // libero la memoria usada para el strtok
    // printf("Finalmente estoy en dir: %s\n", actual_dir -> name);
    return actual_dir;
}
