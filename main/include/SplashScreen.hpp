#ifndef SPLASH_SCREEN_HPP
#define SPLASH_SCREEN_HPP

#include <functional>
#include <lvgl.h>

extern int screen_width;
extern int screen_height;

void show_splash(lv_obj_t *screen, std::function<void()> on_complete);

#endif