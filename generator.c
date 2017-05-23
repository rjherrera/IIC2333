#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

char* create_instructions[2] = {"mkdir", "mkfile"};
char* modify_instructions[5] = {"mv", "rm", "ad", "rd"};
static char abc[] = "abcdefghijklmnopqrstuvwxyz";
static char num[] = "0123456789";


char** generate_random_files(int amount){
    int random_index = 1; // minimo 1 creacion
    char** inst = malloc(sizeof(char*) * amount);
    for (int i = 0; i < amount; ++i){
        char* command = create_instructions[random_index];
        if (random_index){ // MKFILE
            int instruction_length = 12 + (rand() % 7); // largo minimo nombre archivo: 12 - 7 = 5, largo maximo 12 - 7 + 7 = 12;
            char* instruction = malloc(sizeof(char) * (instruction_length + 5));
            for (int j = 0; j < instruction_length + 4; ++j){
                if (j < 6) instruction[j] = command[j];
                else if (j == 6) instruction[j] = ' ';
                else if (j > 6 && j < instruction_length) instruction[j] = abc[rand() % 26];
                else if (j == instruction_length) instruction[j] = '.';
                else instruction[j] = abc[rand() % 26];
            }
            instruction[instruction_length + 4] = '\0';
            inst[i] = instruction;
        }
        else { // MKDIR
            int instruction_length = 11 + (rand() % 7); // largo minimo nombre archivo: 11 - 6 = 5, largo maximo 11 - 6 + 7 = 12;
            char* instruction = malloc(sizeof(char) * (instruction_length + 1));
            for (int j = 0; j < instruction_length; ++j){
                if (j < 5) instruction[j] = command[j];
                else if (j == 5) instruction[j] = ' ';
                else instruction[j] = abc[rand() % 26];
            }
            instruction[instruction_length] = '\0';
            inst[i] = instruction;
        }
        random_index = rand() % 2;
    }
    return inst;
}


char** modify_random_files(char** files, int n_files, int amount){
    char** inst = malloc(sizeof(char*) * amount);
    for (int i = 0; i < amount; ++i){
        int random_index = rand() % 4;
        char* command = modify_instructions[random_index];
        char* file = files[rand() % n_files];
        if (!random_index){ // MV
            int old_file_length = strlen(file);
            int new_file_length = 6 + rand() % 7;
            int instruction_length = 4 + old_file_length + new_file_length;
            char* instruction = malloc(sizeof(char) * (instruction_length + 5));
            for (int j = 0; j < instruction_length + 4; ++j){
                if (j < 2) instruction[j] = command[j];
                else if (j == 2) instruction[j] = ' ';
                else if (j > 2 && j < old_file_length + 3) instruction[j] = file[j - 3];
                else if (j == old_file_length + 3) instruction[j] = ' ';
                else if (j > old_file_length + 3 && j < instruction_length) instruction[j] = abc[rand() % 26];
                else if (j == instruction_length) instruction[j] = '.';
                else instruction[j] = abc[rand() % 26];
            }
            instruction[instruction_length + 4] = '\0';
            inst[i] = instruction;
        }
        else if (random_index == 1){ // RM
            int old_file_length = strlen(file);
            int instruction_length = 3 + old_file_length;
            char* instruction = malloc(sizeof(char) * (instruction_length + 1));
            for (int j = 0; j < instruction_length; ++j){
                if (j < 2) instruction[j] = command[j];
                else if (j == 2) instruction[j] = ' ';
                else instruction[j] = file[j - 3];
            }
            instruction[instruction_length + 1] = '\0';
            inst[i] = instruction;
        }
        else if (random_index > 1){ // AD & RD
            int old_file_length = strlen(file);
            int instruction_length = 7 + old_file_length;
            char* instruction = malloc(sizeof(char) * (instruction_length + 1));
            for (int j = 0; j < instruction_length; ++j){
                if (j < 2) instruction[j] = command[j];
                else if (j == 2) instruction[j] = ' ';
                else if (j > 2 && j < old_file_length + 3) instruction[j] = file[j - 3];
                else if (j == old_file_length + 3) instruction[j] = ' ';
                else if (j > old_file_length + 3 && j < old_file_length + 5) instruction[j] = num[rand() % 10];
                else if (j == old_file_length + 5) instruction[j] = ' ';
                else if (j > old_file_length + 5) instruction[j] = num[rand() % 10];
            }
            instruction[instruction_length + 1] = '\0';
            inst[i] = instruction;
        }
    }
    return inst;
}

char** generate_random_instructions(seed, amount){
    srand(seed);
    int generations = 1 + rand() % amount; // minimo 1 generacion
    int modifications = amount - generations;
    // generar las instrucciones de creacion de archivos/directorios
    char** gens = generate_random_files(generations);
    // printf("Generaciones: %d, Modificaciones: %d\n", generations, modifications);
    // generar una lista de los filenames (de las instrucciones provenientes de mkfile y no de mkdir)
    char** files = malloc(sizeof(char*) * generations);
    int mkfiles = 0;
    for (int i = 0; i < generations; ++i){
        // printf("f[%d]: %s\n", i, gens[i]);
        if (!strncmp("mkfile", gens[i], 6)){
            char* file_name = malloc(sizeof(char) * (strlen(gens[i]) - 6));
            for (int j = 7; j < strlen(gens[i]) + 1; ++j) file_name[j - 7] = gens[i][j];
            files[mkfiles++] = file_name;
        }
    }
    // imprimir los q fueron filenames
    // for (int i = 0; i < mkfiles; ++i) printf("filename: %s\n", files[i]);
    // generar las instrucciones de modificacion de archivos a partir de los archivos creados
    char** mods = modify_random_files(files, mkfiles, modifications);
    char** instructions = malloc(sizeof(char*) * amount);
    for (int i = 0; i < generations; ++i) instructions[i] = gens[i];
    for (int i = 0; i < modifications; ++i) instructions[i + generations] = mods[i];
    return instructions;
}

// int main(int argc, char** argv){
//     int seed = atoi(argv[1]);
//     printf("Seed: %d\n", seed);
//     char** hola = generate_random_instructions(seed, 10);
//     for (int i = 0; i < 10; ++i){
//         printf("inst[%d]: %s\n", i, hola[i]);
//     }
//     return 0;
// }