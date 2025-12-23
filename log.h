#ifndef LOG_H
#define LOG_H

#include "core.h"
#include <stdio.h>

// Global log file pointer
extern FILE *logFile;

// Basic logging
void log_event(const char *msg);
void append_line(const char *fmt, ...);

// Game-specific logging
void log_start_game(int players, int cardsEach);
void log_starting_hands(struct Card hands[][MAX_HAND], int handCount[], int players);
void log_turn_play(int playerIndex, const char *action, struct Card card, int direction);
void log_card_drawn(int playerIndex, struct Card card);
void log_effect(const char *effect);
void log_winner_and_final(int winner, struct Card hands[][MAX_HAND], int handCount[], int players);

#endif // LOG_H
