#pragma once
#include <iostream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#else
#include <GL/freeglut.h>
#endif
#include <vector>
#include "Entity.h"
#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Boss.h"
#include "Perk.h"
#include "Particle.h"
#include "AIDirector.h"
#include "CampaignManager.h"

class Game: IDrawable {
private:
    char* filename;
    char* filename2;
    char* filename3;

    Player* player;
    std::vector<Bullet*> bullets;
    std::vector<Enemy*>  enemies;
    std::vector<Perk*>   perks;
    std::vector<Particle*> particles;
    Boss* boss;
    AIDirector* director;
    CampaignManager* campaign;

    int   score;
    int   killCount;
    unsigned int gameMode;
    int   spawnTimer;

    int   activeWeapon;    // 0=pistol 1=rapid 2=flame
    int   weaponTimer;
    bool  shieldActive;
    int   shieldTimer;

    bool  bossAlive;
    float bossMaxHealth;
    int   nextBossKillThreshold;

    float bgAnimTimer;
    int   perkDropCounter;
    int   waveFlashTimer;
    int   damageFlashTimer;

    // Screen shake
    float screenShakeX;
    float screenShakeY;
    float screenShakeIntensity;

    // Auto-fire
    bool  mouseHeld;
    float mouseX, mouseY;
    int   fireRateTimer;

    // Ammo system
    int   ammo;
    int   maxAmmo;
    bool  reloading;
    int   reloadTimer;
    int   reloadDuration;

    // Game state
    bool  isGameOver;
    bool  paused;
    bool  isVictory;
    
    // Combo System
    int   killCombo;
    int   comboTimer;
    float comboMultiplier;
    
    // Player Enhancements
    float speedBoostTimer;
    float criticalMultiplier;
    float damageBoostTimer;
    float slowMotionTimer;
    float slowMotionScale;
    
    // Visual Effects
    float hitFlashTimer;
    float weaponFlashTimer;
    int   criticalHitCounter;

public:
    Game();
    virtual ~Game();
    void reset();
    void setGameMode(unsigned int mode);
    void updateMovement(bool keys[256]);
    void onKeyPressed(unsigned char key, int x, int y);
    void onMouseClicked(int button, int state, int x, int y);
    void onMouseMove(int x, int y);
    void timer(void (*t)(int));
    void draw();
    bool getIsGameOver() const { return isGameOver; }
    void togglePause();
    bool isPaused() const;

    // Make accessible for AIDirector
    void SpawnEnemy(int count = 1, bool isHorde = false);
    void dropPerk(float x, float y);
    void spawnBlood(float x, float y, int count);
    void spawnExplosion(float x, float y, int count);
    void spawnSparks(float x, float y, int count);

private:
    void drawBackground();
    void drawEnemyHealthBars();
    void drawHUD();
    void drawPauseOverlay();
    void drawVictorySummary();
    void drawMiniMap();
    void SpawnBoss();
    void moveEnemy(Enemy* e);
    void fireBullet(float mx, float my);
    void tryPickupPerks();
    bool detectCollision(Entity* a, Entity* b);
    void pushBack(Entity* e1, Entity* e2);
    void clampPlayer();
    void updateScreenShake();
    void handleBossProjectiles();
};
