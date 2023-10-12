#version 450

layout (location = 0) in vec3 fragColour;
layout (location = 1) in vec3 fragPositionWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColour;


layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec3 directionToLight;
} ubo;

layout(push_constant) uniform Push {
    mat4 meshMatrix; // projection * view * mesh
    mat4 normalMatrix;
} push;

const float AMBIENT = 0.1;
const vec3 SKY_COLOR = vec3(0.69, 0.84, 0.89); // Blue sky color

void main() {
    float lightIntensity = max(dot(fragNormalWorld, ubo.directionToLight), 0);
	outColour = vec4(lightIntensity * fragColour +  AMBIENT * SKY_COLOR, 1.0);
}