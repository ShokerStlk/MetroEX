#include "common.hlsli"

struct VSInput {
    float3  pos     : POSITION; 
    uint4   bones   : TEXCOORD0;
    float4  normal  : TEXCOORD1;
    float4  weights : TEXCOORD2;
    float2  uv      : TEXCOORD3;
};

void Skin(VSInput IN, out float3 skinnedPos, out float3 skinnedNorm) {
    if (IN.weights.x > 0.0f) {
        matrix skinMat = Bones[IN.bones.x] * IN.weights.x;
        if (IN.weights.y > 0.0f) {
            skinMat += Bones[IN.bones.y] * IN.weights.y;
            if (IN.weights.z > 0.0f) {
                skinMat += (Bones[IN.bones.z] * IN.weights.z) + (Bones[IN.bones.w] * IN.weights.w);
            }
        }

        skinnedPos = mul(skinMat, float4(IN.pos, 1.0f)).xyz;
        skinnedNorm = mul((float3x3)skinMat, IN.normal.xyz);
    } else {
        skinnedPos = IN.pos.xyz;
        skinnedNorm = IN.normal.xyz;
    }
}

VSOutput main(VSInput IN) {
    VSOutput OUT;

    float3 skinnedPos, skinnedNorm;
    Skin(IN, skinnedPos, skinnedNorm);

    OUT.pos = mul(MatModelViewProj, float4(skinnedPos - BSphere.xyz, 1.0f));
    OUT.normal.xyz = mul((float3x3)MatModel, skinnedNorm);
    OUT.normal.w = IN.normal.w;
    OUT.uv = IN.uv;

    return OUT;
}
