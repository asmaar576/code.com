#include "log.h"
#include <stdarg.h>
#include <stdio.h>

FILE *logFile = NULL;

void log_event(const char *msg) {
    if (logFile) fprintf(logFile, "%s\n", msg);
}

void append_line(const char *fmt, ...) {
    if (!logFile) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(logFile, fmt, args);
    fprintf(logFile, "\n");
    va_end(args);
}

void log_start_game(int players, int cardsEach) {
    append_line("=== New UNO Game ===");
    append_line("Players: %d, Starting cards each: %d", players, cardsEach);
}

void log_starting_hands(struct Card hands[][MAX_HAND], int handCount[], int players) {
    for (int p = 0; p < players; p++) {
        fprintf(logFile, "Player %d starting hand: ", p+1);
        for (int i = 0; i < handCount[p]; i++) {
            struct Card c = hands[p][i];
            fprintf(logFile, "[%d,%d] ", c.color, c.value);
        }
        fprintf(logFile, "\n");
    }
}

void log_turn_play(int playerIndex, const char *action, struct Card card, int direction) {
    append_line("Player %d %s card [Color:%d, Value:%d] (Direction:%d)",
                playerIndex+1, action, card.color, card.value, direction);
}

void log_card_drawn(int playerIndex, struct Card card) {
    append_line("Player %d draws card [Color:%d, Value:%d]", playerIndex+1, card.color, card.value);
}

void log_effect(const char *effect) {
    append_line("Effect applied: %s", effect);
}

void log_winner_and_final(int winner, struct Card hands[][MAX_HAND], int handCount[], int players) {
    append_line("=== Game Over ===");
    append_line("Winner: Player %d", winner+1);
    for (int p = 0; p < players; p++) {
        fprintf(logFile, "Player %d remaining hand: ", p+1);
        for (int i = 0; i < handCount[p]; i++) {
            struct Card c = hands[p][i];
            fprintf(logFile, "[%d,%d] ", c.color, c.value);
        }
        fprintf(logFile, "\n");
    }
}
