#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

bool executing_child = false;


char *str_replace(char *orig, char *rep, char *with) {
    // str_replace function from: http://stackoverflow.com/a/779960
    char *result;
    char *ins;
    char *tmp;
    int len_rep;
    int len_with;
    int len_front;
    int count;

    if (!orig || !rep) return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0) return NULL;
    len_with = strlen(with);
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result) return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep;
    }
    strcpy(tmp, orig);
    return result;
}


// SIGNAL HANDLER, exit child if exists (on foreground)
// else exit himself (which will exit all child background processes too)
void handler(int foo){
    if (!executing_child) exit(0);
    else printf("\n");
}


void main_loop() {
    // bool pipe = false;
    signal(SIGINT, handler);
    char *input = NULL;
    size_t buffer_size = 0;
    bool using_shell = true;
    char *PROMPT = "փ:"; // prompt default
    int status = 0; // parte por defecto el exit_code del ultimo como 0

    while (using_shell) {
        printf("%s ", PROMPT);

        // se lee la linea ingresada por el usuario, y en caso de ser:
        // 1) exit: se sale. 2) nada: se continua.
        getline(&input, &buffer_size, stdin);
        if (!strcmp(input, "\n")) continue;
        if (!strcmp(input, "exit\n")) using_shell = false;

        // se splitean los argumentos para pasar al comando
        int tok_buffer_size = 128;
        int i = 0;
        char *tok;
        char **tokens = malloc(tok_buffer_size * sizeof(char*));
        tok = strtok(input, " \n");
        while(tok) {
            tokens[i++] = tok;
            if (i >= tok_buffer_size) {
                tok_buffer_size += 128;
                tokens = realloc(tokens, tok_buffer_size * sizeof(char*));
            }
            tok = strtok(NULL, " \n");
        }
        tokens[i] = NULL;

        // progresos para el bonus (la idea era leer del stdout y hacer fork, cuando hubiera pipe)
        // int j;
        // for (j = 0; j < i - 1; ++j) {
        //     if (!strcmp(tokens[j], "|")){
        //         pipe = true;
        //         break;
        //     }
        // }

        // if (pipe) {
        //     char **comando1 = malloc((j) * sizeof(char*));
        //     char **comando2 = malloc((i - j) * sizeof(char*));
        //     for (int k = 0; k < j + 1; ++k){
        //         comando1[k] = tokens[k];
        //     }
        //     for (int k = j + 1; k < i; ++k)
        //     {
        //         comando2[k] = tokens[k];
        //     }
        //     printf("%s\n", comando2[i-1]);
        //     printf("%s\n", comando1[j-1]);
        // }

        // si el comando es setprompt se cambia el prompt segun las condiciones
        if (!strcmp(tokens[0], "setPrompt")) {
            if (strstr(tokens[1], "*") != NULL) {
                char *st;
                sprintf(st, "%d", status);
                tokens[1] = str_replace(tokens[1], "*", st);
            };
            PROMPT = tokens[1];
            continue;
        }

        // si se manda un "&" se elimina ese & de los argumentos
        bool on_background = false;
        if (!strcmp(tokens[i-1], "&")){
            tokens[i-1] = NULL;
            on_background = true;
        }

        // y luego al hacer fork se waitea dependiendo dependiendo de si es
        // o no un proceso del background
        int id_forked_proccess = fork();
        if (!id_forked_proccess){
            if (execvp(tokens[0], tokens) == -1){
                exit(-1);
            }
        }
        else {
            if (on_background){
                printf("%s executing on background\n", tokens[0]);
            }
            else {
                executing_child = true;
                waitpid(id_forked_proccess, &status, 0);
                executing_child = false;
            }
        }

    }
    exit(0);
}


int main(int argc, char** argv) {

    main_loop();

    return 0;
}