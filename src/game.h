#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 270
#define TOWER_COUNT 8
#define MAX_ZOMBIES 64
#define MAX_EFFECTS 96
#define WAVE_COUNT 10
#define PATH_COUNT 8
#define UNIT_COUNT 3
#define ENEMY_COUNT 4
#define STEP_SECONDS (1.0f / 60.0f)

typedef enum { ARCHER, RIFLE, CANNON } UnitType;
typedef enum { NORMAL, RAGE, ARMORED, MUTANT } EnemyType;
typedef enum { PREPARING, FIGHTING, VICTORY, DEFEAT } Phase;

typedef struct {
    const char *name;
    int price;
    int damage;
    float range;
    float interval;
} UnitStats;

typedef struct {
    const char *name;
    int health;
    float speed;
    int reward;
    int baseDamage;
} EnemyStats;

typedef struct {
    Vector2 position;
    int type; /* -1 berarti pos kosong. */
    int level;
    float cooldown;
} Tower;

typedef struct {
    bool active;
    int type;
    int health;
    float distance;
    Vector2 position;
    float flash;
} Zombie;

typedef struct {
    bool active;
    int type;
    Vector2 from;
    Vector2 to;
    float remaining;
} ShotEffect;

typedef struct {
    int count[ENEMY_COUNT];
    int order[ENEMY_COUNT];
    float interval;
} GameWave;

typedef struct {
    Tower towers[TOWER_COUNT];
    Zombie zombies[MAX_ZOMBIES];
    ShotEffect effects[MAX_EFFECTS];
    Phase phase;
    int health;
    int scrap;
    int wave; /* Jumlah wave yang sudah dimulai. */
    int spawned;
    int kills;
    int selected;
    int speed;
    bool paused;
    float spawnTimer;
    float elapsed;
    float noticeTimer;
    char notice[100];
} Game;

extern const UnitStats UNITS[UNIT_COUNT];
extern const EnemyStats ENEMIES[ENEMY_COUNT];
extern const GameWave WAVES[WAVE_COUNT];
extern const Vector2 PATH[PATH_COUNT];

void ResetGame(Game *game);
void StartWave(Game *game);
void UpdateGame(Game *game, float seconds);
bool ChangeUnit(Game *game, int slot, int type);
bool UpgradeTower(Game *game, int slot);
int UpgradeCost(const Tower *tower);
int ReplacementCost(const Tower *tower, int newType);
int TowerDamage(const Tower *tower);
float TowerRange(const Tower *tower);
int WaveSize(int waveIndex);
float PathLength(void);
Vector2 PositionOnPath(float distance);
void SetNotice(Game *game, const char *text);

#endif
