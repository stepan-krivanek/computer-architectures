#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "snake.h"
#include "menu.h"

static struct termios orig_termios;

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

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;   
  raw.c_cc[VTIME] = 1;  //timeout 1/10 s

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void moveSnakes(Snake* snake1, Snake* snake2){
  char controls1[4] = {'w', 's', 'a', 'd'};
  char controls2[4] = {'i', 'k', 'j', 'l'};

  char input;
  while(read(STDIN_FILENO, &input, 1) > 0){
    moveSnake(snake1, controls1, input);
    moveSnake(snake2, controls2, input);
  }
}

void startSnake(Display* display, uint8_t* lcd, int speed, bool multiplayer){
  for (int i = 0; i < V_PIXELS * H_PIXELS; ++i){
    display->pixels[i] = BLACK_16;
  }

  Point start1 = {
    .x = display->width / 4,
    .y = display->height / 2
  };

  Point start2 = {
    .x = display->width * 3 / 4,
    .y = display->height / 2
  };
  
  Snake snake1 = getSnake(start1, dir.RIGHT, 1);
  Snake snake2 = getSnake(start2, dir.LEFT, 2);
  Point frog = setRandomFrog(display, &snake1, &snake2);

  changeLedColor(snake1.id, GREEN_32);
  changeLedColor(snake2.id, GREEN_32);

  bool death1 = false;
  bool death2 = false;
  int sleepTime = 0;//700 / speed;
  while (!death1 && !death2){
    msSleep(sleepTime);

    moveAI(display, &snake1, &snake2, frog);
    moveAI(display, &snake2, &snake1, frog);
    /*
    moveSnakes(&snake1, &snake2);
    if (!multiplayer){
      moveAI(display, &snake1, &snake2, frog);
    }*/

    bool frogWasEaten1 = updateSnake(display, &snake1, frog);
    bool frogWasEaten2 = updateSnake(display, &snake2, frog);
    if (frogWasEaten1 || frogWasEaten2){
      frog = setRandomFrog(display, &snake1, &snake2);
    }

    draw(display, lcd);

    bool headsEqualX = snake1.head->point.x == snake2.head->point.x;
    bool headsEqualY = snake1.head->point.y == snake2.head->point.y;
    if (headsEqualX && headsEqualY){
      death1 = true;
      death2 = true;
      break;
    }

    death1 = (
      outOfBounds(display, snake1.head->point) ||
      snakeCollision(snake1.head->point, &snake2) ||
      snakeCollision(snake1.head->point, &snake1)
    );

    death2 = (
      outOfBounds(display, snake2.head->point) ||
      snakeCollision(snake2.head->point, &snake1) ||
      snakeCollision(snake2.head->point, &snake2)
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
  
  Display display = {
    .width = width,
    .height = height,
    .pixels = pixels
  };

  menu(&display, lcd);

  enableRawMode();
  startSnake(&display, lcd, 2, false);
  disableRawMode();

  free(pixels);
  return 0;
}