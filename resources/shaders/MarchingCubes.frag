#version 330 core

in vec3 fragPos;
in vec3 normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

out vec4 fragColor;

const float ambientStrength = 0.4f;
const float specularStrength = 0.8f;

vec3 ambientLight()
{
    return ambientStrength * lightColor;
}

vec3 diffuseLight(vec3 lightDir, vec3 normal)
{
    float diff = max(dot(normal, lightDir), 0.f);

    return diff * lightColor;
}

vec3 specularLight(vec3 lightDir, vec3 normal)
{
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.f), 32);

    return specularStrength * spec * lightColor;
}

void main()
{
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 norm = normalize(normal);

    vec3 result = (ambientLight() + diffuseLight(lightDir, norm) + specularLight(lightDir, norm)) * objectColor;
    fragColor = vec4(result, 1.f);
}
