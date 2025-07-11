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
#include <vector>

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
void handleKeys(SDL_Scancode key);

//Per frame update
void update(float deltaTime);

//Renders quad to the screen
void render(float deltaTime);

//Frees media and shuts down SDL
void close();

//Shader loading utility programs
void printProgramLog(GLuint program);
void printShaderLog(GLuint shader);

//The window we'll be rendering to
SDL_Window* gWindow = nullptr;

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
GLuint mvLoc, pLoc, vLoc;
GLuint tfLoc;
float aspect, timeFactor;
glm::mat4 pMat, vMat, tMat, rMat, mMat, mvMat;
float cubeLocX, cubeLocY, cubeLocZ;

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) // Fixed: Correct check for SDL_Init failure
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
        gWindow = SDL_CreateWindow("SDL Tutorial", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if (gWindow == nullptr)
        {
            SDL_Log("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            //Create context
            gContext = SDL_GL_CreateContext(gWindow);
            if (gContext == nullptr)
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
                    success = false;
                }

                //Use Vsync
                if (!SDL_GL_SetSwapInterval(0))
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

char* readShaderSource(const char* filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filename << "\n";
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Read entire file into buffer
    file.close(); // Explicitly close the file

    std::string content = buffer.str();

    // Allocate a new C-string copy of the content
    char* result = new char[content.size() + 1];
    std::copy(content.begin(), content.end(), result);
    result[content.size()] = '\0';

    return result; // Caller must delete[] the returned pointer
}

void setupVertices()
{
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
        1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f };

    float pyramidPositions[54] =
    { -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, // front face
     1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // right face
     1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // back face
     -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, // left face
     -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, // base – left front
     1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f };



    glGenVertexArrays(1, vao);
    glBindVertexArray(vao[0]);
    glGenBuffers(numVBOs, vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidPositions), pyramidPositions, GL_STATIC_DRAW);
}

GLuint createShaderProgram()
{
    const char* vertexShaderSrc = readShaderSource("vertShader.glsl");
    const char* fragShaderSrc = readShaderSource("fragShader.glsl");

    if (!vertexShaderSrc || !fragShaderSrc)
    {
        SDL_Log("Failed to read shader source files.\n");
        return 0;
    }

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vShader, 1, &vertexShaderSrc, nullptr);
    glShaderSource(fShader, 1, &fragShaderSrc, nullptr);
    glCompileShader(vShader);
    glCompileShader(fShader);

    // Check for shader compilation errors
    GLint success;
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        printShaderLog(vShader);
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        delete[] vertexShaderSrc;
        delete[] fragShaderSrc;
        return 0;
    }
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        printShaderLog(fShader);
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        delete[] vertexShaderSrc;
        delete[] fragShaderSrc;
        return 0;
    }

    GLuint vfProgram = glCreateProgram();
    glAttachShader(vfProgram, vShader);
    glAttachShader(vfProgram, fShader);
    glLinkProgram(vfProgram);

    // Check for program linking errors
    glGetProgramiv(vfProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        printProgramLog(vfProgram);
        glDeleteProgram(vfProgram);
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        delete[] vertexShaderSrc;
        delete[] fragShaderSrc;
        return 0;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    delete[] vertexShaderSrc;
    delete[] fragShaderSrc;

    return vfProgram;
}

bool initGL()
{
    renderingProgram = createShaderProgram();
    if (renderingProgram == 0)
    {
        SDL_Log("Failed to create shader program.\n");
        return false;
    }

    // Initialize camera and cube positions
    camera = glm::vec3(0.0f, 0.0f, 8.0f);
    cube = glm::vec3(0.0f, -2.0f, 0.0f);

    // Initialize projection matrix
    aspect = static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT;
    pMat = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 1000.0f);

    // Cache uniform locations
    pLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
    vLoc = glGetUniformLocation(renderingProgram, "v_matrix");
    tfLoc = glGetUniformLocation(renderingProgram, "tf");

    setupVertices();

    return true;
}

void handleKeys(SDL_Scancode key)
{
    if (key == SDL_SCANCODE_Q)
    {
        gRenderQuad = !gRenderQuad;
    }
}

void update(float deltaTime)
{
    timeFactor += deltaTime; // Update time factor based on delta time
}

void render(float deltaTime)
{
    glClear(GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!gRenderQuad)
        return;

    glUseProgram(renderingProgram);

    // Update view matrix
    vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-camera.x, -camera.y, -camera.z));

    // Update model matrix
    mMat = glm::translate(glm::mat4(1.0f), cube);
    mvMat = vMat * mMat; // Model-view matrix

    // Pass uniforms to shader
    glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
    glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(mvMat)); // Use model-view matrix
    glUniform1f(tfLoc, timeFactor);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    for(int i=0;i<10000;i++)
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36,10000);

    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error in render: " << err << std::endl;
    }
}

void close()
{
    // Deallocate OpenGL resources
    glDeleteProgram(renderingProgram);
    glDeleteVertexArrays(numVAOs, vao);
    glDeleteBuffers(numVBOs, vbo);

    // Destroy window
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;

    // Quit SDL subsystems
    SDL_Quit();
}

void printProgramLog(GLuint program)
{
    if (glIsProgram(program))
    {
        int infoLogLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            std::vector<char> infoLog(infoLogLength);
            glGetProgramInfoLog(program, infoLogLength, nullptr, infoLog.data());
            SDL_Log("Program Log: %s\n", infoLog.data());
        }
    }
    else
    {
        SDL_Log("Name %d is not a program\n", program);
    }
}

void printShaderLog(GLuint shader)
{
    if (glIsShader(shader))
    {
        int infoLogLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            std::vector<char> infoLog(infoLogLength);
            glGetShaderInfoLog(shader, infoLogLength, nullptr, infoLog.data());
            SDL_Log("Shader Log: %s\n", infoLog.data());
        }
    }
    else
    {
        SDL_Log("Name %d is not a shader\n", shader);
    }
}

int main(int argc, char* args[])
{
    if (!init())
    {
        SDL_Log("Failed to initialize!\n");
        return 1;
    }

    bool quit = false;
    SDL_Event e;
    Uint32 lastTime = SDL_GetTicks();

    while (!quit)
    {
        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f; // Convert to seconds
        lastTime = currentTime;

        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_EVENT_KEY_DOWN)
            {
                handleKeys(e.key.scancode);
            }
        }

        update(deltaTime);
        render(deltaTime);
        SDL_GL_SwapWindow(gWindow);
    }

    close();
    return 0;
}