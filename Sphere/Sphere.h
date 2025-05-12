#pragma once
#include "../glm/glm.hpp"


struct Vertex {
	glm:: vec4 coords;
	glm::vec2 texCoords;
};
struct VertexWithNormal {
	glm::vec4 coords;
	glm::vec3 normals;
};

class Sphere
{
private:
	glm::vec3 Position;
	//Sphere vertices data with normals
	glm::vec4 * coords;
	glm::vec3 * normals;
	unsigned int* sphereIndices;          //Sphere triangle indices    

	int stacks; // nunber of segments
	int slices; // number of segments
	int numVerts;
	float radius;

	void CreateSpherewithNormal();
public:
	Sphere();
	~Sphere();

	void SetPosition(glm::vec3 newPos);
	glm::vec3 GetPosition(void);
	glm::vec4* GetCoords(int&);
	glm::vec3* GetNormals(int&);
	unsigned int* GetTriData(int&);
};

