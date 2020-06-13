#include "menu.h"

void printLine(char *string, uint32_t* buffer, int x, int y) {
    if (string == NULL) return;
    char c;
    int h, w, index, set;
    int len = 0;
    int i = 0;
    
    while ((c = string[i]) != '\0') {
        index = ((int) c) - font_winFreeSystem14x16.firstchar;

        for (h = 0; h < font_winFreeSystem14x16.height; ++h) {
            for (w = 0; w < font_winFreeSystem14x16.width[index]; ++w) {
                set = (
                  font_winFreeSystem14x16.bits[index * font_winFreeSystem14x16.height + h]
                  >> (font_winFreeSystem14x16.maxwidth - w)
                ) & 1;

                buffer[x + len + w + (y + h + 4) * H_PIXELS] = set ? WHITE_16 : BLACK_16;
            }
        }

        len += font_winFreeSystem14x16.width[index];
        i++;
    }
}

void menu(Display* display, uint8_t* lcd){
    int numOfPlayers = -1;
    int speed = -1;

    printLine("NUMBER OF PLAYERS:", (uint32_t*)display->pixels, 60, 20);
    printLine("1 OR 2", (uint32_t*)display->pixels, 90, 50);
    printLine("SNAKE SPEED:", (uint32_t*)display->pixels, 60, 80);
    printLine("1x OR 2x", (uint32_t*)display->pixels, 90, 110);
    draw(display, lcd);

    while(numOfPlayers != 1 && numOfPlayers != 2){
        scanf("%d", &numOfPlayers);
        printf("players: %d\n", numOfPlayers);
    }

    while(speed != 1 && speed != 2){
        scanf("%d", &speed);
        printf("speed: %d\n", speed);
    }
}