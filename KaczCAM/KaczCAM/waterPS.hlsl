texture2D normTex : register(t0);
sampler samp;
float4 camPos;
textureCUBE envMap;
float time;

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 localPos : POSITION0;
    float3 worldPos : POSITION1;
};

static const float n1 = 1.33f;
static const float n2 = 1.0f;

static const float F0 = 0.14;
static const float gamma = 0.4545f;

float fresnel(float3 N, float3 V)
{
    float costheta = max(dot(N, V), 0);
    return F0 + (1 - F0) * pow((1 - costheta), 5);
}

float intersectRay(float3 p, float3 d)
{
    float3 tplus = (-1 - p) / d;
    float3 tminus = (1 - p) / d;

    float3 t = max(tplus, tminus);
    return min(t.x, min(t.y, t.z));
}

float4 main(PSInput i) : SV_TARGET
{    
    float3 viewVec = normalize(camPos.xyz - i.worldPos);
    
    float3 norm = normTex.Sample(samp, (i.localPos.xz + 1.0f) / 2.0f).xyz;
    
    float fresnelCoeff;
    
    
    if (dot(norm, viewVec) < 0)
    {
        norm *= -1;
        fresnelCoeff = n1 / n2;
    }
    else
    {
        fresnelCoeff = n2 / n1;
    }
    
    float3 reflected = reflect(-viewVec, norm);
    float3 refracted = refract(-viewVec, norm, fresnelCoeff);
    
    float t1 = intersectRay(i.localPos, reflected);
    float3 texCoord1 = i.localPos + t1 * reflected;
    float3 color1 = envMap.Sample(samp, texCoord1);
    
    if (!any(refracted))
    {
        color1 = pow(color1, gamma);
        return float4(color1, 1.0f);
    }
    
    float t2 = intersectRay(i.localPos, refracted);
    float3 texCoord2 = i.localPos + t2 * refracted;
    float3 color2 = envMap.Sample(samp, texCoord2);
    
    float coeff = fresnel(norm, viewVec);

    float3 color = lerp(color2, color1, coeff);
    color = pow(color, gamma);
    
    return float4(color, 1.0f);
}