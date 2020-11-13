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
static int players = 1;
static int speed = 1;

void startingSignal(){
  *(volatile uint32_t *)(LED + SPILED_REG_LED_LINE_o) = 0xFFFFFFFF;
  sleep(1);
  *(volatile uint32_t *)(LED + SPILED_REG_LED_LINE_o) = 0;
}

/**
 * Draws whole display to the color specified.
 */
void setBackgroundColor(Display* display, uint16_t color){
  for (int i = 0; i < V_PIXELS * H_PIXELS; ++i){
    display->pixels[i] = color;
  }
}

/**
 * Makes the current thread sleep for 'ms' milliseconds.
 * 
 * @param ms milliseconds to sleep
 */
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

/**
 * Enables to read keys entered in terminal
 * without confirmation (no enter press needed).
 */
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

/**
 * Converts specified number of digits of number to string.
 * 
 * @param buffer buffer to write the number into
 */
void itos(int num, char* buffer, int digits){
  for (int i = digits - 1; i >= 0; --i){
    buffer[i] = (num % 10) + '0';
    num /= 10;
  }
}

///////////////////////////////////////////////////////
//-----------------------Snake-----------------------//
///////////////////////////////////////////////////////

/**
 * Prints score into left bottom corner.
 */
void printScore(int score1, int score2, uint16_t* pixels){
  char score[8];
  itos(score1, score, 3);
  itos(score2, (score + 4), 3);
  score[3] = ':';
  score[7] = '\0';

  Point pos = {20, 140};
  printLine(score, (uint32_t*)pixels, pos, WHITE_16, BLACK_16);
}

/**
 * Prints time into right bottom corner.
 * Time format is "00:00" (minutes:seconds).
 * 
 * @param secs time to print in seconds
 */
void printTime(int secs, uint16_t* pixels){
  int minutes = secs / 60;
  secs %= 60;
  char time[6];
  itos(minutes, time, 2);
  itos(secs, (time + 3), 2);
  time[2] = ':';
  time[5] = '\0';

  Point pos = {180, 140};
  printLine(time, (uint32_t*)pixels, pos, WHITE_16, BLACK_16);
}

/**
 * Checks keys pressed and adjusts snakes directions
 */
void moveSnakes(Snake* snake1, Snake* snake2){
  char controls1[4] = {'w', 's', 'a', 'd'};
  char controls2[4] = {'i', 'k', 'j', 'l'};

  char input;
  while(read(STDIN_FILENO, &input, 1) > 0){
    moveSnake(snake1, controls1, input);
    moveSnake(snake2, controls2, input);
  }
}

/**
 * Starts the snake game.
 */
void startSnake(Display* display, uint8_t* lcd){
  setBackgroundColor(display, BLACK_16);

  // Draws line dividing playground and score + time
  int lineStart = H_PIXELS * display->height * SNAKE_SIZE;
  for (int i = 0; i < 2 * H_PIXELS; ++i){
    display->pixels[lineStart + i] = BLUE_16;
  }

  // Initializes snakes and frog
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
  printScore(snake1.length, snake2.length, display->pixels);

  // Game start
  changeLedColor(snake1.id, GREEN_32);
  changeLedColor(snake2.id, GREEN_32);

  bool death1 = false;
  bool death2 = false;
  int sleepTime = 500 / speed;
  time_t startTime = time(NULL);
  while (!death1 && !death2){
    msSleep(sleepTime);
    printTime((time(NULL) - startTime), display->pixels);

    moveSnakes(&snake1, &snake2);
    if (players == 1){
      moveAI(display, &snake1, &snake2, frog);
    }

    bool frogWasEaten1 = updateSnake(display, &snake1, frog);
    bool frogWasEaten2 = updateSnake(display, &snake2, frog);
    if (frogWasEaten1 || frogWasEaten2){
      frog = setRandomFrog(display, &snake1, &snake2);
      printScore(snake1.length, snake2.length, display->pixels);
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

  // Adjusts led colors according to result
  if (death1){
    changeLedColor(snake1.id, RED_32);
  }

  if (death2){
    changeLedColor(snake2.id, RED_32);
  }

  freeSnake(&snake1);
  freeSnake(&snake2);
}

///////////////////////////////////////////////////////
//-----------------------Menu------------------------//
///////////////////////////////////////////////////////

/**
 * Creates a new game menu and shows it on display.
 * 
 * @return -1 = error, 0 = quit, 1 = start
 */
int showMenu(Display* display, uint8_t* lcd){
  setBackgroundColor(display, BLACK_16);

  // menu initialization
  initMenu(display, lcd);
  int cursor = redrawCursor(display, lcd, false);
  players = changePlayers(display, lcd, true);
  speed = changeSpeed(display, lcd, true);
  int start = -1;

  // user input handeling
  while(start == -1){
    char input = '*';
    if (read(STDIN_FILENO, &input, 1) < 0) {
      break;
    }

    if (input == 'w') cursor = redrawCursor(display, lcd, false);
    if (input == 's') cursor = redrawCursor(display, lcd, true);
    if (input == 'a') {
      if (cursor == 1){
        players = changePlayers(display, lcd, false);
      } else if (cursor == 2){
        speed = changeSpeed(display, lcd, false);
      }
    }

    if (input == 'd'){
      switch (cursor){
        case 0:
          start = 1;
          break;
        
        case 1:
          players = changePlayers(display, lcd, true);
          break;

        case 2:
          speed = changeSpeed(display, lcd, true);
          break;

        case 3:
          start = 0;
          break;
      }
    }
  }

  return start;
}

///////////////////////////////////////////////////////
//-----------------------Main------------------------//
///////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
  startingSignal();

  // variables init
  uint8_t *lcd = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  parlcd_hx8357_init(lcd);
  parlcd_write_cmd(lcd, 0x2c);

  srand(time(NULL));
  int botBar = 40;
  
  Display display = {
    .width = H_PIXELS / SNAKE_SIZE,
    .height = (V_PIXELS - botBar) / SNAKE_SIZE,
    .pixels = malloc(V_PIXELS * H_PIXELS * sizeof(uint16_t))
  };

  // swaps between menu and the game
  enableRawMode();
  while (1){
    if (showMenu(&display, lcd) < 1){
      break;
    }

    startSnake(&display, lcd);
    sleep(3);
  }
  disableRawMode();

  setBackgroundColor(&display, BLACK_16);
  draw(&display, lcd);

  free(display.pixels);
  return 0;
}