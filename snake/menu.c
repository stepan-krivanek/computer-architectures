#include "menu.h"

#define FIRST_ROW 20
#define ROW_SPACING 30
#define NUM_OF_ROWS 4

static int players = 1;
static int speed = 1;
static int cursor = 1;

int changePlayers(Display* display, uint8_t* lcd, bool more){
    int playersNum = players + (more ? 1 : -1);
    if (playersNum < 1 || playersNum > 2){
        return players;
    }
    players = playersNum;

    Point x1 = {150, FIRST_ROW + ROW_SPACING};
    Point x2 = {170, FIRST_ROW + ROW_SPACING};

    if (players == 1){
        printLine("1", (uint32_t*)display->pixels, x1, BLACK_16, WHITE_16);
        printLine("2", (uint32_t*)display->pixels, x2, WHITE_16, BLACK_16);
    } else {
        printLine("1", (uint32_t*)display->pixels, x1, WHITE_16, BLACK_16);
        printLine("2", (uint32_t*)display->pixels, x2, BLACK_16, WHITE_16);
    }
    
    draw(display, lcd);
    return players;
}

int changeSpeed(Display* display, uint8_t* lcd, bool more){
    int newSpeed = speed + (more ? 1 : -1); 
    if (newSpeed < 1 || newSpeed > 2){
        return speed;
    }
    speed = newSpeed;

    Point x1 = {150, FIRST_ROW + ROW_SPACING * 2};
    Point x2 = {170, FIRST_ROW + ROW_SPACING * 2};

    if (speed == 1){
        printLine("1", (uint32_t*)display->pixels, x1, BLACK_16, WHITE_16);
        printLine("2", (uint32_t*)display->pixels, x2, WHITE_16, BLACK_16);
    } else {
        printLine("1", (uint32_t*)display->pixels, x1, WHITE_16, BLACK_16);
        printLine("2", (uint32_t*)display->pixels, x2, BLACK_16, WHITE_16);
    }
    
    draw(display, lcd);
    return speed;
}

int redrawCursor(Display* display, uint8_t* lcd, bool more){
    cursor = (cursor + NUM_OF_ROWS + (more ? 1 : -1)) % NUM_OF_ROWS;

    Point p = {50, FIRST_ROW};
    for (int i = 0; i < NUM_OF_ROWS; ++i){
        printLine(">", (uint32_t*)display->pixels, p, BLACK_16, BLACK_16);
        p.y += ROW_SPACING;
    }

    p.y = FIRST_ROW + ROW_SPACING * cursor;
    printLine(">", (uint32_t*)display->pixels, p, WHITE_16, BLACK_16);
    draw(display, lcd);

    return cursor;
}

void initMenu(Display* display, uint8_t* lcd){
    players = 1;
    speed = 1;
    cursor = 1;

    char names[NUM_OF_ROWS][10] = {"START", "PLAYERS", "SPEED", "QUIT"};
    Point p = {80, FIRST_ROW}; 

    for (int i = 0; i < NUM_OF_ROWS; i++){
        printLine(names[i], (uint32_t*)display->pixels, p, WHITE_16, BLACK_16);
        p.y += ROW_SPACING;
    }
    
    draw(display, lcd);
}