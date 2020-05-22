//
// Created by Agnieszka on 20/05/2020.
//
#include <stdlib.h>
#ifndef ZAD1_COMMON_H
#define ZAD1_COMMON_H

#define MAX_PLAYERS 20
#define MAX_BACKLOG 10
#define MSG_LEN 256

typedef enum { F_EMPTY, F_O, F_X } field;
// commands protocol
typedef enum {CMD_ADD, CMD_MOVE, CMD_QUIT, CMD_PING, CMD_PONG} command;

typedef struct {
    int o_move;
    field objects[9];
} board_t;

board_t new_board() {
    board_t board = {1, {F_EMPTY}};
    return board;
}

int make_move(board_t* board, int position) {
    if (position < 0 || position > 9 || board->objects[position] != F_EMPTY)
        return 0;
    board->objects[position] = board->o_move ? F_O : F_X;
    board->o_move = !board->o_move;
    return 1;
}

field same_column(board_t* board) {
    for (int x = 0; x < 3; x++) {
        field first = board->objects[x];
        field second = board->objects[x + 3];
        field third = board->objects[x + 6];
        if (first == second && first == third && first != F_EMPTY) return first;
    }
    return F_EMPTY;
}

field same_row(board_t* board) {
    for (int y = 0; y < 3; y++) {
        field first = board->objects[3 * y];
        field second = board->objects[3 * y + 1];
        field third = board->objects[3 * y + 2];
        if (first == second && first == third && first != F_EMPTY) return first;
    }
    return F_EMPTY;
}

field same_diagonal(board_t* board) {
    field first = board->objects[3 * 0 + 0];
    field second = board->objects[3 * 1 + 1];
    field third = board->objects[3 * 2 + 2];
    if (first == second && first == third && first != F_EMPTY) return first;

    first = board->objects[3 * 0 + 2];
    second = board->objects[3 * 1 + 1];
    third = board->objects[3 * 2 + 1];
    if (first == second && first == third && first != F_EMPTY) return first;

    return F_EMPTY;
}

field get_winner(board_t* board) {
    field column = same_column(board);
    field row = same_row(board);
    field diagonal = same_diagonal(board);

    return column != F_EMPTY ? column : row != F_EMPTY ? row : diagonal;
}

#endif //ZAD1_COMMON_H
