#include "World/CustomModel.h"
#include "World/Tree.h"
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
#include <GL/glext.h>
#include <atomic>
#include <chrono>
#include <glm/ext/vector_float3.hpp>
#include <imgui_impl_glfw.h>
#include <iostream>
#include <ostream>
#include <thread>
#include <vector>

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
    Program *compShaderProg = new Program("./shaders/compShader.glsl");
    Program *testRenderProgram = new Program("./shaders/testFragShader.glsl", "./shaders/testVertShader.glsl");

    glUseProgram(compShaderProg->program);

    GLint max;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max);
    std::cout << "Max invocations: " << max << std::endl;

    int numTrees = 100;
    Tree* trees[numTrees];

    std::vector<float> inDataVec;
    for (int i = 0; i < numTrees; i++) {
        trees[i] = new Tree(50);

        std::vector<glm::vec3>* branches = trees[i]->GetBranches();

        inDataVec.push_back(branches->size() * 3 + 4);
        inDataVec.push_back(10 * i);
        inDataVec.push_back(0);
        inDataVec.push_back(0);

        for (glm::vec3 bPos : *branches) {
            inDataVec.push_back(bPos.x);
            inDataVec.push_back(bPos.y);
            inDataVec.push_back(bPos.z);
        }
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);

    GLuint ssboInData;
    glGenBuffers(1, &ssboInData);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboInData);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * inDataVec.size() * 2, inDataVec.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboInData);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    uint maxIndicies = 4000000;
    uint maxVerticies = 1000000;

    GLuint ssboOutIndicies;
    glGenBuffers(1, &ssboOutIndicies);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOutIndicies);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * maxIndicies, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboOutIndicies);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLuint ssboOutVerticies;
    glGenBuffers(1, &ssboOutVerticies);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOutVerticies);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3) * maxVerticies, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboOutVerticies);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLuint numIndicies = 0;

    GLuint indiciesCounterBuff;
    GLuint inCounter = 0;
    glGenBuffers(1, &indiciesCounterBuff);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, indiciesCounterBuff);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &inCounter, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 5, indiciesCounterBuff);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    GLuint vertCounterBuff;
    glGenBuffers(1, &vertCounterBuff);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, vertCounterBuff);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &inCounter, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, vertCounterBuff);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    std::cout << "Curr invocations: " << 5 * 10 * numTrees * 5 << std::endl;
    glDispatchCompute(5, 10 * numTrees, 5);
    /*glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);*/

    glUseProgram(testRenderProgram->program);
    glBindVertexArray(vao);

    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOut);*/
    /*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboOut);*/

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOutVerticies);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboOutVerticies);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOutIndicies);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboOutIndicies);

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
    shaders.shaderList[COMP_SHADER] = compShaderProg;

    glEnable(GL_DEPTH_TEST);

    /*Cube *cube = new Cube();*/
    /*Terrain *terrain = new Terrain(&state, &shaders, 0, 0, 100, 100);*/
    /*Terrain *terrain2 = new Terrain(&state, &shaders, 100, 0, 100, 100);*/

    int width = 100;
    int length = 100;

    int size = 1;
    int chunkSize = 100;
    int numTerrains = size * size;

    Terrain** world = new Terrain*[size * size];

    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            world[x + y * size] = new Terrain(&state, &shaders, x * chunkSize, y * chunkSize, chunkSize, chunkSize);
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
    gd.treeChanceThresh = 3;
    gd.avgRenderTime = 0;
    gd.updateRate = 0;
    gd.recentRenderTime = 0;
    gd.updateTerrain = true;

    int treeIter = 10;
    uint treeI = 0;

    while (!glfwWindowShouldClose(window)) {
        std::chrono::time_point<std::chrono::high_resolution_clock> startFrame = std::chrono::high_resolution_clock::now();

        if (gd.updateTerrain) {
            std::vector<float> inDataVec;
            for (int i = 0; i < numTrees; i++) {
                //trees[i] = new Tree(50);

                std::vector<glm::vec3>* branches = trees[i]->GetBranches();

                inDataVec.push_back(branches->size() * 3 + 4);
                inDataVec.push_back(10 * i);
                inDataVec.push_back(0);
                inDataVec.push_back(0);

                for (glm::vec3 bPos : *branches) {
                    inDataVec.push_back(bPos.x);
                    inDataVec.push_back(bPos.y);
                    inDataVec.push_back(bPos.z);
                }
            }

            glUseProgram(compShaderProg->program);
            glUniform1i(7, treeI * numTrees / treeIter);

            if (treeI == 0) {
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboInData);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * inDataVec.size(), inDataVec.data());

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOutIndicies);
                glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, indiciesCounterBuff);
                GLuint vertNum = 0;
                glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &vertNum);

                glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, vertCounterBuff);
                glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &vertNum);
            }

            // Actually dispatching the compute shader
            glDispatchCompute(5, 10 * numTrees / treeIter, 5);
        }
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
            world[ti]->Render(&shaders);
        }
        // Waiting for compute shader to complete
        glUseProgram(compShaderProg->program);
        /*glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);*/

        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        // Getting the number of triangles to render from the atomic counter
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, indiciesCounterBuff);
        glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &numIndicies);
        /*std::cout << "Num indicies: " << numIndicies << std::endl;*/

        GLuint numVerts;
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, vertCounterBuff);
        glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &numVerts);
        /*std::cout << "Num verts: " << numVerts << std::endl;*/

        treeI++;
        if (treeIter == treeI) {
            treeI = 0;
        }

        //glUseProgram(testRenderProgram->program);
        //glBindVertexArray(vao);
        /*glDrawArrays(GL_POINTS, 0, 4000);*/


        //glDrawArrays(GL_TRIANGLES, 0, numVerticies / 2);
        //glDrawArrays(GL_TRIANGLES, 0, numIndicies);

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
