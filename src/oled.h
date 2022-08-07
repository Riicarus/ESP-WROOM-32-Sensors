#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define OLED_CLK 18
#define OLED_MOSI 19
#define OLED_RES 16
#define OLED_DC 17
#define OLED_CS 5

#define OLED_ADDRESS 0x3C

U8G2_SH1106_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_CS, OLED_DC, OLED_RES);

std::string two_part_mode_cache;

void oled_setup()
{
    u8g2.begin();
    u8g2.enableUTF8Print();
}

void oled_draw_in_two(const char *chars)
{
    u8g2.setFont(u8g2_font_unifont_t_chinese2);
    u8g2.firstPage();
    do
    {
        u8g2.setCursor(0, 20);
        u8g2.print(two_part_mode_cache.c_str());
        u8g2.setCursor(0, 40);
        u8g2.print(chars);
    } while (u8g2.nextPage());

    two_part_mode_cache = chars;
}

#endif