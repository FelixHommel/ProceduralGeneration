struct Uniforms
{
    float3 lightPos;
    float3 viewPos;
    float3 lightColor;
    float3 objectColor;
};

ConstantBuffer<Uniforms> gUniforms;

struct FragmentInput
{
    float3 normal : TEXCOORD0;
    float3 fragPos : TEXCOORD1;
};

static const float ambientStrength = 0.4;
static const float specularStrength = 0.8;

float3 ambientLight()
{
    return ambientStrength * gUniforms.lightColor;
}

float3 diffuseLight(float3 lightDir, float3 normal)
{
    const float diff = max(dot(normal, lightDir), 0.0);

    return diff * gUniforms.lightColor;
}

float3 specularLight(float3 fragPos, float3 lightDir, float3 normal)
{
    const float viewDir = normalize(gUniforms.viewPos - fragPos);
}

[shader("fragment")]
float4 fragmentMain(FragmentInput input)
{
}
