#ifndef GAME_H
#define GAME_H
void setup_game(int* stroki, int* stolbci, char* p1_sym, char* p2_sym, int* mode);

void make_m(char** board, int stroki, int stolbci, int player_num, char symbol);

void make_404_m(char** board, int stroki, int stolbci, char h404_sym, char human_sym);

int check_win(char** board, int stroki, int stolbci, char symbol, int* win_type, int* win_idx, int* win_diag);

int check_diagonals(char** board, int stroki, int stolbci, char symbol, int* type, int* r0, int* c0);
#endif