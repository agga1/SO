#include <stdlib.h>
#ifndef ZAD1_COMMON_H
#define ZAD1_COMMON_H

#define MAX_CLIENTS 20
#define MAX_BACKLOG 10
#define MSG_LEN 256

// commands protocol
typedef enum {CMD_ADD, CMD_MOVE, CMD_QUIT, CMD_PING, CMD_PONG} command;
// field enum
typedef enum { F_EMPTY, F_O, F_X } field_t;
static char field_char[3] = " OX";

typedef enum {
    META,
    ADD,
    WAIT_FOR_MOVE,
    ENEMY_MOVED,
    MOVE,
    QUIT
} gamestate;

int make_move(field_t board[9], int idx, field_t player) {
    if (idx < 0 || idx > 9 || board[idx] != F_EMPTY)
        return -1;  // can't move
    board[idx] = player;
    return 0;
}

field_t get_winner_or_empty(field_t *board) {
    // check in columns
    for (int x = 0; x < 3; x++) {
        if(board[x] == F_EMPTY) continue;
        if (board[x] == board[x + 3] && board[x + 3] == board[x + 6]) return board[x];
    }
    // check in row
    for (int x = 0; x < 3; x++) {
        if(board[x*3] == F_EMPTY) continue;
        if (board[x*3] == board[x*3+1] && board[x*3] == board[x*3+2]) return board[x];
    }
    // check diagonals
    if (board[0]!= F_EMPTY &&
        board[0] == board[4] && board[4] == board[8]) return board[0];
    if (board[2]!= F_EMPTY &&
        board[2] == board[4] && board[4] == board[6]) return board[0];
    return F_EMPTY; // no winner
}

void draw_board(field_t *board) {
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            int idx = y * 3 + x;
            printf("|%c|", board[idx] == F_EMPTY ? '1'+idx : field_char[board[idx]]);
        }
        puts("\n---------");
    }
}
#endif //ZAD1_COMMON_H
