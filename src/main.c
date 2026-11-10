#include "game.h"
#include "view.h"
#include <stdio.h>
#include <string.h>

static void HandleGameInput(Game *game, Vector2 mouse)
{
    if (IsKeyPressed(KEY_ESCAPE)) game->selected = -1;
    bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (IsKeyPressed(KEY_P) || (clicked && CheckCollisionPointRec(mouse, PauseButton()))) {
        game->paused = !game->paused;
    }
    if (IsKeyPressed(KEY_F) || (clicked && CheckCollisionPointRec(mouse, SpeedButton()))) {
        game->speed = game->speed == 1 ? 2 : 1;
    }
    if (IsKeyPressed(KEY_SPACE) || (clicked && CheckCollisionPointRec(mouse, WaveButton()))) StartWave(game);
    if (!clicked || game->paused) return;
    for (int type = 0; type < UNIT_COUNT; type++) {
        if (game->selected >= 0 && CheckCollisionPointRec(mouse, UnitButton(type))) {
            Tower *tower = &game->towers[game->selected];
            if (tower->type == type) UpgradeTower(game, game->selected);
            else ChangeUnit(game, game->selected, type);
            return;
        }
    }
    if (mouse.y < 28 || mouse.y >= 197) return;
    game->selected = -1;
    for (int i = 0; i < TOWER_COUNT; i++) {
        Vector2 pos = game->towers[i].position;
        if (CheckCollisionPointRec(mouse, (Rectangle){pos.x - 12, pos.y - 13, 24, 26})) {
            game->selected = i;
            break;
        }
    }
}

int main(int argc, char **argv)
{
    /* --screenshot merender satu frame untuk pemeriksaan tampilan. */
    bool screenshot = argc > 1 && strcmp(argv[1], "--screenshot") == 0;
    bool preview = screenshot && argc > 2 && strcmp(argv[2], "battle") == 0;
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2, "Pos Terakhir - Tower Defense");
    if (!IsWindowReady()) return 1;
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    View view = {0};
    if (!LoadView(&view)) {
        TraceLog(LOG_ERROR, "Sprite tidak ditemukan atau rusak. Pastikan assets/sprites.png berada di samping executable.");
        CloseWindow();
        return 1;
    }
    RenderTexture2D canvas = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);
    Game game;
    ResetGame(&game);
    bool menu = true;
    bool quit = false;
    float accumulator = 0;
    if (preview) {
        menu = false;
        game.scrap = 5000;
        for (int i = 0; i < TOWER_COUNT; i++) ChangeUnit(&game, i, i % UNIT_COUNT);
        game.wave = 7;
        StartWave(&game);
        for (int i = 0; i < 700; i++) UpdateGame(&game, STEP_SECONDS);
        game.selected = 4;
    }

    while (!WindowShouldClose() && !quit) {
        Vector2 mouse = GetMousePosition();
        mouse.x /= 2;
        mouse.y /= 2;
        bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        if (menu) {
            if (clicked && CheckCollisionPointRec(mouse, PlayButton())) {
                ResetGame(&game);
                menu = false;
                accumulator = 0;
            }
            if (clicked && CheckCollisionPointRec(mouse, MenuButton())) quit = true;
        } else if (game.phase == VICTORY || game.phase == DEFEAT) {
            if (clicked && CheckCollisionPointRec(mouse, PlayButton())) {
                ResetGame(&game);
                accumulator = 0;
            }
            if (clicked && CheckCollisionPointRec(mouse, MenuButton())) {
                ResetGame(&game);
                menu = true;
                accumulator = 0;
            }
        } else {
            HandleGameInput(&game, mouse);
            float frameSeconds = GetFrameTime();
            if (frameSeconds > 0.1f) frameSeconds = 0.1f;
            if (game.paused) accumulator = 0;
            else accumulator += frameSeconds * game.speed;
            while (accumulator >= STEP_SECONDS) {
                UpdateGame(&game, STEP_SECONDS);
                accumulator -= STEP_SECONDS;
            }
        }

        BeginTextureMode(canvas);
        if (menu) DrawMenu(&game, &view, mouse);
        else {
            DrawGame(&game, &view, mouse);
            if (game.phase == VICTORY || game.phase == DEFEAT) DrawResult(&game, mouse);
        }
        EndTextureMode();
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(canvas.texture, (Rectangle){0, 0, SCREEN_WIDTH, -SCREEN_HEIGHT},
                       (Rectangle){0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2},
                       (Vector2){0, 0}, 0, WHITE);
        EndDrawing();
        if (screenshot) {
            Image capture = LoadImageFromTexture(canvas.texture);
            ImageFlipVertical(&capture);
            ImageResizeNN(&capture, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2);
            ExportImage(capture, preview ? "preview-battle.png" : "preview-menu.png");
            UnloadImage(capture);
            break;
        }
    }
    UnloadRenderTexture(canvas);
    UnloadView(&view);
    CloseWindow();
    return 0;
}
