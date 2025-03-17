#include "NVSManager.hpp"
#include "esp_log.h"
#include "esp_random.h"
#include <cstdlib>
#include <cstdio>

std::string NVSManager::generateUUID() {
    char uuid[9];
    sprintf(uuid, "%08X", esp_random());
    ESP_LOGI("NVS", "ID: %s", uuid);
    return std::string(uuid);
}

void NVSManager::init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
}

void NVSManager::updateSinglePostIt(const PostItData& postIt) {
    if (postIt.id.empty()) return;

    std::vector<PostItData> postIts = loadPostIts();

    bool found = false;
    for (auto& p : postIts) {
        if (p.id == postIt.id) {
            p = postIt;
            found = true;
            break;
        }
    }

    if (!found) {
        ESP_LOGW("NVS", "Post-it with ID %s not found, adding a new one.", postIt.id.c_str());
        postIts.push_back(postIt);
    }
    cJSON *root = cJSON_CreateArray();
    for (const auto& p : postIts) {
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
    ESP_LOGI("NVS", "Updated JSON: %s", json_str);
    cJSON_Delete(root);

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to open NVS for writing!");
        free(json_str);
        return;
    }

    err = nvs_set_str(nvs_handle, "postits", json_str);
    if (err == ESP_OK) {
        ESP_LOGI("NVS", "Post-its successfully updated in NVS.");
    } else {
        ESP_LOGE("NVS", "Failed to write Post-its to NVS.");
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to commit data to NVS!");
    } else {
        ESP_LOGI("NVS", "NVS commit successful.");
    }

    nvs_close(nvs_handle);
    free(json_str);
}

std::vector<PostItData> NVSManager::loadPostIts() {
    std::vector<PostItData> postIts;
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);

    if (err != ESP_OK) {
        ESP_LOGW("NVS", "Failed to open NVS for reading.");
        return postIts;
    }

    size_t required_size = 0;
    err = nvs_get_str(nvs_handle, "postits", NULL, &required_size);
    if (err != ESP_OK || required_size == 0) {
        ESP_LOGW("NVS", "No saved Post-its in NVS.");
        nvs_close(nvs_handle);
        return postIts;
    }

    char *json_str = (char*)malloc(required_size);
    err = nvs_get_str(nvs_handle, "postits", json_str, &required_size);
    nvs_close(nvs_handle);

    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to read Post-its from NVS.");
        free(json_str);
        return postIts;
    }

    ESP_LOGI("NVS", "Loaded JSON from NVS: %s", json_str);

    cJSON *root = cJSON_Parse(json_str);
    free(json_str);

    if (!root) {
        ESP_LOGE("NVS", "Failed to parse JSON from NVS.");
        return postIts;
    }

    int count = cJSON_GetArraySize(root);
    ESP_LOGI("NVS", "Restoring %d Post-it notes", count);

    for (int i = 0; i < count; i++) {
        cJSON *postit_obj = cJSON_GetArrayItem(root, i);
        if (!postit_obj) continue;

        PostItData data;
        data.id = cJSON_GetObjectItem(postit_obj, "id")->valuestring;
        data.x = cJSON_GetObjectItem(postit_obj, "x")->valueint;
        data.y = cJSON_GetObjectItem(postit_obj, "y")->valueint;
        data.width = cJSON_GetObjectItem(postit_obj, "width")->valueint;
        data.height = cJSON_GetObjectItem(postit_obj, "height")->valueint;
        data.text = cJSON_GetObjectItem(postit_obj, "text")->valuestring;

        postIts.push_back(data);
    }

    cJSON_Delete(root);
    return postIts;
}
