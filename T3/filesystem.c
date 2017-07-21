#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

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
    al -> name = malloc(strlen(name)*sizeof(char)+1);
    strcpy(al -> name, name);
    al -> mem_dir = assigned_disk_block;
    al -> memory_used = 50;
    return al;
}

void destroy_file(File* file)
{
    free(file -> name);
    free(file);
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
    dir -> absolute_path = NULL;
    dir -> name = malloc(strlen(name)*sizeof(char));
    strcpy(dir -> name, name);
    dir -> n_subdirs = 0;
    dir -> subdirs = malloc(sizeof(Dir*) * FIXED_SIZE);
    dir -> files = malloc(sizeof(File*) * FIXED_SIZE);
    dir -> n_files = 0;
    dir -> parent_dir = NULL;
    dir -> mem_dir = assigned_disk_block;
    return dir;
}

uint32_t get_pointer(uint32_t block)
{
    return (block >> 8);
}

uint32_t get_metadata(uint32_t block)
{
    return (block & ((1 << 8)-1));
}

void set_parent(Dir* dir, Dir* parent_dir)
{
    // SETS PARENT AND NEW ABSOLUTE_PATH
    // abs_path always ends with /
    if (dir -> absolute_path)
    {
        free(dir -> absolute_path);
    }
    dir -> parent_dir = parent_dir;
    dir -> absolute_path = malloc(sizeof(char) * (strlen(parent_dir -> absolute_path) + strlen(dir -> name) + 2));
    strcpy(dir -> absolute_path, parent_dir -> absolute_path);
    strcat(dir -> absolute_path, "/");
    strcat(dir -> absolute_path, dir -> name);
}

void insert_dir(Dir* dir, Dir* subdir){
    if (dir -> n_subdirs % FIXED_SIZE == 0) {
        dir -> subdirs = realloc(dir -> subdirs, (dir -> n_subdirs/FIXED_SIZE + 1) * sizeof(Dir*));
    }
    dir -> subdirs[dir -> n_subdirs] = subdir;
    // Set parent and absolute_path
    set_parent(subdir, dir);
    dir -> n_subdirs++;
}

void update_absolute_path(Dir* directory, char* new_parent_absolute_path, uint32_t* simdisk, int* current_accesses, FILE* access_file)
{
    if (directory -> absolute_path)
    {
        free(directory -> absolute_path);
    }
    directory -> absolute_path = malloc(sizeof(char) * (strlen(new_parent_absolute_path) + strlen(directory -> name) + 2));
    strcpy(directory -> absolute_path, new_parent_absolute_path);
    strcat(directory -> absolute_path, "/");
    strcat(directory -> absolute_path, directory -> name);

    fprintf(access_file, "%s.txt\n", directory->absolute_path);
    simdisk[directory -> mem_dir] = ((*current_accesses << 8) | get_metadata(simdisk[directory -> mem_dir]));
    *current_accesses +=1;

    for (int subd_index = 0; subd_index < directory -> n_subdirs; ++subd_index)
    {
        update_absolute_path(directory -> subdirs[subd_index], directory -> absolute_path, simdisk, current_accesses, access_file);
    }
}

void remove_dir(Dir* dir, Dir* subdir_to_remove){
    Dir* last_inserted = dir -> subdirs[dir -> n_subdirs - 1];
    if (last_inserted == subdir_to_remove)
    {
        dir -> subdirs[dir -> n_subdirs - 1] = NULL;
    }
    else
    {
        for (int index = 0; index < dir -> n_subdirs; ++index)
        {
            if (dir -> subdirs[index] == subdir_to_remove) dir -> subdirs[index] = last_inserted;
        }
    }
    dir ->n_subdirs--;
}

void recursive_release(uint32_t* simdisk, uint32_t block_index){
    uint32_t block_pointer = get_pointer(simdisk[block_index]);
    simdisk[block_index] = FREE_BLOCK;
    if (block_pointer != ENDOFFILE)
    {
        recursive_release(simdisk, block_pointer);
    }
}

int find_number_of_block_of_line(uint32_t number, int* remaining_till_full){
    int counter = 0;
    if (number < 4046)
    {
        *remaining_till_full = 4046 - number;
        return 0;
    }
    number -= 4046;
    counter++;
    while (number > 4096)
    {
        number -= 4096;
        counter++;
    }
    *remaining_till_full = 4096 - number;
    return counter;
}

void release_block_trail(File* file_to_release, uint32_t* simdisk){
    // TO DO
    uint32_t block_index = file_to_release -> mem_dir;
    recursive_release(simdisk, block_index);
}

uint32_t recursive_get_last_block_index(uint32_t* simdisk, uint32_t block_index){
    uint32_t block_pointer = get_pointer(simdisk[block_index]);
    if (block_pointer != ENDOFFILE)
    {
        return recursive_get_last_block_index(simdisk, block_pointer);
    }
    else
    {
        return block_index;
    }
}

uint32_t get_last_block_index(File* file, uint32_t* simdisk){
    uint32_t block_index = file -> mem_dir;
    return recursive_get_last_block_index(simdisk, block_index);
}

void delete_release_and_destroy_dir(Dir* dir_to_rm, uint32_t* simdisk)
{
    simdisk[dir_to_rm -> mem_dir] = FREE_BLOCK;
    for (int subdir_index = 0; subdir_index < dir_to_rm -> n_subdirs; ++subdir_index)
    {
        delete_release_and_destroy_dir(dir_to_rm -> subdirs[subdir_index], simdisk);
    }
    for (int file_index = 0; file_index < dir_to_rm -> n_files; ++file_index)
    {
        release_block_trail(dir_to_rm -> files[file_index], simdisk);
        destroy_file(dir_to_rm -> files[file_index]);
    }
    free(dir_to_rm -> name);
    free(dir_to_rm -> absolute_path);
    free(dir_to_rm -> subdirs);
    free(dir_to_rm -> files);
    free(dir_to_rm);
}

void remove_file(Dir* dir, File* file_to_remove){
    File* last_inserted = dir -> files[dir -> n_files - 1];
    if (last_inserted == file_to_remove)
    {
        dir -> files[dir -> n_files - 1] = NULL;
    }
    else
    {
        for (int index = 0; index < dir -> n_files; ++index)
        {
            if (dir -> files[index] == file_to_remove) dir -> files[index] = last_inserted;
        }
    }
    dir ->n_files--;
}

void insert_file(Dir* dir, File* file){
    if (dir -> n_files % FIXED_SIZE == 0) {
        dir -> files = realloc(dir -> files, (dir -> n_files/FIXED_SIZE + 1) * sizeof(File*));
    }
    dir -> files[dir -> n_files] = file;
    dir -> n_files++;
}

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

    if (strcmp(path, ".") == 0)
    {
        return actual_dir;
    }

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

void save_create_dir(Dir* dir){
    FILE *output;
    char* file_name = malloc(sizeof(char) * (strlen(dir -> name) + 4));
    sprintf(file_name, "%s/%s.txt", dir -> absolute_path, dir -> name);
    mkdir(dir -> absolute_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    output = fopen(file_name, "w");
    for (int i = 0; i < dir -> n_files; ++i){
        File* ifile = dir -> files[i];
        fprintf(output, "%s,%u,%u\n", ifile -> name, ifile -> mem_dir, ifile -> memory_used);
    }
    fclose(output);
    free(file_name);
    // recursive part
    for (int i = 0; i < dir -> n_subdirs; ++i){
        save_create_dir(dir -> subdirs[i]);
    }

}


