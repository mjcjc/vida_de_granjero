#include "TileableNoise.hlsli"

#define MATERIAL_SSAO_USE        0x01
#define MATERIAL_BLOOM_USE        0x02
#define SSAO_KERNEL_SIZE 16

static const float pi = 3.141593;

RWTexture2D<float4> gOutput : register(u0);
Texture2D gtxtHDRMap : register(t0);
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
    float BloomThreshold : packoffset(c8.w);    // 밝기 임계값 (ex. 1.0 ~ 2.0)
    float BloomSoftKnee : packoffset(c9.x);     // 부드러운 임계 영역 (0 ~ 1)
    float BloomIntensity : packoffset(c9.y);    // 초기 강도 (선택)
};

cbuffer cbSSAO : register(b1)
{
	float3 gSSAOKernel[SSAO_KERNEL_SIZE];
	float padding;
};

//cbuffer cbBloom : register(b2)
//{
//	float BloomThreshold;    // 밝기 임계값 (ex. 1.0 ~ 2.0)
//	float BloomSoftKnee;     // 부드러운 임계 영역 (0 ~ 1)
//	float BloomIntensity;   // 초기 강도 (선택)
//	float Padding;
//};
[numthreads(32, 32, 1)] //x,y의 모든 쓰레드 갯수가 화면 해상도를 기반으로 넘겼기에 SV_DispatchTheadID를 픽셀 좌표로 봐도 된다.
void main(uint3 gID : SV_GroupID, uint3 DTid : SV_DispatchThreadID)
{
    uint2 pixel = DTid.xy;

    // HDR Scene Color (SSAO + Lighting 완료된 결과)
    float3 hdrColor = gtxtHDRMap.Load(int3(pixel, 0)).rgb;

    // Bloom 비활성화면 그냥 0
    if ((gnFlages & MATERIAL_BLOOM_USE) == 0)
    {
        gOutput[pixel] = float4(0, 0, 0, 1);
        return;
    }

    // 밝기 계산 (Max 방식, Bloom에 더 잘 어울림)
    float brightness = max(max(hdrColor.r, hdrColor.g), hdrColor.b);

    // Soft Knee 계산
    float knee = BloomThreshold * BloomSoftKnee;
    float soft = brightness - BloomThreshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = (soft * soft) / (4.0 * knee + 1e-5);

    // Bloom 기여도
    float contribution = max(brightness - BloomThreshold, soft);

    // 원본 색상 기준으로 비율 계산
    float bloomFactor = contribution / max(brightness, 1e-5);

    float3 bloomColor = hdrColor * bloomFactor * BloomIntensity;

    gOutput[pixel] = float4(bloomColor, 0.0);
}