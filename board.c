#include <stdio.h>
#include <stdlib.h>
#include "board.h"

char** create_board(int stroki, int stolbci)
{
	char** board = (char**)malloc(stroki * sizeof(char*));
	for (int i = 0; i < stroki; i++)
	{
		board[i] = (char*)malloc(stolbci * sizeof(char));
		for (int j = 0; j < stolbci; j++)
		{ board[i][j] = '_'; }
	}
	return board;
}

void free_board(char** board, int stroki)
{
	for (int i = 0; i < stroki; i++)
	{ free(board[i]); }
	free(board);
}

static void print_hor_boarder(int stolbci)
{
	printf(" ");
	for (int j = 0; j < stolbci; j++)
	{ printf("- "); }
	printf("-\n");
}

void print_board(char** board, int stroki, int stolbci, int win_type, int win_idx, int win_diag)
{
#indef _WIN32
    system("cls");
#else
	system("clear");
#endif
    printf("\n");
    print_hor_boarder(stolbci);

    for (int i = 0; i < stroki; i++)
    {
        char ed_fill = (win_type == 1 && i == win_idx) ? '-' : ' ';
        printf("|%c", ed_fill);

        for (int j = 0; j < stolbci; j++)
        {
            printf("%c", board[i][j]);

            if (j < stolbci - 1)
            {
                char sep = ' ';
                if (win_type == 1 && i == win_idx)
                    sep = '-';
                else if (win_type == 3 && i - j == win_idx - win_diag && i + 1 < stroki)
                    sep = '\\';
                else if (win_type == 4 && i + (j + 1) == win_idx + win_diag && i + 1 < stroki)
                    sep = '/';
                printf("%c", sep);
            }
        }
        printf("%c|\n", ed_fill);
    }
    print_hor_boarder(stolbci);
    printf("\n");
}

int is_board_full(char** board, int stroki, int stolbci)
{
	for (int i = 0; i < stroki; i++)
	{
		for (int j = 0; j < stolbci; j++)
		{ if (board[i][j] == '_') return 0; }
	}
	return 1;
}
