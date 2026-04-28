#include "Particle.h"
#include <cmath>
#include <cstdlib>

Particle::Particle(float startX, float startY, float vX, float vY, float sz, float lifetime, ParticleType t)
    : x(startX), y(startY), vx(vX), vy(vY), size(sz), life(lifetime), initialLife(lifetime), type(t) {
}

void Particle::update() {
    x += vx;
    y += vy;
    
    // Friction
    if (type == PARTICLE_BLOOD) {
        vx *= 0.85f;
        vy *= 0.85f;
    } else if (type == PARTICLE_SPARK) {
        vx *= 0.95f;
        vy *= 0.95f;
    } else if (type == PARTICLE_EXPLOSION) {
        vx *= 0.90f;
        vy *= 0.90f;
        size += 1.5f; // explosions expand
    }
    
    life -= 1.0f;
}

bool Particle::isDead() const {
    return life <= 0.0f;
}

void Particle::draw() {
    if (isDead()) return;
    
    float alpha = life / initialLife;
    
    glPushMatrix();
    glTranslatef(x, y, 0);
    
    if (type == PARTICLE_BLOOD) {
        glColor4f(0.8f, 0.0f, 0.0f, alpha);
        glBegin(GL_POLYGON);
        for(int i=0; i<8; i++) {
            float a = i * 3.14159f / 4.0f;
            glVertex2f(size * cosf(a), size * sinf(a));
        }
        glEnd();
    } 
    else if (type == PARTICLE_SPARK) {
        glColor4f(1.0f, 0.8f, 0.2f, alpha);
        glBegin(GL_QUADS);
        glVertex2f(-size, -size);
        glVertex2f(size, -size);
        glVertex2f(size, size);
        glVertex2f(-size, size);
        glEnd();
    }
    else if (type == PARTICLE_EXPLOSION) {
        // Outer orange/red
        glColor4f(1.0f, 0.3f, 0.0f, alpha * 0.8f);
        glBegin(GL_POLYGON);
        for(int i=0; i<12; i++) {
            float a = i * 3.14159f / 6.0f;
            glVertex2f(size * cosf(a), size * sinf(a));
        }
        glEnd();
        
        // Inner yellow
        glColor4f(1.0f, 0.9f, 0.2f, alpha);
        glBegin(GL_POLYGON);
        for(int i=0; i<12; i++) {
            float a = i * 3.14159f / 6.0f;
            glVertex2f(size * 0.5f * cosf(a), size * 0.5f * sinf(a));
        }
        glEnd();
    }
    
    glPopMatrix();
}
