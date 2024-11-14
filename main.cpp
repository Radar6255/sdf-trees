#include "World/CustomModel.h"
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
#include <chrono>
#include <glm/ext/vector_float3.hpp>
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

    Program *terrainProgram = new Program("./shaders/fragShader.glsl", "./shaders/vertShader.glsl");
    glUseProgram(terrainProgram->program);
    Program *treeProgram = new Program("./shaders/treeFragShader.glsl", "./shaders/treeVertShader.glsl");


    // START TESTING
    Program *testProgram = new Program("./shaders/compShader.glsl");
    Program *testRenderProgram = new Program("./shaders/testFragShader.glsl", "./shaders/testVertShader.glsl");

    // TODO Here generate a buffer for testing...
    glm::vec3 data[2];

    int in = 0;
    data[0] = {0, 0, 0};
    data[1] = {0, 1, 0};

    glm::vec3 out[1000];

    glUseProgram(testProgram->program);
    GLuint outIndex = glGetUniformLocation(testProgram->program, "outIndex");
    glUniform1ui(outIndex, 0);

    /*GLuint ssbo;*/
    /*glGenBuffers(1, &ssbo);*/
    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);*/
    /*glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(data), data, GL_DYNAMIC_COPY);*/
    /*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);*/
    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/

    GLuint vao;
    glGenVertexArrays(1, &vao);

    GLuint ssboOut;
    glGenBuffers(1, &ssboOut);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOut);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(out), out, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboOut);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glm::vec3 outIndicies[20000];
    GLuint ssboOutIndicies;
    glGenBuffers(1, &ssboOutIndicies);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOutIndicies);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(outIndicies), outIndicies, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboOutIndicies);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    struct OutData {
        GLuint outIndex;
    } outData;
    outData.outIndex = 0;

    GLuint ssboOutData;
    glGenBuffers(1, &ssboOutData);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ssboOutData);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(struct OutData), &outData, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, ssboOutData);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(testRenderProgram->program);
    glBindVertexArray(vao);

    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOut);*/
    /*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboOut);*/

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOutIndicies);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboOutIndicies);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    /*glBindBuffer(GL_ARRAY_BUFFER, ssboOut);*/
    /*glBindBufferBase(GL_ARRAY_BUFFER, 0, ssboOut);*/
    /*glBindBuffer(GL_ARRAY_BUFFER, 0);*/

    //glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    // END TESTING


    Shaders shaders; // = {terrainProgram, treeProgram};
    shaders.shaderList[TERRAIN_SHADER] = terrainProgram;
    shaders.shaderList[TREE_SHADER] = treeProgram;
    shaders.shaderList[TEST_SHADER] = testRenderProgram;

    glEnable(GL_DEPTH_TEST);

    /*Cube *cube = new Cube();*/
    Terrain *terrain = new Terrain(&state, 0, 0, 100, 100);
    Terrain *terrain2 = new Terrain(&state, 100, 0, 100, 100);

    int width = 100;
    int length = 100;

    int size = 1;
    int chunkSize = 100;
    int numTerrains = size * size;

    Terrain** world = new Terrain*[size * size];

    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            world[x + y * size] = new Terrain(&state, x * chunkSize, y * chunkSize, chunkSize, chunkSize);
        }
    }

    /*CustomModel *cm = new CustomModel("assets/models/untitled.obj");*/

    cam = new Camera(&shaders, &state);

    // Registering key callbacks to handle input
    // Register in the key_callback function
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    std::thread t1[numTerrains];
    int iter = 0;
    std::atomic<short> updatedTerrain(numTerrains);
    int updates = 0;
    int frameCount = 0;
    unsigned int frameTime = 0;

    int shortFrameCount = 0;
    uint shortFrameTime = 0;

    int shortUpdateCount = 0;
    double lastShortUpdate = glfwGetTime();

    GuiData gd;
    gd.alterSize = 0.005;
    gd.treeChanceThresh = 0.4;
    gd.avgRenderTime = 0;
    gd.updateRate = 0;
    gd.recentRenderTime = 0;
    gd.updateTerrain = true;

    while (!glfwWindowShouldClose(window)) {
        std::chrono::time_point<std::chrono::high_resolution_clock> startFrame = std::chrono::high_resolution_clock::now();
        if (shortFrameCount == 600) {
            std::cout << "Recent average frame time: " << shortFrameTime / 600 << std::endl;
            shortFrameTime = 0;
            shortFrameCount = 0;
        }

        if (gd.updateTerrain && updatedTerrain == numTerrains) {
            // TODO Here I want to update the buffer
            if (iter) {
                for (std::thread &t: t1) {
                    t.join();
                }
                shortUpdateCount++;
            }

            if (shortUpdateCount == 100) {
                shortUpdateCount = 0;
                lastShortUpdate = glfwGetTime();
            }

            // This update is to update the buffer
            /*std::cout << "Updates per second: " << updates / glfwGetTime() << std::endl;*/
            /*if (frameCount) {*/
            /*    std::cout << "Avg FrameTime: " << frameTime / frameCount << std::endl;*/
            /*}*/

            updatedTerrain = 0;
            int i = 0;
            for (int ti = 0; ti < size * size; ti++) {
                Terrain *t = world[ti];

                t->alterSize = gd.alterSize;
                t->treeChanceThresh = gd.treeChanceThresh;

                t->Update();

                // Here we are updating the mesh that we made
                t1[i] = std::thread([&updatedTerrain, t] {
                    /*std::cout << "Starting update..." << std::endl;*/
                    t->UpdateTerrain();
                    updatedTerrain++;
                });
                i++;
            }
            updates++;
        }
        iter++;

        glUseProgram(terrainProgram->program);
        glfwPollEvents();
        cam->Update(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int ti = 0; ti < size * size; ti++) {
            /*world[ti]->Render(&shaders);*/
        }

        glUseProgram(testRenderProgram->program);
        glBindVertexArray(vao);
        glPointSize(3);
        /*glDrawArrays(GL_POINTS, 0, 4000);*/
        glDrawArrays(GL_TRIANGLES, 0, 4000);

        /*cm->Render();*/

        myimgui.Update(&gd);
        myimgui.Render();


        glfwSwapBuffers(window);
        uint t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startFrame).count();
        frameTime += t;
        /*std::cout << "Render time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startFrame).count() << std::endl;*/
        shortFrameTime += t;
        shortFrameCount++;
        frameCount++;
        gd.recentRenderTime = (float) shortFrameTime / shortFrameCount;
        gd.avgRenderTime = (float) frameTime / frameCount;
        gd.updateRate = updates / glfwGetTime();
        gd.recentUpdateRate = shortUpdateCount / (glfwGetTime() - lastShortUpdate);
    }
    myimgui.Shutdown();

    return 0;
}
