file(GLOB_RECURSE IMAGE_SOURCES images/*.c)

idf_component_register(SRCS "main.c" "lvgl_demo_ui.c" "lcd_panel_gc9a01.c" ${IMAGE_SOURCES}
                       INCLUDE_DIRS ".")
