struct VSOutput {
    float4 pos      : SV_Position;
    float4 normal   : TEXCOORD0;
    float2 uv       : TEXCOORD1;
};

cbuffer ConstantBuffer0 : register(b0) {
    float4  BSphere;
    matrix  MatModel;
    matrix  MatView;
    matrix  MatProjection;
    matrix  MatModelViewProj;
    float4  CamParams;          // x - FOV, y - aspect, zw - vacant
    matrix  Bones[256];
};
