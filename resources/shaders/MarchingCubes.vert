#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 normal;
out vec3 fragPos;

void main()
{
    vec4 worldPos = model * vec4(inPosition, 1.f);
    gl_Position = projection * view * worldPos;

    normal = inNormal;
    fragPos = worldPos.xyz;
}
