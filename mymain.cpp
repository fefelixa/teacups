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

CShader* myShader;  ///shader object 
CShader* myBasicShader;

//MODEL LOADING
#include "3DStruct\threeDModel.h"
#include "Obj\OBJLoader.h"

float amount = 0;
float temp = 0.002f;
	
CThreeDModel boxLeft, boxRight, boxFront;
CThreeDModel model; //A threeDModel object is needed for each model loaded
COBJLoader objLoader;	//this object is used to load the 3d models.
///END MODEL LOADING

glm::mat4 ProjectionMatrix; // matrix for the orthographic projection
glm::mat4 ModelViewMatrix;  // matrix for the modelling and viewing

glm::mat4 objectRotation;
glm::vec3 translation = glm::vec3(0.0, 0.0, 0.0);
glm::vec3 pos = glm::vec3(0.0f,0.0f,0.0f); //vector for the position of the object.

//Material properties
float Material_Ambient[4] = {0.1f, 0.1f, 0.1f, 1.0f};
float Material_Diffuse[4] = {0.8f, 0.8f, 0.5f, 1.0f};
float Material_Specular[4] = {0.9f,0.9f,0.8f,1.0f};
float Material_Shininess = 50;

//Light Properties
float Light_Ambient_And_Diffuse[4] = {0.8f, 0.8f, 0.6f, 1.0f};
float Light_Specular[4] = {1.0f,1.0f,1.0f,1.0f};
float LightPos[4] = {0.0f, 0.0f, 1.0f, 0.0f};

//
int	mouse_x=0, mouse_y=0;
bool LeftPressed = false;
int screenWidth=600, screenHeight=600;

//booleans to handle when the arrow keys are pressed or released.
bool Left = false;
bool Right = false;
bool Up = false;
bool Down = false;
bool Home = false;
bool End = false;

float spin=180;
float speed=0;

//OPENGL FUNCTION PROTOTYPES
void display();				//called in winmain to draw everything to the screen
void reshape(int width, int height);				//called when the window is resized
void init();				//called in winmain when the program starts.
void processKeys();         //called in winmain to process keyboard input
void idle();		//idle function
void updateTransform(float xinc, float yinc, float zinc);

/*************    START OF OPENGL FUNCTIONS   ****************/
void display()									
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(myShader->GetProgramObjID());  // use the shader

	//Part for displacement shader.
	amount += temp;
	if(amount > 1.0f || amount < -1.5f)
		temp = -temp;
	//amount = 0;
	glUniform1f(glGetUniformLocation(myShader->GetProgramObjID(), "displacement"), amount);

	//Set the projection matrix in the shader
	GLuint projMatLocation = glGetUniformLocation(myShader->GetProgramObjID(), "ProjectionMatrix");  
	glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, &ProjectionMatrix[0][0]);

	glm::mat4 viewingMatrix = glm::mat4(1.0f);
	
	//translation and rotation for view
	viewingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0, 0, -50));

	//apply a rotation to the view
	//static float angle = 0.0f;
	//angle += 0.01;
	//viewingMatrix = glm::rotate(viewingMatrix, angle, glm::vec3(1.0f, 0.0f, 0.0));

	//use of glm::lookAt for viewing instead.
	//viewingMatrix = glm::lookAt(glm::vec3(0,0,0), glm::vec3(0,0,100), glm::vec3(0.0f, 1.0f, 0.0));

	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ViewMatrix"), 1, GL_FALSE, &viewingMatrix[0][0]);

	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "LightPos"), 1, LightPos);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "light_ambient"), 1, Light_Ambient_And_Diffuse);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "light_diffuse"), 1, Light_Ambient_And_Diffuse);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "light_specular"), 1, Light_Specular);

	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "material_ambient"), 1, Material_Ambient);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "material_diffuse"), 1, Material_Diffuse);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "material_specular"), 1, Material_Specular);
	glUniform1f(glGetUniformLocation(myShader->GetProgramObjID(), "material_shininess"), Material_Shininess);

	//pos.x += objectRotation[2][0]*0.0003;
	//pos.y += objectRotation[2][1]*0.0003;
	//pos.z += objectRotation[2][2]*0.0003;
	
	glm::mat4 modelmatrix = glm::translate(glm::mat4(1.0f), pos);
	ModelViewMatrix = viewingMatrix * modelmatrix * objectRotation;
	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);

	
	glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	
	model.DrawElementsUsingVBO(myShader);

	//Switch to basic shader to draw the lines for the bounding boxes
	glUseProgram(myBasicShader->GetProgramObjID());
	projMatLocation = glGetUniformLocation(myBasicShader->GetProgramObjID(), "ProjectionMatrix");
	glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, &ProjectionMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(myBasicShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);

	//model.DrawAllBoxesForOctreeNodes(myBasicShader);
	//model.DrawBoundingBox(myBasicShader);
	//	model.CalcBoundingBox()
	//model.DrawOctreeLeaves(myBasicShader);

	//switch back to the shader for textures and lighting on the objects.
	glUseProgram(myShader->GetProgramObjID());  // use the shader

	ModelViewMatrix = glm::translate(viewingMatrix, glm::vec3(0, 0, 0));
	
	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	boxLeft.DrawElementsUsingVBO(myShader);
	boxRight.DrawElementsUsingVBO(myShader);
	boxFront.DrawElementsUsingVBO(myShader);

	glFlush();
	glutSwapBuffers();
}

void reshape(int width, int height)		// Resize the OpenGL window
{
	screenWidth=width; screenHeight = height;           // to ensure the mouse coordinates match 
														// we will use these values to set the coordinate system

	glViewport(0,0,width,height);						// Reset The Current Viewport

	//Set the projection matrix
	ProjectionMatrix = glm::perspective(glm::radians(60.0f), (GLfloat)screenWidth/(GLfloat)screenHeight, 1.0f, 200.0f);
}
void init()
{
	glClearColor(1.0,1.0,1.0,0.0);						//sets the clear colour to yellow
														//glClear(GL_COLOR_BUFFER_BIT) in the display function
														//will clear the buffer to this colour
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);


	myShader = new CShader();
	//if(!myShader->CreateShaderProgram("BasicView", "glslfiles/basicTransformationsWithDisplacement.vert", "glslfiles/basicTransformationsWithDisplacement.frag"))
	if(!myShader->CreateShaderProgram("BasicView", "glslfiles/basicTransformations.vert", "glslfiles/basicTransformations.frag"))
	{
		cout << "failed to load shader" << endl;
	}		

	myBasicShader = new CShader();
	if(!myBasicShader->CreateShaderProgram("Basic", "glslfiles/basic.vert", "glslfiles/basic.frag"))
	{
		cout << "failed to load shader" << endl;
	}		

	glUseProgram(myShader->GetProgramObjID());  // use the shader

	glEnable(GL_TEXTURE_2D);

	//lets initialise our object's rotation transformation 
	//to the identity matrix
	objectRotation = glm::mat4(1.0f);

	cout << " loading model " << endl;
	if(objLoader.LoadModel("MyModels/monkey.obj"))//returns true if the model is loaded
	{
		cout << " model loaded " << endl;		

		//copy data from the OBJLoader object to the threedmodel class
		model.ConstructModelFromOBJLoader(objLoader);

		//if you want to translate the object to the origin of the screen,
		//first calculate the centre of the object, then move all the vertices
		//back so that the centre is on the origin.
		model.CalcCentrePoint();
		model.CentreOnZero();

	
		model.InitVBO(myShader);
	}
	else
	{
		cout << " model failed to load " << endl;
	}
	

	
}

void special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		Left = true;
		break;
	case GLUT_KEY_RIGHT:
		Right = true;
		break;
	case GLUT_KEY_UP:
		Up = true;
		break;
	case GLUT_KEY_DOWN:
		Down = true;
		break;
	case GLUT_KEY_HOME:
		Home = true;
		break;
	case GLUT_KEY_END:
		End = true;
		break;
	}
}

void specialUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		Left = false;
		break;
	case GLUT_KEY_RIGHT:
		Right = false;
		break;
	case GLUT_KEY_UP:
		Up = false;
		break;
	case GLUT_KEY_DOWN:
		Down = false;
		break;
	case GLUT_KEY_HOME:
		Home = false;
		break;
	case GLUT_KEY_END:
		End = false;
		break;		
	}
}

void processKeys()
{
	float spinXinc = 0.0f, spinYinc = 0.0f, spinZinc = 0.0f;
	if (Left)
	{
		spinYinc = -0.001f;
	}
	if (Right)
	{
		spinYinc = 0.001f;
	}
	if (Up)
	{
		spinXinc = 0.001f;
	}
	if (Down)
	{
		spinXinc = -0.001f;
	}
	if (Home)
	{
		spinZinc = 0.001f;
	}
	if (End)
	{
		spinZinc = -0.001f;
	}
	updateTransform(spinXinc, spinYinc, spinZinc);
}


void updateTransform(float xinc, float yinc, float zinc)
{
	objectRotation = glm::rotate(objectRotation, xinc, glm::vec3(1,0,0));
	objectRotation = glm::rotate(objectRotation, yinc, glm::vec3(0,1,0));
	objectRotation = glm::rotate(objectRotation, zinc, glm::vec3(0,0,1));
}

void idle()
{
	spin += speed;
	if(spin > 360)
		spin = 0;

	processKeys();

	glutPostRedisplay();
}
/**************** END OPENGL FUNCTIONS *************************/

int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("OpenGL FreeGLUT Example: Obj loading");

	//This initialises glew - it must be called after the window is created.
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cout << " GLEW ERROR" << endl;
	}

	//Check the OpenGL version being used
	int OpenGLVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);
	cout << OpenGLVersion[0] << " " << OpenGLVersion[1] << endl;


	//initialise the objects for rendering
	init();

	glutReshapeFunc(reshape);
	//specify which function will be called to refresh the screen.
	glutDisplayFunc(display);

	glutSpecialFunc(special);
	glutSpecialUpFunc(specialUp);

	glutIdleFunc(idle);

	//starts the main loop. Program loops and calls callback functions as appropriate.
	glutMainLoop();

	return 0;
}
