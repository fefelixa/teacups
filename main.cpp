#include <iostream>
using namespace std;

//--- OpenGL ---
#include "GL\glew.h"
#include "GL\wglew.h"
#pragma comment(lib, "glew32.lib")
//--------------

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "glm\gtc\matrix_inverse.hpp"

#include "GL\freeglut.h"

#include "Images\FreeImage.h"

#include "shaders\Shader.h"

CShader *myShader; /// shader object
CShader *myBasicShader;

// MODEL LOADING
#include "3DStruct\threeDModel.h"
#include "Obj\OBJLoader.h"

float amount = 0;
float temp = 0.002f;

CThreeDModel boxLeft, boxRight, boxFront;
CThreeDModel cups[3][3],
	plates[4],
	floorPlane;			  // A threeDModel object is needed for each model loaded
float cups_aabb[3][3][6]; // [i][j][x1,y1,z1,x2,y2,z2]

float cupAngles[3][3];
COBJLoader objLoader; // this object is used to load the 3d models.
/// END MODEL LOADING

glm::mat4 ProjectionMatrix; // matrix for the orthographic projection
glm::mat4 ModelViewMatrix;	// matrix for the modelling and viewing

// Material properties
float Material_Ambient[4] = {0.6f, 0.6f, 0.6f, 1.0f};
float Material_Diffuse[4] = {0.8f, 0.8f, 0.5f, 1.0f};
float Material_Specular[4] = {0.9f, 0.9f, 0.8f, 1.0f};
float Material_Shininess = 50;

// Light Properties
float Light_Ambient_And_Diffuse[4] = {0.6f, 0.6f, 0.6f, 1.0f};
float Light_Specular[4] = {0.8f, 0.8f, 0.8f, 1.0f};
float LightPos[4] = {0.0f, 0.1f, 0.0f, 0.0f};

int mouse_x = 0, mouse_y = 0;
bool LeftPressed = false;
int screenWidth = 600, screenHeight = 600;

// booleans to handle when the arrow keys are pressed or released.
bool keyLeft, keyRight, keyUp, keyDown, keyHome, keyEnd, keyQ, key0, key1, key2, key3, keyA, keyD, keyS, keyW, keySpace, keyEsc; // keyboard keys
bool modS, modC, modA;																											 // shift, ctrl, alt modifier keys

bool debug1 = false;

float deltaTime = 0.0f,
	  lastFrame = 0.0f,
	  currentFrame = 0.0f;

static float angle1 = 0.0f,
			 angle2 = 0.0f,
			 angle3 = 0.0f,
			 speed1 = 0.0002f,
			 speed2 = 0.0002f,
			 speed3 = 0.0002f;

glm::vec3 freeCamPos = glm::vec3(0.0f, 10.0f, 50.0f);
glm::vec3 freeCamAngle = glm::vec3(0, -PI / 2, 0);
glm::vec3 freeCamFront = glm::vec3(0, 0, -1.0f);
glm::vec3 lookAtPos = glm::vec3(0.0f);
bool lookAt;
int cameraMode = 0;
float fov = 0.6f;
float targetFov = 0.6f;

int lastX = 300, lastY = 300;
// OPENGL FUNCTION PROTOTYPES
void display();						 // called in winmain to draw everything to the screen
void reshape(int width, int height); // called when the window is resized
void init();						 // called in winmain when the program starts.
void rotateCamera(float pitch, float yaw);
void moveCamera(float dx, float dy, float dz);
void processKeys();							  // called in winmain to process keyboard input
void mouseWheel(int btn, int state, int x, int y); // mouse function
void idle();								  // idle function
void closeGlut();

/*************    START OF OPENGL FUNCTIONS   ****************/
void display()
{
	if (ceil(targetFov * 100) != ceil(fov * 100))
	{
		fov += (targetFov - fov) * deltaTime / 100;
		ProjectionMatrix = glm::perspective((float)glm::radians(85 * pow(2, fov) - 70), (GLfloat)screenWidth / (GLfloat)screenHeight, 1.0f, 200.0f);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(myShader->GetProgramObjID()); // use the shader

	// Part for displacement shader.
	amount += temp;
	if (amount > 1.0f || amount < -1.5f)
		temp = -temp;
	// amount = 0;
	glUniform1f(glGetUniformLocation(myShader->GetProgramObjID(), "displacement"), amount);

	// Set the projection matrix in the shader
	GLuint projMatLocation = glGetUniformLocation(myShader->GetProgramObjID(), "ProjectionMatrix");
	glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, &ProjectionMatrix[0][0]);

	glm::mat4 viewingMatrix = glm::mat4(1.0f);
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat3 normalMatrix;

	angle1 += speed1 * deltaTime; // single cup rotation
	angle2 += speed2 * deltaTime; // small plate (3 cups) rotation
	angle3 += speed3 * deltaTime; // big plate (9 cups) rotation

	// camera modes controlled with number keys
	switch (cameraMode)
	{
	case 0: // free cam
		viewingMatrix = glm::lookAt(freeCamPos, freeCamPos + freeCamFront, glm::vec3(0, 1.0f, 0));
		break;
	case 1:																												// view from the ground
		viewingMatrix = glm::lookAt(glm::vec3(0.0f, 5.0f, -70.0f), cups[0][0].pos.toGlm(), glm::vec3(0.0f, 1.0f, 0.0)); // lok at a teacup from above
		break;
	case 2: // view from the ride
		glm::vec3 cam2pos = -cups[0][0].pos.toGlm();
		cam2pos.y -= 3.0f; // elevate camera slightly out of teacup
		viewingMatrix = glm::rotate(viewingMatrix, -angle1 - angle2 - angle3, glm::vec3(0, 1, 0));
		viewingMatrix = glm::translate(viewingMatrix, cam2pos);
		break;
	default:
		cameraMode = 0;
		break;
	}

	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ViewMatrix"), 1, GL_FALSE, &viewingMatrix[0][0]);

	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "LightPos"), 1, LightPos);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "light_ambient"), 1, Light_Ambient_And_Diffuse);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "light_diffuse"), 1, Light_Ambient_And_Diffuse);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "light_specular"), 1, Light_Specular);

	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "material_ambient"), 1, Material_Ambient);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "material_diffuse"), 1, Material_Diffuse);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "material_specular"), 1, Material_Specular);
	glUniform1f(glGetUniformLocation(myShader->GetProgramObjID(), "material_shininess"), Material_Shininess);

	// pos.x += objectRotation[2][0]*0.0003;
	// pos.y += objectRotation[2][1]*0.0003;
	// pos.z += objectRotation[2][2]*0.0003;

	// floor plane
	ModelViewMatrix = viewingMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

	plates[3].DrawElementsUsingVBO(myShader);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -1.0f, 0));
	ModelViewMatrix = viewingMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	floorPlane.DrawElementsUsingVBO(myShader);
	glm::vec3 move1 = glm::vec3(10.0f, 3.5f, 0);
	glm::vec3 move2 = glm::vec3(20.0f, 0, 0);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			modelMatrix = glm::mat4(1.0f);
			modelMatrix = glm::rotate(modelMatrix, angle3 + glm::radians(i * 120.0f), glm::vec3(0, 1, 0));
			modelMatrix = glm::translate(modelMatrix, move2);
			modelMatrix = glm::rotate(modelMatrix, angle2 + glm::radians(j * 120.0f), glm::vec3(0.0, 1.0, 0.0));
			modelMatrix = glm::translate(modelMatrix, move1);
			modelMatrix = glm::rotate(modelMatrix, angle1, glm::vec3(0.0, 1.0, 0.0));
			glm::vec4 newPos = modelMatrix * glm::vec4(1.0f);
			cups[i][j].pos.x = newPos[0];
			cups[i][j].pos.y = newPos[1];
			cups[i][j].pos.z = newPos[2];
			cupAngles[i][j] = angle1 + angle2 + glm::radians(j * 120.0f) + angle3 + glm::radians(i * 120.0f);
			// std::cout << cups[i][j].pos << endl;
			ModelViewMatrix = viewingMatrix * modelMatrix;
			glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);

			normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
			glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

			cups[i][j].DrawElementsUsingVBO(myShader);
			cups[i][j].CalcBoundingBox();
			cups[i][j].DrawBoundingBox(myBasicShader);
		}
	}
	for (int i = 0; i < 3; i++)
	{
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.1f, 0));
		modelMatrix = glm::rotate(modelMatrix, angle3 + glm::radians(i * 120.0f), glm::vec3(0, 1, 0));
		modelMatrix = glm::translate(modelMatrix, move2);
		modelMatrix = glm::rotate(modelMatrix, angle2, glm::vec3(0.0, 1.0, 0.0));

		ModelViewMatrix = viewingMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);

		normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
		glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

		plates[i].DrawElementsUsingVBO(myShader);
	}
	// matrix1

	// Switch to basic shader to draw the lines for the bounding boxes
	glUseProgram(myBasicShader->GetProgramObjID());
	projMatLocation = glGetUniformLocation(myBasicShader->GetProgramObjID(), "ProjectionMatrix");
	glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(myBasicShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);

	// cups[0][0].DrawAllBoxesForOctreeNodes(myBasicShader);

	// cups[0][0].DrawOctreeLeaves(myBasicShader);

	// switch back to the shader for textures and lighting on the objects.
	glUseProgram(myShader->GetProgramObjID()); // use the shader

	ModelViewMatrix = glm::translate(viewingMatrix, glm::vec3(0, 0, 0));

	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);

	glFlush();
	glutSwapBuffers();
}

void reshape(int width, int height) // Resize the OpenGL window
{
	screenWidth = width;
	screenHeight = height; // to ensure the mouse coordinates match
						   // we will use these values to set the coordinate system

	glViewport(0, 0, width, height); // Reset The Current Viewport

	// Set the projection matrix
	ProjectionMatrix = glm::perspective((float)glm::radians(85 * pow(2, fov) - 70), (GLfloat)screenWidth / (GLfloat)screenHeight, 1.0f, 200.0f);
}
void init()
{
	glClearColor(178.0f / 255.0f, 212.0f / 255.0f, 232.0f / 255.0f, 0.5f); // sets the bg colour to blue
	// glClear(GL_COLOR_BUFFER_BIT) in the display function
	// will clear the buffer to this colour
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE);

	myShader = new CShader();
	// if(!myShader->CreateShaderProgram("BasicView", "glslfiles/basicTransformationsWithDisplacement.vert", "glslfiles/basicTransformationsWithDisplacement.frag"))
	if (!myShader->CreateShaderProgram("BasicView", "glslfiles/basicTransformations.vert", "glslfiles/basicTransformations.frag"))
	{
		cout << "failed to load shader" << endl;
	}

	myBasicShader = new CShader();
	if (!myBasicShader->CreateShaderProgram("Basic", "glslfiles/basic.vert", "glslfiles/basic.frag"))
	{
		cout << "failed to load shader" << endl;
	}

	glUseProgram(myShader->GetProgramObjID()); // use the shader

	glEnable(GL_TEXTURE_2D);

	std::string modelFolder = "MyModels/teacups/nh/";
	char teacupDir[33];
	cout << " loading model " << endl;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			sprintf_s(teacupDir, sizeof(teacupDir), "MyModels/teacups/nh/teacup%d.obj", i + 1);
			if (objLoader.LoadModel(teacupDir)) // returns true if the model is loaded
			{
				printf("loaded teacup%d.obj\n", i);
				// copy data from the OBJLoader object to the threedmodel class
				cups[j][i].ConstructModelFromOBJLoader(objLoader);

				cups[j][i].CalcCentrePoint();
				cups[j][i].CentreOnZero();

				cups[j][i].InitVBO(myShader);
			}
			else
			{
				printf("teacup %d failed to load\n", i);
			}
		}
	}

	if (objLoader.LoadModel("MyModels/teacups/nh/plate1.obj"))
	{
		printf("loaded plate1.obj\n");
		for (int i = 0; i < 3; i++)
		{
			plates[i].ConstructModelFromOBJLoader(objLoader);
			plates[i].CalcCentrePoint();
			plates[i].CentreOnZero();
			plates[i].InitVBO(myShader);
		}
	}
	else
		printf("failed to load plate1.obj\n");

	if (objLoader.LoadModel("MyModels/teacups/nh/plate2.obj"))
	{
		printf("loaded plate2.obj\n");
		plates[3].ConstructModelFromOBJLoader(objLoader);
		plates[3].CalcCentrePoint();
		plates[3].CentreOnZero();
		plates[3].InitVBO(myShader);
	}
	else
		cout << "failed to load plate2.obj" << endl;

	if (objLoader.LoadModel(modelFolder + "floorplane.obj"))
	{
		floorPlane.ConstructModelFromOBJLoader(objLoader);
		floorPlane.InitVBO(myShader);
	}
}

void special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		keyLeft = true;
		break;
	case GLUT_KEY_RIGHT:
		keyRight = true;
		break;
	case GLUT_KEY_UP:
		keyUp = true;
		break;
	case GLUT_KEY_DOWN:
		keyDown = true;
		break;
	case GLUT_KEY_HOME:
		keyHome = true;
		break;
	case GLUT_KEY_END:
		keyEnd = true;
		break;
	case 27: // escape
		keyEsc = true;
		break;
	case GLUT_KEY_SHIFT_L:
		modS = true;
		break;
	case GLUT_KEY_SHIFT_R:
		modS = true;
		break;
	case GLUT_KEY_CTRL_L:
		modC = true;
		break;
	case GLUT_KEY_CTRL_R:
		modC = true;
		break;
	case GLUT_KEY_ALT_L:
		modA = true;
		break;
	case GLUT_KEY_ALT_R:
		modA = true;
		break;
	}
}

void specialUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		keyLeft = false;
		break;
	case GLUT_KEY_RIGHT:
		keyRight = false;
		break;
	case GLUT_KEY_UP:
		cout << speed1 << endl
			 << speed2 << endl
			 << speed3 << endl;
		keyUp = false;
		break;
	case GLUT_KEY_DOWN:
		keyDown = false;
		break;
	case GLUT_KEY_HOME:
		keyHome = false;
		break;
	case GLUT_KEY_END:
		keyEnd = false;
		break;
	case 27: // escape
		keyEsc = false;
		break;

	case GLUT_KEY_SHIFT_L:
		modS = false;
		break;
	case GLUT_KEY_SHIFT_R:
		modS = false;
		break;
	case GLUT_KEY_CTRL_L:
		modC = false;
		break;
	case GLUT_KEY_CTRL_R:
		modC = false;
		break;
	case GLUT_KEY_ALT_L:
		modA = false;
		break;
	case GLUT_KEY_ALT_R:
		modA = false;
		break;
	}
}

void keyPressDown(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		keyQ = true;
		break;

	case 'a':
		keyA = true;
		break;
	case 'd':
		keyD = true;
		break;
	case 's':
		keyS = true;
		break;
	case 'w':
		keyW = true;
		break;

	case '0':
		key0 = true;
		break;
	case '1':
		key1 = true;
		break;
	case '2':
		key2 = true;
		break;
	case '3':
		key3 = true;
		break;
	case ' ':
		keySpace = true;
		break;

	case 27:
		keyEsc = true;
		break;
	}
}

void keyPressUp(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		keyQ = false;
		break;

	case 'a':
		keyA = false;
		break;
	case 'd':
		keyD = false;
		break;
	case 's':
		keyS = false;
		break;
	case 'w':
		keyW = false;
		break;

	case '0':
		key0 = false;
		break;
	case '1':
		key1 = false;
		break;
	case '2':
		key2 = false;
		break;
	case '3':
		key3 = false;
		break;
	case ' ':
		keySpace = false;
		debug1 = false;
		break;
	case 27:
		keyEsc = false;
		break;
	}
}

float mouseSens = 0.005f;
void mouseMove(int x, int y)
{
	int dx = 300 - x;
	int dy = 300 - y;
	lastX = x;
	lastY = y;
	rotateCamera(dy * mouseSens, -dx * mouseSens);
	glutWarpPointer(300, 300);
}
void mouseWheel(int btn, int state, int x, int y)
{
	if (state == 1)
	{
		if (btn == 3)
		{ // zoom in - decrease fov
			targetFov = max(0.0f, targetFov - 0.05f);

			// fov -= 0.01f;
			// cout << "fov: " << targetFov << endl;
		}
		else if (btn == 4)
		{ // zoom out - increase fov
			targetFov = min(1.0f, targetFov + 0.05f);
			// fov += 0.01f;
			// cout << "fov: " << targetFov << endl;
		}
		else if (btn == 1)
		{
			targetFov = 0.6f;
			// cout << "fov: " << targetFov << endl;
		}
		mouseSens = 0.004f * (targetFov + 1);
	}
}

void moveCamera(float dx, float dy, float dz)
{
	glm::vec3 newCamPos = freeCamPos;
	if (dx != 0)
	{
		newCamPos += glm::normalize(glm::cross(freeCamFront, glm::vec3(0, 1.0f, 0))) * dx;
	}
	if (dy != 0)
	{
		newCamPos.y += dy;
	}
	if (dz != 0)
	{
		newCamPos += dz * freeCamFront;
	}
	if (newCamPos.y < 0.1f)
	{
		newCamPos.y = 0.1f;
	}
	bool collides = false;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if ((newCamPos.x > cups[i][j].aabb[0] && newCamPos.x < cups[i][j].aabb[3]) &&
				(newCamPos.y > cups[i][j].aabb[1] && newCamPos.y < cups[i][j].aabb[4]) &&
				(newCamPos.z > cups[i][j].aabb[2] && newCamPos.z < cups[i][j].aabb[5])) {
					collides = true;
					cout << i<<j << endl;
			}
		}
	}
	if (!collides)
		freeCamPos = newCamPos;
}

void rotateCamera(float dpitch, float dyaw)
{

	float newPitch = freeCamAngle.x + dpitch; // up and down
	float newYaw = freeCamAngle.y + dyaw;	  // left and right

	// dont break your neck - limit pitch to 180�
	if (abs(newPitch) < PI / 2)
		freeCamAngle.x = newPitch;
	freeCamAngle.y = newYaw;

	glm::vec3 newCamFront;
	newCamFront.x = cos(freeCamAngle.y) * cos(freeCamAngle.x);
	newCamFront.y = sin(freeCamAngle.x);
	newCamFront.z = sin(freeCamAngle.y) * cos(freeCamAngle.x);

	freeCamFront = glm::normalize(newCamFront);
}

float maxCupSpeed = 0.01f;
void processKeys()
{
	float camRoteSpeed = 0.002f * deltaTime;
	float camMoveSpeed = 0.05f * deltaTime;
	float cupKeySpeed = 0.00001f * deltaTime;

	if (keyLeft)
	{
		rotateCamera(0, -camRoteSpeed);
	}
	if (keyRight)
	{
		rotateCamera(0, camRoteSpeed);
	}
	if (keyUp)
	{
		if (modS)
			speed1 = min(maxCupSpeed, cupKeySpeed + speed1);
		else if (modC)
			speed2 = min(maxCupSpeed, cupKeySpeed + speed2);
		else if (modA)
			speed3 = min(maxCupSpeed, cupKeySpeed + speed3);
		else
			rotateCamera(camRoteSpeed, 0);
	}
	if (keyDown)
	{
		if (modS)
		{
			speed1 = max(-maxCupSpeed, speed1 - cupKeySpeed);
		}
		else if (modC)
			speed2 = max(-maxCupSpeed, speed2 - cupKeySpeed);
		else if (modA)
			speed3 = max(-maxCupSpeed, speed3 - cupKeySpeed);
		else
			rotateCamera(-camRoteSpeed, 0);
	}
	if (keyA)
	{
		moveCamera(-camMoveSpeed, 0, 0);
	}
	if (keyD)
	{
		moveCamera(camMoveSpeed, 0, 0);
	}
	if (keyW)
	{
		moveCamera(0, 0, camMoveSpeed);
	}
	if (keyS)
	{
		moveCamera(0, 0, -camMoveSpeed);
	}

	if (keyEsc)
	{
		glutLeaveMainLoop();
	}
	if (key0)
	{
		freeCamPos = glm::vec3(0.0f, 10.0f, 50.0f);
		freeCamAngle = glm::vec3(0, -PI / 2, 0);
		freeCamFront = glm::vec3(0, 0, -1.0f);
		cameraMode = 0;
	}
	if (key1)
	{
		cameraMode = 1;
	}
	if (key2)
	{
		cameraMode = 2;
	}
	if (keySpace)
	{
		moveCamera(0, camMoveSpeed, 0);
	}
}

void idle()
{
	currentFrame = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	processKeys();

	glutPostRedisplay();
}
void closeGlut()
{
}
/**************** END OPENGL FUNCTIONS *************************/

int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("OpenGL FreeGLUT Example: Obj loading");

	// This initialises glew - it must be called after the window is created.
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cout << " GLEW ERROR" << endl;
	}

	// Check the OpenGL version being used
	int OpenGLVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);
	cout << OpenGLVersion[0] << " " << OpenGLVersion[1] << endl;

	// initialise the objects for rendering
	init();

	glutReshapeFunc(reshape);
	// specify which function will be called to refresh the screen.
	glutDisplayFunc(display);

	glutSpecialFunc(special);
	glutSpecialUpFunc(specialUp);
	glutKeyboardFunc(keyPressDown);
	glutKeyboardUpFunc(keyPressUp);
	glutMouseFunc(mouseWheel);
	glutPassiveMotionFunc(mouseMove);
	glutIdleFunc(idle);

	// starts the main loop. Program loops and calls callback functions as appropriate.
	glutMainLoop();

	return 0;
}
