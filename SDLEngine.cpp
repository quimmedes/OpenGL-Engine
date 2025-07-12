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
GLuint mvLoc, pLoc, vLoc, uColorLoc;
GLuint tfLoc;
float aspect, timeFactor = 0.0f;
glm::mat4 pMat, vMat, tMat, rMat, mMat, mvMat;
float cubeLocX, cubeLocY, cubeLocZ;

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) // Fixed: SDL3 returns true on success
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
                if (!SDL_GL_SetSwapInterval(1)) // Fixed: Enable vsync
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

// Default vertex shader if file doesn't exist
const char* defaultVertexShader = R"(
#version 410
layout (location = 0) in vec3 position;

uniform mat4 mv_matrix;
uniform mat4 p_matrix;

void main()
{
    gl_Position = p_matrix * mv_matrix * vec4(position, 1.0);
}
)";

// Default fragment shader if file doesn't exist
const char* defaultFragmentShader = R"(
#version 410
uniform vec4 uColor;
out vec4 outColor;

void main()
{
    outColor = uColor;
}
)";

char* readShaderSource(const char* filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filename << ", using default shader\n";
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string content = buffer.str();

    char* result = new char[content.size() + 1];
    std::copy(content.begin(), content.end(), result);
    result[content.size()] = '\0';

    return result;
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

    glGenVertexArrays(numVAOs, vao);
    glBindVertexArray(vao[0]);
    glGenBuffers(numVBOs, vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidPositions), pyramidPositions, GL_STATIC_DRAW);
}

GLuint createShaderProgram()
{
    const char* vertexShaderSrc = readShaderSource("defaultVertexShader.glsl");
    const char* fragShaderSrc = readShaderSource("defaultFragShader.glsl");

    // Use default shaders if files don't exist
    if (!vertexShaderSrc)
        vertexShaderSrc = defaultVertexShader;
    if (!fragShaderSrc)
        fragShaderSrc = defaultFragmentShader;

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
        if (vertexShaderSrc != defaultVertexShader)
            delete[] vertexShaderSrc;
        if (fragShaderSrc != defaultFragmentShader)
            delete[] fragShaderSrc;
        return 0;
    }

    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        printShaderLog(fShader);
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        if (vertexShaderSrc != defaultVertexShader)
            delete[] vertexShaderSrc;
        if (fragShaderSrc != defaultFragmentShader)
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
        if (vertexShaderSrc != defaultVertexShader)
            delete[] vertexShaderSrc;
        if (fragShaderSrc != defaultFragmentShader)
            delete[] fragShaderSrc;
        return 0;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    if (vertexShaderSrc != defaultVertexShader)
        delete[] vertexShaderSrc;
    if (fragShaderSrc != defaultFragmentShader)
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

    // Cache uniform locations
    pLoc = glGetUniformLocation(renderingProgram, "p_matrix");
    mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
    uColorLoc = glGetUniformLocation(renderingProgram, "uColor");

    // Build perspective matrix
    aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
    pMat = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 1000.0f);

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
    timeFactor += deltaTime;
}

glm::mat4 buildRotateZ(float rad) {
    glm::mat4 zrot = glm::mat4(cos(rad), -sin(rad), 0.0, 0.0,
        sin(rad), cos(rad), 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0);
    return zrot;
}

glm::mat4 buildRotateX(float rad) {
    glm::mat4 xrot = glm::mat4(1.0, 0.0, 0.0, 0.0,
        0.0, cos(rad), -sin(rad), 0.0,
        0.0, sin(rad), cos(rad), 0.0,
        0.0, 0.0, 0.0, 1.0);
    return xrot;
}

// builds and returns a matrix that performs a rotation around the Y axis
glm::mat4 buildRotateY(float rad) {
    glm::mat4 yrot = glm::mat4(cos(rad), 0.0, sin(rad), 0.0,
        0.0, 1.0, 0.0, 0.0,
        -sin(rad), 0.0, cos(rad), 0.0,
        0.0, 0.0, 0.0, 1.0);
    return yrot;
}

void render(float deltaTime)
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // Fixed: Clear both buffers at once
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if (!gRenderQuad)
        return;

    glUseProgram(renderingProgram);

    // Update view matrix
    vMat = glm::translate(glm::mat4(1.0f), -camera);

    // Bind VAO
    glBindVertexArray(vao[0]);

    // Draw cube
    mMat = glm::translate(glm::mat4(1.0f), cube);
    mvMat = vMat * mMat * buildRotateY(glm::radians(40.0f));

    glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
    glUniform4f(uColorLoc, 1.0f, 0.0f, 0.0f, 1.0f); // Red color for cube

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Draw pyramid
    mMat = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 1.0f, 1.0f)); // Fixed: Better positioning
    mvMat = vMat * mMat * buildRotateX(glm::radians(30.0f));

    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
    glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(pMat));
    glUniform4f(uColorLoc, 0.0f, 1.0f, 0.0f, 1.0f); // Green color for pyramid

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 18);

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
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        while (SDL_PollEvent(&e))
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