#include "view.h"
#include <math.h>
#include <stdio.h>

static const Color INK = {19, 29, 33, 255};
static const Color PANEL = {29, 43, 47, 255};
static const Color PAPER = {229, 222, 190, 255};
static const Color MUTED = {148, 164, 151, 255};
static const Color COLOR_GOLD = {235, 183, 85, 255};
static const Color COLOR_GREEN = {129, 186, 133, 255};
static const Color COLOR_RED = {221, 113, 88, 255};

Rectangle UnitButton(int type) { return (Rectangle){8 + type * 116, 235, 108, 29}; }
Rectangle WaveButton(void) { return (Rectangle){366, 215, 106, 23}; }
Rectangle PauseButton(void) { return (Rectangle){366, 243, 50, 21}; }
Rectangle SpeedButton(void) { return (Rectangle){421, 243, 51, 21}; }
Rectangle PlayButton(void) { return (Rectangle){155, 160, 170, 28}; }
Rectangle MenuButton(void) { return (Rectangle){155, 195, 170, 25}; }

bool LoadView(View *view)
{
    const char *file = TextFormat("%sassets/sprites.png", GetApplicationDirectory());
    if (!FileExists(file)) return false;
    view->sprites = LoadTexture(file);
    if (view->sprites.id == 0) return false;
    if (view->sprites.width != 32 || view->sprites.height != 112) {
        UnloadTexture(view->sprites);
        view->sprites.id = 0;
        return false;
    }
    SetTextureFilter(view->sprites, TEXTURE_FILTER_POINT);
    return true;
}

void UnloadView(View *view)
{
    if (view->sprites.id != 0) UnloadTexture(view->sprites);
}

static void CenterText(const char *text, int x, int y, int width, int size, Color color)
{
    DrawText(text, x + (width - MeasureText(text, size)) / 2, y, size, color);
}

static void Button(Rectangle rect, const char *label, bool enabled, Vector2 mouse)
{
    bool hover = enabled && CheckCollisionPointRec(mouse, rect);
    Color fill = enabled ? (Color){57, 79, 72, 255} : (Color){37, 49, 50, 255};
    if (hover) fill = (Color){79, 104, 84, 255};
    DrawRectangleRec((Rectangle){rect.x, rect.y + 2, rect.width, rect.height}, INK);
    DrawRectangleRec(rect, fill);
    DrawRectangleLinesEx(rect, 1, enabled ? COLOR_GREEN : (Color){65, 79, 76, 255});
    CenterText(label, (int)rect.x, (int)rect.y + ((int)rect.height - 10) / 2,
               (int)rect.width, 10, enabled ? PAPER : MUTED);
}

static void Sprite(const View *view, int row, int frame, Vector2 position, int size, bool flip, Color tint)
{
    Rectangle source = {(float)frame * 16, (float)row * 16, 16, 16};
    if (flip) source.width = -16;
    Rectangle destination = {floorf(position.x) - size / 2, floorf(position.y) - size / 2,
                             (float)size, (float)size};
    DrawTexturePro(view->sprites, source, destination, (Vector2){0, 0}, 0, tint);
}

static void Ruin(int x, int y, int width, int height)
{
    DrawRectangle(x + 3, y + 3, width, height, (Color){38, 46, 43, 255});
    DrawRectangle(x, y, width, height, (Color){75, 81, 72, 255});
    DrawRectangle(x + 2, y + 2, width - 4, height - 4, (Color){58, 66, 62, 255});
    DrawRectangle(x + 5, y + 5, 5, 6, INK);
    DrawRectangle(x + width - 10, y + 5, 5, 6, INK);
    DrawLine(x + width / 2, y, x + width / 2 - 4, y + height / 2, INK);
    DrawLine(x + width / 2 - 4, y + height / 2, x + width / 2 + 2, y + height - 1, INK);
}

static void DrawMap(const Game *game)
{
    ClearBackground((Color){69, 83, 65, 255});
    /* Pola tetap membuat rumput tidak berubah saat frame berganti. */
    for (int i = 0; i < 230; i++) {
        int x = (i * 73 + 19) % SCREEN_WIDTH;
        int y = 29 + (i * 47 + 11) % 179;
        DrawRectangle(x, y, 2, 1, (Color){87, 99, 70, 255});
        if (i % 4 == 0) DrawRectangle(x, y - 2, 1, 2, (Color){47, 66, 53, 255});
    }
    Ruin(8, 133, 40, 42);
    Ruin(146, 30, 36, 33);
    Ruin(269, 165, 37, 32);
    Ruin(397, 32, 58, 40);
    DrawRectangle(15, 188, 21, 10, (Color){87, 100, 87, 255});
    DrawRectangle(18, 185, 12, 5, INK);
    DrawRectangle(17, 196, 4, 3, INK);
    DrawRectangle(30, 196, 4, 3, INK);
    for (int i = 1; i < PATH_COUNT; i++) {
        DrawLineEx(PATH[i - 1], PATH[i], 25, (Color){42, 49, 47, 255});
        DrawRectangle((int)PATH[i].x - 12, (int)PATH[i].y - 12, 25, 25, (Color){42, 49, 47, 255});
    }
    for (int i = 1; i < PATH_COUNT; i++) {
        DrawLineEx(PATH[i - 1], PATH[i], 19, (Color){112, 112, 91, 255});
        DrawRectangle((int)PATH[i].x - 9, (int)PATH[i].y - 9, 19, 19, (Color){112, 112, 91, 255});
    }
    for (int d = 10; d < (int)PathLength() - 15; d += 28) {
        Vector2 p = PositionOnPath((float)d);
        DrawRectangle((int)p.x - 2, (int)p.y - 1, 4, 2, (Color){170, 157, 111, 255});
        DrawLine((int)p.x + 5, (int)p.y + 3, (int)p.x + 8, (int)p.y + 6, (Color){70, 78, 67, 255});
    }
    /* Pagar dan tenda markas di ujung jalur. */
    for (int y = 93; y < 198; y += 6) DrawRectangle(441, y, 3, 4, (Color){181, 163, 111, 255});
    DrawRectangle(445, 94, 33, 44, (Color){35, 49, 44, 255});
    DrawTriangle((Vector2){446, 106}, (Vector2){477, 106}, (Vector2){462, 85}, (Color){137, 151, 106, 255});
    DrawRectangle(450, 107, 24, 27, (Color){98, 119, 83, 255});
    DrawRectangle(459, 113, 7, 21, INK);
    DrawRectangle(449, 145, 27, 20, (Color){39, 55, 51, 255});
    DrawRectangleLines(449, 145, 27, 20, COLOR_GOLD);
    DrawText("SAFE", 450, 150, 10, PAPER);
    DrawRectangle(449, 172, 25, 3, INK);
    DrawRectangle(449, 172, game->health * 25 / 20, 3, COLOR_GREEN);
    DrawText(">>", 4, 58, 10, COLOR_GOLD);
}

static void DrawRange(Vector2 position, float range)
{
    DrawCircleV(position, range, (Color){145, 196, 139, 20});
    DrawCircleLines((int)position.x, (int)position.y, range, (Color){170, 211, 147, 170});
}

static void DrawUnits(const Game *game, const View *view, Vector2 mouse)
{
    int preview = -1;
    if (game->selected >= 0) {
        const Tower *selected = &game->towers[game->selected];
        for (int i = 0; i < UNIT_COUNT; i++) {
            if (CheckCollisionPointRec(mouse, UnitButton(i))) preview = i;
        }
        if (preview >= 0 && preview != selected->type) DrawRange(selected->position, UNITS[preview].range);
        else if (selected->type >= 0) DrawRange(selected->position, TowerRange(selected));
    }
    for (int i = 0; i < TOWER_COUNT; i++) {
        const Tower *tower = &game->towers[i];
        int x = (int)tower->position.x;
        int y = (int)tower->position.y;
        bool hover = CheckCollisionPointRec(mouse, (Rectangle){x - 11, y - 11, 22, 22});
        DrawRectangle(x - 11, y - 9, 23, 20, (Color){48, 55, 46, 255});
        DrawRectangleLines(x - 11, y - 9, 23, 20,
                           i == game->selected ? COLOR_GOLD : (hover ? PAPER : (Color){125, 133, 94, 255}));
        if (tower->type < 0) {
            DrawRectangle(x - 3, y, 7, 1, MUTED);
            DrawRectangle(x, y - 3, 1, 7, MUTED);
        } else {
            int frame = (int)(game->elapsed * 3) % 2;
            Sprite(view, tower->type, frame, (Vector2){x, y - 3}, 24, false, WHITE);
            for (int level = 0; level < tower->level; level++) {
                DrawRectangle(x - 6 + level * 5, y + 8, 3, 2, COLOR_GOLD);
            }
        }
        DrawText(TextFormat("%d", i + 1), x - 12, y - 17, 10, MUTED);
    }
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        const Zombie *zombie = &game->zombies[i];
        if (!zombie->active) continue;
        int frame = (int)(game->elapsed * 6 + i) % 2;
        int size = zombie->type == MUTANT ? 30 : 18;
        Vector2 next = PositionOnPath(zombie->distance + 2);
        bool flip = next.x < zombie->position.x;
        Color tint = zombie->flash > 0 ? (Color){255, 200, 150, 255} : WHITE;
        Sprite(view, zombie->type + 3, frame, zombie->position, size, flip, tint);
        int barX = (int)zombie->position.x - size / 2;
        int barY = (int)zombie->position.y - size / 2 - 4;
        DrawRectangle(barX, barY, size, 3, INK);
        DrawRectangle(barX, barY, zombie->health * size / ENEMIES[zombie->type].health, 2,
                       zombie->type == MUTANT ? COLOR_RED : COLOR_GREEN);
    }
    for (int i = 0; i < MAX_EFFECTS; i++) {
        const ShotEffect *effect = &game->effects[i];
        if (!effect->active) continue;
        float alpha = effect->remaining / 0.18f;
        Color color = effect->type == ARCHER ? PAPER : COLOR_GOLD;
        DrawLineV(effect->from, effect->to, Fade(color, alpha));
        DrawRectangle((int)effect->from.x - 1, (int)effect->from.y - 1, 3, 3, COLOR_GOLD);
        if (effect->type == CANNON) {
            float radius = 24 * (1 - alpha);
            DrawCircleV(effect->to, radius, Fade(COLOR_GOLD, alpha * 0.25f));
            DrawCircleLines((int)effect->to.x, (int)effect->to.y, radius, Fade(COLOR_GOLD, alpha));
        } else {
            DrawRectangle((int)effect->to.x - 1, (int)effect->to.y - 1, 3, 3, Fade(PAPER, alpha));
        }
    }
}

static void DrawHud(const Game *game)
{
    DrawRectangle(0, 0, SCREEN_WIDTH, 27, INK);
    DrawRectangle(0, 26, SCREEN_WIDTH, 1, (Color){96, 111, 86, 255});
    DrawText("POS TERAKHIR", 8, 5, 10, PAPER);
    DrawText("KETAHANAN", 125, 4, 10, MUTED);
    DrawText(TextFormat("%02d / 20", game->health), 125, 15, 10, game->health <= 5 ? COLOR_RED : COLOR_GREEN);
    DrawText("SCRAP", 235, 4, 10, MUTED);
    DrawText(TextFormat("%04d", game->scrap), 235, 15, 10, COLOR_GOLD);
    DrawText(TextFormat("WAVE %02d / 10", game->wave), 329, 5, 10, PAPER);
    int total = WaveSize(game->wave - 1);
    if (game->phase == FIGHTING) DrawText(TextFormat("MASUK %d/%d", game->spawned, total), 329, 16, 10, MUTED);
    else DrawText("SIAPKAN POS", 329, 16, 10, COLOR_GREEN);
    DrawRectangle(0, 211, SCREEN_WIDTH, 59, PANEL);
    DrawRectangle(0, 211, SCREEN_WIDTH, 1, (Color){96, 111, 86, 255});
}

static void DrawShop(const Game *game, const View *view, Vector2 mouse)
{
    const Tower *tower = NULL;
    if (game->selected >= 0) tower = &game->towers[game->selected];
    if (tower == NULL) {
        DrawText("KLIK POS + UNTUK MEMILIH UNIT", 8, 218, 10, PAPER);
    } else if (tower->type < 0) {
        DrawText(TextFormat("POS %d / KOSONG / PILIH UNIT", game->selected + 1), 8, 218, 10, COLOR_GOLD);
    } else {
        DrawText(TextFormat("POS %d / LV %d / DMG %d / JARAK %.0f", game->selected + 1,
                 tower->level, TowerDamage(tower), TowerRange(tower)), 8, 218, 10, COLOR_GOLD);
    }
    for (int type = 0; type < UNIT_COUNT; type++) {
        Rectangle rect = UnitButton(type);
        bool upgrading = tower != NULL && tower->type == type;
        bool maximum = upgrading && tower->level == 3;
        int cost = UNITS[type].price;
        if (tower != NULL) cost = upgrading ? UpgradeCost(tower) : ReplacementCost(tower, type);
        bool enabled = tower != NULL && !maximum && game->scrap >= cost && !game->paused;
        Button(rect, "", enabled, mouse);
        Sprite(view, type, 0, (Vector2){rect.x + 12, rect.y + 14}, 20, false, enabled ? WHITE : Fade(WHITE, 0.5f));
        const char *label = UNITS[type].name;
        if (upgrading) label = maximum ? "LV MAKS" : "UPGRADE";
        bool replacing = tower != NULL && tower->type >= 0 && !upgrading;
        if (replacing) label = "GANTI";
        DrawText(label, (int)rect.x + 25, (int)rect.y + 4, 10, enabled ? PAPER : MUTED);
        const char *price = maximum ? "LEVEL 3" : TextFormat("%d SCRAP", cost);
        if (replacing) price = TextFormat("%s %d", UNITS[type].name, cost);
        DrawText(price, (int)rect.x + 25, (int)rect.y + 16, 10, enabled ? COLOR_GOLD : MUTED);
        if (tower != NULL && CheckCollisionPointRec(mouse, rect)) {
            DrawRectangle(6, 197, 350, 13, INK);
            if (maximum) DrawText("Level maksimum tercapai.", 10, 199, 10, PAPER);
            else if (upgrading) DrawText(TextFormat("Upgrade: DMG %d / JARAK %.0f",
                       UNITS[type].damage * (tower->level + 2) / 2, TowerRange(tower) + 7), 10, 199, 10, PAPER);
            else DrawText(TextFormat("%s: DMG %d / JARAK %.0f / %.2f dtk%s", UNITS[type].name,
                       UNITS[type].damage, UNITS[type].range, UNITS[type].interval,
                       tower->type >= 0 ? " / GANTI LV 1" : ""), 10, 199, 10, PAPER);
        }
    }
    Button(WaveButton(), game->phase == FIGHTING ? "WAVE AKTIF" : TextFormat("MULAI WAVE %d", game->wave + 1),
           game->phase == PREPARING && !game->paused, mouse);
    Button(PauseButton(), game->paused ? "LANJUT" : "JEDA", true, mouse);
    Button(SpeedButton(), TextFormat("%dx", game->speed), true, mouse);
}

void DrawGame(const Game *game, const View *view, Vector2 mouse)
{
    DrawMap(game);
    DrawUnits(game, view, mouse);
    if (game->noticeTimer > 0) {
        int width = MeasureText(game->notice, 10) + 12;
        DrawRectangle(5, 197, width, 13, Fade(INK, 0.94f));
        DrawText(game->notice, 11, 199, 10, PAPER);
    }
    DrawHud(game);
    DrawShop(game, view, mouse);
    if (game->paused) {
        DrawRectangle(150, 84, 180, 54, Fade(INK, 0.96f));
        DrawRectangleLines(150, 84, 180, 54, COLOR_GOLD);
        CenterText("JEDA", 150, 94, 180, 20, COLOR_GOLD);
        CenterText("Klik LANJUT atau tekan P", 150, 120, 180, 10, PAPER);
    }
}

void DrawMenu(const Game *game, const View *view, Vector2 mouse)
{
    DrawMap(game);
    DrawUnits(game, view, (Vector2){-100, -100});
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(INK, 0.85f));
    DrawRectangle(43, 38, 394, 202, PANEL);
    DrawRectangleLines(43, 38, 394, 202, (Color){104, 130, 102, 255});
    CenterText("CATATAN PENYINTAS / HARI 037", 43, 51, 394, 10, COLOR_GOLD);
    CenterText("POS TERAKHIR", 43, 76, 394, 30, PAPER);
    CenterText("Wabah menyisakan satu tempat berlindung.", 43, 115, 394, 10, MUTED);
    CenterText("Isi pos. Perkuat pasukan. Tahan 10 gelombang.", 43, 132, 394, 10, PAPER);
    Button(PlayButton(), "MULAI BERTAHAN", true, mouse);
    Button(MenuButton(), "KELUAR", true, mouse);
    CenterText("MOUSE: PILIH / SPASI: WAVE / P: JEDA / F: 2x", 0, 250, SCREEN_WIDTH, 10, MUTED);
}

void DrawResult(const Game *game, Vector2 mouse)
{
    bool won = game->phase == VICTORY;
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(INK, 0.87f));
    DrawRectangle(66, 41, 348, 192, PANEL);
    DrawRectangleLines(66, 41, 348, 192, won ? COLOR_GREEN : COLOR_RED);
    CenterText(won ? "FAJAR MASIH ADA" : "POS TELAH JATUH", 66, 59, 348, 20, won ? COLOR_GREEN : COLOR_RED);
    CenterText(won ? "Sepuluh gelombang berlalu. Para penyintas selamat."
                   : "Gerbang runtuh. Bangun pertahanan baru.", 66, 91, 348, 10, PAPER);
    CenterText(TextFormat("WAVE %d/10 / KALAHKAN %d / MARKAS %d", game->wave, game->kills, game->health),
               66, 114, 348, 10, COLOR_GOLD);
    CenterText(TextFormat("SISA SCRAP %d", game->scrap), 66, 133, 348, 10, MUTED);
    Button(PlayButton(), "MAIN LAGI", true, mouse);
    Button(MenuButton(), "MENU", true, mouse);
}
