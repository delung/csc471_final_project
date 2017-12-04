/**
 * Base code
 * Draws two meshes and one ground plane, one mesh has textures, as well
 * as ground plane.
 * Must be fixed to load in mesh with multiple shapes (dummy.obj)
 */

#include <iostream>
#include <glad/glad.h>
#include <string>

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
	std::shared_ptr<Program> prog;

	// terrain vertex
	std::vector<GLuint> terrain_VertexArrayID;
	std::vector<GLuint> terrain_Vertexbuffer;
	std::vector<GLuint> terrain_Normalbuffer;

	// Cursor controls
	float cursorX;
	float cursorZ;

	std::vector<GLfloat> vertexArray;
	std::vector<GLfloat> normalArray;

	shared_ptr<Terrain> terrain;

	float theta = 0;
	float cTheta = 0;
	float cPhi = -.5;

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

	bool mouseMode = false;
	float hScale = 100;
	int hRadius = 10;
	int maps = 3;
	int mapCurr = 0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		else if (key == GLFW_KEY_M && action == GLFW_PRESS)
		{
			if(++mapCurr >= maps)
				mapCurr = 0;
			setMap(mapCurr);
		}

		else if (key == GLFW_KEY_V && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			mouseMode = !mouseMode;
			if(mouseMode)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		}
		else if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
				hRadius--;
		}
		else if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
				hRadius++;
		}
		else if (mouseMode)
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
		else if(!mouseMode)
		{
			if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				if(++cursorX > terrain->getWidth() - 1)
					cursorX = terrain->getWidth() - 1;

			}
			else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				if(--cursorX < 0)
					cursorX = 0;
			}
			else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				if(--cursorZ < 0)
					cursorZ = 0;
			}
			else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				if(++cursorZ > terrain->getLength() - 1)
					cursorZ = terrain->getLength() - 1;
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
		if ((action == GLFW_PRESS || action == GLFW_REPEAT) && button == GLFW_MOUSE_BUTTON_LEFT)
		{
			incHeight();
		}

		if ((action == GLFW_PRESS || action == GLFW_REPEAT) && button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			decHeight();
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
		prog->addUniform("Cursor");
		prog->addUniform("Radius");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");


		// Init Terrain
		terrain = make_shared<Terrain>();
		terrain->setFilename(resourceDirectory + "/heightmap0.png");
		terrain->init(hScale);
	 }

	 void setMap(int id)
	 {
		terrain->setFilename("../resources/heightmap" + to_string(id) + ".png");
		terrain->init(hScale);
		updateGeom();
		cursorX = terrain->getWidth() / 2.0;
		cursorZ = terrain->getLength() / 2.0;
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
				//printf("< %d, %f, %d >\n", x, terrain->getHeight(x, z), z);
				vertexArray.push_back(x);
				vertexArray.push_back(terrain->getHeight(x, z + 1));
				vertexArray.push_back(z + 1);
				//printf("< %d, %f, %d >\n", x, terrain->getHeight(x, z + 1), z + 1);


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
		// Initialize camera
		eye = vec3(0.0, 0.0, -1.0);
		up = vec3(0.0, 1.0, 0.0);

		// Initialize Cursor
		cursorX = terrain->getWidth() / 2.0;
		cursorZ = terrain->getLength() / 2.0;
	 }

	 void updateGeom()
	 {
	  for(int z = 0; z < terrain->getLength() - 1; z++)
	  {
	 	 vertexArray.clear();
	 	 normalArray.clear();

		 glGenVertexArrays(1, &terrain_VertexArrayID[z]);
		 glBindVertexArray(terrain_VertexArrayID[z]);

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

	}

	void calcCam()
	{
		float radius = 0.5;
		x = radius * cos(cPhi) * cos(cTheta);
		y = radius * sin(cPhi);
		z = radius * cos(cPhi) * sin(cTheta);
		center = eye + vec3(x, y, z);
	}

	void incHeight()
	{
		int r = (int) hRadius;
		int x, z;
		float inc = 0.05;
		x = (int) cursorX;
		z = (int) cursorZ;
		for(int i = x - r; i <= x + r; i++)
		{
			for(int j = z - r; j < z + r; j++)
			{
				if((i - x) * (i - x) + (j - z) * (j - z) <= r * r)
				{
					if(i >= 0 && i < terrain->getWidth() - 1 && j >= 0 && j < terrain->getLength() - 1)
					{
						terrain->setHeight(i, j, terrain->getHeight(i,j) + (hScale * inc));
					}

				}

			}
		}
		updateGeom();
	}

	void decHeight()
	{
		int r = (int) hRadius;
		int x, z;
		float inc = 0.05;
		x = (int) cursorX;
		z = (int) cursorZ;
		for(int i = x - r; i <= x + r; i++)
		{
			for(int j = z - r; j < z + r; j++)
			{
				if((i - x) * (i - x) + (j - z) * (j - z) <= r * r)
				{
					if(i >= 0 && i < terrain->getWidth() - 1 && j >= 0 && j < terrain->getLength() - 1)
					{
						terrain->setHeight(i, j, terrain->getHeight(i,j) - (hScale * inc));
					}
				}
			}
		}
		updateGeom();
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
			MV->translate(vec3(4.0, -6.0, -15.0));

			MV->pushMatrix();
				MV->scale(0.05);
				prog->bind();
				glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()) );
				glUniformMatrix4fv(prog->getUniform("View"), 1, GL_FALSE, value_ptr(lookAt(eye, center, up)));
				glUniform3f(prog->getUniform("Cursor"), cursorX, terrain->getHeight((int)cursorX, (int)cursorZ), cursorZ);
				glUniform1f(prog->getUniform("Radius"), hRadius);
				SetMaterial(1);
				for(int z = 0; z < terrain->getLength() - 1; z++)
				{
					glBindVertexArray(terrain_VertexArrayID[z]);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexArray.size() / 3.0);
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
