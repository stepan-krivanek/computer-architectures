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