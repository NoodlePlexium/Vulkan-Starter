#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColour;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionView;
    vec4 ambientLightColour;
	vec3 lightPosition;
	vec4 lightColour;
    mat4 view;
} ubo;

void main()
{
    outColour = vec4(ubo.lightColour.xyz, 1.0);
}