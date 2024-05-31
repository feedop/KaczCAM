#define NLIGHTS 1

sampler samp;

float4 lightPos[NLIGHTS];
float3 lightColor[NLIGHTS];
texture2D duckTex;

static const float ka = 0.2f;
static const float kd = 0.7f;
static const float ks = 0.5f;
static const float m = 32;

float4 phong(float3 surfaceColor, float3 worldpos, float3 norm, float3 view)
{
    view = normalize(view);
    norm = normalize(norm);
    float3 color = surfaceColor * ka; //ambient
    for (int k = 0; k < NLIGHTS; ++k)
    {
        float3 lightvec = normalize(lightPos[k].xyz - worldpos);
        float3 halfvec = normalize(view + lightvec);
        color += lightColor[k] * kd * surfaceColor * saturate(dot(norm, lightvec)); //diffuse
        color += lightColor[k] * ks * pow(saturate(dot(norm, halfvec)), m); //specular
    }
    return saturate(float4(color, 1.0f));
}

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float3 norm : NORMAL0;
    float3 view : VIEWVEC0;
    float2 tex : TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
    float3 color = duckTex.Sample(samp, i.tex);
    return phong(color, i.worldPos, i.norm, i.view);
}