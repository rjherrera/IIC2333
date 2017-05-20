#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SEED 1
#define ACTIONS 2
#define UNTILL 4
#define DISK 8


typedef struct archivo
{
    char* name;
    char subdirs;
    int memory_used;
} MyFile;

typedef struct directory
{
    char* absolute_path;
    char* name;
    struct directory** subdirs;
    MyFile** files;
} Dir;



char* select_random_root(int seed)
{

}

// void generate_new_instruction(int seed)
// {
//     srand(seed);
//     char* instructions[5] = {
//         "cd"

//     };
//     int index = rand();
// }

int main(int argc, char** argv)
{
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

    if (acum_flags & DISK)
    {
        // load disk
    }
    else
    {
        // create_disk
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
        int executed_instructions = 0;
        while (executed_instructions < steps && fscanf(action_file, "%s\n", key) == 1)
        {
            if (strcmp(key, "cd") == 0)
            {
                fscanf(action_file, "%s\n", buff);

            }
            else if (strcmp(key, "mkdir") == 0)
            {
                fscanf(action_file, "%s\n", buff);
                // char * strtok ( char * str, const char * delimiters );
                char* path_piece = strtok(buff,"/");
                if (path_piece == NULL)
                {
                    printf("File name is just %s\n",buff);
                }
                char last[100];
                while (path_piece != NULL)
                {
                    strcpy(last,path_piece);
                    path_piece = strtok(NULL,"/");
                    if (path_piece != NULL)
                    {
                        printf("This piece is %s\n", path_piece);
                    }
                }
                printf("Dir made has name %s\n",last);

            }
            else if (strcmp(key, "mkfile") == 0)
            {
                
            }
            else if (strcmp(key, "mv") == 0)
            {
                
            }
            else if (strcmp(key, "rm") == 0)
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
            executed_instructions++;
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