Texture2D gtxtTextureAlbedo : register(t1);
Texture2D gtxtTextureNormal : register(t2);
Texture2D gtxtTextureRoughnness : register(t3);
Texture2D gtxtTextureMetal : register(t4);
Texture2D gtxtTextureOcclusion : register(t5);
Texture2D gtxtTextureSpecluarF0 : register(t6);
Texture2D gtxtTextureEmission : register(t7);

Texture2D gtxtTextureSpecluarBRDF : register(t11); // t11 : IBL BRDF LUT
TextureCube gtxtTextureSpecluarIBL : register(t12); // t12 : Specluar IBL
TextureCube gtxtTextureDiffuseIrradianceIBL : register(t13); // t13 : Irradiance


SamplerState glinearSamplerState : register(s0);
SamplerState gpointSamplerState : register(s1);



float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}