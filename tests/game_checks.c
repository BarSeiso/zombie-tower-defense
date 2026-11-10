#include "game.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "GAGAL baris %d: %s\n", __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

static void EmptyFight(Game *game)
{
    ResetGame(game);
    game->phase = FIGHTING;
    game->wave = 1;
    game->spawnTimer = 10000;
}

static void PutZombie(Game *game, int index, int type, float distance)
{
    game->zombies[index] = (Zombie){.active = true, .type = type,
        .health = ENEMIES[type].health, .distance = distance,
        .position = PositionOnPath(distance)};
}

static void CheckTransactions(void)
{
    Game game;
    ResetGame(&game);
    CHECK(game.health == 20 && game.scrap == 180 && game.selected == -1);
    CHECK(ChangeUnit(&game, 0, ARCHER));
    CHECK(game.scrap == 110 && game.towers[0].level == 1);
    CHECK(UpgradeTower(&game, 0));
    CHECK(game.scrap == 75 && TowerDamage(&game.towers[0]) == 13);
    CHECK(TowerRange(&game.towers[0]) == 63);
    CHECK(UpgradeTower(&game, 0));
    CHECK(game.scrap == 23 && TowerDamage(&game.towers[0]) == 18);
    CHECK(!UpgradeTower(&game, 0));
    CHECK(!ChangeUnit(&game, 0, CANNON));
    CHECK(game.scrap == 23 && game.towers[0].type == ARCHER && game.towers[0].level == 3);
    CHECK(ReplacementCost(&game.towers[0], RIFLE) == 87);
    game.scrap = 87;
    CHECK(ChangeUnit(&game, 0, RIFLE));
    CHECK(game.scrap == 0 && game.towers[0].level == 1);
    CHECK(fabsf(game.towers[0].cooldown - 0.70f) < 0.001f);
    game.scrap = 1000;
    game.paused = true;
    CHECK(!ChangeUnit(&game, 1, CANNON));
    CHECK(!UpgradeTower(&game, 0));
    StartWave(&game);
    CHECK(game.wave == 0 && game.scrap == 1000);
    float elapsed = game.elapsed;
    UpdateGame(&game, STEP_SECONDS);
    CHECK(game.elapsed == elapsed);
    game.paused = false;
    CHECK(!ChangeUnit(&game, -1, ARCHER));
    CHECK(!ChangeUnit(&game, TOWER_COUNT, ARCHER));
    CHECK(!ChangeUnit(&game, 1, UNIT_COUNT));
}

static void CheckCombat(void)
{
    Game game;
    EmptyFight(&game);
    CHECK(ChangeUnit(&game, 0, ARCHER));
    game.towers[0].cooldown = 0;
    PutZombie(&game, 0, NORMAL, 68);
    PutZombie(&game, 1, NORMAL, 90);
    UpdateGame(&game, STEP_SECONDS);
    CHECK(game.zombies[0].health == 30);
    CHECK(game.zombies[1].health == 21); /* Sasaran paling maju. */

    EmptyFight(&game);
    game.towers[0].position = (Vector2){60, 0};
    CHECK(ChangeUnit(&game, 0, ARCHER));
    game.towers[0].cooldown = 0;
    PutZombie(&game, 0, NORMAL, 68);
    UpdateGame(&game, STEP_SECONDS);
    CHECK(game.zombies[0].health == 30); /* Di luar jangkauan. */

    EmptyFight(&game);
    CHECK(ChangeUnit(&game, 0, CANNON));
    game.towers[0].cooldown = 0;
    PutZombie(&game, 0, NORMAL, 68);
    PutZombie(&game, 1, NORMAL, 76);
    PutZombie(&game, 2, ARMORED, 60);
    int before = game.scrap;
    UpdateGame(&game, STEP_SECONDS);
    CHECK(!game.zombies[0].active && !game.zombies[1].active);
    CHECK(game.zombies[2].health == 85);
    CHECK(game.scrap == before + 14 && game.kills == 2);
    UpdateGame(&game, STEP_SECONDS);
    CHECK(game.scrap == before + 14 && game.kills == 2);

    EmptyFight(&game);
    PutZombie(&game, 0, MUTANT, PathLength() - 0.1f);
    UpdateGame(&game, STEP_SECONDS);
    CHECK(game.phase == DEFEAT && game.health == 0 && game.kills == 0);
}

static void CheckWaves(void)
{
    Game game;
    ResetGame(&game);
    StartWave(&game);
    StartWave(&game);
    CHECK(game.wave == 1);
    UpdateGame(&game, STEP_SECONDS);
    CHECK(game.spawned == 1 && game.zombies[0].type == NORMAL);
    for (int i = 0; i < 60 * 200 && game.phase == FIGHTING; i++) UpdateGame(&game, STEP_SECONDS);
    CHECK(game.phase == PREPARING && game.health == 12 && game.scrap == 210);
    int reward = game.scrap;
    UpdateGame(&game, STEP_SECONDS);
    CHECK(game.scrap == reward); /* Bonus hanya sekali. */
    StartWave(&game);
    for (int i = 0; i < 60 * 200 && game.phase == FIGHTING; i++) UpdateGame(&game, STEP_SECONDS);
    CHECK(game.phase == DEFEAT && game.health == 0);

    EmptyFight(&game);
    game.wave = 10;
    game.spawned = WaveSize(9) - 1;
    game.spawnTimer = 0;
    UpdateGame(&game, STEP_SECONDS);
    CHECK(game.spawned == 37 && game.zombies[0].type == RAGE);
    game.zombies[0].active = false;
    UpdateGame(&game, STEP_SECONDS);
    CHECK(game.phase == VICTORY);

    EmptyFight(&game);
    game.wave = 10;
    game.spawned = 24;
    game.spawnTimer = 0;
    UpdateGame(&game, STEP_SECONDS);
    CHECK(game.zombies[0].type == MUTANT);
}

static void Buy(Game *game, int slot, int type, int level)
{
    if (level == 1) CHECK(ChangeUnit(game, slot, type));
    else CHECK(UpgradeTower(game, slot));
}

static void CheckWinningSession(void)
{
    Game game;
    ResetGame(&game);
    for (int wave = 1; wave <= WAVE_COUNT; wave++) {
        switch (wave) {
            case 1: Buy(&game, 2, ARCHER, 1); Buy(&game, 4, ARCHER, 1); break;
            case 2: Buy(&game, 2, ARCHER, 2); Buy(&game, 4, ARCHER, 2); break;
            case 3: Buy(&game, 0, CANNON, 1); break;
            case 4: Buy(&game, 6, RIFLE, 1); Buy(&game, 2, ARCHER, 3); break;
            case 5: Buy(&game, 1, CANNON, 1); Buy(&game, 4, ARCHER, 3); break;
            case 6: Buy(&game, 5, RIFLE, 1); Buy(&game, 0, CANNON, 2); break;
            case 7: Buy(&game, 0, CANNON, 3); Buy(&game, 1, CANNON, 2); Buy(&game, 6, RIFLE, 2); break;
            case 8: Buy(&game, 7, CANNON, 1); Buy(&game, 3, RIFLE, 1); Buy(&game, 1, CANNON, 3); break;
            case 9: Buy(&game, 7, CANNON, 2); Buy(&game, 6, RIFLE, 3); Buy(&game, 5, RIFLE, 2); Buy(&game, 3, RIFLE, 2); break;
            case 10: Buy(&game, 7, CANNON, 3); Buy(&game, 5, RIFLE, 3); Buy(&game, 3, RIFLE, 3); break;
        }
        StartWave(&game);
        int frames = 0;
        while (game.phase == FIGHTING && frames < 60 * 300) {
            UpdateGame(&game, STEP_SECONDS);
            frames++;
        }
        CHECK(game.phase != FIGHTING && game.phase != DEFEAT);
        CHECK(game.health == 20);
    }
    CHECK(game.phase == VICTORY && game.kills == 236);
    ResetGame(&game);
    CHECK(game.wave == 0 && game.kills == 0 && game.scrap == 180 && game.health == 20);
    for (int i = 0; i < TOWER_COUNT; i++) CHECK(game.towers[i].type == -1);
    for (int i = 0; i < MAX_ZOMBIES; i++) CHECK(!game.zombies[i].active);
}

int main(void)
{
    CheckTransactions();
    CheckCombat();
    CheckWaves();
    CheckWinningSession();
    puts("LULUS: transaksi, jangkauan, target, ledakan, jeda, wave, menang, kalah, reset.");
    return 0;
}
