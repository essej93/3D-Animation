// include C++ headers
#include <cstdio>
#include <iostream>
//using namespace std;	// to avoid having to use std::

#include "utilities.h"
#include "SimpleModel.h"

// include OpenGL related headers
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>

// global variables
// settings
unsigned int gWindowWidth = 800;
unsigned int gWindowHeight = 600;

// frame stats
float gFrameRate = 60.0f;
float gFrameTime = 1 / gFrameRate;

// scene variables
glm::mat4 gViewMatrix;			// view matrix
glm::mat4 gProjectionMatrix;	// projection matrix
std::map<std::string, glm::mat4> gModelMatrix;			// model matrix
Light gLight;			// light properties

// Materials Globals
enum class MaterialType { PEARL, JADE };
std::map<std::string, Material> gMaterials;

// Model globals
enum class ModelType { SPHERE, CUBE, SUZANNE };
std::map<std::string, SimpleModel> gModels;

// shaders global
std::map<std::string, ShaderProgram> gShaders; // holds multiple shaders



// controls
bool gWireframe = false;	// wireframe control
float gOrbitSpeed = 0.5f;
float gRotationSpeed[2] = { 1.0f, 1.0f };

// function initialise scene and render settings
static void init(GLFWwindow* window)
{
	// set the color the color buffer should be cleared to
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	glEnable(GL_DEPTH_TEST); // enable debth buffer test

	// link shaders
	gShaders["Simple"].compileAndLink("simpleColor.vert", "simpleColor.frag");
	gShaders["Animation"].compileAndLink("animation.vert", "animation.frag");

	//gShader.compileAndLink("animation.vert", "animation.frag");

	// initialise view matrix
	gViewMatrix = glm::lookAt(glm::vec3(1.0f, 5.0f, 15.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// initialise projection matrix
	gProjectionMatrix = glm::perspective(glm::radians(45.0f),
		static_cast<float>(gWindowWidth) / gWindowHeight, 0.1f, 100.0f);

	// defining materials
	gMaterials["Pearl"].Ka = glm::vec3(0.25f, 0.21f, 0.21f);
	gMaterials["Pearl"].Kd = glm::vec3(1.0f, 0.83f, 0.83f);
	gMaterials["Pearl"].Ks = glm::vec3(0.3f, 0.3f, 0.3f);
	gMaterials["Pearl"].shininess = 11.3f;

	gMaterials["Jade"].Ka = glm::vec3(0.14f, 0.22f, 0.16f);
	gMaterials["Jade"].Kd = glm::vec3(0.53f, 0.89f, 0.63f);
	gMaterials["Jade"].Ks = glm::vec3(0.3f, 0.3f, 0.3f);
	gMaterials["Jade"].shininess = 12.8f;

	gMaterials["Brass"].Ka = glm::vec3(0.33f, 0.22f, 0.03f);
	gMaterials["Brass"].Kd = glm::vec3(0.78f, 0.57f, 0.11f);
	gMaterials["Brass"].Ks = glm::vec3(0.99f, 0.94f, 0.8f);
	gMaterials["Brass"].shininess = 27.9f;

	// initialise point light properties
	gLight.dir = glm::vec3(0.3f, -0.7f, -0.5f);
	gLight.La = glm::vec3(0.8f);
	gLight.Ld = glm::vec3(0.8f);
	gLight.Ls = glm::vec3(0.8f);

	// initialise model matrices
	gModelMatrix["Sphere"] = glm::mat4(1.0f);
	gModelMatrix["OrbitObj1"] = glm::mat4(1.0f);
	gModelMatrix["OrbitObj2"] = glm::mat4(1.0f);

	// load models
	gModels["Sphere"].loadModel("./models/sphere.obj");
	gModels["Cube"].loadModel("./models/cube.obj");
	gModels["Suzanne"].loadModel("./models/suzanne.obj");
}

// function used to update the scene
static void update_scene(GLFWwindow* window)
{

	static glm::vec3 moveVec(0.0f);
	static float orbitAngle = 0.0f;
	static float rotationAngle[2] = { 0.0f, 0.0f };

	orbitAngle += gOrbitSpeed * gFrameTime;
	rotationAngle[0] += gRotationSpeed[0] * gFrameTime;
	rotationAngle[1] += gRotationSpeed[1] * gFrameTime;

	gModelMatrix["Sphere"] = glm::translate(moveVec);

	gModelMatrix["OrbitObj1"] = gModelMatrix["Sphere"]
		* glm::rotate(orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(5.0f, 0.0f, 0.0f))
		* glm::rotate(rotationAngle[0] - orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::vec3(0.7f, 0.7f, 0.7f));
}

// function to render the scene
static void render_scene()
{
	// clear colour buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ShaderProgram* gShader = &gShaders["Animation"]; // points to the shader we want to use
	//gShaders["Animation"].use(); // set shader to use

	gShader->use();

	// Sphere render
	// set light properties
	gShader->setUniform("uLight.dir", gLight.dir);
	gShader->setUniform("uLight.La", gLight.La);
	gShader->setUniform("uLight.Ld", gLight.Ld);
	gShader->setUniform("uLight.Ls", gLight.Ls);

	// set material properties
	gShader->setUniform("uMaterial.Ka", gMaterials["Brass"].Ka);
	gShader->setUniform("uMaterial.Kd", gMaterials["Brass"].Kd);
	gShader->setUniform("uMaterial.Ks", gMaterials["Brass"].Ks);
	gShader->setUniform("uMaterial.shininess", gMaterials["Brass"].shininess);

	// set viewing position
	gShader->setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));

	// calculate matrices
	glm::mat4 MVP = gProjectionMatrix * gViewMatrix * gModelMatrix["Sphere"];
	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["Sphere"])));


	// set uniform variables
	gShader->setUniform("uModelViewProjectionMatrix", MVP);
	gShader->setUniform("uModelMatrix", gModelMatrix["Sphere"]);
	gShader->setUniform("uNormalMatrix", normalMatrix);

	gModels["Sphere"].drawModel();


	// Object 1 render

	// set light properties
	gShader->setUniform("uLight.dir", gLight.dir);
	gShader->setUniform("uLight.La", gLight.La);
	gShader->setUniform("uLight.Ld", gLight.Ld);
	gShader->setUniform("uLight.Ls", gLight.Ls);

	// set material properties
	gShader->setUniform("uMaterial.Ka", gMaterials["Brass"].Ka);
	gShader->setUniform("uMaterial.Kd", gMaterials["Brass"].Kd);
	gShader->setUniform("uMaterial.Ks", gMaterials["Brass"].Ks);
	gShader->setUniform("uMaterial.shininess", gMaterials["Brass"].shininess);

	// set viewing position
	gShader->setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));

	// calculate matrices
	MVP = gProjectionMatrix * gViewMatrix * gModelMatrix["OrbitObj1"];
	normalMatrix = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["OrbitObj1"])));

	// set uniform variables
	gShader->setUniform("uModelViewProjectionMatrix", MVP);
	gShader->setUniform("uModelMatrix", gModelMatrix["OrbitObj1"]);
	gShader->setUniform("uNormalMatrix", normalMatrix);

	gModels["Suzanne"].drawModel();

	// Object 2 render

	//	// set light properties
	//gShader->setUniform("uLight.dir", gLight.dir);
	//gShader->setUniform("uLight.La", gLight.La);
	//gShader->setUniform("uLight.Ld", gLight.Ld);
	//gShader->setUniform("uLight.Ls", gLight.Ls);

	//// set material properties
	//gShader->setUniform("uMaterial.Ka", gMaterials["Brass"].Ka);
	//gShader->setUniform("uMaterial.Kd", gMaterials["Brass"].Kd);
	//gShader->setUniform("uMaterial.Ks", gMaterials["Brass"].Ks);
	//gShader->setUniform("uMaterial.shininess", gMaterials["Brass"].shininess);

	//// set viewing position
	//gShader->setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));

	//// calculate matrices
	//MVP = gProjectionMatrix * gViewMatrix * gModelMatrix["OrbitObj2"];
	//normalMatrix = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["OrbitObj2"])));

	//gModels["Cube"].drawModel();



	// Sphere render

	//// set light properties
	//gShaders["Animation"].setUniform("uLight.dir", gLight.dir);
	//gShaders["Animation"].setUniform("uLight.La", gLight.La);
	//gShaders["Animation"].setUniform("uLight.Ld", gLight.Ld);
	//gShaders["Animation"].setUniform("uLight.Ls", gLight.Ls);

	//// set material properties
	//gShaders["Animation"].setUniform("uMaterial.Ka", gMaterials["Brass"].Ka);
	//gShaders["Animation"].setUniform("uMaterial.Kd", gMaterials["Brass"].Kd);
	//gShaders["Animation"].setUniform("uMaterial.Ks", gMaterials["Brass"].Ks);
	//gShaders["Animation"].setUniform("uMaterial.shininess", gMaterials["Brass"].shininess);

	//gShaders["Animation"].setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));

	//glm::mat4 MVP = gProjectionMatrix * gViewMatrix * gModelMatrix["Sphere"];
	//glm::mat4 normalMatrix = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["Sphere"])));

	//gShaders["Animation"].setUniform("uModelViewProjectionMatrix", MVP);
	//gShaders["Animation"].setUniform("uModelMatrix", gModelMatrix["Sphere"]);
	//gShaders["Animation"].setUniform("uNormalMatrix", normalMatrix);

	


	// flush the graphics pipeline
	glFlush();
}

// key press or release callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// close the window when the ESCAPE key is pressed
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
}

// mouse movement callback function
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// pass cursor position to tweak bar
	TwEventMousePosGLFW(static_cast<int>(xpos), static_cast<int>(ypos));
}

// mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// pass mouse button status to tweak bar
	TwEventMouseButtonGLFW(button, action);
}

// error callback function
static void error_callback(int error, const char* description)
{
	std::cerr << description << std::endl;	// output error description
}

TwBar* create_UI(const std::string name) {

	TwBar* twBar = TwNewBar(name.c_str());

	// give tweak bar the size of graphics window
	TwWindowSize(gWindowWidth, gWindowHeight);
	TwDefine(" TW_HELP visible=false ");	// disable help menu
	TwDefine(" GLOBAL fontsize=3 ");		// set large font size

	TwDefine(" Main label='User Interface' refresh=0.02 text=light size='250 450' ");

	// create frame stat entries
	TwAddVarRO(twBar, "Frame Rate", TW_TYPE_FLOAT, &gFrameRate, " group='Frame Stats' precision=2 ");
	TwAddVarRO(twBar, "Frame Time", TW_TYPE_FLOAT, &gFrameTime, " group='Frame Stats' ");

	// scene controls
	TwAddVarRW(twBar, "Wireframe", TW_TYPE_BOOLCPP, &gWireframe, " group='Controls' ");

	// light controls
	TwAddVarRW(twBar, "Direction", TW_TYPE_DIR3F, &gLight.dir, " group='Light' opened=true ");
	TwAddVarRW(twBar, "La", TW_TYPE_COLOR3F, &gLight.La, " group='Light' ");
	TwAddVarRW(twBar, "Ld", TW_TYPE_COLOR3F, &gLight.Ld, " group='Light' ");
	TwAddVarRW(twBar, "Ls", TW_TYPE_COLOR3F, &gLight.Ls, " group='Light' ");


	return twBar;
}

int main(void)
{
	GLFWwindow* window = nullptr;	// GLFW window handle

	glfwSetErrorCallback(error_callback);	// set GLFW error callback function

	// initialise GLFW
	if (!glfwInit())
	{
		// if failed to initialise GLFW
		exit(EXIT_FAILURE);
	}

	// minimum OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window and its OpenGL context
	window = glfwCreateWindow(gWindowWidth, gWindowHeight, "Assignment 2 - 3D Animation", nullptr, nullptr);

	// check if window created successfully
	if (window == nullptr)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);	// set window context as the current context
	glfwSwapInterval(1);			// swap buffer interval

	// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		std::cerr << "GLEW initialisation failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	// set GLFW callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// initialise scene and render settings
	init(window);

	// initialise AntTweakBar
	TwInit(TW_OPENGL_CORE, nullptr);
	TwBar* tweakBar = create_UI("Main");		// create and populate tweak bar elements

	// timing data
	double lastUpdateTime = glfwGetTime();	// last update time
	double elapsedTime = lastUpdateTime;	// time since last update
	int frameCount = 0;						// number of frames since last update


	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		update_scene(window); // update the scene


		// if wireframe set polygon render mode to wireframe
		if (gWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		render_scene();		// render the scene

		// set polygon render mode to fill
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		TwDraw();				// draw tweak bar

		glfwSwapBuffers(window);	// swap buffers
		glfwPollEvents();			// poll for events

		frameCount++;
		elapsedTime = glfwGetTime() - lastUpdateTime;	// time since last update

		// if elapsed time since last update > 1 second
		if (elapsedTime > 1.0)
		{
			gFrameTime = elapsedTime / frameCount;	// average time per frame
			gFrameRate = 1 / gFrameTime;			// frames per second
			lastUpdateTime = glfwGetTime();			// set last update time to current time
			frameCount = 0;							// reset frame counter
		}
	}

	// uninitialise tweak bar
	TwDeleteBar(tweakBar);
	TwTerminate();

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}