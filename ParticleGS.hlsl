#include "Common.hlsli"
#include "TileableNoise.hlsli"

struct GS_INPUT
{
    float3 positionW : POSITION;
    float2 size : TEXCOORD0; // halfWidth, halfHeight
};

struct GS_OUTPUT
{
    float4 positionH : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normalW : NORMAL;
};

[maxvertexcount(4)]
void main(
    point GS_INPUT input[1],
    inout TriangleStream<GS_OUTPUT> output
)
{
    float3 centerW = input[0].positionW;
    float2 size = input[0].size; // ★ 핵심

    float3 camPosW = gf3EyeWorld;
    float3 toCamera = normalize(camPosW - centerW);

    float3 up = float3(0, 1, 0);
    float3 right;
    float3 forward;

    // -----------------------------
    // 빌보드 타입 분기
    // -----------------------------
    if (nParticleFlags & (PF_RAIN | PF_SNOW))
    {
        // Y축 고정 빌보드
        forward = normalize(float3(toCamera.x, 0.0, toCamera.z));
        right = normalize(cross(up, forward));
    }
    else // PF_DUST
    {
        // 완전 빌보드
        right = normalize(cross(up, toCamera));
        up = normalize(cross(toCamera, right));
        forward = toCamera;
    }

    float3 normalW = normalize(forward);

    // -----------------------------
    // XY 비율 기반 quad 확장
    // -----------------------------
    float3 v0 = centerW - right * size.x - up * size.y;
    float3 v1 = centerW + right * size.x - up * size.y;
    float3 v2 = centerW - right * size.x + up * size.y;
    float3 v3 = centerW + right * size.x + up * size.y;

    GS_OUTPUT o;
    o.normalW = normalW;

    // v0
    o.positionH = mul(float4(v0, 1.0), gmtxView);
    o.positionH = mul(o.positionH, gmtxProj);
    o.uv = float2(0, 1);
    output.Append(o);

    // v1
    o.positionH = mul(float4(v1, 1.0), gmtxView);
    o.positionH = mul(o.positionH, gmtxProj);
    o.uv = float2(1, 1);
    output.Append(o);

    // v2
    o.positionH = mul(float4(v2, 1.0), gmtxView);
    o.positionH = mul(o.positionH, gmtxProj);
    o.uv = float2(0, 0);
    output.Append(o);

    // v3
    o.positionH = mul(float4(v3, 1.0), gmtxView);
    o.positionH = mul(o.positionH, gmtxProj);
    o.uv = float2(1, 0);
    output.Append(o);

    output.RestartStrip();
}