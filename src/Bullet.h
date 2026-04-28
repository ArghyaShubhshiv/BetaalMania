#pragma once
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#else
#include <GL/freeglut.h>
#endif
#include "Entity.h"
#include <iostream>
#include <vector>

enum BulletType {
    BULLET_PISTOL,
    BULLET_RAPID,
    BULLET_FLAME,
    BULLET_SHOTGUN,
    BULLET_SNIPER,
    BULLET_GRENADE
};

class Bullet: public Entity {
public:
    BulletType bulletType;

    Bullet(float x, float y, float angle, BulletType type = BULLET_PISTOL);
    ~Bullet();
    void draw() override;
    void moveBullet();
    void setDamage(float damage);
    float getDamage();
};