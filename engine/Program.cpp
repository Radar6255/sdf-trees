#include <glad/glad.h>
#include <iostream>
#include <ostream>

#include "Program.h"
#include "ShaderCode.h"

Program::Program(const char* fragShaderStr, const char* vertShaderStr) {
    this->Init(fragShaderStr, NULL, vertShaderStr);
}

Program::Program(const char* fragShaderStr, const char* geoShaderStr, const char* vertShaderStr) {
    this->Init(fragShaderStr, geoShaderStr, vertShaderStr);
}

void Program::Init(const char* fragShaderStr, const char* geoShaderStr, const char* vertShaderStr) {
    // TODO Maybe move this out of here
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    ShaderCode *f = new ShaderCode(fragShaderStr);
    GLuint fragShader = f->createShader(GL_FRAGMENT_SHADER);
    delete f;

    GLuint geoShader = 0;
    if (geoShaderStr != NULL) {
        ShaderCode *g = new ShaderCode(geoShaderStr);
        geoShader = g->createShader(GL_GEOMETRY_SHADER);
        delete g;
    }

    ShaderCode *v = new ShaderCode(vertShaderStr);
    GLuint vertShader = v->createShader(GL_VERTEX_SHADER);
    delete v;


    std::cout << "Creating OpenGL program..." << std::endl;

    GLuint program = glCreateProgram();
    glAttachShader(program, fragShader);
    if (geoShader)
        glAttachShader(program, geoShader);
    glAttachShader(program, vertShader);

    glLinkProgram(program);

    // Cleaning up
    glDetachShader(program, fragShader);
    glDeleteShader(fragShader);

    if (geoShader) {
        glDetachShader(program, geoShader);
        glDeleteShader(geoShader);
    }

    glDetachShader(program, vertShader);
    glDeleteShader(vertShader);
    // End clean up

    /*glUseProgram(program);*/
    this->program = program;

    GLint params;
    glGetProgramiv(program, GL_LINK_STATUS, &params);

    std::cout << "Program linked " << (params == GL_TRUE ? "Success" : "Failed") << "!" << std::endl;
    // TODO Handle when program fails to link
    if (params != GL_TRUE) {
        exit(1);
    }
}
