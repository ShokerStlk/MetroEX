#include "common.hlsli"

Texture2D TexAlbedo     : register(t0);
SamplerState Sampler0   : register(s0);

float4 main(VSOutput IN) : SV_Target0 {
    float3 normal = normalize(IN.normal);
    float diffuse = clamp(dot(normal, normalize(float3(-0.5f, 0.75f, 0.5f))), 0.35f, 1.0f);

    float4 albedo = TexAlbedo.Sample(Sampler0, IN.uv);
    if (albedo.a < 0.1f) {
        clip(-1.0f);
    }

    return float4(albedo.rgb * diffuse, 1.0f);
}
