#include "LVGL_WidgetManager.hpp"
#include "esp_log.h"
#include <algorithm>
#include "esp_timer.h"

#define DOUBLE_TAP_TIME 300
#define MAX_POSTITS 10

LV_IMG_DECLARE(icon_resize);
LV_FONT_DECLARE(sticky_font_32)

lv_obj_t *LVGL_WidgetManager::screen_ = nullptr;
lv_obj_t *LVGL_WidgetManager::kb_ = nullptr;
LVGL_WidgetManager *LVGL_WidgetManager::instance = nullptr;
int LVGL_WidgetManager::screen_width_ = 0;
int LVGL_WidgetManager::screen_height_ = 0;
std::vector<lv_obj_t *> LVGL_WidgetManager::widgets_;

LVGL_WidgetManager::LVGL_WidgetManager(lv_obj_t *screen, lv_obj_t *kb)
{

  instance = this;
  screen_ = screen;
  kb_ = kb;
  widgets_.reserve(MAX_POSTITS);

  printf("LVGL_WidgetManager Initialized - Parent: %p, Keyboard: %p\n", screen_, kb_);

  screen_width_ = lv_obj_get_width(lv_screen_active());
  screen_height_ = lv_obj_get_height(lv_screen_active());

  printf("LVGL_WidgetManager Initialized - screen_width_: %d, screen_height_: %d\n", screen_width_, screen_height_);
  static lv_style_t style_fab;
  lv_style_init(&style_fab);

  lv_style_set_radius(&style_fab, LV_RADIUS_CIRCLE);
  lv_style_set_bg_opa(&style_fab, LV_OPA_COVER);
  lv_style_set_bg_color(&style_fab, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_shadow_width(&style_fab, 10);
  lv_style_set_shadow_color(&style_fab, lv_palette_darken(LV_PALETTE_GREY, 2));
  lv_style_set_text_color(&style_fab, lv_color_white());
  lv_obj_t *fab_btn = lv_btn_create(screen_);
  lv_obj_set_size(fab_btn, 80, 80);
  lv_obj_add_style(fab_btn, &style_fab, LV_PART_MAIN);
  lv_obj_align(fab_btn, LV_ALIGN_BOTTOM_RIGHT, 20, 20);
  lv_obj_set_style_bg_image_src(fab_btn, LV_SYMBOL_PLUS, 0);
  lv_obj_add_event_cb(fab_btn, [](lv_event_t *e)
  {
    if (instance)
    { 
      ESP_LOGE("LVGL", "fab_btn_event_cb");
      instance->createPostIt(20, 20, 300, 300, "", NVSManager::generateUUID());
    }
  }, LV_EVENT_CLICKED, nullptr);
}

void LVGL_WidgetManager::createPostIt(int x, int y, int width, int height, const char* text, std::string id)
{

  if (widgets_.size() >= MAX_POSTITS)
  {
    ESP_LOGW("LVGL", "Maximum Post-it limit reached (%d). Cannot create more.", MAX_POSTITS);
    return;
  }

  if (!screen_)
  {
    ESP_LOGE("LVGL", "Error: Parent object is NULL!");
    return;
  }

  if (!kb_)
  {
    printf("Warning: kb_ is NULL. Creating a default keyboard.\n");
    return;
  }

  static lv_style_t style_postit;
  lv_style_init(&style_postit);

  lv_style_set_bg_opa(&style_postit, LV_OPA_COVER);
  lv_style_set_bg_color(&style_postit, lv_color_make(230, 230, 230));
  lv_style_set_border_color(&style_postit, lv_color_black());
  lv_style_set_border_width(&style_postit, 2);
  lv_style_set_radius(&style_postit, 5);

  lv_obj_t *postit = lv_obj_create(screen_);
  if (!postit)
  {
    ESP_LOGE("LVGL", "Error: Failed to create Post-it object!");
    return;
  }
  lv_obj_add_style(postit, &style_postit, LV_PART_MAIN);
  lv_obj_clear_flag(postit, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_pos(postit, x, y);
  lv_obj_set_size(postit, width, height);

  static lv_style_t style_textarea;
  lv_style_init(&style_textarea);

  lv_style_set_bg_opa(&style_textarea, LV_OPA_COVER);
  lv_style_set_bg_color(&style_textarea, lv_color_white());
  lv_style_set_border_color(&style_textarea, lv_color_black());
  lv_style_set_border_width(&style_textarea, 1);
  lv_style_set_text_color(&style_textarea, lv_color_black());
  lv_style_set_pad_all(&style_textarea, 5);
  lv_style_set_text_font(&style_textarea, &sticky_font_32);

  lv_obj_t *textarea = lv_textarea_create(postit); 
  lv_obj_set_size(textarea, width - 20, height - 30);
  lv_obj_align(textarea, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_add_style(textarea, &style_textarea, LV_PART_MAIN);
  lv_obj_clear_flag(textarea, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(textarea, textarea_event_handler, LV_EVENT_PRESSING, nullptr);
  lv_obj_add_event_cb(textarea, textarea_event_handler, LV_EVENT_RELEASED, nullptr);
  lv_obj_add_event_cb(textarea, textarea_kb_event_handler, LV_EVENT_CLICKED, kb_);
  lv_obj_add_event_cb(textarea, textarea_kb_event_handler, LV_EVENT_DEFOCUSED, kb_);
  lv_keyboard_set_textarea(kb_, textarea);

  if (text == nullptr || text[0] == '\0') {
    lv_textarea_set_placeholder_text(textarea, "Enter text here...");
  }else{
    lv_textarea_set_text(textarea, text);
    lv_textarea_set_cursor_pos(textarea, 0);
  }
  lv_obj_t *close_btn = lv_btn_create(postit);
  lv_obj_set_size(close_btn, 30, 30);
  lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, 20, -20);
  lv_obj_set_style_bg_color(close_btn, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);

  lv_obj_t *close_label = lv_label_create(close_btn);
  lv_label_set_text(close_label, "X");
  lv_obj_center(close_label);
  lv_obj_add_event_cb(close_btn, [](lv_event_t *e)
                      {
        lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
        lv_obj_t* postIt = (lv_obj_t*)lv_obj_get_parent(btn);
        LVGL_WidgetManager::instance->removePostIt(postIt); }, LV_EVENT_CLICKED, nullptr);

  lv_obj_t *resize_btn = lv_button_create(postit);
  lv_obj_set_size(resize_btn, 40, 40);
  lv_obj_align(resize_btn, LV_ALIGN_BOTTOM_RIGHT, 20, 20);
  lv_obj_set_style_bg_opa(resize_btn, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_border_color(resize_btn, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
  lv_obj_set_style_border_width(resize_btn, 2, LV_PART_MAIN);

  lv_obj_t *resize_icon = lv_img_create(resize_btn);
  lv_img_set_src(resize_icon, &icon_resize);
  lv_obj_center(resize_icon);

  lv_obj_add_event_cb(resize_btn, drag_event_handler, LV_EVENT_PRESSING, textarea);
  lv_obj_add_event_cb(resize_btn, drag_event_handler, LV_EVENT_RELEASED, textarea);

  
  lv_obj_set_user_data(postit, (void *)new std::string(id));
  widgets_.push_back(postit);
  ESP_LOGE("LVGL", "Post-it created. Total count: %d\n", (int)widgets_.size());

  PostItData postItData = {id, x, y, width, height, text};
  NVSManager::updateSinglePostIt(postItData);
}

void LVGL_WidgetManager::removePostIt(lv_obj_t *postit)
{
  if (!postit) return;
  std::string *id_ptr = static_cast<std::string *>(lv_obj_get_user_data(postit));
  if (!id_ptr) {
      ESP_LOGE("LVGL", "Failed to get Post-it ID for deletion.");
      return;
  }

  std::string postItID = *id_ptr;
  delete id_ptr;
  lv_obj_del(postit);
  auto it = std::find(widgets_.begin(), widgets_.end(), postit);
  if (it != widgets_.end()) {
      widgets_.erase(it);
  }
  NVSManager::removePostItFromNVS(postItID);
  ESP_LOGI("LVGL", "Post-it removed successfully. ID: %s", postItID.c_str());
}

void LVGL_WidgetManager::deleteAllWidgets()
{
  for (auto widget : widgets_)
  {
    lv_obj_del(widget);
  }
  widgets_.clear();
}

int LVGL_WidgetManager::getWidgetCount() const
{
  return widgets_.size();
}

void LVGL_WidgetManager::btn_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED)
  {
    static uint8_t cnt = 0;
    cnt++;

    /*Get the first child of the button which is the label and change its text*/
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text_fmt(label, "Button: %d", cnt);
  }
}

void LVGL_WidgetManager::drag_event_handler(lv_event_t *e)
{

  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *postit = (lv_obj_t *)lv_obj_get_parent(obj);
  if (postit == NULL) return;

  if (code == LV_EVENT_RELEASED) {
    ESP_LOGI("LVGL", "Post-it resize finalized and saved.");
    instance->saveSinglePostItToNVS(postit);
    return;
  }

  lv_indev_t *indev = lv_indev_active();
  if (indev == NULL) return;

  lv_point_t vect;
  lv_indev_get_vect(indev, &vect);

  if (vect.x == 0 && vect.y == 0) return;
  
  lv_obj_move_foreground(postit);

  int32_t x = lv_obj_get_width(postit) + vect.x;
  int32_t y = lv_obj_get_height(postit) + vect.y;
  lv_obj_set_size(postit, x, y);

  lv_obj_t *textarea = (lv_obj_t *)lv_event_get_user_data(e);
  if (textarea == NULL) return;

  x = lv_obj_get_width(textarea) + vect.x;
  y = lv_obj_get_height(textarea) + vect.y;
  lv_obj_set_size(textarea, x, y);
}

void LVGL_WidgetManager::textarea_event_handler(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *textarea = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *postit = (lv_obj_t *)lv_obj_get_parent(textarea);

  if (!postit || !screen_) return;

  lv_obj_move_foreground(postit);

  if (code == LV_EVENT_PRESSING){
    lv_indev_t *indev = lv_indev_active();
    if (indev == NULL)
      return;

    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);

    int32_t x = lv_obj_get_x_aligned(postit) + vect.x;
    int32_t y = lv_obj_get_y_aligned(postit) + vect.y;
    lv_obj_set_pos(postit, x, y);
  }else if (code == LV_EVENT_RELEASED){
      instance->saveSinglePostItToNVS(postit);
  }
}

void LVGL_WidgetManager::textarea_kb_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *textarea = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  lv_obj_t *postit = (lv_obj_t *)lv_obj_get_parent(textarea);
  
  if (!postit || !screen_)
    return;

  int screen_mid = screen_height_ / 2;
  int postit_y = lv_obj_get_y_aligned(postit);
  int kb_height = screen_height_ / 3;

  static int original_screen_y = 0;

  static int64_t last_tap_time = 0;
  int64_t current_time = esp_timer_get_time() / 1000;

  if (code == LV_EVENT_CLICKED) {
      if (current_time - last_tap_time < DOUBLE_TAP_TIME) {
          ESP_LOGI("LVGL", "Double-tap detected! Opening keyboard.");

          lv_obj_move_foreground(postit);
          lv_keyboard_set_textarea(kb, textarea);
          lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
          if (postit_y > screen_mid)
          {
            original_screen_y = lv_obj_get_y_aligned(screen_);
            lv_obj_set_y(screen_, original_screen_y - kb_height);
          }
      }
      last_tap_time = current_time;
  }else if (code == LV_EVENT_DEFOCUSED)
  {
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_set_y(screen_, original_screen_y);
    instance->saveSinglePostItToNVS(postit);
  }
}

void LVGL_WidgetManager::saveSinglePostItToNVS(lv_obj_t *postit)
{
  std::string *id_ptr = static_cast<std::string *>(lv_obj_get_user_data(postit));
  ESP_LOGI("LVGL", "saveSinglePostItToNVS: Post-it ID: %s", *id_ptr);

  if (!id_ptr)
    return;

  PostItData postIt;
  postIt.id = *id_ptr;
  postIt.x = lv_obj_get_x(postit);
  postIt.y = lv_obj_get_y(postit);
  postIt.width = lv_obj_get_width(postit);
  postIt.height = lv_obj_get_height(postit);
  postIt.text = lv_textarea_get_text(lv_obj_get_child(postit, 0));

  NVSManager::updateSinglePostIt(postIt);
}

void LVGL_WidgetManager::loadPostItsFromNVS() {
  std::vector<PostItData> postIts = NVSManager::loadPostIts();
  if (postIts.empty()) {
      ESP_LOGW("LVGL", "No Post-its found in NVS.");
      return;
  }

  for (const auto& postIt : postIts) {
      ESP_LOGI("LVGL", "Creating Post-it ID: %s at (%d, %d)", postIt.id.c_str(), postIt.x, postIt.y);
      createPostIt(postIt.x, postIt.y, postIt.width, postIt.height, postIt.text.c_str(), postIt.id);
  }
}

void NVSManager::removePostItFromNVS(const std::string &postItID) {
  std::vector<PostItData> postIts = loadPostIts();
  auto it = std::remove_if(postIts.begin(), postIts.end(),
                           [&](const PostItData &p) { return p.id == postItID; });

  if (it == postIts.end()) {
      ESP_LOGW("NVS", "Post-it with ID %s not found in NVS.", postItID.c_str());
      return;
  }

  postIts.erase(it, postIts.end());
  cJSON *root = cJSON_CreateArray();
  for (const auto &p : postIts) {
      cJSON *postit_obj = cJSON_CreateObject();
      cJSON_AddStringToObject(postit_obj, "id", p.id.c_str());
      cJSON_AddNumberToObject(postit_obj, "x", p.x);
      cJSON_AddNumberToObject(postit_obj, "y", p.y);
      cJSON_AddNumberToObject(postit_obj, "width", p.width);
      cJSON_AddNumberToObject(postit_obj, "height", p.height);
      cJSON_AddStringToObject(postit_obj, "text", p.text.c_str());
      cJSON_AddItemToArray(root, postit_obj);
  }

  char *json_str = cJSON_Print(root);
  cJSON_Delete(root);

  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
  if (err != ESP_OK) {
      ESP_LOGE("NVS", "Failed to open NVS for deletion!");
      free(json_str);
      return;
  }

  err = nvs_set_str(nvs_handle, "postits", json_str);
  if (err == ESP_OK) {
      ESP_LOGI("NVS", "Post-it successfully deleted from NVS.");
  } else {
      ESP_LOGE("NVS", "Failed to update NVS after deletion.");
  }

  err = nvs_commit(nvs_handle);
  if (err != ESP_OK) {
      ESP_LOGE("NVS", "Failed to commit NVS after deletion!");
  } else {
      ESP_LOGI("NVS", "NVS commit successful after deletion.");
  }

  nvs_close(nvs_handle);
  free(json_str);
}
