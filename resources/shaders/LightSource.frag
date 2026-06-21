#version 430 core

in vec3 fragPos;
in vec3 normal;

uniform vec3 lightColor;

out vec4 fragColor;

void main()
{
    fragColor = vec4(lightColor, 1.f);
}
