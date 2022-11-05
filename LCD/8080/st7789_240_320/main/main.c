/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"

#include "lvgl.h"
#include "lcd_panel_st7789.h"

static const char *TAG = "example";

static lv_disp_drv_t disp_drv;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LCD_PIXEL_CLOCK_HZ      (20 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL   (1)
#define LCD_BK_LIGHT_OFF_LEVEL  (!EXAMPLE_LCD_BK_LIGHT_ON_LEVEL)

#define PIN_NUM_LCD_WR          (GPIO_NUM_37)
#define PIN_NUM_LCD_CS          (GPIO_NUM_36)
#define PIN_NUM_LCD_DC          (GPIO_NUM_38)
#define PIN_NUM_LCD_DATA0       (GPIO_NUM_3)
#define PIN_NUM_LCD_DATA1       (GPIO_NUM_4)
#define PIN_NUM_LCD_DATA2       (GPIO_NUM_5)
#define PIN_NUM_LCD_DATA3       (GPIO_NUM_6)
#define PIN_NUM_LCD_DATA4       (GPIO_NUM_7)
#define PIN_NUM_LCD_DATA5       (GPIO_NUM_8)
#define PIN_NUM_LCD_DATA6       (GPIO_NUM_9)
#define PIN_NUM_LCD_DATA7       (GPIO_NUM_10)
#define PIN_NUM_LCD_DATA8       (GPIO_NUM_11)
#define PIN_NUM_LCD_DATA9       (GPIO_NUM_12)
#define PIN_NUM_LCD_DATA10      (GPIO_NUM_13)
#define PIN_NUM_LCD_DATA11      (GPIO_NUM_14)
#define PIN_NUM_LCD_DATA12      (GPIO_NUM_15)
#define PIN_NUM_LCD_DATA13      (GPIO_NUM_16)
#define PIN_NUM_LCD_DATA14      (GPIO_NUM_17)
#define PIN_NUM_LCD_DATA15      (GPIO_NUM_18)
#define PIN_NUM_BK_LIGHT        (GPIO_NUM_21)
#define PIN_NUM_LCD_RST         (GPIO_NUM_NC)

#define LCD_H_RES               (240)
#define LCD_V_RES               (320)

// Bit number used to represent command and parameter
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

#define LVGL_TICK_PERIOD_MS    2

extern void lvgl_demo_ui(lv_obj_t *disp);

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

static void increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

void app_main(void)
{
    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)

    ESP_LOGI(TAG, "Initialize i80 bus");
    esp_lcd_i80_bus_config_t i80_config = {
        .bus_width = 16,
        .clk_src = LCD_CLK_SRC_PLL160M,
        .max_transfer_bytes = LCD_H_RES * LCD_V_RES * 2 + 10,
        .psram_trans_align = 64,
        .sram_trans_align = 4,
        .dc_gpio_num = PIN_NUM_LCD_DC,
        .wr_gpio_num = PIN_NUM_LCD_WR,
        .data_gpio_nums = {
            PIN_NUM_LCD_DATA0,
            PIN_NUM_LCD_DATA1,
            PIN_NUM_LCD_DATA2,
            PIN_NUM_LCD_DATA3,
            PIN_NUM_LCD_DATA4,
            PIN_NUM_LCD_DATA5,
            PIN_NUM_LCD_DATA6,
            PIN_NUM_LCD_DATA7,
            PIN_NUM_LCD_DATA8,
            PIN_NUM_LCD_DATA9,
            PIN_NUM_LCD_DATA10,
            PIN_NUM_LCD_DATA11,
            PIN_NUM_LCD_DATA12,
            PIN_NUM_LCD_DATA13,
            PIN_NUM_LCD_DATA14,
            PIN_NUM_LCD_DATA15,
        },
    };
    esp_lcd_i80_bus_handle_t i80_bus;
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&i80_config, &i80_bus));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_i80_config_t io_config = {
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .cs_gpio_num = PIN_NUM_LCD_CS,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .dc_levels = {
            .dc_cmd_level = 0,
            .dc_data_level = 1,
            .dc_dummy_level = 0,
            .dc_idle_level = 0,
        },
        .trans_queue_depth = 10,
        .on_color_trans_done = notify_lvgl_flush_ready,
        .user_ctx = (void *)&disp_drv,
    };
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install ST7789 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_LCD_RST,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
#else
    ESP_ERROR_CHECK(esp_lcd_panel_disp_off(panel_handle, false));
#endif

    if (PIN_NUM_BK_LIGHT != GPIO_NUM_NC) {
        ESP_LOGI(TAG, "Turn on LCD backlight");
        gpio_config_t bk_gpio_config = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << PIN_NUM_BK_LIGHT
        };
        ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
        gpio_set_level(PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_ON_LEVEL);
    }

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    // alloc draw buffers used by LVGL
    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
    lv_color_t *buf1 = heap_caps_malloc(LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
    // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, LCD_H_RES * 20);

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    lvgl_demo_ui(lv_scr_act());

    while (1) {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }
}
