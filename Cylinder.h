#pragma once
#include "glm/glm.hpp"
#include "shaders/Shader.h"
#include "GL/glew.h"
class Cylinder
{
private:
	glm::vec3 pos;
	static const int precision = 6; // the number of points comprising the base/circle
	float radius;
	float height;
	glm::vec3 coords[precision * 2];
public:
	Cylinder(float r, float h, glm::vec3 p);
	Cylinder(float r, float h);
	Cylinder();
	void setPos(glm::vec3 newPos);
	glm::vec3 getPos();
	glm::vec3* getCoords();
};

