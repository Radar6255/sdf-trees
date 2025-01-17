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
#include "lib/semaphore.h"

#include <GL/gl.h>
#include <GL/glext.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <glm/ext/vector_float3.hpp>
#include <imgui_impl_glfw.h>
#include <iostream>
#include <ostream>
#include <sys/types.h>
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
    if (type != GL_DEBUG_TYPE_ERROR) {
        return;
    }
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


    // BEGIN TESTING
    intVec3 dim;
    dim.x = 100;
    dim.y = 50;
    dim.z = 100;

    intVec3 cellDim;
    cellDim.x = 40;
    cellDim.y = 20;
    cellDim.z = 40;

    SpatialHash leafPoints(dim, cellDim);

    // Now we need to populated all of the points
    for (int i = 0; i < 100000; i++) {
        // Generating random points from 0 - dim in each axis
        glm::vec3 pos = {
            dim.x * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)), 
            dim.y * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)),
            dim.z * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
        };

        leafPoints.AddChild(pos);
    }

    std::vector<glm::vec3> currTreeLeaves = leafPoints.GetNearPoints(glm::vec3(50, 20, 50), 10);
    std::cout << "Num tree leaves: " << currTreeLeaves.size() << std::endl;

    for (glm::vec3 lp: currTreeLeaves) {
        std::cout << "Pos: ("<<lp.x<<", "<<lp.y<<", "<<lp.z<<")"<<std::endl;
    }

    Tree t(glm::vec3(50, 5, 50), currTreeLeaves);

    std::vector<glm::vec3>* branches = t.GetBranches();

    for (glm::vec3 bPos : *branches) {
        std::cout << "Branch pos: (" << bPos.x << ", " << bPos.y << ", " << bPos.z << ")" << std::endl;
    }
    /*return 0;*/
    // END TESTING


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


    Program *compShaderProg = new Program("./shaders/compShader.glsl");
    Program *testRenderProgram = new Program("./shaders/testFragShader.glsl", "./shaders/testVertShader.glsl");

    glUseProgram(compShaderProg->program);

    GLint max;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max);
    std::cout << "Max invocations: " << max << std::endl;

    glGetIntegerv(GL_MAX_COMPUTE_ATOMIC_COUNTERS, &max);
    std::cout << "Max atomic counters: " << max << std::endl;

    glGetIntegerv(GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS, &max);
    std::cout << "Max atomic counter buffers: " << max << std::endl;

    glGetIntegerv(GL_MAX_COMBINED_ATOMIC_COUNTERS, &max);
    std::cout << "Max combined atomic counters: " << max << std::endl;

    glGetIntegerv(GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS, &max);
    std::cout << "Max combined atomic counter buffers: " << max << std::endl;

    /*glGetIntegerv(GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS, &max);*/
    /*std::cout << "Max atomic counter buffers: " << max << std::endl;*/

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
    gd.recentUpdateRate = 0;
    gd.updateRate = 0;
    gd.recentRenderTime = 0;
    gd.updateTerrain = true;

    int treeIter = 10;
    uint treeI = 0;

    uint terrainFinishedCounter = 0;
    semaphore s;

    int i = 0;
    for (int ti = 0; ti < numTerrains; ti++) {
        Terrain *t = world[ti];

        t->alterSize = gd.alterSize;
        t->treeChanceThresh = gd.treeChanceThresh;

        t->Update();

        // Here we are updating the mesh that we made
        t1[i] = std::thread([&updatedTerrain, &window, &s, t] {
            /*std::cout << "Starting update..." << std::endl;*/
            while (!glfwWindowShouldClose(window)) {
                t->UpdateTerrain();
                //updatedTerrain++;

                s.acquire();
            }
        });
        i++;
    }

    while (!glfwWindowShouldClose(window)) {
        std::chrono::time_point<std::chrono::high_resolution_clock> startFrame = std::chrono::high_resolution_clock::now();

        if (shortFrameCount == 600) {
            std::cout << "Recent average frame time: " << shortFrameTime / 600 << std::endl;
            shortFrameTime = 0;
            shortFrameCount = 0;
        }

        if (gd.updateTerrain && s.get_count() == numTerrains) {
            if (shortUpdateCount == 100) {
                shortUpdateCount = 0;
                lastShortUpdate = glfwGetTime();
            }
            for (int ti = 0; ti < numTerrains; ti++) {
                Terrain *t = world[ti];

                t->alterSize = gd.alterSize;
                t->treeChanceThresh = gd.treeChanceThresh;

                t->Update();
            }
            shortUpdateCount++;

            s.reset();

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

    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            world[x + y * size]->destroy();
        }
    }

    s.reset();
    for (std::thread &t : t1) {
        t.join();
    }

    myimgui.Shutdown();

    return 0;
}
