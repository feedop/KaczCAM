matrix modelMtx, viewProjMtx;
float waterLevel;
struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 localPos : POSITION0;
    float3 worldPos : POSITION1;
};
VSOutput main(float3 pos : POSITION0)
{
    VSOutput o;
    o.localPos = pos + waterLevel;
    o.worldPos = mul(modelMtx, float4(o.localPos, 1.0f));
    o.pos = mul(viewProjMtx, float4(o.worldPos, 1.0f));
    return o;
}