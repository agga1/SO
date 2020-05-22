#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdbool.h>

#include "game_util.h"

typedef struct {
    char* nick;
    int fd;
    int enemy_idx;
    bool alive;
} client_t;

int clients_nr = 0;
client_t* clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void easysend(int to, int cmd, char *arg);
int fd_from_poll(int loc_socket, int net_socket, struct pollfd *pfds);
int by_nick(char *nick);
int find_free_space(){
    for(int i=0;i<MAX_CLIENTS;i++)
        if(clients[i] == NULL) return i;
    return -1;
}
int set_opponent(int idx){
    for(int i=0;i<MAX_CLIENTS;i++)
        if(i != idx && clients[i] != NULL && clients[i]->enemy_idx == -1) {
            clients[i]->enemy_idx = idx;
            clients[idx]->enemy_idx = i;
            return 0;
        }
    return -1;
}
int add_client(char* nickname, int fd);
void delete_client(int idx);
void ping_loop();
int set_local(char *path);
int set_net(char *port);
void handle_msg(int from, char *msg);

int main(int argc, char* argv[]) {
    srand(time(NULL));

    if (argc != 3) {
        fprintf(stderr, "Provide arguments:\n ./server port UNIX_path");
        return 1;
    }

    int net_socket = set_net(argv[1]);
    int loc_socket = set_local(argv[2]);
    printf("set local socket %d, network socket %d\n", loc_socket, net_socket);
    pthread_t pthread;
    pthread_create(&pthread, NULL, (void *(*)(void *)) ping_loop, NULL);

    struct pollfd* pfds = calloc(2 + MAX_CLIENTS, sizeof(struct pollfd));
    pfds[0].fd = loc_socket;
    pfds[0].events = POLLIN;
    pfds[1].fd = net_socket;
    pfds[1].events = POLLIN;

    while (1) {
        int client_fd = fd_from_poll(loc_socket, net_socket, pfds);

        char buffer[MSG_LEN + 1];
        recv(client_fd, buffer, MSG_LEN, 0);
        handle_msg(client_fd, buffer);
    }

}
void easysend(int to, int cmd, char *arg) {
    char buffer[MSG_LEN+1];
    snprintf(buffer, MSG_LEN, "%d|%s", cmd, arg);
    send(to, buffer, MSG_LEN, 0);
}

void handle_msg(int from, char *msg) {
    int cmd = atoi(strtok(msg, "|"));
    char* arg = strtok(NULL, "|");
    char* nick = strtok(NULL, "|");

    pthread_mutex_lock(&clients_mutex);
    if (cmd == CMD_ADD) {
        if(clients_nr == MAX_CLIENTS){
            easysend(from, cmd, "max_player_reached");
            close(from);
            return;
        }
        int idx = add_client(nick, from);
        printf("new client fd %d\n", clients[idx]->fd);
        if (idx == -1) {
            easysend(from, cmd, "taken");
            close(from);
        } else if (set_opponent(idx) == -1) {
            easysend(from, cmd, "no_enemy");
        } else {
            int enemy_idx = clients[idx]->enemy_idx;
            easysend(clients[idx]->fd, cmd, "O");
            easysend(clients[enemy_idx]->fd, cmd, "X");
        }
    }
    if (cmd == CMD_MOVE) {
        int idx = by_nick(nick);
        int enemy_idx = clients[idx]->enemy_idx;
        easysend(clients[enemy_idx]->fd, CMD_MOVE, arg);
    }
    if (cmd == CMD_QUIT) {
        delete_client(by_nick(nick));
    }
    if (cmd == CMD_PONG) {
        int player = by_nick(nick);
        if (player != -1) {
            clients[player]->alive = 1;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

}

void ping_loop() {
    puts("!ping!");

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL ) {
            if(clients[i]->alive){
                easysend(clients[i]->fd, CMD_PING, "");
                clients[i]->alive = false;
            }else{
                printf("%s unresponsive \n", clients[i]->nick);
                delete_client(i);
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    sleep(3);
    ping_loop();
}

int set_local(char *path) {
    int local_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un local_sockaddr;
    memset(&local_sockaddr, 0, sizeof(struct sockaddr_un));
    local_sockaddr.sun_family = AF_UNIX;
    strcpy(local_sockaddr.sun_path, path);

    unlink(path);
    bind(local_socket, (struct sockaddr*)&local_sockaddr,
         sizeof(struct sockaddr_un));

    listen(local_socket, MAX_BACKLOG);

    return local_socket;
}

int set_net(char *port) {
    struct addrinfo* info;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port, &hints, &info);

    int network_socket =
            socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    bind(network_socket, info->ai_addr, info->ai_addrlen);

    listen(network_socket, MAX_BACKLOG);

    freeaddrinfo(info);
    return network_socket;
}

void delete_client(int idx) {
    if (idx == -1) return;
    printf("deleting %s\n", clients[idx]->nick);
    int enemy_idx = clients[idx]->enemy_idx;

    free(clients[idx]->nick);
    free(clients[idx]);
    clients[idx] = NULL;
    clients_nr--;

    if (enemy_idx != -1) {
        clients[enemy_idx]->enemy_idx = -1;
        easysend(clients[enemy_idx]->fd, CMD_QUIT, 0);
        delete_client(enemy_idx);
    }
}

int add_client(char *nickname, int fd) {
    if (by_nick(nickname) != -1) return -1;

    int idx = find_free_space();
    if(idx == -1) return -1;

    client_t* new_client = calloc(1, sizeof(client_t));
    new_client->nick = calloc(MSG_LEN, sizeof(char));
    strcpy(new_client->nick, nickname);
    new_client->fd = fd;
    new_client->alive = 1;
    new_client->enemy_idx = -1;
    clients[idx] = new_client;
    clients_nr++;

    return idx;
}

int by_nick(char *nick) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i] != NULL && strcmp(clients[i]->nick, nick) == 0)
            return i;
    return -1;
}

int fd_from_poll(int loc_socket, int net_socket, struct pollfd *pfds) {

    int fd = -1;
    if(poll(pfds,  clients_nr + 2, -1) > 0){ // sth received
        for (int i = 0; i <  clients_nr+ 2; i++) {
            if (pfds[i].revents & POLLIN) {
                fd = pfds[i].fd;
                break;
            }
        }
    }

    if (fd == loc_socket || fd == net_socket) {
        fd = accept(fd, NULL, NULL);
        printf("accepting new client fd %d\n", fd);

        pthread_mutex_lock(&clients_mutex); // add new fd to poll
        pfds[clients_nr + 1].fd = fd;
        pfds[clients_nr + 1].events = POLLIN;
        pthread_mutex_unlock(&clients_mutex);
    }
    return fd;
}
#pragma clang diagnostic pop