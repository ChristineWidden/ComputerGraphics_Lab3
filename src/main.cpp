/*
ZJ Wood CPE 471 Lab 3 base code
*/

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "WindowManager.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	Program prog;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexColorIDBox, IndexBufferIDBox;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;
		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);


		glGenBuffers(1, &VertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		GLfloat cube_vertices[] = {
			// front
			-1.0, -1.0,  1.0,
			1.0, -1.0,  1.0,
			1.0,  1.0,  1.0,
			-1.0,  1.0,  1.0,
			// back
			-1.0, -1.0, -1.0,
			1.0, -1.0, -1.0,
			1.0,  1.0, -1.0,
			-1.0,  1.0, -1.0,
		};
		//make it a bit smaller
		for (int i = 0; i < 24; i++)
			cube_vertices[i] *= 0.5;
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

		//color
		GLfloat cube_colors[] = {
			// front colors
			1.0, 0.0, 0.0,
			0.0, 1.0, 0.0,
			0.0, 0.0, 1.0,
			1.0, 1.0, 1.0,
			// back colors
			1.0, 0.0, 0.0,
			0.0, 1.0, 0.0,
			0.0, 0.0, 1.0,
			1.0, 1.0, 1.0,
		};
		glGenBuffers(1, &VertexColorIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexColorIDBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_colors), cube_colors, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort cube_elements[] = {
			// front
			0, 1, 2,
			2, 3, 0,
			// top
			1, 5, 6,
			6, 2, 1,
			// back
			7, 6, 5,
			5, 4, 7,
			// bottom
			4, 0, 3,
			3, 7, 4,
			// left
			4, 5, 1,
			1, 0, 4,
			// right
			3, 2, 6,
			6, 7, 3,
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);


		glBindVertexArray(0);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		
		prog.setVerbose(true);
		prog.setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog.init())
			{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //[SHADERBUG] ? breakpoint here!! if program hits that codeline, check the command line window, it will contain the line and place of the error in the shader file
			}
		prog.addUniform("P");
		prog.addUniform("V");
		prog.addUniform("M");
		prog.addAttribute("vertPos");
		prog.addAttribute("vertColor");
	}


	mat4 createTranslateMat(float x, float y, float z) {
		float matrix[4][4] = { 1, 0, 0, 0,
							0, 1, 0, 0,
							0, 0, 1, 0,
							x, y, z, 1 };

		return make_mat4x4(&(matrix[0][0]));
	}

	mat4 createRotationMatrixZ(float angle_z) {
		float c = cos(angle_z);
		float s = sin(angle_z);

		float matrix[4][4] = { c,	-1 * s,0,	0,
								s,	c,	0,	0,
								0,	0,	1,	0,
								0,	0,	0,	1 };
		return make_mat4x4(&(matrix[0][0]));
	};

	mat4 createRotationMatrixX(float angle_x) {
		float c = cos(angle_x);
		float s = sin(angle_x);

		float matrix[4][4] = { 1,	0,	0,	0,
								0,	c,	-1 * s,0,
								0,	s,	c,	0,
								0,	0,	0,	1 };
		return make_mat4x4(&(matrix[0][0]));
	};

	mat4 createRotationMatrixY(float angle_y) {
		float c = cos(angle_y);
		float s = sin(angle_y);

		float matrix[4][4] = { c,	0,	-1 * s,0,
								0,	1,	0,	0,
								s,	0,	c,	0,
								0,	0,	0,	1 };
		return make_mat4x4(&(matrix[0][0]));
	};

	mat4 createScaleMat(float x, float y, float z) {
		float matrix[4][4] = { x, 0, 0, 0,
								0, y, 0, 0,
								0, 0, z, 0,
								0, 0, 0, 1 };
		return make_mat4x4(&(matrix[0][0]));
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrices
		
		mat4 V, M, P; //View, Model and Perspective matrix
		V = mat4(1);
		M = mat4(1);


		/*
		mat4 T = translate(mat4(1), vec3(0, 0, -4));
		static float w = 0.05;
		//w += 0.05;
		w = w * 1.001;

		mat4 R = rotate(mat4(1), w, vec3(1, 1, 0));
		//mat4 S = scale(mat4(1), w * vec3(0.001, 0.001, 0.001));
		mat4 S = mat4(1);
		M = T * R * S;
		*/
		
		static float r = 0;
		//r += 0.05;


		

		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);		
		if (width < height)
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 100.0f);
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 100.0f); //so much type casting... GLM metods are quite funny ones

		
		// Draw the box using GLSL.
		prog.bind();

		//bind the cube's VAO:
		glBindVertexArray(VertexArrayID);
		
		//send the matrices to the shaders
		glUniformMatrix4fv(prog.getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog.getUniform("V"), 1, GL_FALSE, &V[0][0]);

		//set model matrix and draw the cube		
		//M = ...
		mat4 TBack, TSide, WR, R1, R2, S;

		/*
		T = createTranslateMat(0, 0, -4);
		R1 = createRotationMatrixX(0);
		R2 = createRotationMatrixY(r);
		S = createScaleMat(0.2, 1, 0.2);
		M = T * R1 * R2 * S;
		glUniformMatrix4fv(prog.getUniform("M"), 1, GL_FALSE, &M[0][0]);	
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
		*/
		static float wr = 0;
		wr += 0.01;
		WR = createRotationMatrixY(wr);
		TBack = createTranslateMat(0, 0, -4);
		
		TSide = createTranslateMat(-0.75, 0, 0);
		R1 = createRotationMatrixZ(0);
		R2 = createRotationMatrixY(r);
		S = createScaleMat(0.4, 1.7, 0.4);
		M = TBack * WR * TSide * R1 * R2 * S;
		glUniformMatrix4fv(prog.getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		TSide = createTranslateMat(-0.11, 0, 0);
		R1 = createRotationMatrixZ(0);
		R2 = createRotationMatrixY(r);
		S = createScaleMat(0.4, 1.7, 0.4);
		M = TBack * WR * TSide * R1 * R2 * S;
		glUniformMatrix4fv(prog.getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		TSide = createTranslateMat(0.5, 0, 0);
		R1 = createRotationMatrixZ(0);
		R2 = createRotationMatrixY(r);
		S = createScaleMat(0.4, 1.7, 0.4);
		M = TBack * WR * TSide * R1 * R2 * S;
		glUniformMatrix4fv(prog.getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		TSide = createTranslateMat(-0.31, 0, 0);
		R1 = createRotationMatrixZ(3.14159 * 0.5);
		R2 = createRotationMatrixY(r);
		S = createScaleMat(0.395, 0.7, 0.395);
		M = TBack * WR * TSide * R1 * R2 * S;
		glUniformMatrix4fv(prog.getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		glBindVertexArray(0);

		prog.unbind();

	}

};


mat4 createIdentityMat() {
	float matrix[4][4] = {1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 0,
						0, 0, 0, 1};
	

	return make_mat4x4(&(matrix[0][0]));
};





//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1200, 1000);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
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
