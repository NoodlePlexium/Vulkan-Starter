#version 450

layout (location = 0) in vec3 fragColour;
layout (location = 1) in vec3 fragPositionWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColour;


layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionView;
    vec4 ambientLightColour;
	vec3 lightPosition;
	vec4 lightColour;
    mat4 view;
} ubo;

layout(push_constant) uniform Push {
    mat4 meshMatrix; // projection * view * mesh
    mat4 normalMatrix;
} push;


void main() {
    vec3 directionToLight = ubo.lightPosition - fragPositionWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight);

    vec3 lightColour = ubo.lightColour.xyz * ubo.lightColour.w;
    vec3 ambientLight = ubo.ambientLightColour.xyz * ubo.ambientLightColour.w;
    vec3 diffuseLight = lightColour * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);

	outColour = vec4((diffuseLight + ambientLight) * fragColour, 1.0);
}