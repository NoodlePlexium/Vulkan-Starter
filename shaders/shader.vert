#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColour;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;


layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec3 directionToLight;
} ubo;

layout(push_constant) uniform Push {
    mat4 meshMatrix; // projection * view * mesh
    mat4 normalMatrix;
} push;


void main() {
    vec4 positionWorld = push.meshMatrix * vec4(position, 1.0);
    gl_Position = ubo.projectionViewMatrix * positionWorld;

    fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
    fragPosWorld = positionWorld.xyz;
    fragColour = colour;
}
