#include "common.hlsli"

TextureCube Cubemap     : register(t0);
SamplerState Sampler0   : register(s0);

float3 CalcRayDir(float2 screenUV) {
    float3 u = float3(MatView[0].xyz);
    float3 v = float3(MatView[1].xyz);
    float3 z = float3(MatView[2].xyz);

    float planeWidth = tan(CamParams.x * 0.5f);

    u *= (planeWidth * CamParams.y);
    v *= planeWidth;

    return normalize(z + (u * screenUV.x) - (v * screenUV.y));
}

float4 main(VSOutput IN) : SV_Target0 {
    float3 ray = CalcRayDir(IN.uv * 2.0f - 1.0f);
    float3 colour = Cubemap.SampleLevel(Sampler0, ray, 0.0f).rgb;
    return float4(colour, 1.0f);
}
