#include "TileableNoise.hlsli"

#define MATERIAL_SSAO_USE        0x01
#define MATERIAL_BLOOM_USE        0x02
#define SSAO_KERNEL_SIZE 16

static const float pi = 3.141593;

RWTexture2D<float4> gOutput : register(u0);
RWTexture2D<float4> gOutput2 : register(u1);
Texture2D gtxtSSAOMap : register(t0);
Texture2D gtxtNormalMap : register(t1);
Texture2D gtxtAmbientLightMap : register(t2);
Texture2D gtxtSSAONoise : register(t3);
SamplerState gpointClampSampler : register(s0);

cbuffer cbImGuiComputeFlages : register(b0)
{
	Matrix invProj : packoffset(c0);
	Matrix Proj : packoffset(c4);
	float gRadius : packoffset(c8.x); //AO 반경 (0.5)
	float gBias : packoffset(c8.y); //self_occlusion 방지 (0.025)
	uint gnFlages : packoffset(c8.z);
};

cbuffer cbSSAO : register(b1)
{
	float3 gSSAOKernel[SSAO_KERNEL_SIZE];
	float padding;
};


// x * y <= 1024여야 한다
//warp => 쓰레드 묶음으로 하드웨어 마다 다르지만 엔비디아는 최소 32개의 warp로 이루어져있다.
//32개의 쓰레드가 warp로 묶여서 관리된다. 그래서 32개의 쓰레드가 같은 일을 처리할때 가장 효율이 오른다
//하나의 쓰레드 그룹을 내가 1024로 지정을 했다 그러면 하드웨어에서는 이걸 32개 씩 warp로 묶어
//총 32개의 그룹으로 처리할 것이다.
[numthreads(32, 32, 1)] //x,y의 모든 쓰레드 갯수가 화면 해상도를 기반으로 넘겼기에 SV_DispatchTheadID를 픽셀 좌표로 봐도 된다.
void main( uint3 gID : SV_GroupID , uint3 DTid : SV_DispatchThreadID )
{
    uint width, height;
    gOutput.GetDimensions(width, height);
    if (DTid.x >= width || DTid.y >= height)
        return;

    uint2 pixel = DTid.xy;
    float2 uv = (float2(pixel) + 0.5f) / float2(width, height);

    //Ambient를 저장한 G-Buffer
    float3 ambient = gtxtAmbientLightMap.SampleLevel(gpointClampSampler, uv, 0).xyz;
    float ao = 1.0f;

    if (gnFlages & MATERIAL_SSAO_USE)
    {
        ao = gtxtSSAOMap
            .SampleLevel(gpointClampSampler, uv, 0).r;

        ao = saturate(ao);
        ao = pow(ao, 2);
    }

    gOutput2[pixel] = gOutput[pixel];
    gOutput[pixel] = gOutput[pixel] + float4(ambient, 1.0f) * ao;

}