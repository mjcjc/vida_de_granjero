#include "Common.hlsli"

Texture2D gtxtTextureAlbedo : register(t1);

struct PS_OUTPUT
{
    float4 pixelColor : SV_Target0;
};

struct GS_OUTPUT
{
    float4 positionH : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normalW : NORMAL;
};

PS_OUTPUT main(GS_OUTPUT input)
{
    PS_OUTPUT output;

    float3 finalColor;
    float alpha;

    
    // Punch 파티클 (텍스처 미사용)
    if (nParticleFlags & PF_PUNCH)
    {
        finalColor = gParticleColor;

        // 중심 진하고 가장자리 페이드
        float2 centeredUV = input.uv - 0.5;
        float dist = length(centeredUV) * 2.0;

        alpha = saturate(1.0 - dist);
        alpha *= fAlphaScale;
    }

    // 일반 파티클 (텍스처 사용)
    else
    {
        float4 texColor = gtxtTextureAlbedo.Sample(
            glinearSamplerState, input.uv
        );

        finalColor = texColor.rgb;
        alpha = texColor.a * fAlphaScale;

        // 색상 오버라이드
        if (nParticleFlags & PF_USE_COLOR)
        {
            finalColor *= gParticleColor;
        }

        // Dust 가장자리 페이드
        if (nParticleFlags & PF_DUST)
        {
            float2 centeredUV = input.uv - 0.5;
            float dist = length(centeredUV) * 2.0;
            float edgeFade = saturate(1.0 - dist);

            alpha *= edgeFade;
        }
    }

    // =====================================================
    // 공통 알파 컷
    // =====================================================
    if (alpha < 0.02f)
        discard;

    output.pixelColor = float4(finalColor, alpha);
    return output;
}