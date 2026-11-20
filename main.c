#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "board.h"
#include "game.h"
#include "sprite.h"

int main()
{
    int stroki, stolbci, p3_404;
    char p1_sym, p2_sym;

    window_setup(64, 28);
    setup_game(&stroki, &stolbci, &p1_sym, &p2_sym, &p3_404);

    // подгон окнa под выбранный размер поля
    int cols = 2 * stolbci + 10; if (cols < 64) cols = 64;
    int lines = stroki + 16; if (lines < 28) lines = 28;
    window_setup(cols, lines);

    char** board = create_board(stroki, stolbci);

    int current_player = 1;
    int win_type = 0, win_idx = -1, win_diag = 0;

    while (1)
    {
        print_board(board, stroki, stolbci, 0, -1, 0);

        if (p3_404 == 2 && current_player == 2)
        {
            make_404_m(board, stroki, stolbci, p2_sym, p1_sym);
        }
        else
        {
            char active_sym = (current_player == 1) ? p1_sym : p2_sym;
            make_m(board, stroki, stolbci, current_player, active_sym);
        }

        char checking_sym = (current_player == 1) ? p1_sym : p2_sym;
        if (check_win(board, stroki, stolbci, checking_sym, &win_type, &win_idx, &win_diag))
        {
            print_board(board, stroki, stolbci, win_type, win_idx, win_diag);
            if (p3_404 == 2 && current_player == 2)
            {
                printf("\n[ 404 ]: Ya viigrala! Gotovsya k sleduyushey partii.\n");
                printf(" --- Pobeda za programmoi (%c)! ---\n", p2_sym);
                sprite_show("smile.PNG");
            }
            else
            {
                if (p3_404 == 2)
                {
                    printf("\n[ 404 ]: Ti viigral... Ya ne ozhidala.\n");
                    sprite_show("file_404.MP4");
                }
                printf(" --- Pobedil Igrok %d (%c)! ---\n", current_player, checking_sym);
            }
            break;
        }

        if (is_board_full(board, stroki, stolbci))
        {
            print_board(board, stroki, stolbci, 0, -1, 0);
            if (p3_404 == 2)
            {
                printf("\n[ 404 ]: Nichiya. Neploho sygrali.\n");
                sprite_show("smle.PNG");
            }
            printf(" --- Nichiya! ---\n");
            break;
        }
        current_player = (current_player == 1) ? 2 : 1;
    }
    printf("\n Nazhmite Enter, chtoby viiti...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    getchar();
    sprite_hide();

    free_board(board, stroki);
    return 0;
}
