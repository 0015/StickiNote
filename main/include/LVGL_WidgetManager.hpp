#ifndef LVGL_WIDGET_MANAGER_H
#define LVGL_WIDGET_MANAGER_H

#include <lvgl.h>
#include <vector>
#include <string>
#include "NVSManager.hpp"

class LVGL_WidgetManager
{
public:
    LVGL_WidgetManager(lv_obj_t *screen, lv_obj_t *kb);
    void createPostIt(int x, int y, int width, int height, const char *text, std::string id);
    void removePostIt(lv_obj_t *postIt);
    void deleteAllWidgets();
    int getWidgetCount() const;
    void loadPostItsFromNVS();
    void saveSinglePostItToNVS(lv_obj_t *postit);

private:
    static lv_obj_t *screen_;
    static lv_obj_t *kb_;
    static LVGL_WidgetManager *instance;
    static std::vector<lv_obj_t *> widgets_;

    static int screen_width_;
    static int screen_height_;

    static void btn_event_cb(lv_event_t *e);
    static void textarea_event_handler(lv_event_t *e);
    static void textarea_kb_event_handler(lv_event_t *e);
    static void drag_event_handler(lv_event_t *e);
};

#endif
