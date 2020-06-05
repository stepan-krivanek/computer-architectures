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
  int id;
  Node* head;
  Node* tail;
  Point dir;
} Snake;

static const Direction dir = {
  .LEFT = {-1, 0},
  .RIGHT = {1, 0},
  .UP = {0, -1},
  .DOWN = {0, 1}
};

// Flushes display buffer onto the lcd
void draw(Display* display, uint8_t* lcd){
  for (register int i = 0; i < H_PIXELS * V_PIXELS; ++i){
    parlcd_write_data(lcd, display->pixels[i]);
  }
}

Point sumPoints(Point a, Point b){
  a.x += b.x;
  a.y += b.y;
  return a;
}

// Checks if the head of snake1 crushed anywhere into snake2
bool snakeCollision(const Snake* snake1, const Snake* snake2){
  Point head = snake1->head->point;

  Node* next = snake2->head->next;
  while(next != NULL){
    if (head.x == next->point.x && head.y == next->point.y){
      return true;
    }
    next = next->next;
  }

  return false;
}

// Checks if a point is out of the display bounds
bool outOfBounds(const Display* display, Point point){
  return (
    (point.y < 0) || (point.y >= display->height) ||
    (point.x < 0) || (point.x >= display->width)
  );
}

// Changes the color of rgb led by id (should be only 1 or 2)
void changeLedColor(int id, uint32_t color){
  int ledOffset = id == 1 ? SPILED_REG_LED_RGB1_o : SPILED_REG_LED_RGB2_o;
  *(volatile uint32_t *)(LED + ledOffset) = color;
}

// Changes a square of snake size to the specified color
void changeSquareColor(const Display* display, Point position, uint16_t color){
  if (outOfBounds(display, position)){
    return;
  }

  int squareY = position.y * SNAKE_SIZE;
  int squareX = position.x * SNAKE_SIZE;

  for (int i = 0; i < SNAKE_SIZE; ++i){
    for (int j = 0; j < SNAKE_SIZE; ++j){
      int row = squareY + i;
      int col = squareX + j;
      *(display->pixels + H_PIXELS * row + col) = color;
    }
  }
}

// Checks if the frog is on any point snake occupies
bool frogIsOnSnake(const Snake* snake, Point frog){
  Node* node = snake->head;

  while (node != NULL){
    if ((node->point.x == frog.x) && (node->point.y == frog.y)){
      return true;
    }

    node = node->next;
  }
  return false;
}

// Sets randomly a frog onto the display 
// It is guaranteed that the frog will not appear at any snake
Point setRandomFrog(const Display* display, const Snake* snake1, const Snake* snake2){
  int width = display->width;
  int displaySize = display->height * width;
  int snakesLength = snake1->length + snake2->length;
  int num = rand() % (displaySize - snakesLength);

  Point frog;
  for (int y = (num / width); y < width; ++y){
    frog.y = y;

    for (int x = (num % width); x < width; ++x){
      frog.x = x;

      if (!frogIsOnSnake(snake1, frog) && !frogIsOnSnake(snake2, frog)){
        x = width;
        y = width;
      }
    }
  }
  
  changeSquareColor(display, frog, RED_16);
  return frog;
}

// Updates snake position and checks if it has eaten the frog
// Return true if snake has eaten a frog, false otherwise
bool updateSnake(const Display* display, Snake* snake, Point frog){
  Node* head = malloc(sizeof(Node));
  head->point = sumPoints(snake->head->point, snake->dir);
  head->next = snake->head;

  snake->head->prev = head;
  snake->head = head;

  changeSquareColor(display, snake->head->point, GREEN_16);

  // Checks if the snake eats a frog and adjusts led and its tail
  bool frogIsEaten = (head->point.x == frog.x) && (head->point.y == frog.y);
  
  if (frogIsEaten){
    changeLedColor(snake->id, BLUE_32);
    snake->length += 1;
  } else {
    changeSquareColor(display, snake->tail->point, BLACK_16);
    changeLedColor(snake->id, GREEN_32);

    Node* tail = snake->tail;
    snake->tail = snake->tail->prev;
    snake->tail->next = NULL;
    free(tail);
  }

  return frogIsEaten;
}

// Frees all nodes (squares) allocated by snake
void freeSnake(Snake* snake){
  while(snake->head != NULL){
    Node* head = snake->head;
    snake->head = snake->head->next;
    free(head);
  }
}

// Creates a new snake structure
Snake getSnake(Point start, Point dir){
  Node* head = malloc(sizeof(Node));
  head->point = start;
  head->next = NULL;
  head->prev = NULL;

  Snake snake = {
    .size = SNAKE_SIZE,
    .length = 1,
    .head = head,
    .tail = head,
    .dir = dir
  };

  return snake;
}

void startingSignal(){
  *(volatile uint32_t *)(LED + SPILED_REG_LED_LINE_o) = 0xFFFFFFFF;
  sleep(1);
  *(volatile uint32_t *)(LED + SPILED_REG_LED_LINE_o) = 0;
}

void msSleep(int ms){
    struct timespec ts = {
      .tv_sec = ms / 1000,
      .tv_nsec = (long)(ms % 1000) * 1000000
    };

    nanosleep(&ts, &ts);
}

int main(int argc, char *argv[]) {
  startingSignal();

  uint8_t *lcd = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0); //volatile ?
  parlcd_hx8357_init(lcd);
  parlcd_write_cmd(lcd, 0x2c);

  srand(time(NULL));
  int width = H_PIXELS / SNAKE_SIZE;
  int height = V_PIXELS / SNAKE_SIZE;
  uint16_t* pixels = malloc(V_PIXELS * H_PIXELS * sizeof(uint16_t));
  for (int i = 0; i < V_PIXELS * H_PIXELS; ++i){
    pixels[i] = BLACK_16;
  }
  
  Display display = {
    .width = width,
    .height = height,
    .pixels = pixels
  };

  Point start1 = {
    .x = width / 4,
    .y = height / 2
  };

  Point start2 = {
    .x = width * 3 / 4,
    .y = height / 2
  };
  
  Snake snake1 = getSnake(start1, dir.RIGHT);
  snake1.id = 1;
  Snake snake2 = getSnake(start2, dir.LEFT);
  snake2.id = 2;

  Point frog = setRandomFrog(&display, &snake1, &snake2);

  changeLedColor(snake1.id, GREEN_32);
  changeLedColor(snake2.id, GREEN_32);

  bool death1 = false;
  bool death2 = false;
  while (!death1 && !death2){
    msSleep(700);

    bool frogWasEaten1 = updateSnake(&display, &snake1, frog);
    bool frogWasEaten2 = updateSnake(&display, &snake2, frog);
    if (frogWasEaten1 || frogWasEaten2){
      setRandomFrog(&display, &snake1, &snake2);
    }

    draw(&display, lcd);

    bool headsEqualX = snake1.head->point.x == snake2.head->point.x;
    bool headsEqualY = snake1.head->point.y == snake2.head->point.y;
    if (headsEqualX && headsEqualY){
      death1 = true;
      death2 = true;
      break;
    }

    death1 = (
      outOfBounds(&display, snake1.head->point) ||
      snakeCollision(&snake1, &snake2) ||
      snakeCollision(&snake1, &snake1)
    );

    death2 = (
      outOfBounds(&display, snake2.head->point) ||
      snakeCollision(&snake2, &snake1) ||
      snakeCollision(&snake2, &snake2)
    );
  }

  if (death1){
    changeLedColor(snake1.id, RED_32);
  }

  if (death2){
    changeLedColor(snake2.id, RED_32);
  }

  freeSnake(&snake1);
  freeSnake(&snake2);
  free(pixels);
  return 0;
}
