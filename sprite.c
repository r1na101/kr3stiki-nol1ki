#define _CRT_SECURE_NO_WARNINGS
#include "sprite.h"
// если не винда, то, к сожалению, без спрайтов
#if defined(_WIN32) && !defined(NO_SPRITES)

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define SPRITE_W    383
#define FONT_HEIGHT 22
#define APP_ICON_ID 101

static HANDLE g_sprite = NULL;

// во сколько раз экранное масштабирование винды больше 100%
static double dpi_scale(void)
{
    HDC dc = GetDC(NULL);
    int dpi = GetDeviceCaps(dc, LOGPIXELSX);
    ReleaseDC(NULL, dc);
    return dpi / 96.0;
}
// Настоящая (классическая) консоль имеет класс окна ConsoleWindowClass.
// В Windows Terminal GetConsoleWindow() возвращает служебное окно, которое двигать нельзя.
static int is_classic_console(HWND w)
{
    char cls[64];
    if (!w) return 0;
    GetClassNameA(w, cls, sizeof(cls));
    return strcmp(cls, "ConsoleWindowClass") == 0;
}
// крупный шрифт консоли
static void set_font_height(int height)
{
    CONSOLE_FONT_INFOEX f;
    memset(&f, 0, sizeof(f));
    f.cbSize = sizeof(f);
    f.dwFontSize.X = 0;
    f.dwFontSize.Y = (SHORT)height;
    f.FontFamily = FF_DONTCARE;
    f.FontWeight = FW_NORMAL;
    wcscpy(f.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &f);
}

void window_setup(int cols, int lines)
{
    SetProcessDPIAware();

    HWND con = GetConsoleWindow();
    if (!is_classic_console(con))
    {
        // XO_CLASSIC - защита от зацикливания
        if (getenv("XO_CLASSIC") == NULL)
        {
            char exe[MAX_PATH], cmd[MAX_PATH + 64];
            GetModuleFileNameA(NULL, exe, MAX_PATH);
            _putenv("XO_CLASSIC=1");
            sprintf(cmd, "start \"\" conhost.exe \"%s\"", exe);
            if (system(cmd) == 0)
                exit(0); // новая игра запущена в другом окне, это окно закрывается
        }
        return; // не вышло - игра идет в данном окне
    }

    // иконка 
    HICON ico = LoadIconA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(APP_ICON_ID));
    if (ico)
    {
        SendMessageA(con, WM_SETICON, ICON_BIG, (LPARAM)ico);
        SendMessageA(con, WM_SETICON, ICON_SMALL, (LPARAM)ico);
    }

    // крупный шрифт (до изменения размера окна, чтобы окно подогналось под него)
    set_font_height(FONT_HEIGHT);

    static int first_call = 1;
    RECT r, wa;
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &wa, 0);
    GetWindowRect(con, &r);
    int cx = (r.left + r.right) / 2;
    int cy = (r.top + r.bottom) / 2;
    if (first_call)
    {
        cx = (wa.left + wa.right) / 2;
        cy = (wa.top + wa.bottom) / 2;
        first_call = 0;
    }

    // размер окошка в символах
    char cmd[64];
    sprintf(cmd, "mode con cols=%d lines=%d >nul", cols, lines);
    system(cmd);
    // позиция окошка по умолчанию - центр экрана
    GetWindowRect(con, &r);
    int w = r.right - r.left;
    int h = r.bottom - r.top;
    int x = cx - w / 2;
    int y = cy - h / 2;
    if (x + w > wa.right) x = wa.right - w;
    if (y + h > wa.bottom) y = wa.bottom - h;
    if (x < wa.left) x = wa.left;
    if (y < wa.top) y = wa.top;
    SetWindowPos(con, NULL, x, y, w, h, SWP_NOZORDER);
}

void sprite_hide(void)
{
    if (g_sprite)
    {
        TerminateProcess(g_sprite, 0);
        CloseHandle(g_sprite);
        g_sprite = NULL;
    }
}

void sprite_show(const char* image_name)
{
    SetProcessDPIAware();
    char dir[MAX_PATH];
    GetModuleFileNameA(NULL, dir, MAX_PATH);
    char* slash = strrchr(dir, '\\');
    if (slash) *slash = '\0';

    // спрайт слева от консоли, если слева нет места, то справа
    double scale = dpi_scale();
    int sw = (int)(SPRITE_W * scale + 0.5);
    int sx = 50, sy = 50;
    HWND con = GetConsoleWindow();
    if (is_classic_console(con))
    {
        RECT r;
        GetWindowRect(con, &r);
        sx = r.left - sw - 10;
        if (sx < 0) sx = r.right + 10;
        sy = r.top;
    }
    char cmd[1024];
    sprintf(cmd,
        "powershell -NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass "
        "-File \"%s\\404\\show_sprite.ps1\" -Path \"%s\\404\\%s\" -X %d -Y %d -Scale %.2f -ParentPid %lu",
        dir, dir, image_name, sx, sy, scale, GetCurrentProcessId());

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        CloseHandle(pi.hThread);
        Sleep(700);
        sprite_hide();
        g_sprite = pi.hProcess;
    }
}

void sprite_pause(int ms)
{
    Sleep(ms);
}
// чудо заглушки для игры без спрайтов и настройки окошка
#else
void sprite_show(const char* image_name) { (void)image_name; }
void sprite_hide(void) {}
void sprite_pause(int ms) { (void)ms; }
void window_setup(int cols, int lines) { (void)cols; (void)lines; }
#endif