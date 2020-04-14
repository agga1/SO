#include <stdio.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "provide file to read");
        return -1;
    }

    return 0;
}