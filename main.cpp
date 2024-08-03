#include "glad/glad.h"
#include "UseImGui.h"
#include "GLFW/glfw3.h"
#include "engine/ShaderCode.h"

#include <GL/gl.h>
#include <iostream>
#include <ostream>

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


int main() {
    std::cout << "Starting game!\n";

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


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    ShaderCode *t = new ShaderCode("./shaders/fragShader.glsl");
    GLuint fragShader = t->createShader(GL_FRAGMENT_SHADER);
    delete t;
    ShaderCode *a = new ShaderCode("./shaders/vertShader.glsl");
    GLuint vertShader = a->createShader(GL_VERTEX_SHADER);
    delete a;

    std::cout << "Creating OpenGL program..." << std::endl;

    GLuint program = glCreateProgram();
    glAttachShader(program, fragShader);
    glAttachShader(program, vertShader);

    glLinkProgram(program);

    glDetachShader(program, fragShader);
    glDetachShader(program, vertShader);
    glDeleteShader(fragShader);
    glDeleteShader(vertShader);

    glUseProgram(program);

    GLint params;
    glGetProgramiv(program, GL_LINK_STATUS, &params);

    std::cout << "Program linked " << (params == GL_TRUE ? "Success" : "Failed") << "!" << std::endl;
    // TODO Handle when program fails to link
    //
    GLuint *buffers = (GLuint *) malloc(sizeof(GLuint *) * RENDER_DATA_BUFFERS);
    /*glCreateBuffers(RENDER_DATA_BUFFERS, buffers);*/
    GLuint VAO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(RENDER_DATA_BUFFERS, buffers);

    // Dealing with verticies
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), rectPoints, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glEnableVertexAttribArray(0);


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);

        // TODO Render square over screen here
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);


        myimgui.Update();
        myimgui.Render();

        // TODO In here is where I can start writing any OpenGL code that I want to be able able to render the game

        glfwSwapBuffers(window);
    }
    myimgui.Shutdown();

    return 0;
}
