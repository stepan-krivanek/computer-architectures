#pragma once
#include <stdint.h>
#include <stdlib.h>

#include "mzapo_regs.h"
#include "mzapo_phys.h"
#include "mzapo_parlcd.h"
#include "font_types.h"

#define LED map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0)

#define H_PIXELS 480
#define V_PIXELS 320
#define SNAKE_SIZE 20

// Colors for 16 bit pixels
#define WHITE_16 0xFFFF
#define BLACK_16 0x0000
#define GREEN_16 0x07E0
#define BLUE_16 0x001F
#define RED_16 0xF800

// Colors for 32 bit pixels
#define RED_32 0xFF0000
#define GREEN_32 0x00FF00
#define BLUE_32 0x0000FF

typedef struct {
  int x;
  int y;
} Point;

typedef struct Node{
  struct Node* next;
  struct Node* prev;
  Point point; // coordinates of this node
} Node;

typedef struct {
  const int size; // size of one square of snake
  int length; // number of squares (only head is 0)
  int id;
  Node* head; // pointer to the first square of the snake
  Node* tail; // pointer to the last square of the snake
  Point dir; // direction snake is heading
} Snake;

typedef struct {
  const int width; // number of horizontal squares of snake size
  const int height; // number of vertical squares of snake size
  uint16_t* pixels;
} Display;

/**
 * @return result of the point sum
 */
Point sumPoints(Point a, Point b);

/**
 * Flushes display buffer onto the lcd,
 * 
 * @param lcd lcd to be drawn onto
 */
void draw(Display* display, uint8_t* lcd);

/**
 * Changes the color of rgb led by id (should be only 1 or 2),
 * 
 * @param id id of the led to be changed
 */
void changeLedColor(int id, uint32_t color);

/**
 * Prints a line of text on position pos into the buffer.
 * 
 * @param string string to be written
 * @param buffer buffer where the string will be written
 * @param pos x, y position in the buffer to write the string
 * @param letterColor color of the string
 * @param bcgrColor color of the text background
 */
void printLine(char *string, uint32_t* buffer, Point pos, uint16_t letterColor, uint16_t bcgrColor);