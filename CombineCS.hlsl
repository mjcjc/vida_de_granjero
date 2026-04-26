#include "TileableNoise.hlsli"

#define MATERIAL_SSAO_USE        0x01
#define MATERIAL_BLOOM_USE        0x02
#define SSAO_KERNEL_SIZE 16

static const float pi = 3.141593;

RWTexture2D<float4> gOutput : register(u0);
Texture2D gtxtBloomMap : register(t0);
Texture2D gtxt : register(t1);
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

[numthreads(32, 32, 1)] //x,y의 모든 쓰레드 갯수가 화면 해상도를 기반으로 넘겼기에 SV_DispatchTheadID를 픽셀 좌표로 봐도 된다.
void main(uint3 gID : SV_GroupID, uint3 DTid : SV_DispatchThreadID)
{
    uint2 pixel = DTid.xy;

    // 이미 계산된 HDR 결과 (gOutput에 들어있음)
    float3 hdrColor = gOutput.Load(int3(pixel, 0)).rgb;

    // Bloom 결과
    float3 bloomColor = float3(0.0, 0.0, 0.0);

    if ((gnFlages & MATERIAL_BLOOM_USE) != 0)
    {
        bloomColor = gtxtBloomMap.Load(int3(pixel, 0)).rgb;
        bloomColor = max(bloomColor, 0.0);
    }

    // HDR 공간에서 Bloom 가산
   float3 hdrFinal = hdrColor + bloomColor;

    //float3 hdrFinal = bloomColor;

    // 다시 HDR로 저장
    gOutput[pixel] = float4(hdrFinal, 1.0);
}