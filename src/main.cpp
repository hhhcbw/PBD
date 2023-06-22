#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "cloth.h"
#include "sphere.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
unsigned int framebuffer;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// position
glm::vec4 lastPos;

// whether is dragging
bool dragging = false;

// view/projection transformations and their reverse
glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
glm::mat4 view = glm::lookAt(glm::vec3(1.3f, -0.3f, 1.2f), glm::vec3(0.7f, -0.45f, 0.5f), glm::vec3(-0.1f, 1.0f, -0.1f));
glm::mat4 projection_inverse = glm::inverse(projection);
glm::mat4 view_inverse = glm::inverse(view);

// world transformation
glm::mat4 model = glm::mat4(1.0f);

Cloth* cloth;
Sphere* sphere;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PBD", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	cloth = new Cloth(20, 20);
	sphere = new Sphere(0.2f, glm::vec3(0.f, -0.7f, -0.5f));

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader zprogram
	// ------------------------------------
	Shader colorShader("colors.vs", "colors.fs");
	Shader colorIDShader("colors.vs", "colorsID.fs");

	// framebuffer configuration
	// -------------------------
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// create a color attachment texture
	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// 1. bind to framebuffer and render sphere to set sphere id
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		
		// make sure we clear the framebuffer's content
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// be sure to activate shader when setting uniforms/drawing objects
		colorIDShader.use();

		colorIDShader.setMat4("projection", projection);
		colorIDShader.setMat4("view", view);
		colorIDShader.setMat4("model", model);

		// set default plot mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// render sphere
		sphere->Draw(colorIDShader);

		// 2. Bind back to default framebuffer and draw scene 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// be sure to activate shader when setting uniforms/drawing objects
		colorShader.use();

		colorShader.setMat4("projection", projection);
		colorShader.setMat4("view", view);
		colorShader.setMat4("model", model);

		// update the cloth, calculate new vertices' velocity, positon and collision
		cloth->update(deltaTime);

		// set wire as plot mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// render the cloth
		cloth->Draw(colorShader);

		// render the sphere
		sphere->Draw(colorShader);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (dragging == true)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		glm::vec4 Pos((float)xpos * 2.0f / SCR_WIDTH - 1.0f, 1.0f - (float)ypos * 2.0f / SCR_HEIGHT, lastPos.z, 1.0f);

		glm::vec4 WorldPos = view_inverse * projection_inverse * Pos;
		glm::vec4 WorldLastPos = view_inverse * projection_inverse * lastPos;
		glm::vec3 WorldDir3 = glm::vec3(WorldPos.x / WorldPos.w, WorldPos.y / WorldPos.w, WorldPos.z / WorldPos.w) - glm::vec3(WorldLastPos.x / WorldLastPos.w, WorldLastPos.y / WorldLastPos.w, WorldLastPos.z / WorldLastPos.w);
		sphere->update(WorldDir3);

		lastPos = Pos;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			std::cout << xpos << " " << ypos << std::endl;

			unsigned char pixel[3];
			float depth;
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			glReadPixels(xpos, SCR_HEIGHT - ypos, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
			glReadPixels(xpos, SCR_HEIGHT - ypos, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			lastPos = glm::vec4((float)xpos * 2.0f / SCR_WIDTH - 1.0f, 1.0f - (float)ypos * 2.0f / SCR_HEIGHT, depth * 2.0f - 1.0f, 1.0f);
			// pick the sphere
			if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0)
				dragging = true;
		}
		else if (action == GLFW_RELEASE)
		{
			dragging = false;
		}
	}
}