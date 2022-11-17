file(GLOB_RECURSE IMAGE_SOURCES images/*.c)

set(LV_DEMO_DIR ../managed_components/lvgl__lvgl/demos)
file(GLOB_RECURSE LV_DEMOS_SOURCES ${LV_DEMO_DIR}/*.c)

idf_component_register(
    SRCS "main.c" "lvgl_demo_ui.c" "lcd_panel_st7796.c" ${IMAGE_SOURCES} ${LV_DEMOS_SOURCES}
    INCLUDE_DIRS "." ${LV_DEMO_DIR})

target_compile_definitions(${COMPONENT_LIB}
                            PUBLIC "-DLV_LVGL_H_INCLUDE_SIMPLE")
