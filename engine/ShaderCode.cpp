#include "ShaderCode.h"
#include <glad/glad.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>


ShaderCode::ShaderCode(const char* filename) {
    std::ifstream shaderFile;

    shaderFile.open(filename);
    if (!shaderFile.is_open()) {
        std::cout << "Unable to open the file (" << filename << ")!\n";
        exit(10);
    }

    std::string line;
    std::stringstream out;
    while (shaderFile) {
        std::getline(shaderFile, line);
        out << line << std::endl;
    }

    this->code = out.str();
}

GLuint ShaderCode::createShader(GLenum shaderType) {
    GLuint out = glCreateShader(shaderType);

    if (!out) {
        std::cerr << "OpenGL failed to create shader to use" << std::endl;
        exit(10);
    }

    const char *gl_code = this->code.c_str();
    glShaderSource(out, 1, &gl_code, NULL);
    glCompileShader(out);

    GLint compileStatus;
    glGetShaderiv(out, GL_COMPILE_STATUS, &compileStatus);
    std::cout << "Shader compile status " << (compileStatus == GL_TRUE ? "Success" : "Failed") << "!" << std::endl;

    return out;
}
