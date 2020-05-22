#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "game_util.h"

int server_socket;
field my_symbol;
char* nick;

int cmd;
char *arg;
field board[9];
gamestate state = META;

pthread_mutex_t msg_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t new_reply = PTHREAD_COND_INITIALIZER;

void client_send(int to, int cmd, int arg);
void handle_msg(char buffer[MSG_LEN]);
int is_game_over();
void game_loop();
void quit() {
    client_send(server_socket, CMD_QUIT, 0);
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Provide arguments:\n ./client name type(local | net) server_address(UNIX path | port)");
        return 1;
    }

    nick = argv[1];
    char* type = argv[2];
    char* server_addr = argv[3];

    signal(SIGINT, quit); // handle ctrl+c

    if (strcmp(type, "local") == 0) {
        server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un local_sockaddr =
                {.sun_family = AF_UNIX, .sun_path = *server_addr};
        connect(server_socket, (struct sockaddr*)&local_sockaddr,
                sizeof(struct sockaddr_un));

    } else if(strcmp(type, "net") == 0) {
        /*specifies criteria for selecting the socket address
          structures returned in the list pointed to by res. */
        struct addrinfo hints = {.ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM};
        struct addrinfo* res;
        getaddrinfo("localhost", server_addr, &hints, &res);

        server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        connect(server_socket, res->ai_addr, res->ai_addrlen);

        freeaddrinfo(res);
    } else{
        fprintf(stderr, "type should be one of: local net");
        exit(0);
    }
    // join server
    client_send(server_socket, CMD_ADD, 0);
    // start game loop (state META)
    pthread_t game;
    pthread_create(&game, NULL, (void* (*)(void*))game_loop, NULL);
    // listen for msg from server
    while (1) {
        char buffer[MSG_LEN + 1];
        recv(server_socket, buffer, MSG_LEN, 0);
        handle_msg(buffer);
    }
}

void handle_msg(char buffer[MSG_LEN]) {
    cmd = atoi(strtok(buffer, ":"));
    arg = strtok(NULL, ":");
    pthread_mutex_lock(&msg_mutex);
    switch(cmd){
        case CMD_ADD:
            state = ADD;
            break;
        case CMD_MOVE:
            state = ENEMY_MOVED;
            break;
        case CMD_QUIT:
            state = QUIT;
            break;
        case CMD_PING:
            client_send(server_socket, CMD_PONG, 0);
            break;
        default:
            printf("unrecognized command %d", cmd);
    }
    pthread_cond_signal(&new_reply);
    pthread_mutex_unlock(&msg_mutex);
}

void game_loop() {
    if (state == ADD) {
        if (strcmp(arg, "taken") == 0) {
            puts("nick already taken :(");
            exit(1);
        }
        else if (strcmp(arg, "no_enemy") == 0) {
            state = META;
            puts("waiting for the other player to join");
            pthread_mutex_lock(&msg_mutex);
            while (state != ADD && state != QUIT)
                pthread_cond_wait(&new_reply, &msg_mutex);
            pthread_mutex_unlock(&msg_mutex);
        }
        else {
            my_symbol = arg[0] == 'O' ? F_O : F_X;
            state = my_symbol == F_O ? MOVE : WAIT_FOR_MOVE;
        }
    }
    else if (state == WAIT_FOR_MOVE) {
        puts("opponent's turn");
        // wait for enemy_move or quit message
        pthread_mutex_lock(&msg_mutex);
        while (state != ENEMY_MOVED && state != QUIT)
            pthread_cond_wait(&new_reply, &msg_mutex);
        pthread_mutex_unlock(&msg_mutex);
    }
    else if (state == ENEMY_MOVED) {
        make_move(board, atoi(arg), my_symbol == F_O ? F_X : F_O);
        state = is_game_over() ? QUIT : MOVE;
    }
    else if (state == MOVE) {
        draw_board(board);

        int move;
        printf("[%c] enter move: ", field_char[my_symbol]);
        scanf("%d", &move);
        move --;
        while (make_move(board, move, my_symbol) != 0){
            puts("illegal move, choose again: ");
            scanf("%d", &move);
            move --;
        }
        draw_board(board);

        client_send(server_socket, CMD_MOVE, move);
        state = is_game_over() ? QUIT : WAIT_FOR_MOVE;

    } else if (state == QUIT) {
        quit();
    }
    game_loop();
}

int is_game_over() {
    // check if somebody won
    field winner = get_winner_or_empty(board);
    if (winner != F_EMPTY) {
        if (winner == my_symbol) puts("You have won the game!");
        else puts("You have lost :(");
        return 1;
    }
    // check for tie
    int all_filled = 1;
    for (int i = 0; i < 9; i++)
        if (board[i] == F_EMPTY)
            all_filled = 0;

    if (all_filled) {
        puts("It's a tie!");
        return 1;
    }
    return 0;
}

void client_send(int to, int cmd, int arg) {
    char buffer[MSG_LEN+1];
    snprintf(buffer, MSG_LEN, "%d:%d:%s", cmd, arg, nick);
    send(to, buffer, MSG_LEN, 0);
}
#pragma clang diagnostic pop