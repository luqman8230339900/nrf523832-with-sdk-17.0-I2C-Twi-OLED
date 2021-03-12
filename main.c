

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"

#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "SSD1306.h"
#include "binary.h"



#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"







#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

static const unsigned char /*PROGMEM*/ logo16_glcd_bmp[] = {
    B00000000, B11000000,
    B00000001, B11000000,
    B00000001, B11000000,
    B00000011, B11100000,
    B11110011, B11100000,
    B11111110, B11111000,
    B01111110, B11111111,
    B00110011, B10011111,
    B00011111, B11111100,
    B00001101, B01110000,
    B00011011, B10100000,
    B00111111, B11100000,
    B00111111, B11110000,
    B01111100, B11110000,
    B01110000, B01110000,
    B00000000, B00110000
};


#define SSD1306_CONFIG_SCL_PIN      27
#define SSD1306_CONFIG_SDA_PIN      26

/* TWI instance ID. */
#if TWI0_ENABLED
#define TWI_INSTANCE_ID     0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID     1
#endif

 /* Number of possible TWI addresses. */
 #define TWI_ADDRESSES      127

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
/////////////////////////////////



void testdrawchar(void)
{
    ssd1306_clear_display();
    ssd1306_set_textsize(1);
    ssd1306_set_textcolor(WHITE);
    ssd1306_set_cursor(0, 0);

    for (uint8_t i = 0; i < 168; i++) {
        if (i == '\n') continue;
        ssd1306_write(i);
        if ((i > 0) && (i % 21 == 0))
            ssd1306_write('\n');
    }
    ssd1306_display();
}


void testdrawline(void)
{
    for (int16_t i = 0; i < ssd1306_width(); i += 4) {
        ssd1306_draw_line(0, 0, i, ssd1306_height() - 1, WHITE);
        ssd1306_display();
    }
    for (int16_t i = 0; i < ssd1306_height(); i += 4) {
        ssd1306_draw_line(0, 0, ssd1306_width() - 1, i, WHITE);
        ssd1306_display();
    }
    nrf_delay_ms(250);

    ssd1306_clear_display();
    for (int16_t i = 0; i < ssd1306_width(); i += 4) {
        ssd1306_draw_line(0, ssd1306_height() - 1, i, 0, WHITE);
        ssd1306_display();
    }
    for (int16_t i = ssd1306_height() - 1; i >= 0; i -= 4) {
        ssd1306_draw_line(0, ssd1306_height() - 1, ssd1306_width() - 1, i, WHITE);
        ssd1306_display();
    }
    nrf_delay_ms(250);

    ssd1306_clear_display();
    for (int16_t i = ssd1306_width() - 1; i >= 0; i -= 4) {
        ssd1306_draw_line(ssd1306_width() - 1, ssd1306_height() - 1, i, 0, WHITE);
        ssd1306_display();
    }
    for (int16_t i = ssd1306_height() - 1; i >= 0; i -= 4) {
        ssd1306_draw_line(ssd1306_width() - 1, ssd1306_height() - 1, 0, i, WHITE);
        ssd1306_display();
    }
    nrf_delay_ms(250);

    ssd1306_clear_display();
    for (int16_t i = 0; i < ssd1306_height(); i += 4) {
        ssd1306_draw_line(ssd1306_width() - 1, 0, 0, i, WHITE);
        ssd1306_display();
    }
    for (int16_t i = 0; i < ssd1306_width(); i += 4) {
        ssd1306_draw_line(ssd1306_width() - 1, 0, i, ssd1306_height() - 1, WHITE);
        ssd1306_display();
    }
    nrf_delay_ms(250);

    ssd1306_display();
    nrf_delay_ms(250);
    ssd1306_clear_display();
}


void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h)
{
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

    uint8_t icons[NUMFLAKES][3];

    // initialize
    for (uint8_t f = 0; f < NUMFLAKES; f++) {
        icons[f][XPOS] = rand() % ssd1306_width();
        icons[f][YPOS] = 0;
        icons[f][DELTAY] = (rand() % 5) + 1;
    }

    while (1) {
        // draw each icon
        for (uint8_t f = 0; f < NUMFLAKES; f++) {
            ssd1306_draw_bitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h, WHITE);
        }
        ssd1306_display();
        nrf_delay_ms(200);

        // then erase it + move it
        for (uint8_t f = 0; f < NUMFLAKES; f++) {
            ssd1306_draw_bitmap(icons[f][XPOS], icons[f][YPOS],  logo16_glcd_bmp, w, h, BLACK);
            // move it
            icons[f][YPOS] += icons[f][DELTAY];
            // if its gone, reinit
            if (icons[f][YPOS] > ssd1306_height()) {
                icons[f][XPOS] = rand() % ssd1306_width();
                icons[f][YPOS] = 0;
                icons[f][DELTAY] = (rand() % 5) + 1;
            }
        }
    }
}




/**
 * @brief TWI initialization.
 */
void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_config = {
       .scl                = ARDUINO_SCL_PIN,
       .sda                = ARDUINO_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}


/**
 * @brief Function for main application entry.
 */
int main(void)
{
    ret_code_t err_code;
    uint8_t address;
    uint8_t sample_data;
    bool detected_device = false;

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    ssd1306_init_i2c(SSD1306_CONFIG_SCL_PIN, SSD1306_CONFIG_SDA_PIN);
    ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);


    NRF_LOG_INFO("TWI scanner started.");
    NRF_LOG_FLUSH();
    //twi_init();
       nrf_delay_ms(1000);

        NRF_LOG_INFO("--- START ---");

        nrf_delay_ms(1000);
        ssd1306_display();
        nrf_delay_ms(1000);

        NRF_LOG_INFO("--- testdrawline ---");
        //
        ssd1306_draw_circle(SSD1306_LCDWIDTH / 2, SSD1306_LCDHEIGHT / 2, 30, WHITE);
        ssd1306_display();
         //nrf_delay_ms(1000);
         //display.drawString(0, 0, "Hello world");
         // nrf_delay_ms(1000);

         for (;;)
        {
                ssd1306_clear_display();
                // draw a single pixel
                ssd1306_draw_pixel(10, 10, WHITE);
                ssd1306_display();
                nrf_delay_ms(1000);

                ssd1306_draw_circle(SSD1306_LCDWIDTH / 2, SSD1306_LCDHEIGHT / 2, 30, WHITE);
                ssd1306_display();
                nrf_delay_ms(1000);

                testdrawchar();
                nrf_delay_ms(1000);

                ssd1306_clear_display();
                ssd1306_display();
                nrf_delay_ms(1000);

                testdrawline();

                ssd1306_clear_display();
                ssd1306_draw_bitmap(30, 16,  logo16_glcd_bmp, 16, 16, 1);
                ssd1306_display();
                nrf_delay_ms(1000);

                // draw a bitmap icon and 'animate' movement
                testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);

                
        }
  

    

    while (true)
    {
        /* Empty loop. */
    }
}

/** @} */
