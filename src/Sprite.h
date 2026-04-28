#pragma once

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#else
#include <GL/freeglut.h>
#endif
#include "IDrawable.h"
#include "RgbImage.h"

class Sprite: IDrawable {
private:
	float x, y;
	float width, height;
	float origoX, origoY;
	float angle;
	GLuint textureName;
	char* filename;
	void loadTextureFromFile(char*);
	void initTexture(char*);

public:
	Sprite(float, float, float, float, char*);
	Sprite(float, float, char*);
	~Sprite();
	void draw();
	void setAngle(float);
	float getAngle();
};

