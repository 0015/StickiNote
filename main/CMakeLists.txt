# List of source files
set(SOURCES
    "main.cpp"
    "LVGL_WidgetManager.cpp"
    "SplashScreen.cpp"
    "NVSManager.cpp"
    "ui_resources/icon_resize.c"
    "ui_resources/notes.c"
    "ui_resources/sticky_font_32.c"
)

# List of include directories
set(INCLUDE_DIRS
    "include"
)

# Required components
set(REQUIRED_COMPONENTS
    nvs_flash
    json
    lvgl   
    esp32_p4_nano
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES ${REQUIRED_COMPONENTS}
)
