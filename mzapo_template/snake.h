#pragma once
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "font_types.h"
#include "game_utils.h"

typedef struct {
  const Point LEFT;
  const Point RIGHT;
  const Point UP;
  const Point DOWN;
} Direction;

static const Direction dir = {
  .LEFT = {-1, 0},
  .RIGHT = {1, 0},
  .UP = {0, -1},
  .DOWN = {0, 1}
};

// Creates a new snake structure
Snake getSnake(Point start, Point dir, int id);

// Frees all nodes (squares) allocated by snake
void freeSnake(Snake* snake);

// Checks if the head of snake1 crushed anywhere into snake2
bool snakeCollision(Point head, const Snake* snake2);

// Checks if a point is out of the display bounds
bool outOfBounds(const Display* display, const Point point);

// Sets randomly a frog onto the display 
// It is guaranteed that the frog will not appear at any snake
Point setRandomFrog(const Display* display, const Snake* snake1, const Snake* snake2);

// Updates snake position and checks if it has eaten the frog
// Return true if snake has eaten a frog, false otherwise
bool updateSnake(const Display* display, Snake* snake, Point frog);

// Compares input to the key controls and eventually
// moves a snake according to it
void moveSnake(Snake* snake, char keys[4], char input);

// Determines best direction for AI snake
void moveAI(Display* display, Snake* snake1, Snake* aiSnake, Point frog);