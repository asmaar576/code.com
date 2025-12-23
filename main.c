#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"
#include "core.h"
#include "log.h"
#include "gui.h"

// ---------- Game States ----------
typedef enum GameState {
    STATE_MENU,
    STATE_OPTIONS,
    STATE_PLAYING
} GameState;

int main(void) {
    srand((unsigned)time(NULL));

    const int screenW = 1280, screenH = 720;
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(screenW, screenH, "UNO - Project (Raylib GUI)");
    SetTargetFPS(60);
    InitAudioDevice();

    Sound cardPlace = LoadSound("sounds/card_place.wav");
    Sound buttonClick = LoadSound("sounds/button_click.wav");
    Sound gameOverSound = LoadSound("sounds/game_over.wav");

    GameState gameState = STATE_MENU;
    int players = 2;
    int cardsEach = 7;

    struct Card deck[MAX_DECK]; int deckSize = 0;
    struct Card hands[MAX_PLAYERS][MAX_HAND]; int handCount[MAX_PLAYERS];
    int deckIndex = 0;
    struct Card discard[MAX_DECK + MAX_HAND]; int discardSize = 0;
    int current = 0, direction = 1;
    struct Card pendingWildCard = {-1, -1};
    bool chooseColorOverlay = false;
    bool gameOver = false;
    int winner = -1;
    float animationScale = 1.0f;
    double lastAIActionTime = 0.0;
    bool ignoreNextClick = false;
    bool firstGameplayFrame = true;

    while (!WindowShouldClose()) {
        Vector2 mp = GetMousePosition();

        // --- MENU ---
        if (gameState == STATE_MENU) {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            Rectangle startBtn = { screenW/2 - 80, 300, 160, 60 };
            Rectangle optionsBtn = { screenW/2 - 80, 380, 160, 60 };
            DrawRectangleRounded(startBtn, 0.2f, 8, GREEN);
            DrawRectangleRoundedLines(startBtn, 0.2f, 8, DARKGREEN);
            DrawRectangleRounded(optionsBtn, 0.2f, 8, BLUE);
            DrawRectangleRoundedLines(optionsBtn, 0.2f, 8, DARKBLUE);
            DrawText("Start Game", (int)(startBtn.x + 10), (int)(startBtn.y + 15), 24, WHITE);
            DrawText("Options", (int)(optionsBtn.x + 25), (int)(optionsBtn.y + 15), 24, WHITE);

            if (!ignoreNextClick && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mp, startBtn)) {
                    PlaySound(buttonClick);
                    init_deck(deck, &deckSize);
                    shuffle_deck(deck, deckSize);
                    deckIndex = 0;

                    for (int p = 0; p < MAX_PLAYERS; p++) handCount[p] = 0;
                    deal(deck, &deckIndex, hands, handCount, players, cardsEach);

                    discardSize = 0;
                    discard[discardSize++] = draw_card(deck, &deckIndex, deckSize, discard, &discardSize);
                    if (discard[discardSize - 1].value >= 13) discard[discardSize - 1].color = rand() % 4;

                    current = 0; direction = 1; gameOver = false; winner = -1;

                    logFile = fopen("project_uno_game.txt", "w");
                    if (logFile) {
                        log_start_game(players, cardsEach);
                        log_starting_hands(hands, handCount, players);
                    }

                    gameState = STATE_PLAYING;
                    ignoreNextClick = true;
                    firstGameplayFrame = true;
                } else if (CheckCollisionPointRec(mp, optionsBtn)) {
                    PlaySound(buttonClick);
                    gameState = STATE_OPTIONS;
                    ignoreNextClick = true;
                }
            }

            EndDrawing();
            continue;
        }

        // --- OPTIONS ---
        if (gameState == STATE_OPTIONS) {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            Rectangle btns[3] = { {screenW/2 - 80, 300, 160, 50}, {screenW/2 - 80, 370, 160, 50}, {screenW/2 - 80, 440, 160, 50} };
            const char* labels[3] = { "2 Players", "3 Players", "4 Players" };

            for (int i = 0; i < 3; i++) {
                DrawRectangleRounded(btns[i], 0.2f, 8, BLUE);
                DrawRectangleRoundedLines(btns[i], 0.2f, 8, DARKBLUE);
                DrawText(labels[i], (int)(btns[i].x + 20), (int)(btns[i].y + 10), 24, WHITE);
            }

            if (!ignoreNextClick && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                for (int i = 0; i < 3; i++) {
                    if (CheckCollisionPointRec(mp, btns[i])) {
                        players = i + 2;
                        PlaySound(buttonClick);
                        gameState = STATE_MENU;
                        ignoreNextClick = true;
                        break;
                    }
                }
            }

            EndDrawing();
            continue;
        }

        // --- GAMEPLAY ---
        if (gameState == STATE_PLAYING && !gameOver) {
            if (!firstGameplayFrame && current != 0) {
                double now = GetTime();
                if (now - lastAIActionTime > 0.5) {
                    lastAIActionTime = now;

                    if (discardSize > 0) {
                        struct Card top = discard[discardSize - 1];
                        int idx = find_playable_index(hands[current], handCount[current], top);

                        if (idx == -1) {
                            struct Card drawn = draw_card(deck, &deckIndex, deckSize, discard, &discardSize);
                            hands[current][handCount[current]++] = drawn;
                            log_card_drawn(current, drawn);
                            current = (current + direction + players) % players;
                        } else {
                            struct Card play = hands[current][idx];
                            if (play.value >= 13) play.color = choose_color_random();
                            discard[discardSize++] = play;
                            remove_from_hand(hands[current], &handCount[current], idx);
                            log_turn_play(current, "Play", play, direction);
                            PlaySound(cardPlace);

                            // Apply effects; current player update happens inside apply_effects
                            apply_effects(&current, &direction, players, play, hands, handCount, deck, &deckIndex, discard, &discardSize);

                            // Only advance if normal card (no skip/+2/+4 handled inside apply_effects)
                            if (play.value != 10 && play.value != 12 && play.value != 14) {
                                current = (current + direction + players) % players;
                            }
                        }
                    }
                }
            }
        }

        // --- GUI ---
        animationScale = 1.0f;
        draw_gui(deck, &deckIndex, deckSize,
                 hands, handCount,
                 discard, &discardSize,
                 &current, &direction,
                 &chooseColorOverlay, &pendingWildCard,
                 animationScale, screenW, screenH,
                 &gameOver, &winner,
                 players);

        if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            ignoreNextClick = false;
            firstGameplayFrame = false;
        }

        int w = check_winner(handCount, players);
        if (w != -1 && !gameOver) {
            gameOver = true; winner = w; PlaySound(gameOverSound);
            log_winner_and_final(winner, hands, handCount, players);
        }
    }

    if (logFile) fclose(logFile);
    UnloadSound(cardPlace); UnloadSound(buttonClick); UnloadSound(gameOverSound);
    CloseAudioDevice(); CloseWindow();
    return 0;
}
