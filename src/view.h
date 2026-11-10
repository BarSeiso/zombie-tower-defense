#ifndef VIEW_H
#define VIEW_H

#include "game.h"

typedef struct {
    Texture2D sprites;
} View;

Rectangle UnitButton(int type);
Rectangle WaveButton(void);
Rectangle PauseButton(void);
Rectangle SpeedButton(void);
Rectangle PlayButton(void);
Rectangle MenuButton(void);
bool LoadView(View *view);
void UnloadView(View *view);
void DrawGame(const Game *game, const View *view, Vector2 mouse);
void DrawMenu(const Game *game, const View *view, Vector2 mouse);
void DrawResult(const Game *game, Vector2 mouse);

#endif
