#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
int main(int argc, char **argv) {
    if(argc < 5){
        puts("expecetd arguments: \n"
             "N, consumer_out_file, nr_of_producents, [res_file_1, ...]*nr_of_producents");
        return 1;
    }
    char *N = argv[1];
    char *consumer_out = argv[2];
    int producers = atoi(argv[3]);
    if (argc < 4+producers){
        puts("not enough resource files for declared producers");
        return 1;
    }
    char *stream_path = "fifo";
    mkfifo(stream_path, 0666);
    for(int i=0;i<producers;i++){
        if(fork()==0){
            execlp("./producer", "./producer", stream_path, argv[4+i], N, NULL);
            return errno;
        }
    }
    execlp("./consumer", "./consumer", stream_path, consumer_out, N, NULL);
    return errno;
}
