#pragma once
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#else
#include <GL/freeglut.h>
#endif
#include <cmath>

enum PerkType {
    PERK_HEALTH,
    PERK_SHIELD,
    PERK_BETTER_GUN,
    PERK_FLAMETHROWER,
    PERK_AMMO,
    PERK_SPEED_BOOST,
    PERK_CRITICAL_BOOST,
    PERK_DAMAGE_BOOST,
    PERK_SLOW_TIME
};

class Perk {
public:
    float x, y;
    PerkType type;
    bool active;
    int lifeTimer;     // counts down
    float pulseTimer;  // for animation

    Perk(float x, float y, PerkType type);
    void draw();
    void update();
    bool isExpired();
};
