#include "Common.hlsli"

Texture2D gtxtTextureAlbedo : register(t1);
Texture2D gtxtTextureNormal : register(t2);
Texture2D gtxtTextureRoughnness : register(t3);
Texture2D gtxtTextureMetal : register(t4);
Texture2D gtxtTextureOcclusion : register(t5);
Texture2D gtxtTextureSpecluarF0 : register(t6);
Texture2D gtxtTextureEmission : register(t7);

TextureCube gtxtSkyCubeTexture : register(t10); // t10 : skybox


struct VS_OUTPUT
{
	float3 positionL : POSITION;
	float4 position : SV_POSITION;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{

	float4 color = gtxtSkyCubeTexture.Sample(glinearSamplerState, input.positionL);

	return color;
}