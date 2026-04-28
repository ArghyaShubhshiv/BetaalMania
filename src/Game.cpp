#include "Game.h"
#include <ctime>
#include <cstring>
#include <cmath>
#include <cstdio>

// Drawing helpers
static void fillRect(float x,float y,float w,float h){glBegin(GL_QUADS);glVertex2f(x,y);glVertex2f(x+w,y);glVertex2f(x+w,y+h);glVertex2f(x,y+h);glEnd();}
static void outlineRect(float x,float y,float w,float h){glBegin(GL_LINE_LOOP);glVertex2f(x,y);glVertex2f(x+w,y);glVertex2f(x+w,y+h);glVertex2f(x,y+h);glEnd();}
static void renderText(float x,float y,void* f,const char* s){glRasterPos2f(x,y);for(const char* c=s;*c;c++)glutBitmapCharacter(f,(int)*c);}
static void renderTextC(float cx,float y,void* f,const char* s){int w=0;for(const char* c=s;*c;c++)w+=glutBitmapWidth(f,(int)*c);renderText(cx-(float)w/2,y,f,s);}

Game::Game() {
    isGameOver=false;
    srand((unsigned)time(nullptr));
    filename=strdup("res/player.bmp"); filename2=strdup("res/enemy.bmp"); filename3=strdup("res/bullet.bmp");
    player=new Player(60,60,5,filename,100); boss=nullptr; director=new AIDirector();
    campaign=new CampaignManager();
    score=0;killCount=0;gameMode=1;spawnTimer=0;activeWeapon=0;weaponTimer=0;
    shieldActive=false;shieldTimer=0;bossAlive=false;bossMaxHealth=800;
    nextBossKillThreshold=15;bgAnimTimer=0;perkDropCounter=0;waveFlashTimer=0;damageFlashTimer=0;
    screenShakeX=0;screenShakeY=0;screenShakeIntensity=0;
    mouseHeld=false;mouseX=0;mouseY=0;fireRateTimer=0;
    ammo=30;maxAmmo=30;reloading=false;reloadTimer=0;reloadDuration=120;
    
    // Initialize new systems
    killCombo=0;comboTimer=0;comboMultiplier=1.0f;
    speedBoostTimer=0;criticalMultiplier=1.0f;damageBoostTimer=0;
    slowMotionTimer=0;slowMotionScale=1.0f;
    hitFlashTimer=0;weaponFlashTimer=0;criticalHitCounter=0;
    paused=false;
    isVictory=false;
    
    player->setPositionX(400);player->setPositionY(300);
}
Game::~Game(){
    free(filename);
    free(filename2);
    free(filename3);
    delete player;
    if(boss)delete boss;
    for(auto e:enemies)delete e;
    for(auto b:bullets)delete b;
    for(auto p:perks)delete p;
    for(auto pt:particles)delete pt;
    delete director;
    delete campaign;
}

void Game::reset(){
    isGameOver=false;
    score=0;killCount=0;spawnTimer=0;activeWeapon=0;weaponTimer=0;shieldActive=false;shieldTimer=0;
    bossAlive=false;bossMaxHealth=800;nextBossKillThreshold=15;bgAnimTimer=0;perkDropCounter=0;waveFlashTimer=0;damageFlashTimer=0;
    
    // Reset new systems
    killCombo=0;comboTimer=0;comboMultiplier=1.0f;
    speedBoostTimer=0;criticalMultiplier=1.0f;damageBoostTimer=0;
    slowMotionTimer=0;slowMotionScale=1.0f;
    hitFlashTimer=0;weaponFlashTimer=0;criticalHitCounter=0;
    
    if(boss){delete boss;boss=nullptr;}
    for(auto e:enemies)delete e;enemies.clear();
    for(auto b:bullets)delete b;bullets.clear();
    for(auto p:perks)delete p;perks.clear();
    for(auto pt:particles)delete pt;particles.clear();
    campaign->reset();
    isVictory=false;
    float hp=(gameMode==0)?140:(gameMode==2)?70:100;
    player->setHealth(hp);player->setPositionX(400);player->setPositionY(300);
}
void Game::setGameMode(unsigned int m){gameMode=m;reset();}

void Game::spawnBlood(float x, float y, int count) {
    for (int i = 0; i < count; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.0f + (rand() % 200) / 100.0f;
        float sz = 2.0f + (rand() % 30) / 10.0f;
        particles.push_back(new Particle(x, y, cosf(angle) * speed, sinf(angle) * speed, sz, 30.0f + rand()%20, PARTICLE_BLOOD));
    }
}

void Game::spawnExplosion(float x, float y, int count) {
    for (int i = 0; i < count; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 2.0f + (rand() % 400) / 100.0f;
        float sz = 5.0f + (rand() % 50) / 10.0f;
        particles.push_back(new Particle(x, y, cosf(angle) * speed, sinf(angle) * speed, sz, 20.0f + rand()%15, PARTICLE_EXPLOSION));
    }
}

void Game::spawnSparks(float x, float y, int count) {
    for (int i = 0; i < count; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 3.0f + (rand() % 500) / 100.0f;
        float sz = 1.5f + (rand() % 20) / 10.0f;
        particles.push_back(new Particle(x, y, cosf(angle) * speed, sinf(angle) * speed, sz, 15.0f + rand()%10, PARTICLE_SPARK));
    }
}

void Game::SpawnEnemy(int count, bool isHorde){
    int W=glutGet(GLUT_WINDOW_WIDTH),H=glutGet(GLUT_WINDOW_HEIGHT);
    if(W<100||H<100)return;

    for(int i=0;i<count;i++){
        // Pick zombie type with weighted random
        ZombieType type=ZOMBIE_WALKER;
        if(isHorde) {
            type=ZOMBIE_RUNNER; // Hordes are runners
        } else {
            int roll=rand()%100;
            if(roll<35) type=ZOMBIE_WALKER;
            else if(roll<55) type=ZOMBIE_RUNNER;
            else if(roll<70) type=ZOMBIE_TANK;
            else if(roll<80) type=ZOMBIE_WITCH;
            else if(roll<90) type=ZOMBIE_SPITTER;
            else type=ZOMBIE_LUNGER;
        }

        float spd,hp,dmg,sz;
        switch(type){
        case ZOMBIE_WALKER:  spd=0.9f;hp=80;dmg=12;sz=50;break;
        case ZOMBIE_RUNNER:  spd=2.2f;hp=40;dmg=8;sz=40;break;
        case ZOMBIE_TANK:    spd=0.5f;hp=250;dmg=25;sz=70;break;
        case ZOMBIE_WITCH:   spd=0.7f;hp=60;dmg=35;sz=45;break;
        case ZOMBIE_SPITTER: spd=0.8f;hp=70;dmg=15;sz=50;break;
        case ZOMBIE_LUNGER:  spd=1.4f;hp=55;dmg=20;sz=42;break;
        default:             spd=1.0f;hp=80;dmg=12;sz=50;break;
        }
        if(gameMode==0){spd*=0.7f;hp*=0.8f;dmg*=0.6f;}
        if(gameMode==2){spd*=1.3f;hp*=1.2f;dmg*=1.4f;}
        spd+=score*0.001f; hp+=killCount*0.3f;

        Enemy* e=new Enemy(sz,sz,spd,filename2,hp,dmg,type);
        float ex,ey; int side=rand()%4;
        if(side==0){ex=(float)(rand()%W);ey=-60;}
        else if(side==1){ex=(float)(rand()%W);ey=(float)(H+60);}
        else if(side==2){ex=-60;ey=(float)(rand()%H);}
        else{ex=(float)(W+60);ey=(float)(rand()%H);}
        e->setPositionX(ex);e->setPositionY(ey);
        enemies.push_back(e);
    }
}

void Game::SpawnBoss(){
    int W=glutGet(GLUT_WINDOW_WIDTH); if(W<100)return;
    boss=new Boss((float)(W/2-65),80,filename2,gameMode);
    bossAlive=true;bossMaxHealth=boss->getHealth();waveFlashTimer=120;
}

void Game::updateMovement(bool keys[256]){
    if (paused) return;
    if(keys['w']||keys['W'])player->moveUP();
    if(keys['s']||keys['S'])player->moveDown();
    if(keys['a']||keys['A'])player->moveLeft();
    if(keys['d']||keys['D'])player->moveRight();
    clampPlayer();
}
void Game::clampPlayer(){
    int W=glutGet(GLUT_WINDOW_WIDTH),H=glutGet(GLUT_WINDOW_HEIGHT);
    if(player->getPositionX()<0)player->setPositionX(0);
    if(player->getPositionY()<0)player->setPositionY(0);
    if(player->getPositionX()+player->getWidth()>W)player->setPositionX((float)(W-player->getWidth()));
    if(player->getPositionY()+player->getHeight()>H)player->setPositionY((float)(H-player->getHeight()));
}

void Game::moveEnemy(Enemy* e){
    float dx=player->getPositionX()-e->getPositionX(),dy=player->getPositionY()-e->getPositionY();
    float len=sqrtf(dx*dx+dy*dy); if(len>0.1f){dx/=len;dy/=len;}
    float spd=e->getMoveSpeed();
    // Special abilities
    e->specialTimer++;
    if(e->zombieType==ZOMBIE_WITCH && e->specialCooldown>0 && e->specialTimer>=e->specialCooldown && len<300){
        e->specialActive=true; e->specialTimer=0;
    }
    if(e->zombieType==ZOMBIE_WITCH && e->specialActive && e->specialTimer>40) e->specialActive=false;

    if(e->zombieType==ZOMBIE_LUNGER && e->specialCooldown>0 && e->specialTimer>=e->specialCooldown && len<400 && len>100){
        e->specialActive=true; e->specialTimer=0;
        e->targetX=player->getPositionX(); e->targetY=player->getPositionY();
    }
    if(e->zombieType==ZOMBIE_LUNGER && e->specialActive){
        float ldx=e->targetX-e->getPositionX(), ldy=e->targetY-e->getPositionY();
        float ll=sqrtf(ldx*ldx+ldy*ldy);
        if(ll>10){dx=ldx/ll;dy=ldy/ll;} else e->specialActive=false;
    }

    // Boss is updated separately in Game::timer via boss->update()
    if(dynamic_cast<Boss*>(e)) return;

    e->setPositionX(e->getPositionX()+dx*spd);
    e->setPositionY(e->getPositionY()+dy*spd);
    e->setAngle(atan2f(dy,dx)*180.0f/3.14159f);
}

void Game::onKeyPressed(unsigned char key,int x,int y){
    if(key==27)exit(0);
    if(key=='1') activeWeapon=0; // Pistol
    if(key=='2') activeWeapon=1; // SMG
    if(key=='3') activeWeapon=3; // Shotgun
    if(key=='4') activeWeapon=4; // Sniper
    if(key=='5') activeWeapon=5; // Grenade Launcher
    if(key=='6') activeWeapon=2; // Flamethrower
    if(key=='q' || key=='q') {
        activeWeapon--;
        if(activeWeapon<0) activeWeapon=5;
    }
    if(key=='e' || key=='E') {
        activeWeapon++;
        if(activeWeapon>5) activeWeapon=0;
    }
}
void Game::fireBullet(float mx,float my){
    // Can't fire while reloading or out of ammo
    if(reloading) return;
    if(ammo <= 0) { reloading=true; reloadTimer=0; return; }

    float px=player->getPositionX()+player->getWidth()*0.5f,py=player->getPositionY()+player->getHeight()*0.5f;
    float ang=atan2f(my-py,mx-px);
    
    // Determine critical hit
    bool isCritical = (rand()%100 < 15);  // 15% chance
    if(isCritical) {
        criticalHitCounter++;
        weaponFlashTimer=8;  // Flash for critical
        screenShakeIntensity+=1.0f;
    }
    
    if(activeWeapon==2){  // Flamethrower
        for(int i=-2;i<=2;i++){
            Bullet* b = new Bullet(px,py,ang+i*0.18f,BULLET_FLAME);
            b->setDamage(b->getDamage() * (isCritical ? 2.0f : 1.0f) * criticalMultiplier);
            bullets.push_back(b);
        }
        ammo-=5; director->reportShotFired();
    }
    else if(activeWeapon==1){  // Rapid-fire
        float p1=-sinf(ang)*5,p2=cosf(ang)*5;
        Bullet* b1 = new Bullet(px+p1,py+p2,ang,BULLET_RAPID);
        Bullet* b2 = new Bullet(px-p1,py-p2,ang,BULLET_RAPID);
        b1->setDamage(b1->getDamage() * (isCritical ? 2.0f : 1.0f) * criticalMultiplier);
        b2->setDamage(b2->getDamage() * (isCritical ? 2.0f : 1.0f) * criticalMultiplier);
        bullets.push_back(b1); bullets.push_back(b2);
        ammo-=2; director->reportShotFired();
        spawnSparks(px, py, 3);
    }
    else if(activeWeapon==3){  // Shotgun
        for(int i=-2;i<=2;i++){
            Bullet* b = new Bullet(px,py,ang+i*0.25f,BULLET_SHOTGUN);
            b->setDamage(b->getDamage() * (isCritical ? 2.0f : 1.0f) * criticalMultiplier);
            bullets.push_back(b);
        }
        ammo-=3; director->reportShotFired();
        spawnSparks(px, py, 8);
    }
    else if(activeWeapon==4){  // Sniper
        Bullet* b = new Bullet(px,py,ang,BULLET_SNIPER);
        b->setDamage(b->getDamage() * (isCritical ? 2.0f : 1.0f) * criticalMultiplier);
        bullets.push_back(b);
        ammo-=1; director->reportShotFired();
        spawnSparks(px, py, 5);
    }
    else if(activeWeapon==5){  // Grenade Launcher
        Bullet* b = new Bullet(px,py,ang,BULLET_GRENADE);
        b->setDamage(b->getDamage() * (isCritical ? 2.0f : 1.0f) * criticalMultiplier);
        bullets.push_back(b);
        ammo-=2; director->reportShotFired();
        spawnSparks(px, py, 4);
    }
    else {  // Pistol
        Bullet* b = new Bullet(px,py,ang,BULLET_PISTOL);
        b->setDamage(b->getDamage() * (isCritical ? 2.0f : 1.0f) * criticalMultiplier);
        bullets.push_back(b);
        ammo--; director->reportShotFired();
        spawnSparks(px, py, 2);
    }
    if(ammo<0) ammo=0;
    // Auto-reload when clip empty
    if(ammo<=0){ reloading=true; reloadTimer=0; }
}
void Game::onMouseClicked(int b,int s,int x,int y){
    if(paused) return;
    if(b==GLUT_LEFT_BUTTON){
        if(s==GLUT_DOWN){
            mouseHeld=true;
            mouseX=(float)x; mouseY=(float)y;
            fireBullet((float)x,(float)y);
            fireRateTimer=0;
        } else {
            mouseHeld=false;
        }
    }
}
void Game::onMouseMove(int x,int y){
    float px=player->getPositionX()+player->getWidth()*0.5f,py=player->getPositionY()+player->getHeight()*0.5f;
    player->setAngle(atan2f((float)y-py,(float)x-px)*180.0f/3.14159f);
    mouseX=(float)x; mouseY=(float)y;
}

void Game::togglePause() {
    paused = !paused;
}

bool Game::isPaused() const {
    return paused;
}

void Game::drawPauseOverlay() {
    int W = glutGet(GLUT_WINDOW_WIDTH);
    int H = glutGet(GLUT_WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); glOrtho(0, W, H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.02f, 0.02f, 0.03f, 0.82f);
    fillRect(0, 0, (float)W, (float)H);

    glColor4f(1.0f, 1.0f, 1.0f, 0.95f);
    const char* title = "GAME PAUSED";
    renderTextC((float)(W/2), (float)(H/2) - 20, GLUT_BITMAP_TIMES_ROMAN_24, title);
    const char* hint = "Press P to resume or ESC to return to menu";
    renderTextC((float)(W/2), (float)(H/2) + 20, GLUT_BITMAP_HELVETICA_18, hint);
    const char* tip = "While paused, powerups can still be seen on screen";
    renderTextC((float)(W/2), (float)(H/2) + 45, GLUT_BITMAP_HELVETICA_12, tip);

    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

void Game::drawVictorySummary() {
    int W = glutGet(GLUT_WINDOW_WIDTH);
    int H = glutGet(GLUT_WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); glOrtho(0, W, H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
    fillRect(0, 0, (float)W, (float)H);

    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
    const char* title = "VICTORY!";
    renderTextC((float)(W/2), (float)(H/2) - 60, GLUT_BITMAP_TIMES_ROMAN_24, title);

    char buf[80];
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    sprintf(buf, "Final Score: %d", score);
    renderTextC((float)(W/2), (float)(H/2) - 20, GLUT_BITMAP_HELVETICA_18, buf);

    sprintf(buf, "Enemies Killed: %d", killCount);
    renderTextC((float)(W/2), (float)(H/2) + 10, GLUT_BITMAP_HELVETICA_18, buf);

    const char* hint = "Press ESC to return to menu";
    renderTextC((float)(W/2), (float)(H/2) + 50, GLUT_BITMAP_HELVETICA_12, hint);

    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

void Game::drawMiniMap() {
    int W = glutGet(GLUT_WINDOW_WIDTH);
    int H = glutGet(GLUT_WINDOW_HEIGHT);
    float mapX = W - 220.0f;
    float mapY = H - 220.0f;
    float mapW = 200.0f;
    float mapH = 200.0f;

    // Background
    glColor4f(0.1f, 0.1f, 0.1f, 0.8f);
    fillRect(mapX, mapY, mapW, mapH);
    glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
    outlineRect(mapX, mapY, mapW, mapH);

    // Scale factors
    float scaleX = mapW / W;
    float scaleY = mapH / H;

    // Player (green)
    float px = player->getPositionX() + player->getWidth() * 0.5f;
    float py = player->getPositionY() + player->getHeight() * 0.5f;
    float mx = mapX + px * scaleX;
    float my = mapY + py * scaleY;
    glColor3f(0.0f, 1.0f, 0.0f);
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    glVertex2f(mx, my);
    glEnd();

    // Enemies (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    for (auto e : enemies) {
        float ex = e->getPositionX() + e->getWidth() * 0.5f;
        float ey = e->getPositionY() + e->getHeight() * 0.5f;
        float emx = mapX + ex * scaleX;
        float emy = mapY + ey * scaleY;
        glVertex2f(emx, emy);
    }
    glEnd();

    // Boss (purple)
    if (bossAlive && boss) {
        float bx = boss->getPositionX() + boss->getWidth() * 0.5f;
        float by = boss->getPositionY() + boss->getHeight() * 0.5f;
        float bmx = mapX + bx * scaleX;
        float bmy = mapY + by * scaleY;
        glColor3f(1.0f, 0.0f, 1.0f);
        glPointSize(8.0f);
        glBegin(GL_POINTS);
        glVertex2f(bmx, bmy);
        glEnd();
    }

    // Waypoints (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    // For act 1, fuel cans
    for (auto& wp : campaign->getFuelCans()) {
        if (!wp.collected) {
            float wmx = mapX + wp.x * scaleX;
            float wmy = mapY + wp.y * scaleY;
            glVertex2f(wmx, wmy);
        }
    }
    // For act 3, extraction zone
    if (campaign->getAct() == ACT_ESCAPE) {
        Waypoint ez = campaign->getExtractionZone();
        float wmx = mapX + ez.x * scaleX;
        float wmy = mapY + ez.y * scaleY;
        glVertex2f(wmx, wmy);
    }
    glEnd();

    glPointSize(1.0f);
}

void Game::dropPerk(float x, float y){
    // 30% ammo, 30% health, 15% shield, 9% rapid, 7% flame, 5% speed, 2% critical, 1% damage, 1% slow-mo
    int roll = rand()%100;
    PerkType t;
    if(roll < 30) t = PERK_AMMO;
    else if(roll < 60) t = PERK_HEALTH;
    else if(roll < 75) t = PERK_SHIELD;
    else if(roll < 84) t = PERK_BETTER_GUN;
    else if(roll < 91) t = PERK_FLAMETHROWER;
    else if(roll < 96) t = PERK_SPEED_BOOST;
    else if(roll < 98) t = PERK_CRITICAL_BOOST;
    else if(roll < 99) t = PERK_DAMAGE_BOOST;
    else t = PERK_SLOW_TIME;
    perks.push_back(new Perk(x,y,t));
}
void Game::tryPickupPerks(){
    float px=player->getPositionX(),py=player->getPositionY(),pw=player->getWidth(),ph=player->getHeight();
    for(auto it=perks.begin();it!=perks.end();){
        Perk* p=*it;
        if(px<p->x+60&&px+pw>p->x&&py<p->y+60&&py+ph>p->y){
            float maxHp=(gameMode==0)?140:(gameMode==2)?70:100;
            switch(p->type){
            case PERK_HEALTH:{float h=player->getHealth()+40;if(h>maxHp)h=maxHp;player->setHealth(h);break;}
            case PERK_SHIELD:shieldActive=true;shieldTimer=1500;break;
            case PERK_BETTER_GUN:activeWeapon=1;weaponTimer=2500;break;
            case PERK_FLAMETHROWER:activeWeapon=2;weaponTimer=2500;break;
            case PERK_AMMO:{ammo+=15;if(ammo>maxAmmo)ammo=maxAmmo;reloading=false;break;}
            case PERK_SPEED_BOOST:speedBoostTimer=600;player->setMoveSpeed(7.5f);break;
            case PERK_CRITICAL_BOOST:criticalMultiplier=2.5f;break;
            case PERK_DAMAGE_BOOST:damageBoostTimer=600;break;
            case PERK_SLOW_TIME:slowMotionTimer=300;slowMotionScale=0.5f;break;
            }
            delete p;it=perks.erase(it);
        }else ++it;
    }
}

bool Game::detectCollision(Entity* a,Entity* b){
    if(!a||!b)return false;
    // Circle collision
    float cx_a = a->getPositionX() + a->getWidth() * 0.5f;
    float cy_a = a->getPositionY() + a->getHeight() * 0.5f;
    float cx_b = b->getPositionX() + b->getWidth() * 0.5f;
    float cy_b = b->getPositionY() + b->getHeight() * 0.5f;
    float dx = cx_a - cx_b;
    float dy = cy_a - cy_b;
    float distSq = dx * dx + dy * dy;
    float rad_a = a->getWidth() * 0.45f; // Slightly smaller radius for leniency
    float rad_b = b->getWidth() * 0.45f;
    float radiiSum = rad_a + rad_b;
    return distSq < (radiiSum * radiiSum);
}
void Game::pushBack(Entity* e1,Entity* e2){
    float cx1 = e1->getPositionX() + e1->getWidth() * 0.5f;
    float cy1 = e1->getPositionY() + e1->getHeight() * 0.5f;
    float cx2 = e2->getPositionX() + e2->getWidth() * 0.5f;
    float cy2 = e2->getPositionY() + e2->getHeight() * 0.5f;
    float dx = cx2 - cx1;
    float dy = cy2 - cy1;
    float len = sqrtf(dx * dx + dy * dy);
    if(len < 0.01f){ dx = 1; dy = 0; len = 1; }
    
    // Push e2 away from e1 based on overlap
    float rad1 = e1->getWidth() * 0.45f;
    float rad2 = e2->getWidth() * 0.45f;
    float overlap = (rad1 + rad2) - len;
    if (overlap > 0) {
        e2->setPositionX(e2->getPositionX() + (dx / len) * overlap * 0.5f);
        e2->setPositionY(e2->getPositionY() + (dy / len) * overlap * 0.5f);
        e1->setPositionX(e1->getPositionX() - (dx / len) * overlap * 0.5f);
        e1->setPositionY(e1->getPositionY() - (dy / len) * overlap * 0.5f);
    }
}

void Game::timer(void(*t)(int)){
    bgAnimTimer+=0.025f;
    if(paused){
        glutPostRedisplay();
        glutTimerFunc(10,t,0);
        return;
    }
    if(weaponTimer>0){weaponTimer--;if(!weaponTimer)activeWeapon=0;}
    if(shieldTimer>0){shieldTimer--;if(!shieldTimer)shieldActive=false;}
    if(waveFlashTimer>0)waveFlashTimer--;
    if(damageFlashTimer>0)damageFlashTimer--;
    
    // Update new systems
    if(comboTimer>0){comboTimer--;}else{if(killCombo>0)killCombo--;}
    if(speedBoostTimer>0){speedBoostTimer--;}else{player->setMoveSpeed(5.0f);}
    if(damageBoostTimer>0){damageBoostTimer--;}
    if(slowMotionTimer>0){slowMotionTimer--;}else{slowMotionScale=1.0f;}
    if(hitFlashTimer>0)hitFlashTimer--;
    if(weaponFlashTimer>0)weaponFlashTimer--;
    if(criticalMultiplier>1.0f)criticalMultiplier-=0.02f;if(criticalMultiplier<1.0f)criticalMultiplier=1.0f;

    // Auto-fire while mouse held (affected by slow motion)
    if(mouseHeld && !reloading){
        fireRateTimer++;
        int baseRate = (activeWeapon==1) ? 8 : (activeWeapon==2) ? 12 : (activeWeapon==4) ? 30 : 20;
        int rate = (int)(baseRate * slowMotionScale);
        if(fireRateTimer >= rate){
            fireBullet(mouseX, mouseY);
            fireRateTimer=0;
        }
    }

    // Auto-reload
    if(reloading){
        reloadTimer++;
        if(reloadTimer >= reloadDuration){
            ammo = maxAmmo;
            reloading = false;
            reloadTimer = 0;
        }
    }

    // Campaign Manager drives spawning instead of pure AI Director
    int W=glutGet(GLUT_WINDOW_WIDTH),H=glutGet(GLUT_WINDOW_HEIGHT);

    // Init waypoints on first frame (need screen size)
    if(!campaign->waypointsInitialized()) {
        campaign->initWaypoints(W, H);
    }

    // Campaign update — returns how many enemies to spawn this frame
    if(!campaign->isCampaignWon()) {
        int campaignSpawns = campaign->update(
            player->getPositionX(), player->getPositionY(),
            player->getWidth(), player->getHeight(), W, H);
        if(campaignSpawns > 0) {
            SpawnEnemy(campaignSpawns, campaign->isHoldoutActive());
        }
    } else if(!isVictory) {
        isVictory = true;
        isGameOver = true;
    }

    // Spawn a health pack when the player is low and none are currently available.
    float maxHp=(gameMode==0)?140:(gameMode==2)?70:100;
    if(player->getHealth() < maxHp * 0.35f) {
        bool healthExists=false;
        for(auto p: perks) {
            if(p->type == PERK_HEALTH) { healthExists = true; break; }
        }
        if(!healthExists) {
            int chance = (player->getHealth() < maxHp * 0.20f) ? 1 : 120;
            if(rand()%chance == 0) {
                float hx = player->getPositionX() + (float)(rand()%241 - 120);
                float hy = player->getPositionY() + (float)(rand()%241 - 120);
                if(hx < 20.0f) hx = 20.0f;
                if(hy < 20.0f) hy = 20.0f;
                if(hx > W-80) hx = W-80;
                if(hy > H-80) hy = H-80;
                perks.push_back(new Perk(hx, hy, PERK_HEALTH));
            }
        }
    }

    // AI Director supplements campaign spawning during non-holdout
    if(!bossAlive && !campaign->isHoldoutActive() && campaign->getAct() != ACT_VICTORY) {
        director->update(this, player, enemies);
    }

    for(auto it=perks.begin();it!=perks.end();){(*it)->update();if((*it)->isExpired()){delete *it;it=perks.erase(it);}else ++it;}
    
    for(auto it=particles.begin();it!=particles.end();){
        (*it)->update();
        if((*it)->isDead()){delete *it;it=particles.erase(it);}
        else ++it;
    }

    for(auto it=bullets.begin();it!=bullets.end();){
        (*it)->moveBullet();float bx=(*it)->getPositionX(),by=(*it)->getPositionY();
        if(bx<-50||bx>W+50||by<-50||by>H+50){delete *it;it=bullets.erase(it);}else ++it;
    }

    // Boss spawns during holdout at a certain kill count, but not in victory
    if(killCount>=nextBossKillThreshold&&!bossAlive&&campaign->getAct()!=ACT_VICTORY){
        SpawnBoss();
        nextBossKillThreshold=killCount+25;
    }

    if(bossAlive&&boss){
        int W2=glutGet(GLUT_WINDOW_WIDTH),H2=glutGet(GLUT_WINDOW_HEIGHT);
        boss->update(player->getPositionX()+player->getWidth()*0.5f,
                     player->getPositionY()+player->getHeight()*0.5f, W2, H2);

        // Pull screen shake from boss
        float bossShake = boss->getShakeIntensity();
        if(bossShake > screenShakeIntensity) screenShakeIntensity = bossShake;
        boss->resetShake();

        // Move boss (unless charge moves it internally)
        float bspd = boss->getEffectiveSpeed();
        if(bspd > 0.01f) {
            float bdx,bdy;
            boss->getMovementDirection(player->getPositionX()+player->getWidth()*0.5f,
                                       player->getPositionY()+player->getHeight()*0.5f, bdx, bdy);
            boss->setPositionX(boss->getPositionX()+bdx*bspd);
            boss->setPositionY(boss->getPositionY()+bdy*bspd);
            boss->setAngle(atan2f(bdy,bdx)*180.0f/3.14159f);
        }

        // Bullet hits on boss
        for(auto bIt=bullets.begin();bIt!=bullets.end();){
            if(detectCollision(boss,*bIt)){
                float dmg = (*bIt)->getDamage();
                boss->setHealth(boss->getHealth()-dmg);
                boss->onDamaged(dmg);
                spawnBlood((*bIt)->getPositionX(), (*bIt)->getPositionY(), 3);
                if((*bIt)->bulletType == BULLET_GRENADE) {
                    spawnExplosion((*bIt)->getPositionX(), (*bIt)->getPositionY(), 10);
                }
                delete *bIt;bIt=bullets.erase(bIt);
                if(boss->getHealth()<=0){
                    dropPerk(boss->getPositionX(),boss->getPositionY());
                    dropPerk(boss->getPositionX()+30,boss->getPositionY());
                    dropPerk(boss->getPositionX()-30,boss->getPositionY()+20);
                    killCombo+=10;  // Bonus combo for boss
                    comboTimer=180;
                    score+=500;killCount++;screenShakeIntensity=20;
                    delete boss;boss=nullptr;bossAlive=false;waveFlashTimer=180;
                    break;
                }
            }else ++bIt;
        }

        // Boss melee damage on player (knockback on player only, boss is immune)
        if(bossAlive&&boss&&detectCollision(player,boss)){
            if(shieldActive){shieldActive=false;shieldTimer=0;}
            else{
                float dmgMult = (boss->getAction()==BOSS_ACTION_CHARGE) ? 0.15f : 0.06f;
                player->setHealth(player->getHealth()-boss->getDamage()*dmgMult);
                damageFlashTimer=12;
                screenShakeIntensity=8;
                // Push ONLY the player (boss is knockback immune)
                float pdx=player->getPositionX()-boss->getPositionX();
                float pdy=player->getPositionY()-boss->getPositionY();
                float plen=sqrtf(pdx*pdx+pdy*pdy);
                if(plen<0.01f){pdx=1;pdy=0;plen=1;}
                float pushDist = (boss->getAction()==BOSS_ACTION_CHARGE) ? 80.0f : 40.0f;
                player->setPositionX(player->getPositionX()+(pdx/plen)*pushDist);
                player->setPositionY(player->getPositionY()+(pdy/plen)*pushDist);
                clampPlayer();
                if(player->getHealth()<=0)isGameOver=true;
            }
        }

        // Boss projectile collision with player
        handleBossProjectiles();

        // Phase 3: Swarm spawn
        if(bossAlive&&boss&&boss->needsSwarmSpawn()){
            SpawnEnemy(boss->getSwarmCount(), true);
            boss->markSwarmSpawned();
            waveFlashTimer=120;
        }
    }

    for(auto eIt=enemies.begin();eIt!=enemies.end();){
        moveEnemy(*eIt);bool died=false;
        for(auto bIt=bullets.begin();bIt!=bullets.end();){
            if(detectCollision(*eIt,*bIt)){
                (*eIt)->setHealth((*eIt)->getHealth()-(*bIt)->getDamage());
                spawnBlood((*bIt)->getPositionX(), (*bIt)->getPositionY(), 3);
                if((*bIt)->bulletType == BULLET_GRENADE) {
                    float bx = (*bIt)->getPositionX(), by = (*bIt)->getPositionY();
                    spawnExplosion(bx, by, 10);
                    for(auto other = enemies.begin(); other != enemies.end(); ++other) {
                        if (*other != *eIt) {
                            float dx = (*other)->getPositionX() - bx;
                            float dy = (*other)->getPositionY() - by;
                            if (dx*dx + dy*dy < 150*150) {
                                (*other)->setHealth((*other)->getHealth() - (*bIt)->getDamage() * 0.7f);
                            }
                        }
                    }
                }
                delete *bIt;bIt=bullets.erase(bIt);
                if((*eIt)->getHealth()<=0){
                    float ex=(*eIt)->getPositionX(),ey=(*eIt)->getPositionY();
                    spawnBlood(ex, ey, 15);
                    int pts=10;
                    if((*eIt)->zombieType==ZOMBIE_TANK)pts=30;
                    if((*eIt)->zombieType==ZOMBIE_WITCH)pts=25;
                    if((*eIt)->zombieType==ZOMBIE_LUNGER)pts=20;
                    
                    // Combo system
                    killCombo++;
                    comboTimer=120;  // 2 second window
                    if(killCombo>1)comboMultiplier=1.0f+killCombo*0.1f;
                    if(killCombo>10)screenShakeIntensity+=2.0f;  // Extra shake on high combo
                    pts=(int)(pts*comboMultiplier);
                    hitFlashTimer=4;
                    
                    delete *eIt;eIt=enemies.erase(eIt);died=true;score+=pts;killCount++;perkDropCounter++;
                    if(perkDropCounter>=3){dropPerk(ex,ey);perkDropCounter=0;} // drop every 3 kills
                    break;
                }
            }else ++bIt;
        }
        if(!died){
            if(detectCollision(player,*eIt)){
                if(shieldActive){shieldActive=false;shieldTimer=0;}
                else{player->setHealth(player->getHealth()-(*eIt)->getDamage()*0.03f);damageFlashTimer=6;pushBack(*eIt,player);if(player->getHealth()<=0)isGameOver=true;}
            }
            
            // Check collision against other enemies (flocking separation)
            for(auto otherIt = enemies.begin(); otherIt != enemies.end(); ++otherIt) {
                if (*eIt != *otherIt && detectCollision(*eIt, *otherIt)) {
                    pushBack(*otherIt, *eIt);
                }
            }
            
            ++eIt;
        }
    }
    tryPickupPerks();
    updateScreenShake();
    glutPostRedisplay();glutTimerFunc(10,t,0);
}

void Game::updateScreenShake() {
    if (screenShakeIntensity > 0.1f) {
        screenShakeX = ((rand()%100)/50.0f - 1.0f) * screenShakeIntensity;
        screenShakeY = ((rand()%100)/50.0f - 1.0f) * screenShakeIntensity;
        screenShakeIntensity *= 0.9f;
    } else {
        screenShakeX = 0;
        screenShakeY = 0;
        screenShakeIntensity = 0;
    }
}

void Game::handleBossProjectiles() {
    if (!boss || !bossAlive) return;
    auto& projs = boss->getProjectiles();
    float px = player->getPositionX() + player->getWidth()*0.5f;
    float py = player->getPositionY() + player->getHeight()*0.5f;
    float prad = player->getWidth() * 0.4f;

    for (auto& p : projs) {
        if (!p.active) continue;
        float dx = px - p.x;
        float dy = py - p.y;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist < prad + p.radius) {
            // Hit!
            if (shieldActive) { shieldActive = false; shieldTimer = 0; }
            else {
                player->setHealth(player->getHealth() - p.damage);
                damageFlashTimer = 12;
                screenShakeIntensity = 6;
                if (player->getHealth() <= 0) isGameOver=true;
            }
            p.active = false;
        }
    }
}

// Drawing
void Game::drawBackground(){
    int W=glutGet(GLUT_WINDOW_WIDTH),H=glutGet(GLUT_WINDOW_HEIGHT);
    if(damageFlashTimer>0)glColor3f(0.35f,0.0f,0.0f);else glColor3f(0.07f,0.09f,0.12f);
    fillRect(0,0,(float)W,(float)H);
    glColor3f(0.12f,0.15f,0.19f);glLineWidth(1.0f);glBegin(GL_LINES);
    for(int gx=0;gx<=W;gx+=64){glVertex2f((float)gx,0);glVertex2f((float)gx,(float)H);}
    for(int gy=0;gy<=H;gy+=64){glVertex2f(0,(float)gy);glVertex2f((float)W,(float)gy);}
    glEnd();
    srand(7777);glColor3f(0.22f,0.05f,0.05f);glPointSize(5.0f);glBegin(GL_POINTS);
    for(int i=0;i<80;i++)glVertex2f((float)(rand()%W),(float)(rand()%H));
    glEnd();glPointSize(1.0f);srand((unsigned)time(nullptr));
}

void Game::drawEnemyHealthBars(){
    for(auto e:enemies){
        float ex=e->getPositionX(),ey=e->getPositionY(),ew=e->getWidth();
        float pct=e->getHealth()/e->getMaxHealth();if(pct>1)pct=1;if(pct<0)pct=0;
        float bw=ew,bh=4,bx=ex,by=ey-8;
        glColor3f(0.3f,0.0f,0.0f);fillRect(bx,by,bw,bh);
        float r=(pct<0.5f)?1:2*(1-pct),g=(pct>0.5f)?1:2*pct;
        glColor3f(r,g,0);fillRect(bx,by,bw*pct,bh);
        glColor3f(0.5f,0.5f,0.5f);outlineRect(bx,by,bw,bh);
        // Type label
        const char* lbl="";
        switch(e->zombieType){case ZOMBIE_RUNNER:lbl="RUNNER";break;case ZOMBIE_TANK:lbl="TANK";break;
        case ZOMBIE_WITCH:lbl="WITCH";break;case ZOMBIE_SPITTER:lbl="SPITTER";break;case ZOMBIE_LUNGER:lbl="LUNGER";break;default:break;}
        if(lbl[0]){glColor3f(0.7f,0.7f,0.7f);glRasterPos2f(ex,ey-12);for(const char*c=lbl;*c;c++)glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10,(int)*c);}
    }
}

void Game::drawHUD(){
    int W=glutGet(GLUT_WINDOW_WIDTH),H=glutGet(GLUT_WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();glOrtho(0,W,H,0,-1,1);
    glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();
    char buf[80];
    
    // Modern L4D2 Style HUD at the bottom left
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Sleek translucent background panel
    glColor4f(0.05f,0.05f,0.05f,0.7f);
    fillRect(20, H - 120, 320, 100);
    
    // Subtle border
    glColor4f(0.4f,0.4f,0.4f,0.5f);
    glLineWidth(2.0f);
    outlineRect(20, H - 120, 320, 100);
    glLineWidth(1.0f);

    // Score & Kills
    glColor4f(0.9f,0.9f,0.9f, 1.0f);
    sprintf(buf,"SCORE: %d",score);
    renderText(35, H - 95, GLUT_BITMAP_HELVETICA_18, buf);
    
    glColor4f(0.6f,0.6f,0.6f, 1.0f);
    sprintf(buf,"KILLS: %d  |  ALIVE: %d",killCount,(int)enemies.size()+(bossAlive?1:0));
    renderText(35, H - 75, GLUT_BITMAP_HELVETICA_12, buf);
    
    // Modern Healthbar
    float maxHp=(gameMode==0)?140:(gameMode==2)?70:100;
    float hpPct=player->getHealth()/maxHp;
    hpPct=hpPct<0?0:(hpPct>1?1:hpPct);
    float bw=280;
    
    glColor4f(0.2f,0.0f,0.0f, 0.8f); // Dark red background
    fillRect(35, H - 60, bw, 18);
    
    // Dynamic color (Green -> Yellow -> Red)
    float rColor = (hpPct > 0.5f) ? (1.0f - (hpPct - 0.5f) * 2.0f) : 1.0f;
    float gColor = (hpPct > 0.5f) ? 1.0f : (hpPct * 2.0f);
    float bColor = 0.1f;
    
    glColor4f(rColor, gColor, bColor, 0.9f);
    fillRect(35, H - 60, bw * hpPct, 18);
    
    // Glossy overlay for healthbar
    glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
    fillRect(35, H - 60, bw * hpPct, 9);
    
    glColor4f(0.8f,0.8f,0.8f, 1.0f);
    outlineRect(35, H - 60, bw, 18);
    
    // Weapon info
    const char*wn="PISTOL";float wr=0.8f,wg=0.8f,wb=0.8f;
    if(activeWeapon==1){wn="SMG (RAPID)";wr=0.4f;wg=0.8f;wb=1.0f;}
    if(activeWeapon==2){wn="FLAMETHROWER";wr=1.0f;wg=0.5f;wb=0.1f;}
    glColor4f(wr,wg,wb, 1.0f);
    sprintf(buf,"WEAPON: %s",wn);
    renderText(35, H - 30, GLUT_BITMAP_HELVETICA_12, buf);

    // Ammo counter (bottom right)
    glColor4f(0.05f,0.05f,0.05f,0.7f);
    fillRect(W - 180, H - 80, 160, 60);
    glColor4f(0.4f,0.4f,0.4f,0.5f);
    outlineRect(W - 180, H - 80, 160, 60);

    if(reloading){
        // Flashing RELOAD
        float flash = 0.5f + 0.5f * sinf(bgAnimTimer * 8.0f);
        glColor4f(1.0f, 0.2f, 0.2f, flash);
        renderText(W - 160, H - 55, GLUT_BITMAP_HELVETICA_18, "RELOADING...");
        // Reload progress bar
        float reloadPct = (float)reloadTimer / (float)reloadDuration;
        glColor4f(0.3f,0.3f,0.3f,0.8f);
        fillRect(W - 170, H - 38, 140, 10);
        glColor4f(1.0f, 0.6f, 0.1f, 0.9f);
        fillRect(W - 170, H - 38, 140 * reloadPct, 10);
    } else {
        // Ammo count
        float ammoPct = (float)ammo / (float)maxAmmo;
        if(ammoPct < 0.25f)
            glColor4f(1.0f, 0.2f, 0.2f, 1.0f); // Red when low
        else if(ammoPct < 0.5f)
            glColor4f(1.0f, 0.8f, 0.2f, 1.0f); // Yellow
        else
            glColor4f(0.9f, 0.9f, 0.9f, 1.0f); // White
        sprintf(buf, "%d / %d", ammo, maxAmmo);
        renderText(W - 155, H - 55, GLUT_BITMAP_HELVETICA_18, buf);
        glColor4f(0.6f,0.6f,0.6f,0.8f);
        renderText(W - 155, H - 35, GLUT_BITMAP_HELVETICA_10, "AMMO");
    }

    if(shieldActive){
        glColor4f(0.2f,0.6f,1.0f, 1.0f);
        renderText(180, H - 30, GLUT_BITMAP_HELVETICA_12,"SHIELD ACTIVE");
    }

    // Stress Level Indicator (AI Director)
    float stress = director->getStressLevel();
    glColor4f(0.8f, 0.2f, 0.2f, 0.8f);
    sprintf(buf,"DIRECTOR STRESS: %d%%", (int)(stress * 100));
    renderText(20, 25, GLUT_BITMAP_HELVETICA_10, buf);

    // Wave/Boss flash
    if(waveFlashTimer>0){
        float a=(float)waveFlashTimer/120.0f;
        if (bossAlive && boss) {
            // Show phase-specific flash text
            const char* flashMsg = "!! BOSS INCOMING !!";
            if (boss->getPhase() == BOSS_PHASE_ENRAGE) flashMsg = "!! THE TANK IS ENRAGED !!";
            else if (boss->getPhase() == BOSS_PHASE_DESPERATION) flashMsg = "!! SWARM INCOMING !!";
            glColor4f(1.0f, 0.2f, 0.2f, a);
            renderTextC((float)(W/2),(float)(H/2)-20,GLUT_BITMAP_TIMES_ROMAN_24, flashMsg);
        } else {
            glColor4f(0.3f, 1.0f, 0.3f, a);
            renderTextC((float)(W/2),(float)(H/2)-20,GLUT_BITMAP_TIMES_ROMAN_24, "BOSS DEFEATED!");
        }
    }

    // Boss HP bar (top center, L4D2 style)
    if(bossAlive&&boss){
        float bHp=boss->getHealth()/bossMaxHealth;if(bHp<0)bHp=0;
        float bBarW=450,bBarH=28,bBarX=(W-bBarW)/2.0f,bBarY=15;

        // Background
        glColor4f(0.03f,0.01f,0.01f, 0.85f);
        fillRect(bBarX-4, bBarY-4, bBarW+8, bBarH+8);

        // Dark inner
        glColor4f(0.15f,0.0f,0.0f, 0.9f);
        fillRect(bBarX, bBarY, bBarW, bBarH);

        // HP fill — color depends on phase
        if (boss->getPhase() == BOSS_PHASE_DESPERATION)
            glColor4f(0.7f, 0.0f, 0.7f, 0.9f);  // Purple
        else if (boss->getPhase() == BOSS_PHASE_ENRAGE)
            glColor4f(1.0f, 0.35f, 0.0f, 0.9f);  // Orange
        else
            glColor4f(0.8f, 0.1f, 0.1f, 0.9f);   // Red

        fillRect(bBarX, bBarY, bBarW*bHp, bBarH);

        // Glossy highlight
        glColor4f(1.0f,1.0f,1.0f, 0.15f);
        fillRect(bBarX, bBarY, bBarW*bHp, bBarH/2);

        // Border
        glColor4f(0.8f,0.3f,0.3f, 0.8f);
        glLineWidth(2.0f);
        outlineRect(bBarX, bBarY, bBarW, bBarH);
        glLineWidth(1.0f);

        // Phase markers (vertical lines at 50% and 15%)
        glColor4f(1.0f,1.0f,0.0f, 0.6f);
        float mark50 = bBarX + bBarW * 0.50f;
        float mark15 = bBarX + bBarW * 0.15f;
        glBegin(GL_LINES);
        glVertex2f(mark50, bBarY); glVertex2f(mark50, bBarY+bBarH);
        glVertex2f(mark15, bBarY); glVertex2f(mark15, bBarY+bBarH);
        glEnd();

        // Label
        const char* phaseLabel = "THE TANK";
        if (boss->getPhase() == BOSS_PHASE_ENRAGE) phaseLabel = "THE TANK - ENRAGED";
        else if (boss->getPhase() == BOSS_PHASE_DESPERATION) phaseLabel = "THE TANK - DESPERATE";
        glColor4f(1.0f,0.9f,0.8f, 1.0f);
        renderTextC(bBarX+bBarW/2, bBarY+bBarH-8, GLUT_BITMAP_HELVETICA_12, phaseLabel);
    }
    
    // COMBO SYSTEM HUD (top left, large and prominent)
    if(killCombo>0){
        // Combo background panel
        float comboX=40,comboY=60;
        glColor4f(0.1f,0.1f,0.1f,0.8f);
        fillRect(comboX-5, comboY-5, 200, 80);
        
        // Combo color (gets more intense with higher combo)
        float comboR=0.2f+(killCombo/20.0f)*0.8f;
        float comboG=0.8f-(killCombo/20.0f)*0.3f;
        float comboB=0.2f;
        glColor4f(0.8f,0.8f,0.8f,0.6f);
        outlineRect(comboX-5, comboY-5, 200, 80);
        
        // Combo counter large
        sprintf(buf,"COMBO x%d",killCombo);
        glColor4f(comboR,comboG,comboB,1.0f);
        renderText(comboX, comboY, GLUT_BITMAP_TIMES_ROMAN_24, buf);
        
        // Multiplier display
        sprintf(buf,"x%.1f Damage",comboMultiplier);
        glColor4f(comboR*0.8f,comboG*0.8f,comboB*0.8f,0.9f);
        renderText(comboX, comboY+25, GLUT_BITMAP_HELVETICA_12, buf);
        
        // Combo timer bar
        float comboTimerPct=(float)comboTimer/120.0f;
        glColor4f(0.3f,0.3f,0.3f,0.7f);
        fillRect(comboX, comboY+45, 180, 10);
        glColor4f(comboR,comboG,comboB,0.8f);
        fillRect(comboX, comboY+45, 180*comboTimerPct, 10);
        glColor4f(0.6f,0.6f,0.6f,0.6f);
        outlineRect(comboX, comboY+45, 180, 10);
    }
    
    // ACTIVE POWERUPS (right side HUD)
    int powerupX=W-220,powerupY=100;
    if(speedBoostTimer>0){
        glColor4f(0.2f,0.8f,0.2f,0.8f);
        fillRect(powerupX-5, powerupY-5, 210, 35);
        glColor4f(0.8f,0.8f,0.8f,0.6f);
        outlineRect(powerupX-5, powerupY-5, 210, 35);
        glColor4f(0.2f,0.8f,0.2f,1.0f);
        sprintf(buf,"SPEED BOOST +50%%");
        renderText(powerupX, powerupY, GLUT_BITMAP_HELVETICA_12, buf);
        float sbPct=(float)speedBoostTimer/600.0f;
        glColor4f(0.3f,0.3f,0.3f,0.6f);
        fillRect(powerupX, powerupY+18, 190, 8);
        glColor4f(0.2f,0.8f,0.2f,0.8f);
        fillRect(powerupX, powerupY+18, 190*sbPct, 8);
        powerupY+=45;
    }
    
    if(criticalMultiplier>1.0f){
        glColor4f(1.0f,0.8f,0.2f,0.8f);
        fillRect(powerupX-5, powerupY-5, 210, 35);
        glColor4f(0.8f,0.8f,0.8f,0.6f);
        outlineRect(powerupX-5, powerupY-5, 210, 35);
        glColor4f(1.0f,0.8f,0.2f,1.0f);
        sprintf(buf,"CRITICAL x%.1f",criticalMultiplier);
        renderText(powerupX, powerupY, GLUT_BITMAP_HELVETICA_12, buf);
        powerupY+=45;
    }
    
    if(damageBoostTimer>0){
        glColor4f(1.0f,0.2f,0.2f,0.8f);
        fillRect(powerupX-5, powerupY-5, 210, 35);
        glColor4f(0.8f,0.8f,0.8f,0.6f);
        outlineRect(powerupX-5, powerupY-5, 210, 35);
        glColor4f(1.0f,0.2f,0.2f,1.0f);
        sprintf(buf,"DAMAGE BOOST +50%%");
        renderText(powerupX, powerupY, GLUT_BITMAP_HELVETICA_12, buf);
        float dbPct=(float)damageBoostTimer/600.0f;
        glColor4f(0.3f,0.3f,0.3f,0.6f);
        fillRect(powerupX, powerupY+18, 190, 8);
        glColor4f(1.0f,0.2f,0.2f,0.8f);
        fillRect(powerupX, powerupY+18, 190*dbPct, 8);
        powerupY+=45;
    }
    
    if(slowMotionTimer>0){
        glColor4f(0.2f,0.6f,1.0f,0.8f);
        fillRect(powerupX-5, powerupY-5, 210, 35);
        glColor4f(0.8f,0.8f,0.8f,0.6f);
        outlineRect(powerupX-5, powerupY-5, 210, 35);
        glColor4f(0.2f,0.6f,1.0f,1.0f);
        sprintf(buf,"SLOW MOTION x0.5");
        renderText(powerupX, powerupY, GLUT_BITMAP_HELVETICA_12, buf);
        float smPct=(float)slowMotionTimer/300.0f;
        glColor4f(0.3f,0.3f,0.3f,0.6f);
        fillRect(powerupX, powerupY+18, 190, 8);
        glColor4f(0.2f,0.6f,1.0f,0.8f);
        fillRect(powerupX, powerupY+18, 190*smPct, 8);
        powerupY+=45;
    }
    
    // Weapon type display (updated for new weapons)
    glColor4f(0.05f,0.05f,0.05f,0.7f);
    fillRect(W-200, H-180, 180, 50);
    glColor4f(0.4f,0.4f,0.4f,0.5f);
    outlineRect(W-200, H-180, 180, 50);
    
    const char*wt="PISTOL";float wtR=0.8f,wtG=0.8f,wtB=0.8f;
    if(activeWeapon==1){wt="SMG";wtR=0.4f;wtG=0.8f;wtB=1.0f;}
    else if(activeWeapon==2){wt="FLAMETHROWER";wtR=1.0f;wtG=0.5f;wtB=0.1f;}
    else if(activeWeapon==3){wt="SHOTGUN";wtR=1.0f;wtG=0.6f;wtB=0.2f;}
    else if(activeWeapon==4){wt="SNIPER";wtR=0.7f;wtG=0.7f;wtB=0.8f;}
    else if(activeWeapon==5){wt="GRENADE";wtR=0.8f;wtG=1.0f;wtB=0.2f;}
    
    glColor4f(wtR,wtG,wtB,1.0f);
    renderText(W-190, H-165, GLUT_BITMAP_HELVETICA_12, wt);
    sprintf(buf,"[Q/E to switch]");
    glColor4f(0.6f,0.6f,0.6f,0.8f);
    renderText(W-190, H-145, GLUT_BITMAP_HELVETICA_10, buf);
    glColor4f(0.7f,0.7f,0.7f,0.8f);
    renderText(20, 40, GLUT_BITMAP_HELVETICA_10, "P = Pause / Resume | Pick up HEALTH PACKS & POWERUPS");
    
    // Damage vignette
    if(damageFlashTimer>0){
        float a=(float)damageFlashTimer/10.0f * 0.5f;
        glColor4f(1.0f, 0.0f, 0.0f, a);
        fillRect(0,0,W,H); // Red flash over screen
    }

    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);glPopMatrix();glMatrixMode(GL_MODELVIEW);glPopMatrix();
}

void Game::draw(){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glPushAttrib(GL_ALL_ATTRIB_BITS);  // Save all GL state
    
    int W=glutGet(GLUT_WINDOW_WIDTH),H=glutGet(GLUT_WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();glOrtho(0,W,H,0,-1,1);
    glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();
    
    // Apply screen shake offset
    if (screenShakeIntensity > 0.1f) {
        glTranslatef(screenShakeX, screenShakeY, 0);
    }
    
    drawBackground();
    
    // Draw campaign waypoints (in world space, affected by screen shake)
    campaign->drawWaypoints();
    
    for(auto p:perks)p->draw();
    for(auto e:enemies)e->draw();
    if(bossAlive&&boss)boss->draw();
    drawEnemyHealthBars();
    for(auto b:bullets)b->draw();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for(auto pt:particles)pt->draw();
    glDisable(GL_BLEND);
    player->draw();
    
    // Dynamic Flashlight Overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float px = player->getPositionX() + player->getWidth() * 0.5f;
    float py = player->getPositionY() + player->getHeight() * 0.5f;
    
    int numSegments = 72;
    float outerRadius = 3000.0f;
    float fov = 65.0f;
    float pAngle = player->getAngle();

    while(pAngle < 0) pAngle += 360.0f;
    while(pAngle >= 360) pAngle -= 360.0f;
    
    auto getLightRadius = [&](float a) {
        float diff = fmod(abs(a - pAngle), 360.0f);
        if (diff > 180.0f) diff = 360.0f - diff;
        
        if (diff < fov / 2.0f) {
            return 700.0f; 
        } else if (diff < fov / 2.0f + 20.0f) {
            float t = (diff - fov / 2.0f) / 20.0f;
            return 700.0f * (1.0f - t) + 120.0f * t;
        } else {
            return 120.0f;
        }
    };

    // 1. Draw outer solid darkness
    glBegin(GL_QUADS);
    for (int i = 0; i < numSegments; ++i) {
        float angle1 = (i * 360.0f / numSegments);
        float angle2 = ((i + 1) * 360.0f / numSegments);
        
        float r1 = getLightRadius(angle1);
        float r2 = getLightRadius(angle2);
        
        float a1_rad = angle1 * 3.14159f / 180.0f;
        float a2_rad = angle2 * 3.14159f / 180.0f;

        glColor4f(0.01f, 0.01f, 0.02f, 0.96f);
        glVertex2f(px + r1 * cosf(a1_rad), py + r1 * sinf(a1_rad));
        glVertex2f(px + r2 * cosf(a2_rad), py + r2 * sinf(a2_rad));
        glVertex2f(px + outerRadius * cosf(a2_rad), py + outerRadius * sinf(a2_rad));
        glVertex2f(px + outerRadius * cosf(a1_rad), py + outerRadius * sinf(a1_rad));
    }
    glEnd();

    // 2. Draw soft inner gradient (light to dark)
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < numSegments; ++i) {
        float angle1 = (i * 360.0f / numSegments);
        float angle2 = ((i + 1) * 360.0f / numSegments);
        
        float r1 = getLightRadius(angle1);
        float r2 = getLightRadius(angle2);
        
        float a1_rad = angle1 * 3.14159f / 180.0f;
        float a2_rad = angle2 * 3.14159f / 180.0f;

        // Center is fully transparent
        glColor4f(0.01f, 0.01f, 0.02f, 0.0f);
        glVertex2f(px, py);
        
        // Edge is fully dark
        glColor4f(0.01f, 0.01f, 0.02f, 0.96f);
        glVertex2f(px + r1 * cosf(a1_rad), py + r1 * sinf(a1_rad));
        glVertex2f(px + r2 * cosf(a2_rad), py + r2 * sinf(a2_rad));
    }
    glEnd();

    glDisable(GL_BLEND);
    
    glMatrixMode(GL_PROJECTION);glPopMatrix();
    glMatrixMode(GL_MODELVIEW);glPopMatrix();

    drawHUD();
    drawMiniMap();
    if(paused) {
        drawPauseOverlay();
    }
    if(isGameOver && isVictory) {
        drawVictorySummary();
    }

    // Draw campaign objectives & dialog (HUD layer, not affected by flashlight)
    {
        int cW=glutGet(GLUT_WINDOW_WIDTH),cH=glutGet(GLUT_WINDOW_HEIGHT);
        glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();glOrtho(0,cW,cH,0,-1,1);
        glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();
        campaign->drawObjectives(cW, cH);
        campaign->drawDialog(cW, cH);
        glMatrixMode(GL_PROJECTION);glPopMatrix();glMatrixMode(GL_MODELVIEW);glPopMatrix();
    }

    glPopAttrib();  // Restore all GL state
    glutSwapBuffers();
}