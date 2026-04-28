#pragma once

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#else
#include <GL/freeglut.h>
#endif

enum ParticleType {
    PARTICLE_BLOOD,
    PARTICLE_SPARK,
    PARTICLE_EXPLOSION
};

class Particle {
public:
    float x, y;
    float vx, vy;
    float size;
    float life;
    float initialLife;
    ParticleType type;
    
    Particle(float startX, float startY, float vX, float vY, float sz, float lifetime, ParticleType t);
    void update();
    void draw();
    bool isDead() const;
};
