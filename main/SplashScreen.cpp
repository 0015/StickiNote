#include "SplashScreen.hpp"
#include "esp_log.h"

#define SPLASH_ANIM_TIME 400
#define SPLASH_HOLD_TIME 1500

LV_IMG_DECLARE(notes);

int screen_width = 800;  // Default value, will be set in main
int screen_height = 480;

struct SplashData {
    lv_obj_t *splash_img;
    std::function<void()> on_complete;
};
static void splash_exit_anim_cb(lv_anim_t *a) {
    SplashData *data = (SplashData *)lv_anim_get_user_data(a);
    lv_obj_del(data->splash_img);
    if (data->on_complete) data->on_complete();
    delete data;
}
static void splash_hold_timer_cb(lv_timer_t *t) {
    SplashData *data = (SplashData *)lv_timer_get_user_data(t);
    lv_anim_t slide_exit;
    lv_anim_init(&slide_exit);
    lv_anim_set_var(&slide_exit, data->splash_img);
    lv_anim_set_values(&slide_exit, screen_height / 4, screen_height);
    lv_anim_set_time(&slide_exit, SPLASH_ANIM_TIME);
    lv_anim_set_exec_cb(&slide_exit, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_deleted_cb(&slide_exit, splash_exit_anim_cb);
    lv_anim_set_user_data(&slide_exit, data);

    lv_anim_start(&slide_exit);
    lv_timer_del(t);
}
static void splash_anim_cb(lv_anim_t *a) {
    SplashData *data = (SplashData *)lv_anim_get_user_data(a);
    lv_timer_t *hold_timer = lv_timer_create(splash_hold_timer_cb, SPLASH_HOLD_TIME, data);
    lv_timer_set_user_data(hold_timer, data);
}
void show_splash(lv_obj_t *screen, std::function<void()> on_complete) {
    SplashData *data = new SplashData;
    data->splash_img = lv_img_create(screen);
    lv_img_set_src(data->splash_img, &notes);
    lv_obj_align(data->splash_img, LV_ALIGN_TOP_MID, 0, -screen_height / 2);
    data->on_complete = on_complete;
    lv_anim_t slide_down;
    lv_anim_init(&slide_down);
    lv_anim_set_var(&slide_down, data->splash_img);
    lv_anim_set_values(&slide_down, -screen_height / 2, screen_height / 4);
    lv_anim_set_time(&slide_down, SPLASH_ANIM_TIME);
    lv_anim_set_exec_cb(&slide_down, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_ready_cb(&slide_down, splash_anim_cb);
    lv_anim_set_user_data(&slide_down, data);

    lv_anim_start(&slide_down);
}