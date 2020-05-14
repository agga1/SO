#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int N, K;
int *waitingRoom;
pthread_t barberThId;
pthread_t *clientsThIds;
int waitingClients=0;
int queueStart=0;
pthread_mutex_t waitingRoomMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t newArrival = PTHREAD_COND_INITIALIZER;

unsigned int getRnd(int from, int to){
    return (unsigned int) (rand() % (to - from) + from);
}
void barber(){
    // immediately quit
    if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
        perror("Barber set cancel type problem");

    while(1){
        pthread_mutex_lock(&waitingRoomMutex);
        if(waitingClients == 0){
            puts("Barber: i'm gonna sleep now.");
            pthread_cond_wait(&newArrival, &waitingRoomMutex);
        }
        int client = waitingRoom[queueStart];
        printf("----got client %d from seat %d\n", client, queueStart);
        // free seat and move queue
        waitingRoom[queueStart] = -1;
        waitingClients--;
        queueStart = (queueStart + 1) % K;

        printf("%d more clients waiting, shaving client %d\n", waitingClients, client);
        printf("queueStart %d\n", queueStart);
        sleep(getRnd(2, 4));
        pthread_cancel(clientsThIds[client]);
        printf("shaved %d\n", client);
        pthread_mutex_unlock(&waitingRoomMutex);
    }
}
void client(int *idx){
    printf("client nr %d, id %lu\n", *idx, clientsThIds[*idx]);

    pthread_mutex_lock(&waitingRoomMutex);

    int spot = (queueStart+waitingClients)%K;
    if(waitingRoom[spot] != -1) {puts("all taken"); printf("on spot %d", waitingRoom[spot]); return;}
    waitingRoom[spot] = *idx; // take a seat
    waitingClients ++;
    printf("[%d] taking seat %d now waiting clients %d\n", *idx, spot, waitingClients);

    if(waitingClients == 1){ // that means no one waits and shop is empty
        pthread_cond_broadcast(&newArrival);
        printf("I am waking up barber, ID: %d\n\n", *idx);
    }else{          // sit in the waiting room
        printf("Waiting room, free seats: %d; ID: %d\n\n", K-waitingClients, *idx);
    }
    pthread_mutex_unlock(&waitingRoomMutex);
    pause();
}


int main(int argc, char **argv ) {
    srand(time(NULL));
    if (setvbuf(stdout, NULL, _IONBF, 0) != 0) {
        printf("Error: buffering mode could not be changed!\n");
        exit(1);
    }

    if(argc != 3){
        printf("usage: K N (K - nr of seats in the waiting room, N - number of clients)");
        exit(1);
    }
    K = (int) strtol(argv[1], NULL, 10);
    N = (int) strtol(argv[2], NULL, 10);

    waitingRoom = malloc((size_t) K*sizeof(int));
    for (int i=0; i<K; ++i)    // Set the first 6 elements in the array
        waitingRoom[i] = -1;
    clientsThIds = calloc((size_t) N, sizeof(pthread_t));

    // making threads
    if(pthread_create(&barberThId, NULL, (void *(*)(void *)) barber, NULL)) {
        perror("barber not created");
        exit(2);
    };
    printf("barb id %lu\n", barberThId);

    int *args = calloc((size_t) N, sizeof(int));
    for (int i = 0; i < N; i++) {
        args[i] = i;
        sleep(getRnd(1, 3));
        if (pthread_create(&clientsThIds[i], NULL, (void *(*)(void *)) client, args+i))
            perror("client's thread creation problem");
    }
    for (int i = 0; i < N; i++) {
        if (pthread_join(clientsThIds[i], NULL) != 0) perror("pthread_join error");
    }
    pthread_cancel(barberThId);

    free(waitingRoom);
    free(clientsThIds);

    return 0;
}
#pragma clang diagnostic pop