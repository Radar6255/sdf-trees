#version 430

layout (std430, binding = 0) buffer vertData {
    vec3 vertexPosition[];
};
//layout (location = 0) in vec3 vertexPosition;
//layout (location = 1) in vec3 vertexNormal;
//layout (location = 2) in vec3 offset;

//layout (binding = 3, std430) readonly buffer ssbo1 {
//    int modifiers[];
//}

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//out vec3 norm;

void main() {
    //gl_Position = vec4(vertexPosition, 1.0);
    gl_Position = projection * view * model * vec4(vertexPosition[gl_VertexID], 1.0);

    //gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
    //norm = (vec4(vertexNormal, 1.0)).xyz;
}
