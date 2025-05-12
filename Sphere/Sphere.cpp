#include "Sphere.h"
#include <fstream>

#include "../GL/glew.h"
#include "../GL/freeglut.h"
#include "../glm/ext/scalar_constants.hpp"

Sphere::Sphere()
{
	stacks = 10; //number of segments
	slices = 10; //number of segments
	numVerts = (stacks + 1) * (slices + 1);
	radius = 6.0f;
	Position = glm::vec3(0);

	coords = (glm::vec4*) malloc(sizeof(glm::vec4) * numVerts); 
	normals = (glm::vec3*) malloc(sizeof(glm::vec3) * numVerts); 
	sphereIndices = (unsigned int*)malloc(sizeof(unsigned int) * 660);
	CreateSpherewithNormal();
}

Sphere::~Sphere()
{
	free(coords);
	free(normals);
	free(sphereIndices);
}

void Sphere::CreateSpherewithNormal(void)
{
	int count;
	count = 0;
	for (int i = 0; i <= stacks; ++i) {

		GLfloat V = i / (float)stacks;
		GLfloat phi = V * glm::pi <float>();

		// Loop Through Slices
		for (int j = 0; j <= slices; ++j) {

			GLfloat U = j / (float)slices;
			GLfloat theta = U * (glm::pi <float>() * 2);

			// Calc The Vertex Positions
			GLfloat x = cosf(theta) * sinf(phi);
			GLfloat y = cosf(phi);
			GLfloat z = sinf(theta) * sinf(phi);

			coords[count] = glm::vec4(x * radius, y * radius + 6.0, z * radius, 1.0);
			normals[count] = glm::vec3(x, y, z); ///Sphere normals

			count++;
		}
	}

	count = 0;
	// Calc The Index Positions
	for (int i = 0; i < slices * stacks + slices; ++i) {

		sphereIndices[count] = i;
		count++;
		sphereIndices[count] = i + slices + 1;
		count++;
		sphereIndices[count] = i + slices;
		count++;

		sphereIndices[count] = i + slices + 1;
		count++;
		sphereIndices[count] = i;
		count++;
		sphereIndices[count] = i + 1;
		count++;
	}

	count = 0;
}

glm::vec4* Sphere::GetCoords(int& verNum)
{
	verNum = numVerts;
	return coords;
}
glm::vec3* Sphere::GetNormals(int& verNum) {
	verNum = numVerts;
	return normals;
}
unsigned int* Sphere::GetTriData(int& triNum)
{
	triNum = 6 * (stacks+1) * slices;
	return sphereIndices;
}

void Sphere::SetPosition(glm::vec3 newPos)
{
	Position = newPos;
}

glm::vec3 Sphere::GetPosition(void)
{
	return Position;
}