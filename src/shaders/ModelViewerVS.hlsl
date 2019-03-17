#include "common.hlsli"

struct VSInput {
    float3  pos     : POSITION;
    float3  normal  : NORMAL;
    float2  uv      : TEXCOORD0;
};

VSOutput main(VSInput IN) {
    VSOutput OUT;

    OUT.pos = mul(MatModelViewProj, float4((IN.pos - BSphere.xyz).zyx, 1.0f));
    OUT.normal = mul(MatModel, float4(IN.normal.zyx, 0.0f)).xyz;
    OUT.uv = IN.uv;

    return OUT;
}
