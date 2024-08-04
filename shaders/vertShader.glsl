#version 330

layout (location = 0) in vec3 vertexPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    //gl_Position = vec4(vertexPosition, 1.0);
    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
}

