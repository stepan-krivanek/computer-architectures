#include "snake.h"

bool snakeCollision(Point head, const Snake* snake2){
  Node* next = snake2->head->next;
  while(next != NULL){
    if (head.x == next->point.x && head.y == next->point.y){
      return true;
    }
    next = next->next;
  }

  return false;
}

bool outOfBounds(const Display* display, const Point point){
  return (
    (point.y < 0) || (point.y >= display->height) ||
    (point.x < 0) || (point.x >= display->width)
  );
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

void freeSnake(Snake* snake){
  while(snake->head != NULL){
    Node* head = snake->head;
    snake->head = snake->head->next;
    free(head);
  }
}

Snake getSnake(Point start,const Point dir, int id){
  Node* head = malloc(sizeof(Node));
  head->point = start;
  head->next = NULL;
  head->prev = NULL;

  Snake snake = {
    .size = SNAKE_SIZE,
    .id = id,
    .length = 1,
    .head = head,
    .tail = head,
    .dir = dir
  };

  return snake;
}

void moveAI(Display* display, Snake* snake1, Snake* aiSnake, Point frog){
  Point dirs[4] = {dir.UP, dir.RIGHT, dir.DOWN, dir.LEFT};
  int lowestDistance = display->width + display->height;

  for (int i = 0; i < 4; ++i){
    Point head = sumPoints(dirs[i], aiSnake->head->point);
    bool death = (
      outOfBounds(display, head) ||
      snakeCollision(head, snake1) ||
      snakeCollision(head, aiSnake)
    );

    if (!death){
      int distance = abs(head.x - frog.x) + abs(head.y - frog.y);

      if (distance < lowestDistance){
        lowestDistance = distance;
        aiSnake->dir = dirs[i];
      }
    }
  }
}

void moveSnake(Snake* snake, char keys[4], char input){
  if(input == keys[0] && abs(snake->dir.y) != 1) snake->dir = dir.UP;
  if(input == keys[1] && abs(snake->dir.y) != 1) snake->dir = dir.DOWN;
  if(input == keys[2] && abs(snake->dir.x) != 1) snake->dir = dir.LEFT;
  if(input == keys[3] && abs(snake->dir.x) != 1) snake->dir = dir.RIGHT;
}