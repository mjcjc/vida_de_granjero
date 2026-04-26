#include "TileableNoise.hlsli"

#define SSAO_KERNEL_SIZE 16

#define MATERIAL_SSAO_USE        0x01
#define MATERIAL_BLOOM_USE        0x02



static const float pi = 3.141593;

RWTexture2D<float4> gOutput : register(u0);   // AO 출력 (R 채널 사용)
Texture2D gtxtDepthStantcilMap : register(t0);
Texture2D gtxtNormalMap : register(t1);

SamplerState gpointClampSampler : register(s0);

cbuffer cbImGuiComputeFlages : register(b0)
{
    Matrix invProj : packoffset(c0);
    Matrix Proj : packoffset(c4);
    float gRadius : packoffset(c8.x); //AO 반경 (0.5)
    float gBias : packoffset(c8.y); //self_occlusion 방지 (0.025)
    uint gnFlages : packoffset(c8.z);
};


cbuffer cbSSAOKernel : register(b1)
{
    float3 gSSAOKernel[SSAO_KERNEL_SIZE];
    float  padding;
};


// View-space 복원
float3 ViewSpace(float2 uv, float depth)
{
    float2 screen = float2(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0);
    screen.y *= -1.0;

    float4 clipPos = float4(screen, depth, 1.0);
    float4 viewPos = mul(clipPos, invProj);
    return viewPos.xyz / viewPos.w;
}

// 픽셀 기반 랜덤 회전 (프레임 고정)
float3 GetRandomVector(uint2 pixel)
{
    float3 h = hash33(float3(pixel, 0));
    h.z = 0.0f; // screen-space 회전
    return normalize(h);
}

float3x3 BuildTBN(float3 normal, uint2 pixel)
{
    float3 rand = GetRandomVector(pixel);
    float3 tangent = normalize(rand - normal * dot(rand, normal));
    float3 bitan = cross(normal, tangent);
    return float3x3(tangent, bitan, normal);
}


// SSAO Compute

[numthreads(32, 32, 1)]
void main(uint3 gID : SV_GroupID, uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    gOutput.GetDimensions(width, height);

    if (DTid.x >= width || DTid.y >= height)
        return;

    uint2 pixel = DTid.xy;

    if ((gnFlages & MATERIAL_SSAO_USE) == 0)
    {
        gOutput[pixel] = float4(1.0f, 1.0f, 1.0f, 1.0f);
        return;
    }

    float2 uv = (float2(pixel) + 0.5f) / float2(width, height);

    float depth = gtxtDepthStantcilMap.SampleLevel(gpointClampSampler, uv, 0).r;
    float3 normalVS = normalize(
        gtxtNormalMap.SampleLevel(gpointClampSampler, uv, 0).xyz
    );

    float3 posVS = ViewSpace(uv, depth);

    float occlusion = 0.0f;
    float3x3 TBN = BuildTBN(normalVS, pixel);

    [unroll]
        for (int i = 0; i < SSAO_KERNEL_SIZE; ++i)
        {
            float3 sampleDirVS = mul(gSSAOKernel[i], TBN);
            float3 samplePosVS = posVS + sampleDirVS * gRadius;

            float4 offset = mul(float4(samplePosVS, 1.0f), Proj);
            offset.xyz /= offset.w;

            float2 sampleUV = float2(
                offset.x * 0.5f + 0.5f,
                -offset.y * 0.5f + 0.5f
            );

            if (sampleUV.x < 0 || sampleUV.x > 1 ||
                sampleUV.y < 0 || sampleUV.y > 1)
                continue;

            float sampleDepth =
                gtxtDepthStantcilMap.SampleLevel(
                    gpointClampSampler, sampleUV, 0).r;

            float3 sampleVS = ViewSpace(sampleUV, sampleDepth);

            float rangeCheck =
                smoothstep(0.0f, 1.0f,
                    gRadius / abs(posVS.z - sampleVS.z));

            occlusion +=
                (sampleVS.z >= samplePosVS.z + gBias ? 1.0f : 0.0f)
                * rangeCheck;
        }

    float ao = 1.0f - (occlusion / SSAO_KERNEL_SIZE);
    ao = saturate(ao);
    ao = pow(ao, 4.0);

    // AO만 출력 (R 채널)
    gOutput[pixel] = float4(ao, ao, ao, 1.0f);

  
   // float3 debugNormal = n * 0.5f + 0.5f;

    //gOutput[pixel] = float4(normalVS, 1.0f);
}