#include "common.hlsli"

VSOutput main(uint vertexId : SV_VertexID) {
    VSOutput OUT;

    const float idDiv2 = (float)(vertexId / 2);
    const float idMod2 = (float)(vertexId % 2);

    OUT.pos = float4(idDiv2 * 4.0f - 1.0f, idMod2 * 4.0f - 1.0f, 0.0f, 1.0f);
    OUT.normal = 0.0f;
    OUT.uv = float2(idDiv2 * 2.0f, 1.0f - idMod2 * 2.0f);

    return OUT;
}
