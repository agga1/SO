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
int atBarberSeat=-1;
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
        if(waitingClients == 0 && atBarberSeat==-1){ // nothing to do
            puts("Barber: i'm gonna sleep now.");
            pthread_cond_wait(&newArrival, &waitingRoomMutex);
        }
        if(atBarberSeat == -1){
            // get client, free seat and move queue
            atBarberSeat = waitingRoom[queueStart];
            waitingRoom[queueStart] = -1;
            waitingClients--;
            queueStart = (queueStart + 1) % K;
        }
        printf("%d more clients waiting, shaving client [%d]\n", waitingClients, atBarberSeat);
        pthread_mutex_unlock(&waitingRoomMutex);

        sleep(getRnd(3, 6));  //  ... shaving ...

        pthread_mutex_lock(&waitingRoomMutex);
        pthread_cancel(clientsThIds[atBarberSeat]);
        printf("client %d shaved\n\n", atBarberSeat);
        atBarberSeat = -1;
        pthread_mutex_unlock(&waitingRoomMutex);
    }
}
int findPlaceToSit(){
    pthread_mutex_lock(&waitingRoomMutex);
    int spot = (queueStart+waitingClients)%K; // next supposedly free spot
    if(waitingRoom[spot] != -1)            // queue tail meets head
        spot = -1; // no place in queue
    pthread_mutex_unlock(&waitingRoomMutex);
    return spot;
}
void client(int *idx){

    int spot = findPlaceToSit();
    while(spot == -1){  // no place in the waiting room
        printf("all seats taken; [%d]\n", *idx);
        sleep(getRnd(4, 6));
        spot = findPlaceToSit();
    }

    pthread_mutex_lock(&waitingRoomMutex);
    if(waitingClients == 0 && atBarberSeat == -1){ // barber is free
        atBarberSeat = *idx;  // sit at barber's
        printf("I am waking up barber; [%d]\n", *idx);
        pthread_cond_broadcast(&newArrival);
    }
    else{  // sit in the waiting room
        waitingRoom[spot] = *idx;
        waitingClients ++;
        printf("Waiting room, free seats: %d; [%d]\n", K-waitingClients, *idx);
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
    for (int i=0; i<K; ++i) waitingRoom[i] = -1;

    clientsThIds = calloc((size_t) N, sizeof(pthread_t));

    // making threads
    if(pthread_create(&barberThId, NULL, (void *(*)(void *)) barber, NULL)) {
        perror("barber not created");
        exit(2);
    };

    int *args = calloc((size_t) N, sizeof(int));
    for (int i = 0; i < N; i++) {
        args[i] = i;
        sleep(getRnd(1, 2));
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