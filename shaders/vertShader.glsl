#version 330

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 norm;

void main() {
    //gl_Position = vec4(vertexPosition, 1.0);
    norm = vertexNormal;
    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
}

