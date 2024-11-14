#pragma once

#include <GL/gl.h>

class Program {
public:
    Program(const char* fragShader, const char* vertShader);
    Program(const char* fragShaderStr, const char* geoShaderStr, const char* vertShaderStr);
    Program(const char* computeShaderStr);

    GLuint program;
private:
    void Init(const char* fragShader, const char* geoShader, const char* vertShader);
};

enum {
    TERRAIN_SHADER,
    TREE_SHADER,
    TEST_SHADER,
    NUM_SHADERS
};

class Shaders {
public:
    Program* shaderList[NUM_SHADERS];
};
