#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"
#include "sprite.h"

/* настройки сложности игры с 404 (в%)
                           с какой вероятностью 404: */
#define BLOCK_CHANCE   80   // замечает угрозу и блокирует её
#define MISTAKE_CHANCE 15   // ходит в случайную клетку, а не в лучшую
#define MAX_SIZE       20  // макс размерность поля

static int read_number(const char* text)
{
    int x;
    while (1)
    {
        printf("%s", text);
        int result = scanf("%d", &x);
        if (result == EOF)
        {
            exit(0);
        }
        while (getchar() != '\n');
        if (result == 1)
        {
            return x;
        }
        printf("_> Oshibka vvoda! Nuzhno chislo.\n");
    }
}

void setup_game(int* stroki, int* stolbci, char* p1_sym, char* p2_sym, int* mode)
{
    printf("> Viberite rezhim:\n1. Igrat s drugim igrokom\n2. Igrat protiv programmi\n\n");
    do
    {
        *mode = read_number("> Vash vibor (1 or 2): ");
        if (*mode != 1 && *mode != 2)
        {
            printf("_> Nuzhno vvesti 1 ili 2.\n");
        }
    } while (*mode != 1 && *mode != 2);

    do
    {
        *stroki = read_number("> Vvedite kol-vo strok N (min 2): ");
        *stolbci = read_number("> Vvedite kol-vo stolbov M (min 2): ");
        if (*stroki < 2 || *stolbci < 2 || *stroki > MAX_SIZE || *stolbci > MAX_SIZE)
        {
            printf("_> Razmer polya dolzhen bit' ot 2x2 do %dx%d.\n", MAX_SIZE, MAX_SIZE);
        }
    } while (*stroki < 2 || *stolbci < 2 || *stroki > MAX_SIZE || *stolbci > MAX_SIZE);

    do
    {
        printf("> Igrok 1, viberite ASCII simvol (krome probela i '_'): ");
        scanf(" %c", p1_sym);
    } while (*p1_sym <= 32 || *p1_sym > 126 || *p1_sym == '_');

    do
    {
        printf("> Viberite simvol dlya %s (%c): ", (*mode == 2) ? "programmi" : "Igroka 2", *p1_sym);
        scanf(" %c", p2_sym);
    } while (*p2_sym <= 32 || *p2_sym > 126 || *p2_sym == *p1_sym || *p2_sym == '_');

    if (*mode == 2)
    {
        srand((unsigned int)time(NULL));
    }
}

void make_m(char** board, int stroki, int stolbci, int player_num, char symbol)
{
    int r, c;
    while (1)
    {
        printf("\n !! Vvod stroki i stolbca dolzen bit' cherez probel !!\n");
        printf("Igrok %d (%c), vvedite stroku (1-%d) i stolbec (1-%d): ",
            player_num, symbol, stroki, stolbci);
        if (scanf(" %d %d", &r, &c) != 2)
        {
            while (getchar() != '\n');
            printf("_> Oshibka vvoda! Vvedite dva chisla.\n");
            continue;
        }
        r--;
        c--;
        if (r < 0 || r >= stroki || c < 0 || c >= stolbci)
        {
            printf("_> Koordinaty vne polya!Poprobuite snova.\n");
        }

        else if (board[r][c] != '_')
        {
            printf("_> Eta kletka yzhe zanyata! Viberite drygyu.\n");
        }

        else
        {
            board[r][c] = symbol;
            break;
        }
    }
}

// симуляция победы 404
static int sim_win(char** board, int stroki, int stolbci, char symbol)
{
    int dummy_type, dummy_idx, dummy_diag;
    return check_win(board, stroki, stolbci, symbol, &dummy_type, &dummy_idx, &dummy_diag);
}

// 404 ищет выигрышную symbol клетку - возвращает 1 и координаты, если нашла
static int find_winning_cell(char** board, int stroki, int stolbci, char symbol, int* res_i, int* res_j)
{
    for (int i = 0; i < stroki; i++)
    {
        for (int j = 0; j < stolbci; j++)
        {
            if (board[i][j] == '_')
            {
                board[i][j] = symbol;
                int win = sim_win(board, stroki, stolbci, symbol);
                board[i][j] = '_';
                if (win)
                {
                    *res_i = i;
                    *res_j = j;
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* ценность одной линии(строки, столбца или диагонали) для клетки, в которую
хотим сходить линия начинается в (r0, c0), идет с шагом (dr, dc), длина len */
static int line_value(char** board, int r0, int c0, int dr, int dc, int len, char h404_sym, char human_sym)
{
    int cnt_404 = 0, cnt_human = 0;
    for (int k = 0; k < len; k++)
    {
        char cell = board[r0 + k * dr][c0 + k * dc];
        if (cell == h404_sym) cnt_404++;
        else if (cell == human_sym) cnt_human++;
    }
    if (cnt_404 > 0 && cnt_human > 0) return 0;      // "общая" линия занята игроком и 404, 404 отступает
    if (cnt_404 > 0) return 2 * cnt_404 * cnt_404;   // "своя" линия - нету символов игрока
    if (cnt_human > 0) return cnt_human * cnt_human; // "чужая" линия - 404 мешает игроку
    return 1;                                        // пустая линия
}

// ценность клетки (i, j) = сумма ценностей всех линий, которые через нее проходят
static int cell_value(char** board, int stroki, int stolbci, int i, int j, char h404_sym, char human_sym)
{
    int value = 0;
    value += line_value(board, i, 0, 0, 1, stolbci, h404_sym, human_sym); // строка
    value += line_value(board, 0, j, 1, 0, stroki, h404_sym, human_sym); // столбец
    if (stroki == stolbci)
    {
        if (i == j) // главная диагональ
            value += line_value(board, 0, 0, 1, 1, stroki, h404_sym, human_sym);
        if (i + j == stroki - 1) // побочная диагональ
            value += line_value(board, 0, stroki - 1, 1, -1, stroki, h404_sym, human_sym);
    }
    return value;
}

// случайная свободная клетка
static void random_cell(char** board, int stroki, int stolbci, int* res_i, int* res_j)
{
    int free_cnt = 0;
    for (int i = 0; i < stroki; i++)
        for (int j = 0; j < stolbci; j++)
            if (board[i][j] == '_') free_cnt++;

    int target = rand() % free_cnt; // номер клетки среди свободных
    for (int i = 0; i < stroki; i++)
    {
        for (int j = 0; j < stolbci; j++)
        {
            if (board[i][j] == '_')
            {
                if (target == 0) { *res_i = i; *res_j = j; return; }
                target--;
            }
        }
    }
}

/* самая выгодная свободная клетка
  если таких несколько, 404 выбирает случайную из них */
static void best_cell(char** board, int stroki, int stolbci, char h404_sym, char human_sym, int* res_i, int* res_j)
{
    int best = -1;
    int ties = 0;
    for (int i = 0; i < stroki; i++)
    {
        for (int j = 0; j < stolbci; j++)
        {
            if (board[i][j] != '_')
                continue;
            int value = cell_value(board, stroki, stolbci, i, j, h404_sym, human_sym);
            if (value > best)
            {
                best = value;
                ties = 1;
                *res_i = i;
                *res_j = j;
            }
            else if (value == best)
            {
                ties++;
                if (rand() % ties == 0)
                {
                    *res_i = i; *res_j = j;
                }
            }
        }
    }
}

typedef struct
{
    const char* text;
    const char* sprite;
}
Phrase;

void make_404_m(char** board, int stroki, int stolbci, char h404_sym, char human_sym)
{
    static const Phrase phrases[] = {
        { "Dymaesh' menya tak prosto perehitrit'?", "smile.GIF" },
        { "Moi hod. Poprobyi obigrat'.", "smile.PNG" },
        { "Interesnaya pozicia... no pobeda za mnoi.", "smle.GIF" },
        { "Ne speshi, ya yzhe proschitala tvoi sledyushii shag.", "smile.GIF" },
        { ". . .", "sad.GIF" },
        { "11010000 10101111 00100000 11010000 10110100 11010000 10111110 11010000 10111011 11010000 10110110 11010000 10111101 11010000 10110000 00100000 11010000 10111111 11010000 10111110 11010000 10110001 11010000 10110101 11010000 10110100 11010000 10111000 11010001 10000010 11010001 10001100","error.GIF" },
        { "Ya ne bydy poddavat'sya", "mog.GIF" },
        { "\n"
          "-------------===-------==.--------------\n"
          "-----------=,    /\\,,,*   /===----------\n"
          "--------=,\\|                  /.--------\n"
          "------=.|                       ,=------\n"
          "------=|        /\\   /\\         |.------\n"
          "------=,*     ___     ___      *,=------\n"
          "-------=/      ||     ||        \\=------\n"
          "------=.\\|                       \\------\n"
          "--------==/        3              ,-----\n"
          "-----------=,/*         |\\,|     \\-----\n"
          "------------.\\|/       \\|\\=--|   /.-----\n"
          "-----------.                .   *.------\n"
          "----------|                 /,* *.------\n"
          "----------                    ----------\n", "fnf.PNG" }
    };

    int idx = rand() % 8;
    printf("\n[ 404 ]: %s\n", phrases[idx].text);
    sprite_show(phrases[idx].sprite);
    printf("404 (%c) dumaet...\n", h404_sym);
    sprite_pause(3000);

    int i = -1, j = -1;

    // 1 - если можно выиграть следующим ходом, 404 пойдет на выигрыш
    if (find_winning_cell(board, stroki, stolbci, h404_sym, &i, &j))
    {
        board[i][j] = h404_sym;
        return;
    }
    // 2 - если игрок выигрывает следующим ходом, 404 блокирует
    if (rand() % 100 < BLOCK_CHANCE && find_winning_cell(board, stroki, stolbci, human_sym, &i, &j))
    {
        board[i][j] = h404_sym;
        return;
    }
    // 3 - 404 ходит на рандом, лазейка игроку
    if (rand() % 100 < MISTAKE_CHANCE)
    {
        random_cell(board, stroki, stolbci, &i, &j);
        board[i][j] = h404_sym;
        return;
    }
    // 4 - обычный ход
    best_cell(board, stroki, stolbci, h404_sym, human_sym, &i, &j);
    board[i][j] = h404_sym;
}

int check_win(char** board, int stroki, int stolbci, char symbol, int* win_type, int* win_idx, int* win_diag)
{
    // горизонталь - перебираются строки
    for (int i = 0; i < stroki; i++)
    {
        int win = 1;
        for (int j = 0; j < stolbci; j++)
        {
            if (board[i][j] != symbol)
            {
                win = 0; break;
            }
        }
        if (win) { *win_type = 1; *win_idx = i; return 1; }
    }
    // вертикаль - перебираются столбцы
    for (int j = 0; j < stolbci; j++)
    {
        int win = 1;
        for (int i = 0; i < stroki; i++)
        {
            if (board[i][j] != symbol)
            {
                win = 0;
                break;
            }
        }
        if (win)
        {
            *win_type = 2; *win_idx = j; return 1;
        }
    }
    // диагональ
    int d_type, d_r, d_c;
    if (check_diagonals(board, stroki, stolbci, symbol, &d_type, &d_r, &d_c))
    {
        *win_type = d_type; // 3 или 4
        *win_idx = d_r;     // строка, где начинается линия
        *win_diag = d_c;    // столбец, где начинается линия
        return 1;
    }
    return 0;
}

/* идёт от клетки(r, c) по диагонали с шагом(dr, dc) до края поля,
   возвращает 1, если клеток больше одной и все они равны symbol */
static int diag_full(char** board, int stroki, int stolbci, char symbol, int r, int c, int dr, int dc)
{
    int length = 0, count = 0;
    while (r >= 0 && r < stroki && c >= 0 && c < stolbci)
    {
        length++;
        if (board[r][c] == symbol) count++;
        r += dr;
        c += dc;
    }
    return (length > 1 && count == length);
}

/* type 3 = главная диагональ \,
   type 4 = побочная /,
   r0, c0 = откуда она начинается */
int check_diagonals(char** board, int stroki, int stolbci, char symbol, int* type, int* r0, int* c0)
{
    /* главные - 1\ начинаются в верхней строке
                 2\ начинаются в левом столбце  */
    for (int c = 0; c < stolbci; c++)
        if (diag_full(board, stroki, stolbci, symbol, 0, c, 1, 1))
        {
            *type = 3; *r0 = 0; *c0 = c; return 1;
        }
    for (int r = 1; r < stroki; r++)
        if (diag_full(board, stroki, stolbci, symbol, r, 0, 1, 1))
        {
            *type = 3; *r0 = r; *c0 = 0; return 1;
        }
    /* побочные - 1/ начинаются в верхней строке
                  2/ начинаются в правом столбце */
    for (int c = 0; c < stolbci; c++)
        if (diag_full(board, stroki, stolbci, symbol, 0, c, 1, -1))
        {
            *type = 4; *r0 = 0; *c0 = c; return 1;
        }
    for (int r = 1; r < stroki; r++)
        if (diag_full(board, stroki, stolbci, symbol, r, stolbci - 1, 1, -1))
        {
            *type = 4; *r0 = r; *c0 = stolbci - 1; return 1;
        }
    return 0;
}