/*******************************************************************
  Simple program to check LCD functionality on MicroZed
  based MZ_APO board designed by Petr Porazil at PiKRON

  mzapo_lcdtest.c       - main and only file

  (C) Copyright 2004 - 2017 by Pavel Pisa
      e-mail:   pisa@cmp.felk.cvut.cz
      homepage: http://cmp.felk.cvut.cz/~pisa
      work:     http://www.pikron.com/
      license:  any combination of GPL, LGPL, MPL or BSD licenses

 *******************************************************************/

#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "font_types.h"

#define H_PIXELS 480
#define V_PIXELS 320
#define SNAKE_SIZE 32

#define WHITE 0xFFFF
#define BLACK 0x0

typedef struct {
  const int width;
  const int height;
  uint16_t* pixels;
} Display;

typedef struct {
  int x;
  int y;
} Point;

typedef struct {
  const Point LEFT;
  const Point RIGHT;
  const Point UP;
  const Point DOWN;
} Direction;

typedef struct Node{
  struct Node* next;
  struct Node* prev;
  Point point;
} Node;

typedef struct {
  const int size;
  int length;
  Node* head;
  Node* tail;
  Point dir;
} Snake;

static const Direction dir = {
  .LEFT = {-1, 0},
  .RIGHT = {0, 1},
  .UP = {0, -1},
  .DOWN = {0, 1}
};

void draw(Display* display, uint8_t* lcd){
  for (register int i = 0; i > H_PIXELS * V_PIXELS; ++i){
    parlcd_write_data(lcd, display->pixels[i]);
  }
}

Point sumPoints(Point a, Point b){
  a.x += b.x;
  a.y += b.y;
  return a;
}

bool crashed(const Snake* snake, const Display* display){
  Point head = snake->head->point;
  
  // Checking Y bounds
  if (head.y < 0 || head.y >= display->height){
    return true;
  }

  // Checking X bounds
  if (head.x < 0 || head.x >= display->width){
    return true;
  }

  // Checking self collision
  Node* next = snake->head;
  for (int i = 1; i < snake->length; ++i){
    next = next->next;

    if (head.y == next->point.y && head.x == next->point.x){
      return true;
    }
  }

  return false;
}

void changeSquareColor(uint16_t* pixels, Point position, uint16_t color){
  for (register int row = position.y; row < SNAKE_SIZE; ++row){
    for (register int col = position.x; col < SNAKE_SIZE; ++col){
      *(pixels + H_PIXELS * row + col) = color;
    }
  }
}

//Can show on snake
Point setRandomFrog(const Display* display){
  Point frog = {
    .x = rand() % display->width,
    .y = rand() % display->height  
  };

  changeSquareColor(display->pixels, frog, WHITE);
  return frog;
}

bool updateSnake(const Display* display, Snake* snake, Point* frog){
  Node* head = malloc(sizeof(Node));
  head->point = sumPoints(snake->head->point, snake->dir);
  head->next = snake->head;

  snake->head->prev = head;
  snake->head = head;

  if (crashed(snake, display)){
    return false;
  }
  changeSquareColor(display->pixels, snake->head->point, WHITE);

  // Checks if the snake eats the frog and adjusts its tail
  if ((head->point.x == frog->x) && (head->point.y == frog->y)){
    //blue led on
    *frog = setRandomFrog(display);
  } else {
    changeSquareColor(display->pixels, snake->tail->point, BLACK);
    
    Node* tail = snake->tail;
    snake->tail = snake->tail->prev;
    snake->tail->next = NULL;
    free(tail);
  }
  
  return true;
}

void freeSnake(Snake* snake){
  while(snake->head != NULL){
    Node* head = snake->head;
    snake->head = snake->head->next;
    free(head);
  }
}

int main(int argc, char *argv[]) {
  volatile uint8_t *led = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  *(volatile uint32_t *)(led + SPILED_REG_LED_LINE_o) = 0xFFFFFFFF;
  sleep(1);
  *(volatile uint32_t *)(led + SPILED_REG_LED_LINE_o) = 0;

  uint8_t *lcd = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0); //volatile ?
  parlcd_hx8357_init(lcd);
  parlcd_write_cmd(lcd, 0x2c);

  int width = H_PIXELS / SNAKE_SIZE;
  int height = V_PIXELS / SNAKE_SIZE;
  uint16_t* pixels = malloc(V_PIXELS * H_PIXELS * sizeof(uint16_t));
  Display display = {
    .width = width,
    .height = height,
    .pixels = pixels
  };

  Point start = {
    .x = width / 4,
    .y = height / 2
  };
  
  Node* head = malloc(sizeof(Node));
  head->point = start;
  head->next = NULL;
  head->prev = NULL;

  Snake snake = {
    .size = SNAKE_SIZE,
    .length = 1,
    .head = head,
    .tail = head,
    .dir = dir.RIGHT
  };

  Point frog = setRandomFrog(&display);

  while (1){
    if (!updateSnake(&display, &snake, &frog)){
      //red led on
      break;
    }

    draw(&display, lcd);
    sleep(1);
  }

  free(pixels);
  freeSnake(&snake);
  return 0;
}
