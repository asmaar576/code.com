#include "gui.h"
#include "log.h"
#include "core.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "raylib.h"

// Map card color to UI color
static Color ui_color_for_card(struct Card c) {
    switch(c.color) {
        case 0: return (Color){10,160,45,255};   // Green
        case 1: return (Color){70,160,255,255};  // Blue
        case 2: return (Color){220,40,40,255};   // Red
        case 3: return (Color){250,210,30,255};  // Yellow
        default: return LIGHTGRAY;               // Wild / unknown
    }
}

// Draw one card rectangle
static void draw_card_rect(Rectangle r, struct Card c, bool faceUp, float scale) {
    Rectangle rr = r;
    rr.width *= scale; rr.height *= scale;

    if (!faceUp) {
        DrawRectangleRounded(rr, 0.12f, 8, DARKGRAY);
        DrawRectangleRoundedLines(rr, 0.12f, 8, BLACK);
        DrawText("UNO", (int)(rr.x + rr.width*0.12f), (int)(rr.y + rr.height*0.35f),
                 (int)fmaxf(10.0f, 20.0f*scale), RAYWHITE);
        return;
    }

    DrawRectangleRounded(rr, 0.12f, 8, ui_color_for_card(c));
    DrawRectangleRoundedLines(rr, 0.12f, 8, BLACK);

    char buf[16];
    if (c.value >= 0 && c.value <= 9) snprintf(buf, sizeof(buf), "%d", c.value);
    else if (c.value == 10) snprintf(buf, sizeof(buf), "SKIP");
    else if (c.value == 11) snprintf(buf, sizeof(buf), "REV");
    else if (c.value == 12) snprintf(buf, sizeof(buf), "+2");
    else if (c.value == 13) snprintf(buf, sizeof(buf), "WILD");
    else if (c.value == 14) snprintf(buf, sizeof(buf), "+4");
    else snprintf(buf, sizeof(buf), "?");

    DrawText(buf, (int)(rr.x + 8.0f*scale), (int)(rr.y + 8.0f*scale),
             (int)fmaxf(10.0f, 20.0f*scale), BLACK);
}

// --- Main GUI drawing & click handler ---
int draw_gui(struct Card deck[], int *deckIndex, int deckSize,
             struct Card hands[][MAX_HAND], int handCount[],
             struct Card discard[], int *discardSize,
             int *current, int *direction,
             bool *chooseColorOverlay, struct Card *pendingWildCard,
             float animationScale,
             int screenW, int screenH,
             bool *gameOver, int *winner,
             int players) {

    int action = 0;
    static bool ignoreHandClick = false;
    static bool ignoreOverlayClick = false;
    static bool ignoreDeckClick = false;

    BeginDrawing();
    ClearBackground(RAYWHITE);

    Vector2 mp = GetMousePosition();

    // --- Deck & Discard ---
    Rectangle deckRect = { screenW/2 - 160.f, screenH/2 - 80.f, 90.f, 130.f };
    Rectangle discRect = { screenW/2 + 80.f, screenH/2 - 80.f, 90.f, 130.f };

    if (*deckIndex < deckSize)
        draw_card_rect(deckRect, (struct Card){-1,-1}, false, animationScale);
    else {
        DrawRectangleRounded(deckRect, 0.12f, 8, LIGHTGRAY);
        DrawRectangleRoundedLines(deckRect, 0.12f, 8, Fade(BLACK,0.3f));
        DrawText("Empty", (int)(deckRect.x+8), (int)(deckRect.y+deckRect.height/2-8), 16, BLACK);
    }

    if (*discardSize > 0)
        draw_card_rect(discRect, discard[*discardSize-1], true, animationScale);
    else {
        DrawRectangleRounded(discRect, 0.12f, 8, DARKGRAY);
        DrawRectangleRoundedLines(discRect, 0.12f, 8, Fade(BLACK,0.3f));
        DrawText("Discard", (int)(discRect.x+8), (int)(discRect.y+discRect.height/2-8), 14, RAYWHITE);
    }

    DrawText("Deck", (int)(deckRect.x+8), (int)(deckRect.y-18), 16, BLACK);
    DrawText("Discard", (int)(discRect.x+8), (int)(discRect.y-18), 16, BLACK);

    // --- Wild Color Overlay ---
    if (*chooseColorOverlay) {
        Rectangle overlay = {220.f, 160.f, screenW-440.f, 400.f};
        DrawRectangleRounded(overlay, 0.12f, 12, Fade(RAYWHITE,0.95f));
        DrawRectangleRoundedLines(overlay, 0.12f, 12, BLACK);
        DrawText("Choose color for Wild card", (int)(overlay.x+24), (int)(overlay.y+20), 26, BLACK);

        Rectangle colors[4] = {
            { overlay.x + 80.f,  overlay.y + 90.f, 120.f, 120.f },
            { overlay.x + 240.f, overlay.y + 90.f, 120.f, 120.f },
            { overlay.x + 400.f, overlay.y + 90.f, 120.f, 120.f },
            { overlay.x + 560.f, overlay.y + 90.f, 120.f, 120.f }
        };
        Color colvals[4] = { (Color){10,160,45,255}, (Color){70,160,255,255},
                              (Color){220,40,40,255}, (Color){250,210,30,255} };
        const char *names[4] = {"Green","Blue","Red","Yellow"};

        for (int i=0;i<4;i++) {
            DrawRectangleRounded(colors[i],0.12f,8,colvals[i]);
            DrawRectangleRoundedLines(colors[i],0.12f,8,BLACK);
            DrawText(names[i], (int)(colors[i].x+18), (int)(colors[i].y+colors[i].height+8), 18, BLACK);

            if (!ignoreOverlayClick && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                CheckCollisionPointRec(mp, colors[i])) {

                struct Card wildCard = *pendingWildCard;
                wildCard.color = i;
                if (*discardSize < MAX_DECK + MAX_HAND) discard[(*discardSize)++] = wildCard;

                char tmp[128];
                snprintf(tmp,sizeof(tmp),"Player 1 plays Wild with color %s",names[i]);
                append_line("%s", tmp);
                log_turn_play(0,"Play",wildCard,*direction);

                apply_effects(current, direction, players, wildCard, hands, handCount, deck, deckIndex, discard, discardSize);
                if (wildCard.value !=10 && wildCard.value !=12 && wildCard.value !=14)
                    *current = (*current + *direction + players)%players;

                *chooseColorOverlay = false;
                action = 2;
                ignoreOverlayClick = true;
                break;
            }
        }
        if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) ignoreOverlayClick = false;
        EndDrawing();
        return action;
    }

    // --- Player Hand ---
    int player = 0; // human
    int count = handCount[player];
    if (count < 0) count = 0;
    float cardW = 84.f*animationScale;
    float cardH = 120.f*animationScale;
    float gap = 18.f;
    float totalW = (count>0)?(count*cardW + (count-1)*gap):0.f;
    float startX = (screenW - totalW)/2.f;
    float y = screenH - cardH - 50.f;

    for (int i=0;i<count;i++) {
        Rectangle r = {startX+i*(cardW+gap),y,cardW,cardH};
        draw_card_rect(r,hands[player][i],true,animationScale);

        if (!(*gameOver) && *current==0 && !ignoreHandClick) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mp,r)) {
                struct Card top = (*discardSize>0)?discard[*discardSize-1]:(struct Card){-1,-1};
                if (!is_playable(top,hands[player][i])) append_line("Invalid play attempted.");
                else {
                    struct Card play = hands[player][i];
                    remove_from_hand(hands[player],&handCount[player],i);

                    if (play.value==13 || play.value==14) {
                        *pendingWildCard = play;
                        *chooseColorOverlay = true;
                    } else {
                        if (*discardSize < MAX_DECK + MAX_HAND) discard[(*discardSize)++] = play;
                        log_turn_play(0,"Play",play,*direction);
                        apply_effects(current,direction,players,play,hands,handCount,deck,deckIndex,discard,discardSize);
                        if (play.value!=10 && play.value!=12 && play.value!=14)
                            *current = (*current + *direction + players)%players;
                    }
                    action = 2;
                    ignoreHandClick = true;
                }
                break;
            }
        }
    }

    // --- Deck Click ---
    if (!(*gameOver) && *current==0 && !ignoreHandClick && !ignoreDeckClick) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mp,deckRect)) {
            struct Card drawn = draw_card(deck, deckIndex, deckSize, discard, discardSize);
            if (handCount[0]<MAX_HAND) hands[0][handCount[0]++] = drawn;
            log_card_drawn(0,drawn);
            append_line("Player 1 draws a card");
            *current = (*current + *direction + players)%players;
            action = 1;
            ignoreDeckClick = true;
        }
    }

    if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        ignoreHandClick = false;
        ignoreDeckClick = false;
    }

    EndDrawing();
    return action;
}
