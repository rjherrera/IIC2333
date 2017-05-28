int send_start_message(int send_to_socket)
{
    // Sends random number and returns it.
    // Returns -1 if connection lost
    int my_random_number = rand();
    char to_send_length = 6;
    char message_type = 11;

    if (send( send_to_socket, &to_send_length, 1, 0) < 0) return -1;
    if (send( send_to_socket, &message_type, 1, 0) < 0) return -1;
    if (send( send_to_socket, &my_random_number, 4, 0 ) < 0) return -1;
    return my_random_number;
}

int send_new_move_message(int send_to_socket, char x, char y)
{
    // Returns -1 if connection lost else 0
    char message_type = 12;
    char to_send_length = 4;

    if (send( send_to_socket, &to_send_length, 1, 0) < 0) return -1;
    if (send( send_to_socket, &message_type, 1, 0) < 0) return -1;
    if (send( send_to_socket, &x, 1, 0 ) < 0) return -1;
    if (send( send_to_socket, &y, 1, 0 ) < 0) return -1;

    return 0;
}

int send_destroyed_message(int send_to_socket, char boats_left)
{
    // Returns -1 if connection lost else 0
    char message_type = 13;
    char to_send_length = 3;

    if (send( send_to_socket, &to_send_length, 1, 0) < 0) return -1;
    if (send( send_to_socket, &message_type, 1, 0) < 0) return -1;
    if (send( send_to_socket, &boats_left, 1, 0 ) < 0) return -1;

    return 0;
}

char receive_message(int socket_to_receive, char* recv_buff, char* process_buffer)
{
    // receives entire message and deposits it WITHOUT length into process_buffer,
    // returns length of message (not considering length char)
    char received_msg_length;
    char missing_data;
    char trash[1];

    recv( socket_to_receive, &received_msg_length, 1, MSG_PEEK );
    missing_data = received_msg_length;

    while (missing_data != 0)
    {
        // make sure to receive entire message
        missing_data -= recv( socket_to_receive, recv_buff+(received_msg_length-missing_data), missing_data, 0 );
        if (recv( socket_to_receive, trash, 1, MSG_PEEK|MSG_DONTWAIT ) == 0){
            return -1;   
        }
    }

    memcpy(process_buffer, &recv_buff[1], received_msg_length-1);
    return received_msg_length-1;
}
