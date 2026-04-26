#include "Common.hlsli"

Texture2D gtxtDeptOnlyTexture : register(t1); //DepthMap으로 읽어오자
Texture2D gtxtTextureNormal : register(t2);
Texture2D gtxtTextureRoughnness : register(t3);
Texture2D gtxtTextureMetal : register(t4);
Texture2D gtxtTextureOcclusion : register(t5);
Texture2D gtxtTextureSpecluarF0 : register(t6);
Texture2D gtxtTextureEmission : register(t7);

Texture2D gtxtTextureSpecluarBRDF : register(t11); // t11 : IBL BRDF LUT
TextureCube gtxtTextureSpecluarIBL : register(t12); // t12 : Specluar IBL
TextureCube gtxtTextureDiffuseIrradianceIBL : register(t13); // t13 : Irradiance

Texture2D gtxtTextureHDR : register(t14); //HDR 후처리 Texture

//const float  fogStrength = 2.0;

//const float  depthScale = 0.1;

float4 ToonMapping(float3 input)
{
    float3 invGamma = float3(1, 1, 1) / gf1Gamma;
    float3 color = clamp(gf1Exposure * input, 0., 1.);
    color = pow(color, invGamma);
    return float4(color, 1.0f);
}


//정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다.
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float2 uv : TEXCOORD;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_Target0;
};


float4 TexcoordToView(float2 texcoord)
{
    float4 posProj;

    // [0, 1]x[0, 1] -> [-1, 1]x[-1, 1]
    posProj.xy = texcoord * 2.0 - 1.0;
    posProj.y *= -1; // 주의: y 방향을 뒤집어줘야 합니다.
    posProj.z = gtxtDeptOnlyTexture.Sample(glinearSamplerState, texcoord).r;
    posProj.w = 1.0;

    // ProjectSpace -> ViewSpace
    float4 posView = mul(posProj, gmtxinvProj);
    posView.xyz /= posView.w;

    return posView; //현 카메라 좌표계 위치값.
}

static const float  fogStrength = 3.0;

static const float  depthScale = 0.1;

float4  main(VS_OUTPUT input) : SV_TARGET
{


    if (gnWeather & WEATHER_DUST)
    {
         float4 posView = TexcoordToView(input.uv);
         float dist = length(posView.xyz); // 눈의 위치가 원점인 좌표계
         float3 fogColor = float3(0.78, 0.72, 0.60);
         float fogmin = 0.1;
         float fogMax = 20.0;
         // Fog
         float distFog = saturate((dist - fogmin) / (fogMax - fogmin));
         // float3 FogRe = lerp(renderTex.Sample(linearClampSampler, input.texcoord).rgb, fogColor, distFog);

         float3 I0 = gtxtTextureHDR.SampleLevel(glinearSamplerState, input.uv, 0).rgb;
         float3 I = I0 * pow(2.71828, -fogStrength * distFog);

         float3 FogRe = lerp(fogColor, gtxtTextureHDR.SampleLevel(glinearSamplerState, input.uv, 0).rgb, exp(-fogStrength * distFog));

         /*  float z = TexcoordToView(input.uv).z * depthScale;
           return float4(z, z, z, 1);*/
           //return float4(I, 1.0);
         return ToonMapping(FogRe);
     }
     else if (gnWeather & WEATHER_FOG)
     {
         float4 posView = TexcoordToView(input.uv);
         float dist = length(posView.xyz); // 눈의 위치가 원점인 좌표계
         float3 fogColor = float3(1.0, 1.0, 1.0);
         float fogmin = 1.0;
         float fogMax = 50.0;

         // Fog
         float distFog = saturate((dist - fogmin) / (fogMax - fogmin));
         // float3 FogRe = lerp(renderTex.Sample(linearClampSampler, input.texcoord).rgb, fogColor, distFog);

         float3 I0 = gtxtTextureHDR.SampleLevel(glinearSamplerState, input.uv, 0).rgb;
         float3 I = I0 * pow(2.71828, -fogStrength * distFog);

         float3 FogRe = lerp(fogColor, gtxtTextureHDR.SampleLevel(glinearSamplerState, input.uv,0).rgb, exp(-fogStrength * distFog));

   
        return ToonMapping(FogRe);
     }


    //return shadowMaps[1].SampleLevel(glinearSamplerState, input.uv,0) * 1 ;
    return ToonMapping(gtxtTextureHDR.SampleLevel(glinearSamplerState, input.uv, 0).rgb);

}