#include <unistd.h>

#include "game_utils.h"

Point sumPoints(Point a, Point b){
  a.x += b.x;
  a.y += b.y;
  return a;
}

void draw(Display* display, uint8_t* lcd){
  for (register int i = 0; i < H_PIXELS * V_PIXELS; ++i){
    parlcd_write_data(lcd, display->pixels[i]);
  }
}

void changeLedColor(int id, uint32_t color){
  int ledOffset = id == 1 ? SPILED_REG_LED_RGB1_o : SPILED_REG_LED_RGB2_o;
  *(volatile uint32_t *)(LED + ledOffset) = color;
}

void printLine(char *string, uint32_t* buffer, Point pos, uint16_t letterColor, uint16_t bcgrColor){
    if (string == NULL) return;
    char c;
    int h, w, index, set;
    int len = 0;
    int i = 0;
    
    while ((c = string[i]) != '\0') {
        index = ((int) c) - font_winFreeSystem14x16.firstchar;

        for (h = 0; h < font_winFreeSystem14x16.height; ++h) {
            for (w = 0; w < font_winFreeSystem14x16.width[index]; ++w) {
                set = (
                  font_winFreeSystem14x16.bits[index * font_winFreeSystem14x16.height + h]
                  >> (font_winFreeSystem14x16.maxwidth - w)
                ) & 1;

                buffer[pos.x + len + w + (pos.y + h + 4) * H_PIXELS] = set ? letterColor : bcgrColor;
            }
        }

        len += font_winFreeSystem14x16.width[index];
        i++;
    }
}