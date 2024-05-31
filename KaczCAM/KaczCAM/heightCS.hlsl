RWTexture2D<unorm float4> normTexUAV;
RWTexture2D<float> heightTex0UAV;
RWTexture2D<float> heightTex1UAV;
sampler samp;
bool swap;
float time;
float2 duckPos;

static const float N = 256;
static const float h = 2.0f / (N - 1);
static const float c = 0.3f;
static const float deltaT = 1.0f / N;
static const float A = c * c * deltaT * deltaT / (h * h);
static const float B = 2 - 4 * A;

float at(RWTexture2D<float> tex, uint3 DTid, int i, int j)
{
    return tex[float2(DTid.x + i, DTid.y + j)];
}

float random(float2 st)
{
    return frac(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float distX = min(DTid.x, N - 1 - DTid.x);
    float distY = min(DTid.y, N - 1 - DTid.y);
    float l = min(distX, distY) * h;
    float d = 0.97f * min(1, l * 5.0);
    
    float height;
    // Height from previous state
    if (swap)
    {
        float oldHeight = heightTex0UAV[DTid.xy];
        height = d * (A * (
        at(heightTex1UAV, DTid, 1, 0) +
        at(heightTex1UAV, DTid, -1, 0) +
        at(heightTex1UAV, DTid, 0, -1) +
        at(heightTex1UAV, DTid, 0, 1)
        ) + B * heightTex1UAV[DTid.xy] - oldHeight);
    }
    else
    {
        float oldHeight = heightTex1UAV[DTid.xy];
        height = d * (A * (
        at(heightTex0UAV, DTid, 1, 0) +
        at(heightTex0UAV, DTid, -1, 0) +
        at(heightTex0UAV, DTid, 0, -1) +
        at(heightTex0UAV, DTid, 0, 1)
        ) + B * heightTex0UAV[DTid.xy] - oldHeight);
    }
    
    // Random disturbances
    if (length(duckPos - 20 * (DTid.xy / 128.0 - 1.0)) < 1e-1)
    {
        height -= 1.0;
    }
    else
    {
        float rand = random(DTid.xy / 255.0 * frac(time));
        if (rand > 0.99992)
            height -= 1.0; //clamp(height + 0.5, -1, 1);
    }
    
    if (swap)
        heightTex0UAV[DTid.xy] = height;
    else
        heightTex1UAV[DTid.xy] = height;
}