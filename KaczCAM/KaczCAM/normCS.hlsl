RWTexture2D<unorm float4> normTexUAV;
RWTexture2D<float> heightTex0UAV;
RWTexture2D<float> heightTex1UAV;
bool swap;


float at(RWTexture2D<float> tex, uint3 DTid, int i, int j)
{
    return tex[float2(DTid.x + i, DTid.y + j)];
}


[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{     
    float3 norm = float3(0, 1, 0);
    if (swap)
    {
        float L = at(heightTex0UAV, DTid, -1, 0);
        float R = at(heightTex0UAV, DTid, 1, 0);
        float T = at(heightTex0UAV, DTid, 0, 1);
        float B = at(heightTex0UAV, DTid, 0, -1);
        norm = normalize(float3(2 * (R - L), 4, 2 * (B - T)));
    }
    else
    {
        float L = at(heightTex1UAV, DTid, -1, 0);
        float R = at(heightTex1UAV, DTid, 1, 0);
        float T = at(heightTex1UAV, DTid, 0, 1);
        float B = at(heightTex1UAV, DTid, 0, -1);
        norm = normalize(float3(2 * (R - L), 4, 2 * (B - T)));
    }
    //norm = float3(0, 1, 0);
    normTexUAV[DTid.xy] = float4(norm, 0);
}