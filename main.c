#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "connections.c"
#include "ships.c"




int main( int argc, char* argv[] )
{
    if(argc != 3 && argc != 4)
    {
        printf("Modo de uso: %s -l <ip_address> <tcp_port>\n", argv[0]);
        printf("\t -l es opcional e indica si se desea ejecutar como servidor.\n");
        printf("\t ip_address: IP donde se recibiran conexiones entrantes si es modo servidor.Si es modo cliente, es el IP con que se desea conectar.\n");
        printf("\t tcp_port: Puerto donde se recibiran conexiones entrantes si es modo servidor.Si es modo cliente, es el puerto con que se desea conectar.\n");
        printf("\t output.txt : Nombre de archivo donde se escribira la respuesta.\n");
        return 1;
    }


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

    // Populate grid

    Ship** my_ships = malloc(sizeof(Ship*)*3);
    for (int index = 0; index < 3; ++index)
    {
        print_grid(grid);
        my_ships[index] = choose_ship(index);
        choose_position(my_ships[index], grid);
    }
    print_grid(grid);

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

    // Prepare for Sockets Configuration

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


    if (argc == 3)
    {
        // Initialize sockets and connection with server
        send_to_socket = socket( AF_INET, SOCK_STREAM, 0);
        setsockopt(send_to_socket ,SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof (reuse));
        name.sin_port = htons(atoi(argv[2]));
        inet_pton(AF_INET, argv[2], &name.sin_addr.s_addr);

        len = sizeof( struct sockaddr_in );

        connect(send_to_socket, (struct sockaddr*) &name, len);

        int same_numbers=1;

        // send start
        while (same_numbers)
        {
            my_random_number = send_start_message(send_to_socket);
            processed_message_length = receive_message(send_to_socket, recv_buffer, process_buffer);
            memcpy(&other_random_number, process_buffer+1, 4); // this case we know its start_msg

            printf("My number is %d and i received number %d\n", my_random_number, other_random_number);
            if (my_random_number != other_random_number) same_numbers = 0;
        }

        // START GAME
        int game_finished = 0;

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

            }

        }     
    }
    else
    {
        // Initialize sockets and connection with client
        sock = socket( AF_INET, SOCK_STREAM, 0);
        setsockopt(sock ,SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof (reuse));
        name.sin_port = htons(atoi(argv[3]));
        name.sin_addr.s_addr = htonl( INADDR_ANY );

        len = sizeof(struct sockaddr_in);

        if( bind( sock, (struct sockaddr *) &name, len ) )
        {
          printf( "bind error\n" );
        }

        listen(sock,1);

        send_to_socket = accept( sock, (struct sockaddr*) &name, &len);

        printf( "Connection from : %s\n", inet_ntoa( name.sin_addr ) );

        close(sock);

        int same_numbers=1;

        // send start
        while (same_numbers)
        {
            my_random_number = send_start_message(send_to_socket);
            processed_message_length = receive_message(send_to_socket, recv_buffer, process_buffer);
            memcpy(&other_random_number, process_buffer+1, 4); // this case we know its start msg

            printf("My number is %d and i received number %d\n", my_random_number, other_random_number);
            if (my_random_number != other_random_number) same_numbers = 0;
        }

        // START GAME
        int game_finished = 0;

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