#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#define _POSIX_C_SOURCE 200112L

#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "common.h"

int server_socket;
int is_o;
char buffer[MSG_LEN + 1];
char* nick;

board_t board;

void clientsend(int to, int cmd, int arg){
    char buffer[MSG_LEN+1];
    snprintf(buffer, MSG_LEN, "%d:%d:%s", cmd, arg, nick);
    send(to, buffer, MSG_LEN, 0);
}
typedef enum {
    WAIT,
    ADD,
    WAIT_FOR_ENEMY,
    WAIT_FOR_MOVE,
    ENEMY_MOVED,
    MOVE,
    QUIT
} state_t;

state_t state = WAIT;
int cmd;
char *arg;

pthread_mutex_t msg_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t new_reply = PTHREAD_COND_INITIALIZER;

void join_server();
void handle_msg(char buffer[MSG_LEN]);

void quit() {
    clientsend(server_socket, CMD_QUIT, 0);
    exit(0);
}

void check_board_status() {
    // check for a win
    int is_won = 0;
    field winner = get_winner(&board);
    if (winner != F_EMPTY) {
        if ((is_o && winner == F_O) || (!is_o && winner == F_X)) {
            puts("You have won the game!");
        } else {
            puts("You have lost :(");
        }

        is_won = 1;
    }

    // check for a draw
    int is_drawn = 1;
    for (int i = 0; i < 9; i++) {
        if (board.objects[i] == F_EMPTY) {
            is_drawn = 0;
            break;
        }
    }

    if (is_drawn && !is_won) {
        puts("Game ended in a draw");
    }

    if (is_won || is_drawn) {
        state = QUIT;
    }
}

void draw_board() {
    char symbols[3] = {' ', 'O', 'X'};
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            symbols[0] = y * 3 + x + 1 + '0';
            printf("|%c|", symbols[board.objects[y * 3 + x]]);
        }
        puts("\n---------");
    }
}

void game_loop() {
    if (state == ADD) {
        if (strcmp(arg, "taken") == 0) {
            puts("nick already taken :(");
            exit(1);
        } else if (strcmp(arg, "no_enemy") == 0) {
            puts("waiting for the other player");
            state = WAIT_FOR_ENEMY;
        } else {
            board = new_board();
            is_o = arg[0] == 'O';
            state = is_o ? MOVE : WAIT_FOR_MOVE;
        }
    } else if (state == WAIT_FOR_ENEMY) {

        pthread_mutex_lock(&msg_mutex);
        while (state != ADD && state != QUIT) {
            pthread_cond_wait(&new_reply, &msg_mutex);
        }
        pthread_mutex_unlock(&msg_mutex);

        board = new_board();
        is_o = arg[0] == 'O';
        state = is_o ? MOVE : WAIT_FOR_MOVE;

    } else if (state == WAIT_FOR_MOVE) {
        puts("Waiting for enemy to make a move");

        pthread_mutex_lock(&msg_mutex);
        // wait for enemy_move or quit message
        while (state != ENEMY_MOVED && state != QUIT) {
            pthread_cond_wait(&new_reply, &msg_mutex);
        }
        pthread_mutex_unlock(&msg_mutex);

    } else if (state == ENEMY_MOVED) {
        make_move(&board, atoi(arg));
        check_board_status();
        if (state != QUIT) {
            state = MOVE;
        }
    } else if (state == MOVE) {
        draw_board();

        int move;
        do {
            printf("Enter next move (%c): ", is_o ? 'O' : 'X');
            scanf("%d", &move);
            move--;
        } while (!make_move(&board, move));

        draw_board();

        clientsend(server_socket, CMD_MOVE, move);

        check_board_status();
        if (state != QUIT) {
            state = WAIT_FOR_MOVE;
        }
    } else if (state == QUIT) {
        quit();
    }
    game_loop();
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
    join_server();
    // start game loop (state WAIT)
    pthread_t game;
    pthread_create(&game, NULL, (void* (*)(void*))game_loop, NULL);
    // listen for msg from server
    while (1) {
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
            clientsend(server_socket, CMD_PONG, 0);
            break;
        default:
            printf("unrecognized command %d", cmd);
    }
    pthread_cond_signal(&new_reply);
    pthread_mutex_unlock(&msg_mutex);
}

void join_server() {
    clientsend(server_socket, CMD_ADD, 0);
}

#pragma clang diagnostic pop