#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "connections.c"
#include "ships.c"

char* CLIENT_FILE_NAME = "stateC.btsp";
char* SERVER_FILE_NAME = "stateS.btsp";
char* ip_address = "0.0.0.0";
int PORT = 12345;
int is_server = 0;


void write_attack(char* file_name, int x, int y, int attacks_amount){
    FILE* file_out = fopen(file_name, "r+");
    size_t bufsize = 32;
    char *buffer = malloc(bufsize * sizeof(char));
    fseek(file_out, 28, SEEK_SET);
    getline(&buffer, &bufsize, file_out);
    int** attacks_already_made = malloc(sizeof(int*) * (attacks_amount - 1));
    for (int i = 0; i < attacks_amount - 1; ++i){
        int old_x;
        int old_y;
        fscanf(file_out, "%d,%d", &old_x, &old_y);
        attacks_already_made[i] = malloc(2 * sizeof(int));
        attacks_already_made[i][0] = old_x;
        attacks_already_made[i][1] = old_y;
    }
    fseek(file_out, 28, SEEK_SET);
    fprintf(file_out, "%d\n", attacks_amount);
    for (int i = 0; i < attacks_amount - 1; ++i){
        int old_x;
        int old_y;
        old_x = attacks_already_made[i][0];
        old_y = attacks_already_made[i][1];
        fprintf(file_out, "%d,%d\n", old_x, old_y);
    }

    fprintf(file_out, "%d,%d\n", x, y);
    fclose(file_out);
}


void write_state(char* file_name, int state){
    FILE* file_out = fopen(file_name, "r+");
    fseek(file_out, 0, SEEK_SET);
    fprintf(file_out, "%d", state);
    fclose(file_out);
}


int parse_arguments(int argc, char* argv[]){
    if (argc == 4 || argc == 6){
        if (!strcmp(argv[1], "-l")){
            is_server = 1;
            if (!strcmp(argv[2], "-i")){
                ip_address = argv[3];
                PORT = atoi(argv[5]);
            }
            else if (!strcmp(argv[2], "-p")){
                PORT = atoi(argv[3]);
            }
        }
    }
    else {
        if (!strcmp(argv[1], "-i")){
            ip_address = argv[2];
            PORT = atoi(argv[4]);
        }
    }
    return is_server;
}


void exit_handler(int foo){
    printf("Exiting battleship correctly\n");
    write_state(CLIENT_FILE_NAME, 1);
    write_state(SERVER_FILE_NAME, 1);
    exit(0);
}


int main( int argc, char* argv[] )
{
    signal(SIGTSTP, exit_handler);
    if(argc != 4 && argc != 5 && argc != 6)
    {
        printf("Modo de uso: %s -l -i <ip_address> -p <tcp_port>\n", argv[0]);
        printf("\t -l es opcional e indica si se desea ejecutar como servidor.\n");
        printf("\t -i ip_address: IP donde se recibiran conexiones entrantes si es modo servidor (opcional). Si es modo cliente, es el IP con que se desea conectar (obligtorio).\n");
        printf("\t -p tcp_port: Puerto donde se recibiran conexiones entrantes si es modo servidor. Si es modo cliente, es el puerto con que se desea conectar.\n");
        return 1;
    }

    is_server = parse_arguments(argc, argv);


    // Initialize self grid (Choose boats and positions)
    Cell*** grid = calloc(10, sizeof(Cell**));
    for (int row = 0; row < 10; ++row)
    {
        grid[row] = calloc(10,sizeof(Cell*));
        for (int col = 0; col < 10; ++col)
        {
            grid[row][col] = cell_init();
        }
    }

    // Initialize ships array
    Ship** my_ships = malloc(sizeof(Ship*)*3);
    int ships_remaining = 3;

    // Create enemy grid
    char** enemy_grid = calloc(10,sizeof(char*));
    for (int row = 0; row < 10; ++row)
    {
        enemy_grid[row] = calloc(10, sizeof(char));
        for (int col = 0; col < 10; ++col)
        {
            enemy_grid[row][col] = ' ';
        }
    }


    // Check if previous game was interrupted, if it was, load the information from de stateX.btsp file
    // which contains 1) the ships and their positions, 2) the attacks made
    FILE* file;
    char* file_name;
    if (!is_server){
        file_name = CLIENT_FILE_NAME;
    }
    else {
        file_name = SERVER_FILE_NAME;
    }
    int state = 1;  // 1 means that it wasn't closed unexpectedly
    int ship_type;
    int ship_x;
    int ship_y;
    int ship_orientation;
    int file_exists = access(file_name, F_OK);
    int make_attacks = 0;
    int should_start = -1;
    // if file exists and the state found on the file is 0, then set the make attacks variable to 1
    if (file_exists != -1){
        file = fopen(file_name, "r");
        fscanf(file, "%d", &state);
        make_attacks = 1;
        if (!state){
            fscanf(file, "%d", &should_start);
            printf("Recovered turn: %s\n", should_start ? "me": "other");
        }
    }

    // if state != 0 nor file exists (which means we dont need to recover) ask for each ship (standard logic)
    if (state || file_exists == -1){
        // write state to 0 since we are in the middle of sth
        FILE* file_out;
        file_out = fopen(file_name, "w");
        fprintf(file_out, "0\n0\n");
        fclose(file_out);
        // close file so that it isnt opened while deciding where to put my ships
        for (int index = 0; index < 3; ++index){
            int x_out, y_out;
            print_grid(grid);
            my_ships[index] = choose_ship(index);
            choose_position(my_ships[index], grid, &x_out, &y_out);
            // write file at each positioning (for the state file)
            file_out = fopen(file_name, "a");
            fprintf(file_out, "%d,%d,%d,%d\n", my_ships[index] -> external_id, x_out, y_out, my_ships[index] -> is_vertical);
            fclose(file_out);
            // end of write file
        }
        // write the amount of attacks yet which is 0;
        file_out = fopen(file_name, "a");
        fprintf(file_out, "0\n");
        fclose(file_out);
    }

    // set ships positions from the file
    else {
        for (int i = 0; i < 3; ++i){
            fscanf(file, "%d,%d,%d,%d", &ship_type, &ship_x, &ship_y, &ship_orientation);
            if (ship_type == 1) my_ships[i] = carrier_init(ship_orientation, i);
            else if (ship_type == 2) my_ships[i] = battleship_init(ship_orientation, i);
            else if (ship_type == 3) my_ships[i] = cruiser_init(ship_orientation, i);
            else if (ship_type == 4) my_ships[i] = submarine_init(ship_orientation, i);
            else my_ships[i] = destroyer_init(ship_orientation, i);
            place_ship(grid, my_ships[i], ship_x, ship_y);
        }
    }
    // Gets attacks from the file
    int attacks_amount = 0;
    int** pending_moves;
    if (make_attacks){
        size_t bufsize = 32;
        char *buffer = malloc(bufsize * sizeof(char));
        fseek(file, 28, SEEK_SET);
        getline(&buffer, &bufsize, file);
        attacks_amount = atoi(buffer);
        int x, y;
        pending_moves = malloc(attacks_amount * sizeof(int*));
        for (int i = 0; i < attacks_amount; ++i){
            fscanf(file, "%d,%d", &x, &y);
            pending_moves[i] = malloc(2 * sizeof(int));
            pending_moves[i][0] = x;
            pending_moves[i][1] = y;
        }
    }

    fclose(file);
    // END OF RESTORE PREVIOUS LOGIC

    // set state to 0 to remind that we not ready (checkpoint)
    write_state(file_name, 0);

    // Prepare for Sockets Configuration
    print_grid(grid);

    int sock;
    socklen_t len;
    struct sockaddr_in name;
    int send_to_socket;
    int reuse = 1;
    name.sin_family = AF_INET;
    char recv_buffer[1024];
    char process_buffer[32];
    char processed_message_length;
    srand(time(NULL));
    int my_random_number;
    int other_random_number;
    int wait_for_op_new_move;


    if (!is_server)
    {
        // Initialize sockets and connection with server
        send_to_socket = socket( AF_INET, SOCK_STREAM, 0);
        setsockopt(send_to_socket ,SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof (reuse));
        name.sin_port = htons(PORT);
        inet_pton(AF_INET, ip_address, &name.sin_addr.s_addr);

        len = sizeof( struct sockaddr_in );

        connect(send_to_socket, (struct sockaddr*) &name, len);

        printf("Connected as Client!\n");


        int same_numbers=1;

        // send start
        while (same_numbers && should_start == -1)
        {
            my_random_number = send_start_message(send_to_socket);
            processed_message_length = receive_message(send_to_socket, recv_buffer, process_buffer);
            memcpy(&other_random_number, process_buffer+1, 4); // this case we know its start_msg

            printf("My number is %d and i received number %d\n", my_random_number, other_random_number);
            if (my_random_number != other_random_number) same_numbers = 0;
        }

        // the next lines for stating who should start considering a previously saved game
        if (should_start == -1){
            FILE* file_out = fopen(file_name, "r+");
            fseek(file_out, 2, SEEK_SET);
            fprintf(file_out, "%d\n", my_random_number > other_random_number ? 1 : 0);
            fclose(file_out);
        }
        if (should_start == 1){
            my_random_number = 1;
            other_random_number = 0;
        }
        else if (should_start == 0){
            my_random_number = 0;
            other_random_number = 1;
        }

        // START GAME
        int game_finished = 0;

        // COMPRESSED LOGIC FOR PREVIOUSLY SAVED ATTACKS
        // If there are attacks to make
                if (make_attacks){
            // Make them and recieve answer (interleaved)
            if (my_random_number > other_random_number){

                for (int i = 0; i < attacks_amount; ++i){
                    int x = pending_moves[i][0];
                    int y = pending_moves[i][1];
                    int send_x, send_y;
                    send_x = (char) x;
                    send_y = (char) y;
                    send_new_move_message(send_to_socket, send_x, send_y);
                    mark_sent_attack(enemy_grid, x, y);
                    receive_message(send_to_socket, recv_buffer, process_buffer);
                    if (process_buffer[0] == 12){
                        int x_attacked = (int) process_buffer[1];
                        int y_attacked = (int) process_buffer[2];
                        if (receive_attack(grid, my_ships, x_attacked, y_attacked)){
                            ships_remaining--;
                            send_destroyed_message(send_to_socket, ships_remaining);
                        }
                    }
                    memset(process_buffer, 0, 32);
                }
                print_both_grids(grid, enemy_grid);
                printf("Recovered from last saved state\n");
            }
            else {
                receive_message(send_to_socket, recv_buffer, process_buffer);
                if (process_buffer[0] == 12){
                    int x_attacked = (int) process_buffer[1];
                    int y_attacked = (int) process_buffer[2];
                    if (receive_attack(grid, my_ships, x_attacked, y_attacked)){
                        ships_remaining--;
                        send_destroyed_message(send_to_socket, ships_remaining);
                    }
                }
                for (int i = 0; i < attacks_amount; ++i){
                    memset(process_buffer, 0, 32);
                    int x = pending_moves[i][0];
                    int y = pending_moves[i][1];
                    int send_x, send_y;
                    send_x = (char) x;
                    send_y = (char) y;
                    send_new_move_message(send_to_socket, send_x, send_y);
                    mark_sent_attack(enemy_grid, x, y);
                    if (i == attacks_amount - 1) break;
                    receive_message(send_to_socket, recv_buffer, process_buffer);
                    if (process_buffer[0] == 12){
                        int x_attacked = (int) process_buffer[1];
                        int y_attacked = (int) process_buffer[2];
                        if (receive_attack(grid, my_ships, x_attacked, y_attacked)){
                            ships_remaining--;
                            send_destroyed_message(send_to_socket, ships_remaining);
                        }
                    }
                }
                print_both_grids(grid, enemy_grid);
                printf("Recovered from last saved state\n");
            }
        }


        // See Who starts
        if (my_random_number > other_random_number)
        {
            // i start

            while (!game_finished)
            {
                // Make a move and send message
                if (!wait_for_op_new_move)
                {
                    fpurge(stdin);
                    print_both_grids(grid, enemy_grid);
                    printf("Choose bomb direction (x, y) in format: 'x y' \n");
                    char tmp[30];
                    int x, y;
                    char send_x, send_y;
                    fgets(tmp, 30, stdin);
                    sscanf(tmp, "%d %d",&x, &y);

                    int incorrect = 0;
                    if (x < 0 || y < 0 || x > 9 || y > 9)
                    {
                        incorrect = 1;
                        printf("Position out of bounds.(0 <= x <= 9) and (0 <= y <= 9).Try again.\n");
                    }
                    while (incorrect)
                    {
                        incorrect = 0;
                        fgets(tmp, 30, stdin);
                        sscanf(tmp, "%d %d",&x, &y);

                        if (x < 0 || y < 0 || x > 9 || y > 9)
                        {
                            incorrect = 1;
                            printf("Position out of bounds.(0 <= x <= 9) and (0 <= y <= 9).Try again.\n");
                        }
                    }

                    send_x = (char) x;
                    send_y = (char) y;

                    // Send New Move
                    if (send_new_move_message(send_to_socket, send_x, send_y) == -1){
                        printf("Opponent's connection lost. You have won the game!\n");
                        break;
                    }

                    mark_sent_attack(enemy_grid, x, y);
                    print_both_grids(grid, enemy_grid);
                    // rewrite in file the amount of attacks and append the attack
                    attacks_amount++;
                    write_attack(file_name, x, y, attacks_amount);
                }

                // Wait for Response
                printf("Wait for opponents turn\n");
                if ((processed_message_length = receive_message(send_to_socket, recv_buffer, process_buffer) == -1))
                {
                    printf("Opponent's connection lost. You have won the game!\n");
                    break;
                }
                // Process Opponents Response

                if (process_buffer[0] == 12)
                {
                    wait_for_op_new_move = 0;
                    int x_attacked = (int) process_buffer[1];
                    int y_attacked = (int) process_buffer[2];

                     if (receive_attack(grid, my_ships, x_attacked, y_attacked))
                     {
                        // ship destroyed!
                        printf("Enemy has destroyed your ship with attack at (%d, %d)\n", x_attacked, y_attacked);
                        ships_remaining--;
                        send_destroyed_message(send_to_socket, ships_remaining);
                        if (ships_remaining == 0)
                        {
                            game_finished = 1;
                            printf("You have lost! Thank you for playing! \n");
                            break;
                        }
                     }
                     else
                     {
                        printf("Enemy has attacked at (%d, %d)\n", x_attacked, y_attacked);
                     }

                }
                else if (process_buffer[0] == 13)
                {
                    wait_for_op_new_move = 1;
                    int enemy_ships_remaining = (int) process_buffer[1];
                    if (enemy_ships_remaining == 0)
                    {
                        printf("Your attack has sunk the last enemy's ship! You have won the game! Congratulations!\n");
                        write_state(file_name, 1);
                        break;
                    }
                    else
                    {
                        printf("Your attack has sunk one of the enemy's ships! Enemy ships remaining: %d\n", enemy_ships_remaining);
                    }
                }
                else
                {
                    printf("Mensaje recibido no cumple con protocolo. Mensaje ignorado.\n");
                }

                memset(process_buffer, 0, 32);
            }
        }
        else
        {
            // other starts
            while (!game_finished)
            {
                // Wait for Response
                printf("Wait for opponents turn\n");
                if ((processed_message_length = receive_message(send_to_socket, recv_buffer, process_buffer) == -1))
                {
                    printf("Opponent's connection lost. You have won the game!\n");
                    break;
                }

                // Process Opponent Response

                if (process_buffer[0] == 12)
                {
                    wait_for_op_new_move = 0;
                    int x_attacked = (int) process_buffer[1];
                    int y_attacked = (int) process_buffer[2];

                     if (receive_attack(grid, my_ships, x_attacked, y_attacked))
                     {
                        // ship destroyed!
                        printf("Enemy has destroyed your ship with attack at (%d, %d)\n", x_attacked, y_attacked);
                        ships_remaining--;
                        send_destroyed_message(send_to_socket, ships_remaining);
                        if (ships_remaining == 0)
                        {
                            game_finished = 1;
                            printf("You have lost! Thank you for playing! \n");
                            break;
                        }
                     }
                     else
                     {
                        printf("Enemy has attacked at (%d, %d)\n", x_attacked, y_attacked);
                     }

                }
                else if (process_buffer[0] == 13)
                {
                    wait_for_op_new_move = 1;
                    int enemy_ships_remaining = (int) process_buffer[1];
                    if (enemy_ships_remaining == 0)
                    {
                        printf("Your attack has sunk the last enemy's ship! You have won the game! Congratulations!\n");
                        write_state(file_name, 1);
                        break;
                    }
                    else
                    {
                        printf("Your attack has sunk one of the enemy's ships! Enemy ships remaining: %d\n", enemy_ships_remaining);
                    }
                }
                else
                {
                    printf("Mensaje recibido no cumple con protocolo. Mensaje ignorado.\n");
                }

                memset(process_buffer, 0, 32);

                // Make a move and send message
                fpurge(stdin);
                print_both_grids(grid, enemy_grid);
                printf("Choose bomb direction (x, y) in format: 'x y' \n");
                char tmp[30];
                int x, y;
                char send_x, send_y;
                fgets(tmp, 30, stdin);
                sscanf(tmp, "%d %d",&x, &y);

                int incorrect = 0;
                if (x < 0 || y < 0 || x > 9 || y > 9)
                {
                    incorrect = 1;
                    printf("Position out of bounds.(0 <= x <= 9) and (0 <= y <= 9).Try again.\n");
                }
                while (incorrect)
                {
                    incorrect = 0;
                    fgets(tmp, 30, stdin);
                    sscanf(tmp, "%d %d",&x, &y);

                    if (x < 0 || y < 0 || x > 9 || y > 9)
                    {
                        incorrect = 1;
                        printf("Position out of bounds.(0 <= x <= 9) and (0 <= y <= 9).Try again.\n");
                    }
                }

                send_x = (char) x;
                send_y = (char) y;

                // Send New Move
                if (send_new_move_message(send_to_socket, send_x, send_y) == -1){
                    printf("Opponent's connection lost. You have won the game!\n");
                    break;
                }

                mark_sent_attack(enemy_grid, x, y);
                print_both_grids(grid, enemy_grid);
                // rewrite in file the amount of attacks and append the attack
                attacks_amount++;
                write_attack(file_name, x, y, attacks_amount);

            }

        }
    }
    else // YOU ARE SERVER
    {
        // Initialize sockets and connection with client
        sock = socket( AF_INET, SOCK_STREAM, 0);
        setsockopt(sock ,SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof (reuse));
        name.sin_port = htons(PORT);

        if (!strcmp(ip_address, "0.0.0.0")) name.sin_addr.s_addr = htonl(INADDR_ANY);
        else name.sin_addr.s_addr = inet_addr( ip_address );

        len = sizeof(struct sockaddr_in);

        if( bind( sock, (struct sockaddr *) &name, len ) )
        {
          printf( "Bind error\n" );
          return 1;
        }
        printf("Connected as Server!\n");

        listen(sock,1);

        send_to_socket = accept( sock, (struct sockaddr*) &name, &len);

        printf( "Connection from : %s\n", inet_ntoa( name.sin_addr ) );

        close(sock);

        int same_numbers=1;

        // send start
        while (same_numbers && should_start == -1)
        {
            my_random_number = send_start_message(send_to_socket);
            processed_message_length = receive_message(send_to_socket, recv_buffer, process_buffer);
            memcpy(&other_random_number, process_buffer+1, 4); // this case we know its start msg

            printf("My number is %d and i received number %d\n", my_random_number, other_random_number);
            if (my_random_number != other_random_number) same_numbers = 0;
        }

        if (should_start == -1){
            FILE* file_out = fopen(file_name, "r+");
            fseek(file_out, 2, SEEK_SET);
            fprintf(file_out, "%d\n", my_random_number > other_random_number ? 1 : 0);
            fclose(file_out);
        }
        if (should_start == 1){
            my_random_number = 1;
            other_random_number = 0;
        }
        else if (should_start == 0){
            my_random_number = 0;
            other_random_number = 1;
        }

        // START GAME
        int game_finished = 0;
        // COMPRESSED LOGIC FOR PREVIOUSLY SAVED ATTACKS
        // IF THERE ARE ATTACKS TO MAKE
        if (make_attacks){
            // Make them and recieve answer (interleaved)
            if (my_random_number > other_random_number){

                for (int i = 0; i < attacks_amount; ++i){
                    int x = pending_moves[i][0];
                    int y = pending_moves[i][1];
                    int send_x, send_y;
                    send_x = (char) x;
                    send_y = (char) y;
                    send_new_move_message(send_to_socket, send_x, send_y);
                    mark_sent_attack(enemy_grid, x, y);
                    receive_message(send_to_socket, recv_buffer, process_buffer);
                    if (process_buffer[0] == 12){
                        int x_attacked = (int) process_buffer[1];
                        int y_attacked = (int) process_buffer[2];
                        if (receive_attack(grid, my_ships, x_attacked, y_attacked)){
                            ships_remaining--;
                            send_destroyed_message(send_to_socket, ships_remaining);
                        }
                    }
                    memset(process_buffer, 0, 32);
                }
                print_both_grids(grid, enemy_grid);
                printf("Recovered from last saved state\n");
            }
            else {
                receive_message(send_to_socket, recv_buffer, process_buffer);
                if (process_buffer[0] == 12){
                    int x_attacked = (int) process_buffer[1];
                    int y_attacked = (int) process_buffer[2];
                    if (receive_attack(grid, my_ships, x_attacked, y_attacked)){
                        ships_remaining--;
                        send_destroyed_message(send_to_socket, ships_remaining);
                    }
                }
                for (int i = 0; i < attacks_amount; ++i){
                    memset(process_buffer, 0, 32);
                    int x = pending_moves[i][0];
                    int y = pending_moves[i][1];
                    int send_x, send_y;
                    send_x = (char) x;
                    send_y = (char) y;
                    send_new_move_message(send_to_socket, send_x, send_y);
                    mark_sent_attack(enemy_grid, x, y);
                    if (i == attacks_amount - 1) break;
                    receive_message(send_to_socket, recv_buffer, process_buffer);
                    if (process_buffer[0] == 12){
                        int x_attacked = (int) process_buffer[1];
                        int y_attacked = (int) process_buffer[2];
                        if (receive_attack(grid, my_ships, x_attacked, y_attacked)){
                            ships_remaining--;
                            send_destroyed_message(send_to_socket, ships_remaining);
                        }
                    }
                }
                print_both_grids(grid, enemy_grid);
                printf("Recovered from last saved state\n");
            }
        }

        // See Who starts
        if (my_random_number > other_random_number)
        {

            // i start
            while (!game_finished)
            {
                // Make a move and send message
                fpurge(stdin);
                print_both_grids(grid, enemy_grid);
                printf("Choose bomb direction (x, y) in format: 'x y' \n");
                char tmp[30];
                int x, y;
                char send_x, send_y;
                fgets(tmp, 30, stdin);
                sscanf(tmp, "%d %d",&x, &y);

                int incorrect = 0;
                if (x < 0 || y < 0 || x > 9 || y > 9)
                {
                    incorrect = 1;
                    printf("Position out of bounds.(0 <= x <= 9) and (0 <= y <= 9).Try again.\n");
                }
                while (incorrect)
                {
                    incorrect = 0;
                    fgets(tmp, 30, stdin);
                    sscanf(tmp, "%d %d",&x, &y);

                    if (x < 0 || y < 0 || x > 9 || y > 9)
                    {
                        incorrect = 1;
                        printf("Position out of bounds.(0 <= x <= 9) and (0 <= y <= 9).Try again.\n");
                    }
                }

                send_x = (char) x;
                send_y = (char) y;

                // Send New Move
                if (send_new_move_message(send_to_socket, send_x, send_y) == -1){
                    printf("Opponent's connection lost. You have won the game!\n");
                    break;
                }

                mark_sent_attack(enemy_grid, x, y);
                print_both_grids(grid, enemy_grid);
                // rewrite in file the amount of attacks and append the attack
                attacks_amount++;
                write_attack(file_name, x, y, attacks_amount);

                // Wait for Response
                printf("Wait for opponents turn\n");
                if ((processed_message_length = receive_message(send_to_socket, recv_buffer, process_buffer) == -1))
                {
                    printf("Opponent's connection lost. You have won the game!\n");
                    break;
                }
                // Process Opponents Response

                if (process_buffer[0] == 12)
                {
                    wait_for_op_new_move = 0;
                    int x_attacked = (int) process_buffer[1];
                    int y_attacked = (int) process_buffer[2];

                     if (receive_attack(grid, my_ships, x_attacked, y_attacked))
                     {
                        // ship destroyed!
                        printf("Enemy has destroyed your ship with attack at (%d, %d)\n", x_attacked, y_attacked);
                        ships_remaining--;
                        send_destroyed_message(send_to_socket, ships_remaining);
                        if (ships_remaining == 0)
                        {
                            game_finished = 1;
                            write_state(file_name, 1);
                            printf("You have lost! Thank you for playing! \n");
                            break;
                        }
                     }
                     else
                     {
                        printf("Enemy has attacked at (%d, %d)\n", x_attacked, y_attacked);
                     }

                }
                else if (process_buffer[0] == 13)
                {
                    wait_for_op_new_move = 1;
                    int enemy_ships_remaining = (int) process_buffer[1];
                    if (enemy_ships_remaining == 0)
                    {
                        printf("Your attack has sunk the last enemy's ship! You have won the game! Congratulations!\n");
                        break;
                    }
                    else
                    {
                        printf("Your attack has sunk one of the enemy's ships! Enemy ships remaining: %d\n", enemy_ships_remaining);
                    }
                }
                else
                {
                    printf("Mensaje recibido no cumple con protocolo. Mensaje ignorado.\n");
                }

                memset(process_buffer, 0, 32);

            }
        }
        else
        {
            // other starts
            while (!game_finished)
            {

                // Wait for Response
                printf("Wait for opponents turn\n");
                if ((processed_message_length = receive_message(send_to_socket, recv_buffer, process_buffer) == -1))
                {
                    printf("Opponent's connection lost. You have won the game!\n");
                    break;
                }

                // Process Opponent Response

                if (process_buffer[0] == 12)
                {
                    wait_for_op_new_move = 0;
                    int x_attacked = (int) process_buffer[1];
                    int y_attacked = (int) process_buffer[2];

                     if (receive_attack(grid, my_ships, x_attacked, y_attacked))
                     {
                        // ship destroyed!
                        printf("Enemy has destroyed your ship with attack at (%d, %d)\n", x_attacked, y_attacked);
                        ships_remaining--;
                        send_destroyed_message(send_to_socket, ships_remaining);
                        if (ships_remaining == 0)
                        {
                            game_finished = 1;
                            write_state(file_name, 1);
                            printf("You have lost! Thank you for playing! \n");
                            break;
                        }
                     }
                     else
                     {
                        printf("Enemy has attacked at (%d, %d)\n", x_attacked, y_attacked);
                     }

                }
                else if (process_buffer[0] == 13)
                {
                    wait_for_op_new_move = 1;
                    int enemy_ships_remaining = (int) process_buffer[1];
                    if (enemy_ships_remaining == 0)
                    {
                        printf("Your attack has sunk the last enemy's ship! You have won the game! Congratulations!\n");
                        break;
                    }
                    else
                    {
                        printf("Your attack has sunk one of the enemy's ships! Enemy ships remaining: %d\n", enemy_ships_remaining);
                    }
                }
                else
                {
                    printf("Mensaje recibido no cumple con protocolo. Mensaje ignorado.\n");
                }

                memset(process_buffer, 0, 32);


                if (!wait_for_op_new_move)
                {
                    // Make a move and send message
                    fpurge(stdin);
                    print_both_grids(grid, enemy_grid);
                    printf("Choose bomb direction (x, y) in format: 'x y' \n");
                    char tmp[30];
                    int x, y;
                    char send_x, send_y;
                    fgets(tmp, 30, stdin);
                    sscanf(tmp, "%d %d",&x, &y);

                    int incorrect = 0;
                    if (x < 0 || y < 0 || x > 9 || y > 9)
                    {
                        incorrect = 1;
                        printf("Position out of bounds.(0 <= x <= 9) and (0 <= y <= 9).Try again.\n");
                    }
                    while (incorrect)
                    {
                        incorrect = 0;
                        fgets(tmp, 30, stdin);
                        sscanf(tmp, "%d %d",&x, &y);

                        if (x < 0 || y < 0 || x > 9 || y > 9)
                        {
                            incorrect = 1;
                            printf("Position out of bounds.(0 <= x <= 9) and (0 <= y <= 9).Try again.\n");
                        }
                    }

                    send_x = (char) x;
                    send_y = (char) y;

                    // Send New Move
                    if (send_new_move_message(send_to_socket, send_x, send_y) == -1){
                        printf("Opponent's connection lost. You have won the game!\n");
                        break;
                    }

                    mark_sent_attack(enemy_grid, x, y);
                    print_both_grids(grid, enemy_grid);
                    // rewrite in file the amount of attacks and append the attack
                    attacks_amount++;
                    write_attack(file_name, x, y, attacks_amount);
                }

            }

        }
    }

    // Free ships
    for (int ship_index = 0; ship_index < 3; ++ship_index)
    {
        destroy_ship(my_ships[ship_index]);
    }
    // Free grid
    for (int row = 0; row < 10; ++row)
    {
        for (int col = 0; col < 10; ++col)
        {
            free(grid[row][col]);
        }
        free(grid[row]);
    }
    free(grid);
    // Close socket.
    close(send_to_socket);

    return 0;

}