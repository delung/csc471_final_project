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
	GLuint terrain_VertexArrayID;
	GLuint terrain_Vertexbuffer;
	GLuint terrain_Normalbuffer;
	GLuint terrain_Indexbuffer;

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
	float moveSpd = 0.15f;

	double oldx;
	double oldy;

	bool mouseMode = true;


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		// else if (key == GLFW_KEY_M && action == GLFW_PRESS)
		// {
		// 	gMat = (gMat + 1) % 4;
		// }
		//
		// else if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
		// {
		// 	mouseMode = !mouseMode;
		// 	if(mouseMode)
		// 	{
		// 		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		// 	}
		// 	else
		// 		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		//
		// }
		// else
		// {
		// 	forwardVec = eye - center;
		// 	forwardVec = normalize(forwardVec);
		//
		// 	if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
		// 	{
		// 		vec3 rightVec = cross(up, forwardVec);
		// 		eye -= rightVec * moveSpd;
		// 		center -= rightVec * moveSpd;
		// 	}
		// 	else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
		// 	{
		// 		vec3 leftVec = cross(up, forwardVec);
		// 		eye += leftVec * moveSpd;
		// 		center += leftVec * moveSpd;
		// 	}
		// 	else if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
		// 	{
		// 		eye -= forwardVec * moveSpd;
		// 		center -= forwardVec * moveSpd;
		// 	}
		// 	else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
		// 	{
		// 		eye += forwardVec * moveSpd;
		// 		center += forwardVec * moveSpd;
		// 	}
		// }
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
		cTheta += (float) -deltaX;
	}

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		// double dX = 0;
		// double dY = 0;
		// if(mouseMode)
		// {
		// 	if(xpos - oldx > 0)
		// 		dX = -1;
		// 	else if(xpos - oldx < 0)
		// 		dX = 1;
		// 	if(ypos - oldy > 0)
		// 		dY = -1;
		// 	else if(ypos - oldy < 0)
		// 		dY = 1;
		//
		// 	cTheta += (float) -1 * dX * 3.14 / 720.0;
		// 	cPhi += (float) dY * 3.14 / 720.0;
		//
		// 	if(cPhi > 80.0 * 3.14 / 180)
		// 		cPhi = 80.0 * 3.14 / 180;
		// 	else if(cPhi <  -80.0 * 3.14 / 180)
		// 		cPhi = -80.0 * 3.14 / 180;
		//
		// 	oldx = xpos;
		// 	oldy= ypos;
		// }
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

	// Code to load in the three textures
	// void initTex(const std::string& resourceDirectory)
	// {
	//  	texture0 = make_shared<Texture>();
	// 	texture0->setFilename(resourceDirectory + "/crate.jpg");
	// 	texture0->init();
	// 	texture0->setUnit(0);
	// 	texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	//
	// 	texture1 = make_shared<Texture>();
	// 	texture1->setFilename(resourceDirectory + "/world.jpg");
	// 	texture1->init();
	// 	texture1->setUnit(1);
	// 	texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	//
	// 	texture3 = make_shared<Texture>();
	// 	texture3->setFilename(resourceDirectory + "/mars.jpg");
	// 	texture3->init();
	// 	texture3->setUnit(1);
	// 	texture3->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	//
	// 	texture2 = make_shared<Texture>();
	// 	texture2->setFilename(resourceDirectory + "/grass.jpg");
	// 	texture2->init();
	// 	texture2->setUnit(2);
	// 	texture2->setWrapModes(GL_REPEAT, GL_REPEAT);
	// }

	//code to set up the two shaders - a diffuse shader and texture mapping
	void init(const std::string& resourceDirectory)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		cTheta = 0;
		// Set background color.
		glClearColor(.5f, .05f, .05f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		// prog = make_shared<Program>();
		// prog->setVerbose(true);
		// prog->setShaderNames(
		// 	resourceDirectory + "/simple_vert.glsl",
		// 	resourceDirectory + "/simple_frag.glsl");
		// if (! prog->init())
		// {
		// 	std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
		// 	exit(1);
		// }
		// prog->addUniform("P");
		// prog->addUniform("MV");
		// prog->addUniform("View");
		// prog->addUniform("MatAmb");
		// prog->addUniform("MatDif");
		// prog->addAttribute("vertPos");
		// prog->addAttribute("vertNor");

		simp_prog = make_shared<Program>();
		simp_prog->setVerbose(true);
		simp_prog->setShaderNames(
			resourceDirectory + "/vert.glsl",
			resourceDirectory + "/frag.glsl");
		if (! simp_prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		simp_prog->addUniform("P");
		simp_prog->addUniform("MV");
		simp_prog->addAttribute("vertPos");

		//initialize the textures we might use
		// initTex(resourceDirectory);
		//
		// texProg = make_shared<Program>();
		// texProg->setVerbose(true);
		// texProg->setShaderNames(
		// 	resourceDirectory + "/tex_vert.glsl",
		// 	resourceDirectory + "/tex_frag0.glsl");
		// if (! texProg->init())
		// {
		// 	std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
		// 	exit(1);
		// }
 	// 	texProg->addUniform("P");
		// texProg->addUniform("MV");
		// texProg->addUniform("View");
		// texProg->addAttribute("vertPos");
		// texProg->addAttribute("vertNor");
		// texProg->addAttribute("vertTex");
		// texProg->addUniform("Texture0");
		//
		// texProg2 = make_shared<Program>();
		// texProg2->setVerbose(true);
		// texProg2->setShaderNames(
		// 	resourceDirectory + "/tex_vert.glsl",
		// 	resourceDirectory + "/tex_frag1.glsl");
		// if (! texProg2->init())
		// {
		// 	std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
		// 	exit(1);
		// }
 	// 	texProg2->addUniform("P");
		// texProg2->addUniform("MV");
		// texProg2->addUniform("View");
		// texProg2->addAttribute("vertPos");
		// texProg2->addAttribute("vertNor");
		// texProg2->addAttribute("vertTex");
		// texProg2->addUniform("Texture0");

		// Init Terrain
		// terrain = make_shared<Terrain>();
		// terrain->setFilename(resourceDirectory + "/heightmap.png");
		// terrain->init(1.0);
	 }

	 void initGeom(const std::string& resourceDirectory)
	 {
		glGenVertexArrays(1, &terrain_VertexArrayID);
		glBindVertexArray(terrain_VertexArrayID);

		glGenBuffers(1, &terrain_Vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, terrain_Vertexbuffer);
		static const GLfloat g_vertex_buffer_data[] =
		{
			-0.9f, -0.9f,  1.0f,
			 0.9f, -0.9f, -1.0f,
			 0.9f,  0.9f,  1.0f,
		 	-0.9f, 0.9f, -1.0f,
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

		glGenBuffers(1, &terrain_Indexbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_Indexbuffer);
		static const GLuint g_index_buffer_data[] =
		{
			0, 1, 2,
			0, 2, 3
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_index_buffer_data), g_index_buffer_data, GL_DYNAMIC_DRAW);
		glBindVertexArray(0);
		printf("RUN\n");
		// std::vector<GLfloat> vertexArray;
		// vertexArray.push_back(0.0);
		// vertexArray.push_back(0.0);
		// vertexArray.push_back(0.0);
		//
		// vertexArray.push_back(0.0);
		// vertexArray.push_back(1.0);
		// vertexArray.push_back(0.0);
		//
		// vertexArray.push_back(1.0);
		// vertexArray.push_back(0.0);
		// vertexArray.push_back(0.0);
		// for(int z = 0; z < terrain->getLength() - 1; z ++)
		// {
		// 	for(int x = 0; x < terrain->getWidth() - 1; x++)
		// 	{
		// 		vertexArray.push_back(x);
		// 		vertexArray.push_back(terrain->getHeight(x, z));
		// 		vertexArray.push_back(z);
		// 	}
		// }
		//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexArray.size(), &vertexArray.front(), GL_STATIC_DRAW);

		//
		//
		// glGenBuffers(1, &terrain_Normalbuffer);
		// glBindBuffer(GL_ARRAY_BUFFER, terrain_Normalbuffer);
		// std::vector<GLfloat> normalArray;
		//
		// normalArray.push_back(0.0);
		// normalArray.push_back(0.0);
		// normalArray.push_back(0.0);
		//
		// normalArray.push_back(0.0);
		// normalArray.push_back(1.0);
		// normalArray.push_back(0.0);
		//
		// normalArray.push_back(1.0);
		// normalArray.push_back(0.0);
		// normalArray.push_back(0.0);
		// // for(int z = 0; z < terrain->getLength() - 1; z ++)
		// // {
		// // 	for(int x = 0; x < terrain->getWidth() - 1; x++)
		// // 	{
		// // 		normalArray.push_back(terrain->getNormal(x, z).x);
		// // 		normalArray.push_back(terrain->getNormal(x, z).y);
		// // 		normalArray.push_back(terrain->getNormal(x, z).z);
		// // 	}
		// // }
		// glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normalArray.size(), &normalArray.front(), GL_DYNAMIC_DRAW);
		// glEnableVertexAttribArray(1);
		// glBindBuffer(GL_ARRAY_BUFFER, terrain_Normalbuffer);
		// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	 }

  /**
	void initGeom(const std::string& resourceDirectory)
	{
		// Load geometry
		// Some obj files contain material information.
		// We'll ignore them for this assignment.
		// this is the tiny obj shapes - not to be confused with our shapes
		vector<tinyobj::shape_t> TOshapes;
		vector<tinyobj::material_t> objMaterials;

		string errStr;
		//load in the mesh and make the shapes
		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr,
						(resourceDirectory + "/dummy.obj").c_str());

		if (!rc)
		{
			cerr << errStr << endl;
		}
		else
		{
			// some data to keep track of where our mesh is in space
			vec3 Gmin, Gmax;
			Gmin = vec3(std::numeric_limits<float>::max());
			Gmax = vec3(-std::numeric_limits<float>::max());
			for (size_t i = 0; i < TOshapes.size(); i++)
			{
				// TODO -- Initialize each mesh
				// 1. make a shared pointer
				shared_ptr<Shape> dummy;
				// 2. createShape for each tiny obj shape
				dummy = make_shared<Shape>();
				dummy->createShape(TOshapes[i]);
				// 3. measure each shape to find out its AABB
				dummy->measure();
				// 4. call init on each shape to create the GPU data
				dummy->init();
				// perform some record keeping to keep track of global min and max


				if(Gmin.x > dummy->min.x) Gmin.x = dummy->min.x;
				if(Gmin.y > dummy->min.y) Gmin.y = dummy->min.y;
				if(Gmin.z > dummy->min.z) Gmin.z = dummy->min.z;

				if(Gmax.x < dummy->max.x) Gmax.x = dummy->max.x;
				if(Gmax.y < dummy->max.y) Gmax.y = dummy->max.y;
				if(Gmax.z < dummy->max.z) Gmax.z = dummy->max.z;

				// Add the shape to AllShapes
				AllShapes.push_back(dummy);
			}

			// think about scale and translate....
			// based on the results of calling measure on each peice

			gXTrans = Gmin + 0.5f*(Gmax - Gmin);
			if (Gmax.x > Gmax.y && Gmax.x > Gmax.z)
			{
				gXScale = 2.0/(Gmax.x - Gmin.x);
			}
			else if (Gmax.y > Gmax.x && Gmax.y > Gmax.z)
			{
				gXScale = 2.0/(Gmax.y - Gmin.y);
			}
			else
			{
				gXScale = 2.0/(Gmax.z - Gmin.z);
			}
		}

		// now read in the sphere for the world
		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr,
						(resourceDirectory + "/sphere.obj").c_str());

		world = make_shared<Shape>();
		world->createShape(TOshapes[0]);
		world->measure();
		world->init();

		// compute its transforms based on measuring it
		gDTrans = world->min + 0.5f*(world->max - world->min);
		if (world->max.x >world->max.y && world->max.x > world->max.z)
		{
			gDScale = 2.0/(world->max.x-world->min.x);
		}
		else if (world->max.y > world->max.x && world->max.y > world->max.z)
		{
			gDScale = 2.0/(world->max.y-world->min.y);
		}
		else
		{
			gDScale = 2.0/(world->max.z-world->min.z);
		}

		// now read in the Nefertiti model
		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr,
						(resourceDirectory + "/Nefertiti-100K.obj").c_str());

		Nef = make_shared<Shape>();
		Nef->createShape(TOshapes[0]);
		Nef->measure();
		Nef->init();

		// compute its transforms based on measuring it
		gTrans = Nef->min + 0.5f * (Nef->max - Nef->min);
		if (Nef->max.x > Nef->max.y && Nef->max.x > Nef->max.z)
		{
			gScale = 2.0 / (Nef->max.x - Nef->min.x);
		}
		else if (Nef->max.y > Nef->max.x && Nef->max.y > Nef->max.z)
		{
			gScale = 2.0 / (Nef->max.y - Nef->min.y);
		}
		else
		{
			gScale = 2.0 / (Nef->max.z - Nef->min.z);
		}

		// Initialize the geometry to render a ground plane
		initQuad();

		// Initialize camera
		eye = vec3(0.0, 4.0, -5.0);
		up = vec3(0.0, 1.0, 0.0);
	}

	**/
	/**** geometry set up for ground plane *****/
	// void initQuad()
	// {
	// 	float g_groundSize = 20;
	// 	float g_groundY = -1.5;
	//
	// 	// A x-z plane at y = g_groundY of dim[-g_groundSize, g_groundSize]^2
	// 	float GrndPos[] = {
	// 		-g_groundSize, g_groundY, -g_groundSize,
	// 		-g_groundSize, g_groundY,  g_groundSize,
	// 		 g_groundSize, g_groundY,  g_groundSize,
	// 		 g_groundSize, g_groundY, -g_groundSize
	// 	};
	//
	// 	float GrndNorm[] = {
	// 		0, 1, 0,
	// 		0, 1, 0,
	// 		0, 1, 0,
	// 		0, 1, 0,
	// 		0, 1, 0,
	// 		0, 1, 0
	// 	};
	//
	//
	// 	float GrndTex[] = {
	// 		0, 0, // back
	// 		0, g_groundSize,
	// 		g_groundSize, g_groundSize,
	// 		g_groundSize, 0
	// 	};
	//
	// 	unsigned short idx[] = {0, 1, 2, 0, 2, 3};
	//
	// 	GLuint VertexArrayID;
	// 	//generate the VAO
	// 	glGenVertexArrays(1, &VertexArrayID);
	// 	glBindVertexArray(VertexArrayID);
	//
	// 	gGiboLen = 6;
	// 	glGenBuffers(1, &GrndBuffObj);
	// 	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
	// 	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);
	//
	// 	glGenBuffers(1, &GrndNorBuffObj);
	// 	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
	// 	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);
	//
	// 	glGenBuffers(1, &GrndTexBuffObj);
	// 	glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
	// 	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);
	//
	// 	glGenBuffers(1, &GIndxBuffObj);
	// 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
	// 	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
	// }
	//
	// void renderGround()
	// {
	//
	// 	glEnableVertexAttribArray(0);
	// 	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
	// 	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//
	// 	glEnableVertexAttribArray(1);
	// 	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
	// 	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//
	// 	glEnableVertexAttribArray(2);
	// 	glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
	// 	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//
	// 	// draw!
	// 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
	// 	glDrawElements(GL_TRIANGLES, gGiboLen, GL_UNSIGNED_SHORT, 0);
	//
	// 	glDisableVertexAttribArray(0);
	// 	glDisableVertexAttribArray(1);
	// 	glDisableVertexAttribArray(2);
	// }
	//
	// void renderTerrain()
	// {
	//
	// }

	// void calcCam()
	// {
	// 	float radius = 0.5;
	// 	x = radius * cos(cPhi) * cos(cTheta);
	// 	y = radius * sin(cPhi);
	// 	z = radius * cos(cPhi) * sin(cTheta);
	// 	center = eye + vec3(x, y, z);
	// }

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

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


		//Draw our scene - two meshes and ground plane
		simp_prog->bind();
		glUniformMatrix4fv(simp_prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));

		// globle transforms for 'camera' (you will likely wantt to fix this)
		MV->pushMatrix();
			MV->loadIdentity();
			MV->scale(vec3(2.0, 2.0, 2.0));
			// globl transforms for 'camera'
			//calcCam();
		  glUniformMatrix4fv(simp_prog->getUniform("MV"), 1, GL_FALSE,value_ptr(MV->topMatrix()) );
			//glUniformMatrix4fv(prog->getUniform("View"), 1, GL_FALSE, value_ptr(lookAt(eye, center, up)));
			glBindVertexArray(terrain_VertexArrayID);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			//glDrawArrays(GL_TRIANGLES, 0, 3);
			simp_prog->unbind();
			//----------------------------------------------------------------------------------------------

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
