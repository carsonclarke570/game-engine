#pragma enable_d3d11_debug_symbols

struct PixelShaderInput {
    float4 Position     : SV_POSITION;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGNT;
    float3 Color        : COLOR;
    float2 UV           : UV;
    float3 WorldPos     : POSITION;
};


struct PixelShaderOutput {
    float4 Diffuse		: SV_Target0;
    float4 Normal		: SV_Target1;
    float4 Position		: SV_Target2;
};

PixelShaderOutput main(in PixelShaderInput INPUT) {
    PixelShaderOutput OUTPUT;

    float3 N = INPUT.Normal;
    float3 T = normalize(INPUT.Tangent - N * dot(INPUT.Tangent, N));
    float3 B = cross(T, N);

    float3x3 tangentFrame = float3x3(normalize(INPUT.Tangent), normalize(B), normalize(INPUT.Normal));
    
    float normal = normalize(N * 2.0f - 1.0f);
    float3 normalWS = mul(N, tangentFrame);

    OUTPUT.Diffuse = float4(INPUT.Color, 1.0f);
    OUTPUT.Normal = float4((INPUT.Normal + float3(1.0f, 1.0f, 1.0f)) / 2, 1.0f);
    OUTPUT.Position = float4(INPUT.WorldPos, 1.0f);

	return OUTPUT;
}