#ifndef NVS_MANAGER_HPP
#define NVS_MANAGER_HPP

#include "nvs_flash.h"
#include "nvs.h"
#include "cJSON.h"
#include <vector>
#include <string>

struct PostItData
{
    std::string id;
    int x, y, width, height;
    std::string text;
};

class NVSManager
{
public:
    static void init();
    static void updateSinglePostIt(const PostItData &postIt);
    static std::vector<PostItData> loadPostIts();
    static std::string generateUUID();
    static void removePostItFromNVS(const std::string &postItID);
};

#endif
