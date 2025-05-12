#include "Cylinder.h"
#include "Structures/Vector3d.h"

Cylinder::Cylinder(float r, float h, glm::vec3 p) {
	radius = r;
	height = h;
	for (int i = 0; i < precision;i++) {
		float angle = 2 * PI * i / precision;
		coords[0] = glm::vec3(r * cos(i), 0, r * sin(i));
		coords[i + precision] = glm::vec3(r*cos(i), h, r*sin(i));
	}
	pos = p;
}
Cylinder::Cylinder(float r, float h) {
	Cylinder::Cylinder(r, h, glm::vec3(0));
}
Cylinder::Cylinder() {
	Cylinder::Cylinder(10, 10);
}
void Cylinder::setPos(glm::vec3 newPos) {
	pos = newPos;
}
glm::vec3 Cylinder::getPos() {
	return pos;
}
glm::vec3* Cylinder::getCoords() {
	glm::vec3 * coords_out=(glm::vec3*) malloc(sizeof(glm::vec3) * precision * 2);
	for (int i = 0; i < precision * 2; i++) {
		coords_out[i] = coords[i] + pos;
	}
	return coords_out;
}
