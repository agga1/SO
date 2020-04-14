#include <stdio.h>
#define LINE_BUFF 256

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "provide file to read");
        return -1;
    }
    char cmd[LINE_BUFF];
    snprintf(cmd, LINE_BUFF, "sort %s", argv[1]);
    FILE* sorted = popen(cmd, "r");
    char line[LINE_BUFF];
    while(fgets(line, LINE_BUFF, sorted)){
        printf("%s", line);
    }
    pclose(sorted);
    return 0;
}