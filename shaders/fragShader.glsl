#version 330
precision mediump float;

// Color that is the result of this shader
out vec4 fragColor;
in vec3 norm;
in float height;

// uniform sampler2D tex0;

void main() {
    vec3 lightDir = vec3(0.3, 0.7, 0.3);
    //fragColor = vec4 (0.5 * norm.x, 0.5, 0.5 * norm.z, 1.0);
    //fragColor = vec4(outNorm, 1.0);
    // vec3 color = vec3(0.8, 0.2, 0.2);
    // fragColor = vec4(dot(normalize(outLightDir), outNorm) * color, 1.0);
    // fragColor = vec4(outLightDir, 1.0);

    float t = height - 7.0;

    //vec3 color = vec3(0.1, 0, 0.1) * t + vec3(0.05, 1.0, 0.05) * (1 - t);
    vec3 color = mix(vec3(0.4, 1.0, 0.39), vec3(0.4, 0.4, 0.39), t);

    //fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    fragColor = vec4(max(dot(lightDir, normalize(norm)), 0.1) * color, 1.0);

    // fragColor = texture(tex0, uvFrag);
}
