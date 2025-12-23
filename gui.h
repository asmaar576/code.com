#ifndef GUI_H
#define GUI_H

#include "core.h"
#include <stdbool.h>
#include "raylib.h"

// Draw the full GUI for the game
// Returns: 0 = nothing, 1 = player drew, 2 = player played a card
int draw_gui(struct Card deck[], int *deckIndex, int deckSize,
             struct Card hands[][MAX_HAND], int handCount[],
             struct Card discard[], int *discardSize,
             int *current, int *direction,
             bool *chooseColorOverlay, struct Card *pendingWildCard,
             float animationScale,
             int screenW, int screenH,
             bool *gameOver, int *winner,
             int players);

#endif // GUI_H
