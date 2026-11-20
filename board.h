#ifndef BOARD_H
#define BOARD_H
char** create_board(int stroki, int stolbci);

void free_board(char** board, int stroki);

void print_board(char** board, int stroki, int stolbci, int win_type, int win_idx, int win_diag);

int is_board_full(char** board, int stroki, int stolbci);
#endif
