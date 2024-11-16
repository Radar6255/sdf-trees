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

out vec3 color;

void main() {
    //gl_Position = vec4(vertexPosition, 1.0);
    gl_Position = projection * view * model * vec4(vertexPosition[2 * gl_VertexID], 1.0);

    //color = vec3(0.2, 0.2, 0.2 + gl_Position.x);

    if (gl_VertexID % 3 == 0) {
        color = vec3(1, 0, 0);
    } else if (gl_VertexID % 3 == 1) {
        color = vec3(0, 1, 0);
    } else {
        color = vec3(0, 0, 1);
    }

    color = vertexPosition[2 * gl_VertexID + 1];

    //gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
    //norm = (vec4(vertexNormal, 1.0)).xyz;
}
