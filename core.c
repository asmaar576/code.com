#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// Initialize deck
void init_deck(struct Card deck[], int *deckSize) {
    int idx = 0;

    for (int c = 0; c < 4; c++) {
        deck[idx++] = (struct Card){c, 0};
        for (int n = 1; n <= 9; n++) {
            deck[idx++] = (struct Card){c, n};
            deck[idx++] = (struct Card){c, n};
        }
        for (int a = 10; a <= 12; a++) {
            deck[idx++] = (struct Card){c, a};
            deck[idx++] = (struct Card){c, a};
        }
    }

    for (int i = 0; i < 4; i++) deck[idx++] = (struct Card){-1, 13}; // Wild
    for (int i = 0; i < 4; i++) deck[idx++] = (struct Card){-1, 14}; // Wild+4

    *deckSize = idx;
}

// Shuffle deck
void shuffle_deck(struct Card deck[], int deckSize) {
    for (int i = deckSize - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        struct Card tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
}

// Draw a card from deck
struct Card draw_card(struct Card deck[], int *deckIndex, int deckSize,
                      struct Card discard[], int *discardSize) {
    if (*deckIndex >= deckSize) {
        if (*discardSize <= 1) return (struct Card){-2, -2};
        struct Card top = discard[*discardSize - 1];
        int newSize = 0;
        for (int i = 0; i < *discardSize - 1; i++) deck[newSize++] = discard[i];
        *deckIndex = 0;
        *discardSize = 1;
        discard[0] = top;
        shuffle_deck(deck, newSize);
    }
    return deck[(*deckIndex)++];
}

// Deal cards to players
void deal(struct Card deck[], int *deckIndex,
          struct Card hands[][MAX_HAND], int handCount[],
          int players, int cardsEach) {
    for (int p = 0; p < players; p++) handCount[p] = 0;
    for (int r = 0; r < cardsEach; r++) {
        for (int p = 0; p < players; p++) {
            hands[p][handCount[p]++] = deck[(*deckIndex)++];
        }
    }
}

// Playability
bool is_playable(struct Card top, struct Card play) {
    if (play.value == 13 || play.value == 14) return true;
    if (play.color == top.color) return true;
    if (play.value == top.value) return true;
    return false;
}

// Random color for AI
int choose_color_random() { return rand() % 4; }

// Find playable card index
int find_playable_index(struct Card hand[], int count, struct Card top) {
    int possible[MAX_HAND], n = 0;
    for (int i = 0; i < count; i++)
        if (is_playable(top, hand[i])) possible[n++] = i;
    if (n == 0) return -1;
    return possible[rand() % n];
}

// Remove card from hand
void remove_from_hand(struct Card hand[], int *count, int idx) {
    for (int i = idx; i < (*count) - 1; i++) hand[i] = hand[i + 1];
    (*count)--;
}

// Apply card effects
void apply_effects(int *current, int *direction, int players,
                   struct Card play,
                   struct Card hands[][MAX_HAND], int handCount[],
                   struct Card deck[], int *deckIndex,
                   struct Card discard[], int *discardSize) {
    switch (play.value) {
        case 10: // Skip
            *current = (*current + *direction + players) % players;
            break;
        case 11: // Reverse
            *direction = -*direction;
            if (players == 2)
                *current = (*current + *direction + players) % players;
            break;
        case 12: // +2
            *current = (*current + *direction + players) % players;
            hands[*current][handCount[*current]++] = draw_card(deck, deckIndex, MAX_DECK, discard, discardSize);
            hands[*current][handCount[*current]++] = draw_card(deck, deckIndex, MAX_DECK, discard, discardSize);
            break;
        case 14: // Wild +4
            *current = (*current + *direction + players) % players;
            for (int i = 0; i < 4; i++)
                hands[*current][handCount[*current]++] = draw_card(deck, deckIndex, MAX_DECK, discard, discardSize);
            break;
        default: break;
    }
}

// Check winner
int check_winner(int handCount[], int players) {
    for (int p = 0; p < players; p++)
        if (handCount[p] == 0) return p;
    return -1;
}
