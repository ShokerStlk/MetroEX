struct VSOutput {
    float4 pos      : SV_Position;
    float4 normal   : TEXCOORD0;
    float2 uv       : TEXCOORD1;
};

cbuffer ConstantBuffer0 : register(b0) {
    float4            BSphere;
    row_major matrix  MatModel;
    row_major matrix  MatView;
    row_major matrix  MatProjection;
    row_major matrix  MatModelViewProj;
    float4            CamParams;          // x - FOV, y - aspect, zw - vacant
    row_major matrix  Bones[256];
};
