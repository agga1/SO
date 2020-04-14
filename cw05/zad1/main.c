#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>

#define MAX_ARGS 5
#define LINE_BUFF 256
#define MAX_COMMANDS 10

void process_line(char line[LINE_BUFF]);
char * trim_spaces(char *str);
char ** to_args(char *cmd_str);
int get_lines(char* filename, char lines[MAX_COMMANDS][LINE_BUFF], int *cnt );
int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "provide file with commands");
        return 1;
    }
    //// preprocess to avoid opened files in child processes
    char lines[MAX_COMMANDS][LINE_BUFF];
    int cnt;
    if(get_lines(argv[1], lines, &cnt)>0){
        puts("error while reading file");
        return 2;
    };

    for(int i=0;i<cnt;i++){
        if(fork() == 0) { // CHILD
            process_line(lines[i]);
            exit(EXIT_SUCCESS);
        }
        int status;
        wait(&status);
        if (status > 0) {
            printf( "Error while executing line %d", i);
            return 3;
        }
    }
    return 0;
}

void process_line(char line[LINE_BUFF]) {
    //// split into commands
    int cmd_cnt = 0;
    char *cmds[MAX_COMMANDS];
    cmds[0] = strtok(line, "|");
    while((cmds[++cmd_cnt] = strtok(NULL, "|") )!= NULL){};
    for(int i=0;i<cmd_cnt;i++) cmds[i] = trim_spaces(cmds[i]);

    int pipes[2][2];
    for (int i = 0; i < cmd_cnt; i++) {
        if (i > 0) { // not first
            close(pipes[i % 2][0]);
            close(pipes[i % 2][1]);
        }

        if(pipe(pipes[i % 2]) == -1) {
            puts("Error on pipe");
            exit(EXIT_FAILURE);
        }

        if (fork() == 0) { // CHILD
            char **args = to_args(cmds[i]);

            if ( i  !=  cmd_cnt - 1) {  // not last
                close(pipes[i % 2][0]);
                if (dup2(pipes[i % 2][1], STDOUT_FILENO) < 0) {
                    exit(EXIT_FAILURE);
                };
            }
            if (i != 0) { // not first
                close(pipes[(i + 1) % 2][1]);
                if (dup2(pipes[(i + 1) % 2][0], STDIN_FILENO) < 0) {
                    close(EXIT_FAILURE);
                }
            }
            execvp(args[0], args);

            exit(EXIT_SUCCESS);
        }
    }
    close(pipes[cmd_cnt % 2][0]);
    close(pipes[cmd_cnt % 2][1]);
    wait(NULL);
    exit(0);
}

char ** to_args(char *cmd_str) {
    /* split string into separate arguments by whitespace */
    char ** tmp_args = malloc((MAX_ARGS+1)* sizeof(char*));
    int cnt = 0;
    tmp_args[0] = strtok(cmd_str, " ");
    while((tmp_args[++cnt] = strtok(NULL, " ")) != NULL){};
    //// resize array to match content
    cnt++; // include null
    char ** args = (char **)realloc(tmp_args, sizeof(char*)*cnt);
    return args;
}

char* trim_spaces(char *str) {
    /*  trims all leading and trailing spaces  */

    while(isspace((unsigned char)*str)) str++;
    if(*str == 0)  return str; // all spaces

    char *end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    return str;
}
int get_lines(char* filename, char lines[MAX_COMMANDS][LINE_BUFF], int *cnt ){
    *cnt = 0;
    FILE* cmd_file = fopen(filename, "r");
    if (cmd_file == NULL) return 1;
    while(fgets(lines[*cnt], LINE_BUFF, cmd_file)){
        *cnt = *cnt+1; // careful,  != *cnt ++
    }
    fclose(cmd_file);
    return 0;
}