#pragma once

#include <GL/gl.h>

class Program {
public:
    Program(const char* fragShader, const char* vertShader);
    Program(const char* fragShader, const char* geoShader, const char* vertShader);

    GLuint program;
private:
    void Init(const char* fragShader, const char* geoShader, const char* vertShader);
};
