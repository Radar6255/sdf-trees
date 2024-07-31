#include "glad/glad.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <string>

class ShaderCode {
public:
  ShaderCode(const char *filename);
  GLuint createShader(GLenum shaderType);

private:
  std::string code;
};
