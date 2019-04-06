#include "common.hlsli"

struct VSInput {
    float3  pos     : POSITION; 
    uint4   bones   : TEXCOORD0;
    float4  normal  : TEXCOORD1;
    float4  weights : TEXCOORD2;
    float2  uv      : TEXCOORD3;
};

void Skin(VSInput IN, out float3 skinnedPos, out float3 skinnedNorm) {
    float4 weights = IN.weights.zyxw;

    if (weights.x > 0.0f) {
        uint4 boneIdxs = IN.bones.zyxw;

        matrix skinMat = Bones[boneIdxs.x] * weights.x;
        if (weights.y > 0.0f) {
            skinMat += Bones[boneIdxs.y] * weights.y;
            if (weights.z > 0.0f) {
                skinMat += (Bones[boneIdxs.z] * weights.z) + (Bones[boneIdxs.w] * weights.w);
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

    OUT.pos = mul(MatModelViewProj, float4((skinnedPos - BSphere.xyz).zyx, 1.0f));
    OUT.normal.xyz = mul((float3x3)MatModel, skinnedNorm.zyx);
    OUT.normal.w = IN.normal.w;
    OUT.uv = IN.uv;

    return OUT;
}
