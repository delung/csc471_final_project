/**
 * Base code
 * Draws two meshes and one ground plane, one mesh has textures, as well
 * as ground plane.
 * Must be fixed to load in mesh with multiple shapes (dummy.obj)
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "WindowManager.h"
#include "GLTextureWriter.h"
#include "Terrain.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;


class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> simp_prog;
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> texProg;
	std::shared_ptr<Program> texProg2;

	// Shapes to be used (from obj file)
	std::vector<shared_ptr<Shape>> AllShapes;
	//meshes with just one shape
	shared_ptr<Shape> world;
	shared_ptr<Shape> Nef;

	//ground plane info
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int gGiboLen;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	// terrain vertex
	std::vector<GLuint> terrain_VertexArrayID;
	std::vector<GLuint> terrain_Vertexbuffer;
	std::vector<GLuint> terrain_Normalbuffer;

	// GLuint terrain_VertexArrayID;
	// GLuint terrain_Vertexbuffer;
	// GLuint terrain_Normalbuffer;


	std::vector<GLfloat> vertexArray;
	std::vector<GLfloat> normalArray;
	//three different textures
	shared_ptr<Texture> texture0;
 	shared_ptr<Texture> texture1;
 	shared_ptr<Texture> texture2;
 	shared_ptr<Texture> texture3;

	shared_ptr<Terrain> terrain;

	int gMat = 0;

	//For each shape, now that they are not resized, they need to be
	//transformed appropriately to the origin and scaled
	//transforms for Nef
	vec3 gTrans = vec3(0);
	float gScale = 1.0;

	//transforms for the world
	vec3 gDTrans = vec3(0);
	float gDScale = 1.0;

	//trasforms for the dummy
	vec3 gXTrans = vec3(0);
	float gXScale = 1.0;

	float theta = 0;
	float cTheta = 0;
	float cPhi = 0;
	bool mouseDown = false;

	// camera controls
	float x = 0;
	float y = 0;
	float z = 0;

	vec3 eye;
	vec3 center;
	vec3 up;
	vec3 forwardVec;
	float moveSpd = 0.3f;

	double oldx;
	double oldy;

	bool mouseMode = true;


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		else if (key == GLFW_KEY_M && action == GLFW_PRESS)
		{
			gMat = (gMat + 1) % 4;
		}

		else if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			mouseMode = !mouseMode;
			if(mouseMode)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		}
		else
		{
			forwardVec = eye - center;
			forwardVec = normalize(forwardVec);

			if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				vec3 rightVec = cross(up, forwardVec);
				eye -= rightVec * moveSpd;
				center -= rightVec * moveSpd;
			}
			else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				vec3 leftVec = cross(up, forwardVec);
				eye += leftVec * moveSpd;
				center += leftVec * moveSpd;
			}
			else if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				eye -= forwardVec * moveSpd;
				center -= forwardVec * moveSpd;
			}
			else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				eye += forwardVec * moveSpd;
				center += forwardVec * moveSpd;
			}
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{

	}

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		double dX = 0;
		double dY = 0;
		if(mouseMode)
		{
			if(xpos - oldx > 0)
				dX = -1;
			else if(xpos - oldx < 0)
				dX = 1;
			if(ypos - oldy > 0)
				dY = -1;
			else if(ypos - oldy < 0)
				dY = 1;

			cTheta += (float) -1 * dX * 3.14 / 720.0;
			cPhi += (float) dY * 3.14 / 720.0;

			if(cPhi > 80.0 * 3.14 / 180)
				cPhi = 80.0 * 3.14 / 180;
			else if(cPhi <  -80.0 * 3.14 / 180)
				cPhi = -80.0 * 3.14 / 180;

			oldx = xpos;
			oldy= ypos;
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			mouseDown = true;
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
		}

		if (action == GLFW_RELEASE)
		{
			mouseDown = false;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}


	//code to set up the two shaders - a diffuse shader and texture mapping
	void init(const std::string& resourceDirectory)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		cTheta = 0;
		// Set background color.
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(
			resourceDirectory + "/simple_vert.glsl",
			resourceDirectory + "/simple_frag.glsl");
		if (! prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("MV");
		prog->addUniform("View");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");


		// Init Terrain
		terrain = make_shared<Terrain>();
		terrain->setFilename(resourceDirectory + "/heightmap.png");
		terrain->init(100.0);
	 }

	 void initGeom(const std::string& resourceDirectory)
	 {
		for(int z = 0; z < terrain->getLength() - 1; z++)
		{
			vertexArray.clear();
			normalArray.clear();

			GLuint vertArrID;
			GLuint buffArrID;
			GLuint normArrID;

			terrain_VertexArrayID.push_back(vertArrID);
			terrain_Vertexbuffer.push_back(buffArrID);
			terrain_Normalbuffer.push_back(normArrID);

			glGenVertexArrays(1, &terrain_VertexArrayID[z]);
			glBindVertexArray(terrain_VertexArrayID[z]);

			glGenBuffers(1, &terrain_Vertexbuffer[z]);
			glBindBuffer(GL_ARRAY_BUFFER, terrain_Vertexbuffer[z]);

			for(int x = 0; x < terrain->getWidth(); x++)
			{
				vertexArray.push_back(x);
				vertexArray.push_back(terrain->getHeight(x, z));
				vertexArray.push_back(z);

				vertexArray.push_back(x);
				vertexArray.push_back(terrain->getHeight(x, z + 1));
				vertexArray.push_back(z + 1);
			}

			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexArray.size(), &vertexArray.front(), GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);

			glGenBuffers(1, &terrain_Normalbuffer[z]);
			glBindBuffer(GL_ARRAY_BUFFER, terrain_Normalbuffer[z]);

			for(int x = 0; x < terrain->getWidth(); x++)
			{
				vec3 normal = terrain->getNormal(x, z);
				normalArray.push_back(normal.x);
				normalArray.push_back(normal.y);
				normalArray.push_back(normal.z);

				normal = terrain->getNormal(x, z + 1);
				normalArray.push_back(normal.x);
				normalArray.push_back(normal.y);
				normalArray.push_back(normal.z);
			}

			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normalArray.size(), &normalArray.front(), GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);

			glBindVertexArray(0);
		}
		//Initialize camera
		eye = vec3(0.0, -0.5, -1.0);
		up = vec3(0.0, 1.0, 0.0);
	 }


	void calcCam()
	{
		float radius = 0.5;
		x = radius * cos(cPhi) * cos(cTheta);
		y = radius * sin(cPhi);
		z = radius * cos(cPhi) * sin(cTheta);
		center = eye + vec3(x, y, z);
	}

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Leave this code to just draw the meshes alone */
		float aspect = width/(float)height;

		// Create the matrix stacks
		auto P = make_shared<MatrixStack>();
		auto MV = make_shared<MatrixStack>();
		// Apply perspective projection.
		P->pushMatrix();
		P->perspective(45.0f, aspect, 0.01f, 100.0f);

		MV->pushMatrix();
			MV->loadIdentity();
			// globl transforms for 'camera'
			calcCam();
			MV->translate(vec3(0.0, 0.0, 0.0));

			MV->pushMatrix();
				MV->scale(0.05);
				prog->bind();
				glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()) );
				glUniformMatrix4fv(prog->getUniform("View"), 1, GL_FALSE, value_ptr(lookAt(eye, center, up)));
				SetMaterial(1);
				for(int z = 0; z < terrain->getLength() - 1; z++)
				{
					glBindVertexArray(terrain_VertexArrayID[z]);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexArray.size());
				}
			prog->unbind();
			MV->popMatrix();

		MV->popMatrix();
		P->popMatrix();
	}

	// helper function to set materials for shading
	void SetMaterial(int i)
	{
		switch (i)
		{
		case 0: //shiny blue plastic
			glUniform3f(prog->getUniform("MatAmb"), 0.02f, 0.04f, 0.2f);
			glUniform3f(prog->getUniform("MatDif"), 0.0f, 0.16f, 0.9f);
			break;
		case 1: // flat grey
			glUniform3f(prog->getUniform("MatAmb"), 0.13f, 0.13f, 0.14f);
			glUniform3f(prog->getUniform("MatDif"), 0.3f, 0.3f, 0.4f);
			break;
		case 2: //brass
			glUniform3f(prog->getUniform("MatAmb"), 0.3294f, 0.2235f, 0.02745f);
			glUniform3f(prog->getUniform("MatDif"), 0.7804f, 0.5686f, 0.11373f);
			break;
		case 3: //copper
			glUniform3f(prog->getUniform("MatAmb"), 0.1913f, 0.0735f, 0.0225f);
			glUniform3f(prog->getUniform("MatDif"), 0.7038f, 0.27048f, 0.0828f);
			break;
		}
	}

};

int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
			resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(512, 512);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}

// glUniformMatrix4fv(prog->getUniform("View"), 1, GL_FALSE, value_ptr(lookAt(eye, center, up)));
//
//
// /* draw left mesh */
// MV->pushMatrix();
// MV->translate(vec3(-2, 0.f, -5));
// MV->rotate(radians(-90.f), vec3(1, 0, 0));
// MV->scale(gScale);
// MV->translate(-1.0f*gTrans);
// SetMaterial(2);
// glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()) );
// Nef->draw(prog);
// MV->popMatrix();
//
// for(unsigned int i = 0; i < AllShapes.size(); i++)
// {
// MV->pushMatrix();
// MV->translate(vec3(0.0, 0.0, -5.0));
// MV->rotate(radians(-90.f), vec3(1, 0, 0));
// MV->scale(gXScale);
// MV->translate(-1.0f*gXTrans);
// SetMaterial(1);
// MV->pushMatrix();
// MV->rotate(radians(-90.f), vec3(0.0, 0.0, 1.0));
// glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()) );
// AllShapes[i]->draw(prog);
// MV->popMatrix();
// MV->popMatrix();
// }
// MV->popMatrix();
// prog->unbind();
//
//
//
// MV->pushMatrix();
// MV->loadIdentity();
// MV->rotate(radians(cTheta), vec3(0, 1, 0));
//
// texProg2->bind();
// glUniformMatrix4fv(texProg2->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
// glUniformMatrix4fv(texProg2->getUniform("View"), 1, GL_FALSE, value_ptr(lookAt(eye, center, up)));
// /* draw right mesh */
// MV->pushMatrix();
// MV->translate(vec3(2, 0.f, -5));
// MV->scale(gDScale);
// MV->translate(-1.0f*gDTrans);
// MV->pushMatrix();
// MV->rotate(theta, vec3(0.0, 1.0, 0.0));
// MV->rotate(theta * 0.5, vec3(0.0, 0.0, 1.0));
// glUniformMatrix4fv(texProg2->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()) );
// texture1->bind(texProg2->getUniform("Texture0"));
// world->draw(texProg2);
// MV->popMatrix();
// MV->popMatrix();
// texProg2->unbind();
//
// texProg->bind();
// glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
// glUniformMatrix4fv(texProg->getUniform("View"), 1, GL_FALSE, value_ptr(lookAt(eye, center, up)));
//
// /* draw righter mesh */
// MV->pushMatrix();
// MV->translate(vec3(2, 0.f, 5));
// MV->scale(gDScale);
// MV->translate(-1.0f*gDTrans);
// MV->pushMatrix();
// MV->rotate(theta, vec3(0.0, 1.0, 0.0));
// MV->rotate(theta * 0.5, vec3(0.0, 0.0, 1.0));
// glUniformMatrix4fv(texProg->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()) );
// texture3->bind(texProg->getUniform("Texture0"));
// world->draw(texProg);
// MV->popMatrix();
// MV->popMatrix();
//
// /*draw the ground */
// glUniformMatrix4fv(texProg->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()) );
// texture2->bind(texProg->getUniform("Texture0"));
// renderGround();
