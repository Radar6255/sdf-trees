#include "glad/glad.h"
#include "UseImGui.h"
#include "GLFW/glfw3.h"
#include "engine/ShaderCode.h"
#include "engine/Program.h"
#include "World/Cube.h"
#include "World/Terrain.h"
#include "engine/Camera.h"
#include "GameState.h"

#include <GL/gl.h>
#include <atomic>
#include <imgui_impl_glfw.h>
#include <iostream>
#include <ostream>
#include <thread>

#define RENDER_DATA_BUFFERS 1

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
    fprintf( stderr, "----OpenGL %s message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "status" ),
             message );

    // On error we want to find the current stack trace so we can figure out what is the cause
//    if (type == GL_DEBUG_TYPE_ERROR){
//        // Print the current stack trace. Taken from https://stackoverflow.com/questions/105659/how-can-one-grab-a-stack-trace-in-c
//        void* callstack[128];
//        int frames = backtrace(callstack, 128);
//        char** strs = backtrace_symbols(callstack, frames);
//        for (int i = 0; i < frames; i++){
//            fprintf(stderr, "%s\n", strs[i]);
//        }
//        free(strs);
//    }
}

static float rectPoints[6][3] = {
    {-0.5f, -0.5f, 1.0f},
    {0.5f, -0.5f, 1.0f},
    {-0.5f, 0.5f, 1.0f},
    {0.5f, -0.5f, 1.0f},
    {0.5f, 0.5f, 1.0f},
    {-0.5f, 0.5f, 1.0f}
};

// This is the camera, defined here so that we can handle keypresses
Camera *cam;

// Register anything that requires keypresses here
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    cam->HandleKeypress(window, key, scancode, action, mods);
}

// Register anything that requires keypresses here
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    cam->HandleMouseMovement(window, xpos, ypos);
}

int main() {
    std::cout << "Starting game!\n";

    GameState state;
    state.paused = false;

    if (!glfwInit()) {
        std::cerr << "Failed to initalize GLFW!\n";
        return 1;
    }

	// GL 3.0 + GLSL 130
	const char *glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

	// Create window with graphics context
	GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui - Example", NULL, NULL);

    if (window == NULL) {
        std::cerr << "Failed to initalize window!\n";
		return 1;
    }

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))  // tie window context to glad's opengl funcs
		throw("Unable to context to OpenGL");

	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);
	glViewport(0, 0, screen_width, screen_height);

    UseImGui myimgui;

    myimgui.Init(window, glsl_version);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, NULL);

    Program *program = new Program("./shaders/fragShader.glsl", "./shaders/vertShader.glsl");
    glUseProgram(program->program);

    glEnable(GL_DEPTH_TEST);

    Cube *cube = new Cube();
    Terrain *terrain = new Terrain(&state);
    cam = new Camera(program, &state);

    // Registering key callbacks to handle input
    // Register in the key_callback function
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    std::thread t1;
    int iter = 0;
    std::atomic<bool> updatedTerrain(true);
    int updates = 0;
    bool updateTerrain = true;

    while (!glfwWindowShouldClose(window)) {
        if (updateTerrain && updatedTerrain) {
            // TODO Here I want to update the buffer
            if (iter) {
                t1.join();
            }

            // This update is to update the buffer
            std::cout << "Updates per second: " << updates / glfwGetTime() << std::endl;
            terrain->Update();

            updatedTerrain = false;
            // Here we are updating the mesh that we made
            t1 = std::thread([&updatedTerrain, &terrain] {
                terrain->UpdateTerrain();
                /*std::cout << "Updated terrain mesh!" << std::endl;*/
                updatedTerrain = true;
            });
            updates++;
        }
        iter++;

        glfwPollEvents();
        cam->Update(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*cube->Render();*/
        terrain->Render();

        myimgui.Update(terrain, &updateTerrain);
        myimgui.Render();


        glfwSwapBuffers(window);
    }
    myimgui.Shutdown();

    return 0;
}
