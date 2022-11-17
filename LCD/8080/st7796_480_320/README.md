| Supported Targets | ESP32 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- |
# LVGL porting example (based on i80 interfaced LCD controller)

LVGL is an open-source graphics library for creating modern GUIs. It has plenty of built-in graphical elements with low memory footprint, which is friendly for embedded GUI applications.

This example can be taken as a skeleton of porting the LVGL library onto the `esp_lcd` driver layer. **Note** that, this example only focuses on the display interface, regardless of the input device driver.

The UI will display two images (one Espressif logo and another Espressif text), which have been converted into C arrays by the [online converting tool](https://lvgl.io/tools/imageconverter), and will be compiled directly into application binary.

This example is constructed by [IDF component manager](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-component-manager.html), all the external dependency will be handled by the CMake build system automatically. In this case, it will help download the lvgl from [registry](https://components.espressif.com/component/lvgl/lvgl), with the version specified in the [manifest file](main/idf_component.yml).

This example uses the [esp_timer](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_timer.html) to generate the ticks needed by LVGL. For more porting guides, please refer to [LVGL porting doc](https://docs.lvgl.io/master/porting/index.html).

## How to use the example

### Hardware Required

* An ESP development board
* An Intel 8080 interfaced (so called MCU interface or parallel interface) LCD (this example can use ST7789)
* An USB cable for power supply and programming

### Hardware Connection

The connection between ESP Board and the LCD is as follows:

```
   ESP Board                      LCD Screen
┌─────────────┐              ┌────────────────┐
│             │              │                │
│         3V3 ├─────────────►│ VCC            │
│             │              │                │
│         GND ├──────────────┤ GND            │
│             │              │                │
│  DATA[0..7] │◄────────────►│ DATA[0..7]     │
│             │              │                │
│          WR ├─────────────►│ WR             │
│             │              │                │
│          CS ├─────────────►│ CS             │
│             │              │                │
│         D/C ├─────────────►│ D/C            │
│             │              │                │
│         RST ├─────────────►│ RST            │
│             │              │                │
│    BK_LIGHT ├─────────────►│ BCKL           │
│             │              │                │
└─────────────┘              └────────────────┘
```

The GPIO number used by this example can be changed in [main.c](main/main.c).
Especially, please pay attention to the binary signal level used to turn the LCD backlight on, some LCD modules need a low level to turn it on, while others require a high level. You can change the backlight level macro `EXAMPLE_LCD_BK_LIGHT_ON_LEVEL` in [main.c](main/main.c).

### Build and Flash

Run `idf.py set-target <target-name>` to select one supported target that can run this example. This step will also apply the default Kconfig configurations into the `sdkconfig` file.

Run `idf.py -p PORT build flash monitor` to build, flash and monitor the project. A fancy animation will show up on the LCD as expected.

The first time you run `idf.py` for the example will cost extra time as the build system needs to address the component dependencies and downloads the missing components from registry into `managed_components` folder.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

### Example Output

```bash
I (0) cpu_start: Starting scheduler on APP CPU.
I (335) example: Initialize i80 bus
I (340) example: Install panel IO
I (343) example: Install ST7789 panel driver
I (608) example: Turn on LCD backlight
I (608) gpio: GPIO[21]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0
I (610) example: Initialize LVGL library
I (615) example: Register display driver to LVGL
I (622) example: Install LVGL tick timer
```


## Troubleshooting

* Program upload failure
    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.
    * Error message with `A fatal error occurred: Could not open /dev/ttyACM0, the port doesn't exist`: Please first make sure development board connected, then make board into "Download Boot" mode to upload by following steps:
        1. keep press "BOOT" button
        2. short press "RST" button
        3. release "BOOT".
        4. upload program and reset

For any technical queries, please open an [issue] (https://github.com/espressif/esp-idf/issues) on GitHub. We will get back to you soon.
