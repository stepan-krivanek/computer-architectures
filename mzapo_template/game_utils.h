#pragma once
#include <stdint.h>

#include "mzapo_regs.h"
#include "mzapo_phys.h"
#include "mzapo_parlcd.h"

#define LED map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0)

#define H_PIXELS 480
#define V_PIXELS 320
#define SNAKE_SIZE 32

#define WHITE_16 0xFFFF
#define BLACK_16 0x0000
#define GREEN_16 0x07E0
#define RED_16 0xF800

#define RED_32 0xFF0000
#define GREEN_32 0x00FF00
#define BLUE_32 0x0000FF

typedef struct {
  const int width;
  const int height;
  uint16_t* pixels;
} Display;

typedef struct {
  int x;
  int y;
} Point;

typedef struct Node{
  struct Node* next;
  struct Node* prev;
  Point point;
} Node;

typedef struct {
  const int size;
  int length;
  int id;
  Node* head;
  Node* tail;
  Point dir;
} Snake;

Point sumPoints(Point a, Point b);

// Flushes display buffer onto the lcd
void draw(Display* display, uint8_t* lcd);

// Changes the color of rgb led by id (should be only 1 or 2)
void changeLedColor(int id, uint32_t color);