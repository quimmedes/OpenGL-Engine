/*This source code copyrighted by Lazy Foo' Productions 2004-2024
and may not be redistributed without written permission.*/

//Using SDL, SDL OpenGL, GLEW, and strings
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <GL/glew.h>
#include <SDL3/SDL_opengl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <numbers>

#define numVAOs 1
#define numVBOs 2


glm::vec3 camera;
glm::vec3 cube;

//Screen dimension constants
const int SCREEN_WIDTH = 1940;
const int SCREEN_HEIGHT = 1080;

//Starts up SDL, creates window, and initializes OpenGL
bool init();

//Initializes rendering program and clear color
bool initGL();

//Input handler
void handleKeys(unsigned char key, int x, int y);

//Per frame update
void update();

//Renders quad to the screen
void render();

//Frees media and shuts down SDL
void close();

//Shader loading utility programs
void printProgramLog(GLuint program);
void printShaderLog(GLuint shader);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

//Render flag
bool gRenderQuad = true;

//Graphics program
GLuint gProgramID = 0;
GLint gVertexPos2DLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;

GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
GLuint mvLoc, pLoc;
float aspect;
glm::mat4 pMat, vMat, tMat, rMat, mMat, mvMat;


bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_Log("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Use OpenGL 4.3 core
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
		if (gWindow == NULL)
		{
			SDL_Log("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext(gWindow);
			if (gContext == NULL)
			{
				SDL_Log("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize GLEW
				glewExperimental = GL_TRUE;
				GLenum glewError = glewInit();
				if (glewError != GLEW_OK)
				{
					SDL_Log("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
				}

				//Use Vsync
				if (!SDL_GL_SetSwapInterval(1))
				{
					SDL_Log("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
				}

				//Initialize OpenGL
				if (!initGL())
				{
					SDL_Log("Unable to initialize OpenGL!\n");
					success = false;
				}
			}
		}
	}

	return success;
}

 char* readShaderSource(const char* filename) {
	std::ifstream file(filename);
	if (!file) {
		std::cerr << "Failed to open file: " << filename << "\n";
		return nullptr;
	}

	std::stringstream buffer;
	buffer<<file.rdbuf();  // Read entire file into buffer

	std::string content = buffer.str();

	// Allocate a new C-string copy of the content
	char* result = new char[content.size() + 1];
	std::copy(content.begin(), content.end(), result);
	result[content.size()] = '\0';

	return result;  // Caller must delete[] the returned pointer
}

 // 36 vertices, 12 triangles, makes 2x2x2 cube placed at origin
 void setupVertices() {
	 float vertexPositions[108] = {
	-1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
	1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
	1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
	1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f
	 };

	 glGenVertexArrays(1, vao);
	 glBindVertexArray(vao[0]);
	 glGenBuffers(numVBOs, vbo);

	 glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);

 }

GLuint createShaderProgram() {
	const char* vertexShaderSrc = readShaderSource("vertShader.glsl");
	const char* fragShaderSrc = readShaderSource("fragShader.glsl");


	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vShader, 1, &vertexShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);
	glCompileShader(vShader);
	glCompileShader(fShader);

	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);
	glLinkProgram(vfProgram);

	delete[] vertexShaderSrc;
	delete[] fragShaderSrc;

	return vfProgram;
}
	

bool initGL()
{
	renderingProgram = createShaderProgram();
	camera.x = 0.0f;
	camera.y = 0.0f;
	camera.z = 8.0f;
	cube.x = 0.0f;
	cube.y = -2.0f;
	cube.z = 0.0f;

	glGenVertexArrays(numVAOs, vao);
	glBindVertexArray(vao[0]);
	setupVertices();

	return true;
}

void handleKeys(unsigned char key)
{
	//Toggle quad
	if (key == 'q')
	{
		gRenderQuad = !gRenderQuad;
	}
}

void update()
{
	//No per frame update needed
}

float x = 0.0f;
float inc = 0.01f;

float currentTime = 0.0f;

void render()
{
	currentTime += 0.01;
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(renderingProgram);
	// get the uniform variables for the MV and projection matrices
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	pLoc = glGetUniformLocation(renderingProgram, "p_matrix");
	// build perspective matrix
	aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	pMat = glm::perspective(60.0f * ((float)std::numbers::pi / 180.0f), aspect, 0.1f, 1000.0f); // 1.0472 radians = 60 degrees


	//build view matrix, model matrix and model-view matrix
	vMat = glm::translate(glm::mat4(1.0f), -camera);
	mMat = glm::translate(glm::mat4(2.0f), cube);

	for (int i = 0; i < 24; i++) {
		float tf = currentTime + i; // tf == "time factor", declared as type float
		tMat = glm::translate(glm::mat4(1.0f), glm::vec3(sin(.35f * tf) * 8.0f, cos(.52f * tf) * 8.0f,
			sin(.70f * tf) * 8.0f));
		rMat = glm::rotate(glm::mat4(1.0f), 1.75f * tf, glm::vec3(0.0f, 1.0f, 0.0f));
		rMat = glm::rotate(rMat, 1.75f * tf, glm::vec3(1.0f, 0.0f, 0.0f));
		rMat = glm::rotate(rMat, 1.75f * tf, glm::vec3(0.0f, 0.0f, 1.0f));
		mMat =  rMat * tMat;
		mvMat = vMat * mMat;
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 36);
	}

}

void close()
{
	//Deallocate program
	glDeleteProgram(gProgramID);

	//Destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

void printProgramLog(GLuint program)
{
	//Make sure name is shader
	if (glIsProgram(program))
	{
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log
		glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			//Print Log
			SDL_Log("%s\n", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		SDL_Log("Name %d is not a program\n", program);
	}
}

void printShaderLog(GLuint shader)
{
	//Make sure name is shader
	if (glIsShader(shader))
	{
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log
		glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			//Print Log
			SDL_Log("%s\n", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		SDL_Log("Name %d is not a shader\n", shader);
	}
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		SDL_Log("Failed to initialize!\n");
	}
	else
	{
		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;

		//Enable text input
		SDL_StartTextInput(gWindow);

		//While application is running
		while (!quit)
		{
			//Handle events on queue
			while (SDL_PollEvent(&e) != 0)
			{
				//User requests quit
				if (e.type == SDL_EVENT_QUIT)
				{
					quit = true;
				}
				//Handle keypress
				else if (e.type == SDL_EVENT_TEXT_INPUT)
				{
					handleKeys(e.text.text[0]);
				}
			}

			//Render quad
			render();

			//Update screen
			SDL_GL_SwapWindow(gWindow);
		}

		//Disable text input
		SDL_StopTextInput(gWindow);
	}

	//Free resources and close SDL
	close();

	return 0;
}
