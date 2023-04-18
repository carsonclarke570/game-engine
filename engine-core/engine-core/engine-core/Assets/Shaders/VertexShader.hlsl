#pragma enable_d3d11_debug_symbols

struct Mat
{
    matrix Model;
    matrix View;
    matrix Projection;
};

ConstantBuffer<Mat> MatCB : register(b0);

struct VertexPositionNormalTexture
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float3 Tangent	: TANGENT;
    float3 Color    : COLOR;
    float2 UV       : UV;
};

struct VertexShaderOutput
{
    float4 Position     : SV_POSITION;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGNT;
    float3 Color        : COLOR;
    float2 UV           : UV;
    float3 WorldPos     : POSITION;
};

VertexShaderOutput main(VertexPositionNormalTexture IN)
{
    VertexShaderOutput OUT;

    matrix mvp = mul(mul(MatCB.Model, MatCB.View), MatCB.Projection);

    // Calculate output position
    OUT.Position = mul(float4(IN.Position, 1.0f), mvp);

    // Calculate world position
    OUT.WorldPos = mul(float4(IN.Position, 1.0f), MatCB.Model).xyz;

    // Calculate transformed normals
    OUT.Normal = normalize(mul(IN.Normal, (float3x3) MatCB.Model));

    OUT.Tangent = normalize(mul(IN.Tangent, (float3x3) MatCB.Model));

    // Calculate uv
    OUT.Color = IN.Color;

    OUT.UV = IN.UV;

    return OUT;
}