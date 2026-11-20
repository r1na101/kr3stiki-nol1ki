#ifndef SPRITE_H
#define SPRITE_H

void sprite_show(const char* image_name);

void sprite_hide(void);

void sprite_pause(int ms);
/* компактное окошко консоли - cols символов в ширину, lines в высоту,
   расположение - центр экрана */
void window_setup(int cols, int lines);
#endif