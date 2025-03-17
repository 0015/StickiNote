#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "LVGL_WidgetManager.hpp"
#include "SplashScreen.hpp"

extern int screen_width;
extern int screen_height;

void createNotebookBackground(lv_obj_t *parent)
{
    lv_obj_t *notebook_bg = lv_obj_create(parent);
    lv_obj_set_size(notebook_bg, screen_width, screen_height);
    lv_obj_align(notebook_bg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(notebook_bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(notebook_bg, lv_color_hex(0xFDF3E7), LV_PART_MAIN);
    lv_obj_set_style_bg_color(notebook_bg, lv_color_hex(0xFDF3E7), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(notebook_bg, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(notebook_bg, LV_OPA_COVER, LV_STATE_DEFAULT);
    for (int y = 40; y < screen_height; y += 40)
    {
        lv_obj_t *line = lv_obj_create(notebook_bg);
        lv_obj_set_size(line, screen_width, 2);
        lv_obj_align(line, LV_ALIGN_TOP_MID, 0, y);
        lv_obj_set_style_bg_color(line, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
        lv_obj_set_style_bg_color(line, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(line, LV_OPA_50, LV_PART_MAIN );
        lv_obj_set_style_bg_opa(line, LV_OPA_50, LV_STATE_DEFAULT);
    }
    lv_obj_t *margin_line = lv_obj_create(notebook_bg);
    lv_obj_set_size(margin_line, 2, screen_height);
    lv_obj_align(margin_line, LV_ALIGN_LEFT_MID, 50, 0);
    lv_obj_set_style_bg_color(margin_line, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN );
    lv_obj_set_style_bg_color(margin_line, lv_palette_main(LV_PALETTE_RED), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(margin_line, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(margin_line, LV_OPA_70, LV_STATE_DEFAULT);

    lv_obj_invalidate(notebook_bg);
}

extern "C" void app_main(void)
{
    NVSManager::init();

    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,
            .sw_rotate = false,
        }};
    bsp_display_start_with_config(&cfg);
    bsp_display_backlight_on();

    bsp_display_lock(0);

    screen_width = BSP_LCD_H_RES;  
    screen_height = BSP_LCD_V_RES;  

    lv_obj_t *screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(screen, screen_width, screen_height);
    lv_obj_center(screen);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    createNotebookBackground(screen);

    lv_obj_t *kb = lv_keyboard_create(lv_screen_active());
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(kb, screen_width, screen_height / 3);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);

    static LVGL_WidgetManager widgetManager(screen, kb);
    show_splash(screen, [&]()
                {
        ESP_LOGI("MAIN", "Splash animation finished. Loading Post-its...");
        widgetManager.loadPostItsFromNVS();
    });

    bsp_display_unlock();
}