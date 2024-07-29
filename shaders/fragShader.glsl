#version 330
precision mediump float;

// Color that is the result of this shader
out vec4 fragColor;

in vec2 uvFrag;
in vec3 outNorm;
in vec3 outLightDir;

// uniform sampler2D tex0;

void main() {
    vec3 lightDir = vec3(0.3, 0.7, 0.3);
    //fragColor = vec4 (0.5 * norm.x, 0.5, 0.5 * norm.z, 1.0);
    //fragColor = vec4(outNorm, 1.0);
    vec3 color = vec3(0.8, 0.2, 0.2);
    fragColor = vec4(dot(normalize(outLightDir), outNorm) * color, 1.0);
    // fragColor = vec4(outLightDir, 1.0);
    // fragColor = vec4(uv.x, uv.y, 0.0, 1.0);

    // fragColor = texture(tex0, uvFrag);
}
