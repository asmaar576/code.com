#ifndef CORE_H
#define CORE_H

#include <stdbool.h>

#define MAX_DECK 108
#define MAX_HAND 108
#define MAX_PLAYERS 4

// ---------- Card structure ----------
struct Card {
    int color; // 0=Green,1=Blue,2=Red,3=Yellow, -1=wild
    int value; // 0-9 numbers, 10 Skip, 11 Reverse, 12 +2, 13 Wild, 14 Wild+4
};

// ---------- Core game functions ----------
void init_deck(struct Card deck[], int *deckSize);
void shuffle_deck(struct Card deck[], int deckSize);

// Updated draw_card with 5 args
struct Card draw_card(struct Card deck[], int *deckIndex, int deckSize,
                      struct Card discard[], int *discardSize);

void deal(struct Card deck[], int *deckIndex,
          struct Card hands[][MAX_HAND], int handCount[],
          int players, int cardsEach);

bool is_playable(struct Card top, struct Card play);
int choose_color_random();
int find_playable_index(struct Card hand[], int count, struct Card top);
void remove_from_hand(struct Card hand[], int *count, int idx);

void apply_effects(int *current, int *direction, int players,
                   struct Card play,
                   struct Card hands[][MAX_HAND], int handCount[],
                   struct Card deck[], int *deckIndex,
                   struct Card discard[], int *discardSize);

int check_winner(int handCount[], int players);

#endif // CORE_H
