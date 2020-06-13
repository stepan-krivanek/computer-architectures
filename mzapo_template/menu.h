#pragma once
#include <stdio.h>
#include <stdbool.h>

#include "game_utils.h"

/**
 * Changes the number of players to play the game if possible
 * and displays the change on lcd.
 * 
 * @param more if true, increases the number of players if possible,
 * if false, decreases the number of player if possible
 * @return new number of players
 */
int changePlayers(Display* display, uint8_t* lcd, bool more);

/**
 * Changes the speed of snakes in game if possible
 * and displays the change on lcd.
 * 
 * @param more if true, increases the speed if possible,
 * if false, decreases the speed if possible
 * @return new speed
 */
int changeSpeed(Display* display, uint8_t* lcd, bool more);

/**
 * Shows menu buttons and sets parameters to default.
 */
void initMenu(Display* display, uint8_t* lcd);

/**
 * Moves and redraws the cursor pointing to buttons.
 * 
 * @return index of the row, cursor is pointing to
 */
int redrawCursor(Display* display, uint8_t* lcd, bool more);