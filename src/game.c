#include "game.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

const UnitStats UNITS[UNIT_COUNT] = {
    {"PEMANAH", 70, 9, 56, 0.50f},
    {"PENEMBAK", 115, 22, 75, 0.70f},
    {"MERIAM", 165, 40, 65, 1.45f}
};

const EnemyStats ENEMIES[ENEMY_COUNT] = {
    {"Zombie Biasa", 30, 28, 7, 1},
    {"Rage Zombie", 42, 58, 11, 1},
    {"Zombie Berzirah", 125, 22, 18, 2},
    {"Mutant Zombie", 620, 18, 55, 20}
};

/* count berurutan Biasa, Rage, Berzirah, Mutant.
   order menentukan kelompok yang keluar lebih dahulu. */
const GameWave WAVES[WAVE_COUNT] = {
    {{8, 0, 0, 0}, {NORMAL, RAGE, ARMORED, MUTANT}, 1.00f},
    {{12, 0, 0, 0}, {NORMAL, RAGE, ARMORED, MUTANT}, 0.85f},
    {{8, 6, 0, 0}, {NORMAL, RAGE, ARMORED, MUTANT}, 0.75f},
    {{10, 8, 2, 0}, {NORMAL, RAGE, ARMORED, MUTANT}, 0.70f},
    {{0, 12, 4, 0}, {RAGE, ARMORED, NORMAL, MUTANT}, 0.65f},
    {{18, 12, 4, 0}, {NORMAL, RAGE, ARMORED, MUTANT}, 0.58f},
    {{0, 14, 9, 0}, {ARMORED, RAGE, NORMAL, MUTANT}, 0.55f},
    {{0, 20, 8, 0}, {RAGE, ARMORED, NORMAL, MUTANT}, 0.50f},
    {{16, 16, 12, 0}, {NORMAL, ARMORED, RAGE, MUTANT}, 0.45f},
    {{0, 28, 8, 1}, {ARMORED, RAGE, MUTANT, NORMAL}, 0.48f}
};

const Vector2 PATH[PATH_COUNT] = {
    {-8, 72}, {104, 72}, {104, 156}, {214, 156},
    {214, 66}, {333, 66}, {333, 154}, {445, 154}
};

static const Vector2 POSITIONS[TOWER_COUNT] = {
    {60, 44}, {67, 109}, {146, 119}, {158, 184},
    {256, 124}, {268, 39}, {374, 114}, {408, 181}
};

static float Distance(Vector2 a, Vector2 b)
{
    float x = a.x - b.x;
    float y = a.y - b.y;
    return sqrtf(x * x + y * y);
}

float PathLength(void)
{
    float total = 0;
    for (int i = 1; i < PATH_COUNT; i++) total += Distance(PATH[i - 1], PATH[i]);
    return total;
}

Vector2 PositionOnPath(float distance)
{
    if (distance < 0) distance = 0;
    for (int i = 1; i < PATH_COUNT; i++) {
        Vector2 start = PATH[i - 1];
        Vector2 end = PATH[i];
        float length = Distance(start, end);
        if (distance <= length) {
            float fraction = distance / length;
            return (Vector2){start.x + (end.x - start.x) * fraction,
                             start.y + (end.y - start.y) * fraction};
        }
        distance -= length;
    }
    return PATH[PATH_COUNT - 1];
}

void SetNotice(Game *game, const char *text)
{
    snprintf(game->notice, sizeof(game->notice), "%s", text);
    game->noticeTimer = 3.0f;
}

void ResetGame(Game *game)
{
    memset(game, 0, sizeof(*game));
    game->health = 20;
    game->scrap = 180;
    game->selected = -1;
    game->speed = 1;
    game->phase = PREPARING;
    for (int i = 0; i < TOWER_COUNT; i++) {
        game->towers[i].position = POSITIONS[i];
        game->towers[i].type = -1;
    }
    SetNotice(game, "Pilih pos kosong untuk membangun pertahanan.");
}

int WaveSize(int waveIndex)
{
    if (waveIndex < 0 || waveIndex >= WAVE_COUNT) return 0;
    int total = 0;
    for (int i = 0; i < ENEMY_COUNT; i++) total += WAVES[waveIndex].count[i];
    return total;
}

void StartWave(Game *game)
{
    if (game->phase != PREPARING || game->paused || game->wave >= WAVE_COUNT) return;
    game->wave++;
    game->phase = FIGHTING;
    game->spawned = 0;
    game->spawnTimer = 0;
    SetNotice(game, "Zombie datang! Upgrade dan ganti unit tetap tersedia.");
}

int TowerDamage(const Tower *tower)
{
    if (tower->type < 0 || tower->type >= UNIT_COUNT) return 0;
    return UNITS[tower->type].damage * (tower->level + 1) / 2;
}

float TowerRange(const Tower *tower)
{
    if (tower->type < 0 || tower->type >= UNIT_COUNT) return 0;
    return UNITS[tower->type].range + 7 * (tower->level - 1);
}

int UpgradeCost(const Tower *tower)
{
    if (tower->type < 0 || tower->type >= UNIT_COUNT || tower->level >= 3) return 0;
    int percent = tower->level == 1 ? 50 : 75;
    return UNITS[tower->type].price * percent / 100;
}

int ReplacementCost(const Tower *tower, int newType)
{
    if (newType < 0 || newType >= UNIT_COUNT) return 0;
    int credit = 0;
    if (tower->type >= 0 && tower->type < UNIT_COUNT) credit = UNITS[tower->type].price * 40 / 100;
    return UNITS[newType].price - credit;
}

static bool CanTrade(const Game *game, int slot)
{
    return slot >= 0 && slot < TOWER_COUNT && !game->paused &&
           (game->phase == PREPARING || game->phase == FIGHTING);
}

bool UpgradeTower(Game *game, int slot)
{
    if (!CanTrade(game, slot)) return false;
    Tower *tower = &game->towers[slot];
    if (tower->type < 0 || tower->level >= 3) return false;
    int cost = UpgradeCost(tower);
    if (game->scrap < cost) {
        SetNotice(game, "Scrap belum cukup untuk upgrade.");
        return false;
    }
    game->scrap -= cost;
    tower->level++;
    SetNotice(game, "Upgrade berhasil. Damage dan jangkauan bertambah.");
    return true;
}

bool ChangeUnit(Game *game, int slot, int type)
{
    if (!CanTrade(game, slot) || type < 0 || type >= UNIT_COUNT) return false;
    Tower *tower = &game->towers[slot];
    if (tower->type == type) return false;
    int cost = ReplacementCost(tower, type);
    if (game->scrap < cost) {
        SetNotice(game, "Scrap belum cukup. Kalahkan zombie untuk mendapat Scrap.");
        return false;
    }
    game->scrap -= cost;
    tower->type = type;
    tower->level = 1;
    tower->cooldown = UNITS[type].interval;
    SetNotice(game, "Unit siap bertugas di pos pertahanan.");
    return true;
}

static int SpawnType(int waveIndex, int index)
{
    /* GameWave terakhir mempunyai dua kelompok Rage yang dipisahkan Mutant. */
    if (waveIndex == 9) {
        if (index < 8) return ARMORED;
        if (index < 24) return RAGE;
        if (index == 24) return MUTANT;
        return RAGE;
    }
    const GameWave *wave = &WAVES[waveIndex];
    for (int i = 0; i < ENEMY_COUNT; i++) {
        int type = wave->order[i];
        if (index < wave->count[type]) return type;
        index -= wave->count[type];
    }
    return NORMAL;
}

static bool SpawnZombie(Game *game)
{
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        Zombie *zombie = &game->zombies[i];
        if (zombie->active) continue;
        int type = SpawnType(game->wave - 1, game->spawned);
        *zombie = (Zombie){.active = true, .type = type,
                            .health = ENEMIES[type].health, .position = PATH[0]};
        game->spawned++;
        return true;
    }
    return false;
}

static void AddEffect(Game *game, const Tower *tower, Vector2 destination)
{
    for (int i = 0; i < MAX_EFFECTS; i++) {
        if (game->effects[i].active) continue;
        game->effects[i] = (ShotEffect){true, tower->type, tower->position,
                                       destination, 0.18f};
        return;
    }
}

static void HurtZombie(Game *game, Zombie *zombie, int damage)
{
    zombie->health -= damage;
    zombie->flash = 0.10f;
    if (zombie->health <= 0) {
        zombie->active = false;
        game->scrap += ENEMIES[zombie->type].reward;
        game->kills++;
    }
}

static void FireTower(Game *game, Tower *tower)
{
    Zombie *target = NULL;
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        Zombie *zombie = &game->zombies[i];
        if (!zombie->active || Distance(tower->position, zombie->position) > TowerRange(tower)) continue;
        if (target == NULL || zombie->distance > target->distance) target = zombie;
    }
    if (target == NULL) return;

    Vector2 impact = target->position;
    AddEffect(game, tower, impact);
    if (tower->type == CANNON) {
        for (int i = 0; i < MAX_ZOMBIES; i++) {
            Zombie *zombie = &game->zombies[i];
            if (zombie->active && Distance(impact, zombie->position) <= 24) {
                HurtZombie(game, zombie, TowerDamage(tower));
            }
        }
    } else {
        HurtZombie(game, target, TowerDamage(tower));
    }
    tower->cooldown = UNITS[tower->type].interval;
}

void UpdateGame(Game *game, float seconds)
{
    if (game->paused || game->phase == VICTORY || game->phase == DEFEAT) return;
    game->elapsed += seconds;
    if (game->noticeTimer > 0) game->noticeTimer -= seconds;
    for (int i = 0; i < MAX_EFFECTS; i++) {
        ShotEffect *effect = &game->effects[i];
        if (!effect->active) continue;
        effect->remaining -= seconds;
        if (effect->remaining <= 0) effect->active = false;
    }
    if (game->phase != FIGHTING) return;

    game->spawnTimer -= seconds;
    if (game->spawned < WaveSize(game->wave - 1) && game->spawnTimer <= 0) {
        if (SpawnZombie(game)) game->spawnTimer += WAVES[game->wave - 1].interval;
    }

    float length = PathLength();
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        Zombie *zombie = &game->zombies[i];
        if (!zombie->active) continue;
        zombie->flash -= seconds;
        zombie->distance += ENEMIES[zombie->type].speed * seconds;
        zombie->position = PositionOnPath(zombie->distance);
        if (zombie->distance >= length) {
            zombie->active = false;
            game->health -= ENEMIES[zombie->type].baseDamage;
            if (game->health <= 0) {
                game->health = 0;
                game->phase = DEFEAT;
                return;
            }
            SetNotice(game, "Zombie menembus gerbang! Ketahanan markas berkurang.");
        }
    }

    for (int i = 0; i < TOWER_COUNT; i++) {
        Tower *tower = &game->towers[i];
        if (tower->type < 0) continue;
        tower->cooldown -= seconds;
        if (tower->cooldown <= 0) FireTower(game, tower);
    }

    if (game->spawned < WaveSize(game->wave - 1)) return;
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (game->zombies[i].active) return;
    }
    if (game->wave == WAVE_COUNT) {
        game->phase = VICTORY;
    } else {
        int bonus = 25 + 5 * game->wave;
        game->scrap += bonus;
        game->phase = PREPARING;
        char message[100];
        snprintf(message, sizeof(message), "GameWave bersih! Bonus %d Scrap. Siapkan pertahanan.", bonus);
        SetNotice(game, message);
    }
}
