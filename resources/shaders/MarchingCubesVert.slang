struct Uniforms
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

ConstantBuffer<Uniforms> gUniforms;

struct VertexInput
{
    float3 position : ATTRIBUTE0;
    float3 normal : ATTRIBUTE1;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal : TEXCOORD0;
    float3 fragPos : TEXCOORD1;
};

[shader("vertex")]
VertexOutput main(VertexInput input)
{
    VertexOutput out;

    float4 worldPos = mul(gUniforms.model, float4(input.position, 1.0));

    out.position = mul(gUniforms.projection, mul(gUniforms.view, worldPos));
    out.normal = input.normal;
    out.fragPos = worldPos.xyz;

    return out;
}
