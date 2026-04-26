#include "Common.hlsli"

//참고자료
// https://github.com/Nadrin/PBR/blob/master/data/shaders/hlsl/pbr.hlsl
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

static const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
static const float pi = 3.141593;

//정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다.
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};

struct PS_OUTPUT
{
	float4 pixelColor : SV_Target0;
	float4 nomalColor : SV_Target1;
	float4 ambientLight : SV_Target2;
};



//노멀 매핑 함수
float3 GetNormal(VS_OUTPUT input)
{
	float3 normalW = input.normalW;

	//노멀맵을 사용할 것이며 노멀맵이 존재하는 경우
	if (gnTextureUseMask & MATERIAL_NORMAL_MAP_USE && gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3 normal = gtxtTextureNormal.SampleLevel(glinearSamplerState,input.uv, 0.0).rgb; // 샘플링
		normal = 2.0 * normal - 1.0; //범위 -1 ~ 1 변경 필요

		//만약 OpenGL용 노멀맵에 경우 y가 반대라는 것도 알아야 한다.

		float3 N = normalW;
		float3 T = normalize(input.tangentW - dot(input.tangentW, N) * N); //Tangent가 N과 수직이 아닐 수 있기에 만드는 것
		float3 B = cross(N, T);

		float3x3 TBN = float3x3(T, B, N);
		normalW = normalize(mul(normal, TBN));
	}

	return normalW;
}

//프레넬 방정식을 계산하여 값을 구하는 과정
float3 SchlickFresnel(float3 F0, float NdotH)
{

	float F = F0 + (1 - F0) * pow(2, -5.55473 * NdotH - 6.98316 * NdotH);

	return float3(F,F,F);

}


float3 DiffuseIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
	float metallic)
{
	float3 F0 = lerp(Fdielectric, albedo, metallic);    //F0의 비율이 얼만큼 넣을지를 metallic으로 조절 해주고 있다.
	float3 F = SchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
	float3 kd = lerp(1.0 - F, 0.0, metallic);

	float3 irradiance = gtxtTextureDiffuseIrradianceIBL.Sample(gLinearmipPointSampler, normalWorld);
	irradiance *= albedo;

	return kd * irradiance;
}

 
float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
	float metallic, float roughness)
{
	// Environment BRDF
	float R = 1 -  roughness;
	float c1 = dot(normalWorld, pixelToEye);
	//SpecularBRDF의 x = cos , y = roughness 값
	float2 specularBRDF = gtxtTextureSpecluarBRDF.Sample(gpointSamplerState, float2(c1, R)).rg;

	float3 specularIrradiance = gtxtTextureSpecluarIBL.SampleLevel(gLinearmipPointSampler, reflect(-pixelToEye, normalWorld),
	  roughness * 5.0f).rgb;

	//const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
	float3 F0 = lerp(Fdielectric, albedo, metallic);

	return (specularBRDF.r * F0 + specularBRDF.g) * specularIrradiance;

}

float3 AmbientLightingByIBL(float3 albedo, float3 normalW, float3 pixelToEye, float ao,
	float metallic, float roughness)
{
	float3 diffuseIBL = DiffuseIBL(albedo, normalW, pixelToEye, metallic);
	float3 specularIBL = SpecularIBL(albedo, normalW, pixelToEye, metallic, roughness);

	//return specularIBL;
	//return diffuseIBL;
	//ao는 애초에 환경맵에 적용하기 위해서 사용하는 값
	return (diffuseIBL + specularIBL) * ao;

	//return (specularIBL) * ao;
}


//F0 읽어오는 함수
float3 F0return(float3 albedo, float metallic)
{
	float3 dielectricF0 = float3(0.04, 0.04, 0.04);
	float3 F = lerp(dielectricF0, albedo, metallic);
	return F;
}

float NdfGGX(float NdotH, float roughness)
{

	float alp = roughness * roughness;
	float alpSq = alp * alp;
	//float D = (alp * alp) / (pi * pow((NdotH * NdotH) * ((alp * alp) - 1) + 1, 2));
	float denom = (NdotH * NdotH) * (alp - 1.0) + 1.0;
	return alpSq / (pi * denom * denom);

	//return D;
}

float SchlickGGX(float NdotI, float NdotO, float roughness)
{
	float k = pow(roughness + 1, 2) / 8.0;
	float G1 = NdotI / (NdotI * (1.0 - k) + k);
	float G2 = NdotO / (NdotO * (1.0 - k) + k);
	return G1 * G2;
}

float ComputeLOD(float dist)
{

	float distMin = 10.0;
	float distMax = 20.0;

	float lod = 5.0 * saturate(dist / (distMax - distMin));
	return lod;
}

[earlydepthstencil]
PS_OUTPUT main(VS_OUTPUT input) 
{

	/// 클러스터 부분
	float3 posView = mul(float4(input.positionW, 1.0), gmtxView).xyz; //클러스터링을 위해 View Space로 가공
	uint clusterIndex = GetClusterIndex_PS(posView); //View Space 기준으로 클러스터링이 되어 있기에 view Space 기준으로 해당 Pixel의 영향을 주는 Grid 공간에 접근
	ClusterLightList cluster = gClusterLightLists[clusterIndex]; //해당 그리드 공간에 어떤 조명이 영향을 주는지 저장되어 있음.

	//uint count = gClusterLightLists[clusterIndex].lightCount;
	//PS_OUTPUT o;
	//o.pixelColor = float4(count / 10.0, 0, 0, 1);
	//o.nomalColor = float4(0, 0, 1, 1);
	//o.ambientLight = float4(0, 0, 0, 1);
	//return o;

	float3 albedo;
	float3 normalW;
	float  roughnessValue;
	float  metalnessValue;
	float3 emission;
	float  aoValue;
	float3 specularF0;
	float lod = 0;
	//환경광
	float3 ambientLighting = float3(0,0,0);

	//직사광 ==> 직접 빛에 영향을 받는 경우
	float3 directLighting = float3(0, 0, 0);

	//눈을 향하는 방향
	float3 pixelToEye = normalize(gf3EyeWorld - input.positionW);
	//pixelToEye = normalize(input.positionW - gf3EyeWorld);
	float3 pixelToEyeD = gf3EyeWorld - input.positionW;
	normalW = GetNormal(input);

	//최소한의 환경광이고 이제 IBL이 나오면 좀 내용이 달라진다.
	//ambientLighting = gMaterial.m_cAmbient;

	// Albedo ==> 사용안하거나 없는 경우 우선 그냥 하얀색 일종의 오류로 볼 수 도 있다
	lod = ComputeLOD(length(pixelToEyeD));

	// Albedo ==> 사용안하거나 없는 경우 우선 그냥 하얀색 일종의 오류로 볼 수 도 있다
	albedo =
		((gnTexturesMask & MATERIAL_ALBEDO_MAP) && (gnTextureUseMask & MATERIAL_ALBEDO_MAP_USE)) ?
		gtxtTextureAlbedo.SampleLevel(glinearSamplerState, input.uv, lod).rgb :
		gMaterial.m_cAlbedo.rgb;
	//albedo = float3(0, 0, 0);
	
	albedo = pow(albedo, gf1Gamma); //SRGB가 제대로 안들어오는 문제가 있어서 ㅈㅈ
	// Roughness
	roughnessValue =
		((gnTexturesMask & MATERIAL_ROUGHNESS_MAP) && (gnTextureUseMask & MATERIAL_ROUGHNESS_MAP_USE)) ?
		gtxtTextureRoughnness.SampleLevel(glinearSamplerState, input.uv, lod).r :
		gf1Roughness;
	//roughnessValue = 0;
	// Metalness
	metalnessValue =
		((gnTexturesMask & MATERIAL_METALNESS_MAP) && (gnTextureUseMask & MATERIAL_METALNESS_MAP_USE)) ?
		gtxtTextureMetal.SampleLevel(glinearSamplerState, input.uv, lod).r :
		gf1Metalness;
	//metalnessValue = 0
	// AO (Ambient Occlusion) ==> Ambient 인 환경광에만 영향을 주는 계산 값.
	aoValue =
		((gnTexturesMask & MATERIAL_AO_MAP) && (gnTextureUseMask & MATERIAL_AO_MAP_USE)) ?
		gtxtTextureOcclusion.SampleLevel(glinearSamplerState, input.uv, lod).r :
		1.0f; // AO가 없으면 영향 없음

	// Specular F0
	specularF0 =
		((gnTexturesMask & MATERIAL_SPECULARF0_MAP) && (gnTextureUseMask & MATERIAL_SPECULARF0_MAP_USE)) ?
		gtxtTextureSpecluarF0.SampleLevel(glinearSamplerState, input.uv, lod).rgb :
		F0return(albedo, metalnessValue);

	// Emission
	emission =
		((gnTexturesMask & MATERIAL_EMISSION_MAP) && (gnTextureUseMask & MATERIAL_EMISSION_MAP_USE)) ?
		gtxtTextureEmission.SampleLevel(glinearSamplerState, input.uv, lod).rgb :
		gMaterial.m_cEmissive;

	//DirectionLight는 클러스터링에 포함 x
	if (gnLightTypeMask & DIRECTION_LIGHT_USE)
	{
		Light dirLight = light[0];

		float3 lightVec = -dirLight.direction;
		float3 halfway = normalize(pixelToEye + lightVec);

		float NdotI = max(0.0, dot(normalW, lightVec));
		float NdotH = max(0.0, dot(normalW, halfway));
		float NdotO = max(0.0, dot(normalW, pixelToEye));

		float3 F0 = lerp(Fdielectric, albedo, metalnessValue);
		float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye)));

		float3 kd = lerp(1.0 - F, 0.0, metalnessValue);
		float3 diffuseBRDF = kd * albedo;

		float D = NdfGGX(NdotH, roughnessValue);
		float G = SchlickGGX(NdotI, NdotO, roughnessValue);
		float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

		float3 radiance =
			dirLight.bIsShadowSet && gnShadowOnOFF
			? LightRadiance(dirLight, input.positionW, normalW, shadowMaps[0])
			: LightRadiance(dirLight, input.positionW, normalW);

		directLighting += (diffuseBRDF + specularBRDF) * radiance * NdotI;
	}

	//클러스터링된 Light에 대하여 Spot 조명들만 처리하는 중
	for (uint i = 0; i < cluster.lightCount; ++i)
	{
		// cluster index 0 → light[1]
		uint lightIndex = cluster.lightIndices[i] + 1;

		Light L = light[lightIndex];
		if (L.bIsUse == 0) continue;

		float3 lightVec =  L.position - input.positionW;

		float lightDist = length(lightVec);
		lightVec /= lightDist;

		float3 halfway = normalize(pixelToEye + lightVec);

		float NdotI = max(0.0, dot(normalW, lightVec));
		float NdotH = max(0.0, dot(normalW, halfway));
		float NdotO = max(0.0, dot(normalW, pixelToEye));

		float3 F0 = lerp(Fdielectric, albedo, metalnessValue);
		float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye)));

		float3 kd = lerp(1.0 - F, 0.0, metalnessValue);
		float3 diffuseBRDF = kd * albedo;

		float D = NdfGGX(NdotH, roughnessValue);
		float G = SchlickGGX(NdotI, NdotO, roughnessValue);
		float3 specularBRDF =
			(F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

		float3 radiance =
			(L.bIsShadowSet && gnShadowOnOFF)
			? LightRadiance(L, input.positionW, normalW, shadowMaps[lightIndex])
			: LightRadiance(L, input.positionW, normalW);

		directLighting += (diffuseBRDF + specularBRDF) * radiance * NdotI;
	}

	//for (int i = 0; i < MAX_LIGHTS; ++i)
	//{
	//	if (light[i].bIsUse == 1)
	//	{
	//		float3 lightVec = light[i].type & LIGHT_DIRECTIONAL ? -light[i].direction : light[i].position - input.positionW;
	//		float lightDist = length(lightVec);
	//		lightVec /= lightDist;
	//		float3 halfway = normalize(pixelToEye + lightVec);

	//		float NdotI = max(0.0, dot(normalW, lightVec));
	//		float NdotH = max(0.0, dot(normalW, halfway));
	//		float NdotO = max(0.0, dot(normalW, pixelToEye));
	//		
	//		//Fresnel
	//		//float3 F = SchlickFresnel(specularF0, max(0.0, dot(halfway, pixelToEye)));
	//		float3 F0 = lerp(Fdielectric, albedo, metalnessValue);
	//		float3 F = SchlickFresnel(F0 , max(0.0, dot(halfway, pixelToEye)));

	//		//Diffuse
	//		float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalnessValue);
	//		float3 diffuseBRDF = kd * albedo;

	//		float D = NdfGGX(NdotH, roughnessValue);
	//		float3 G = SchlickGGX(NdotI, NdotO, roughnessValue);
	//		float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

	//		float3 radiance = 0.0f;

	//		if (light[i].bIsShadowSet && gnShadowOnOFF) // 그림자를 지원하는 경우
	//		{
	//			radiance = LightRadiance(light[i], input.positionW, normalW, shadowMaps[i]);
	//		}
	//		else //그림자를 지원하지 않는 경우
	//		{
	//			radiance = LightRadiance(light[i], input.positionW, normalW);
	//		}

	//		directLighting += (diffuseBRDF + specularBRDF) * radiance * NdotI;
	//	}
	//}

	ambientLighting = AmbientLightingByIBL(albedo, normalW, pixelToEye, aoValue, metalnessValue, roughnessValue);
	//DIRECT_LIGHT_USE, AMBIENT_LIGHT_USE

	//재질이 어떤 조명 타입에 영향을 받을지를 지정해주는 것.
	ambientLighting = gnLightTypeMask & AMBIENT_LIGHT_USE ? ambientLighting : float3(0, 0, 0);
	directLighting = gnLightTypeMask & DIRECT_LIGHT_USE ? directLighting : float3(0, 0, 0);

	ambientLighting = ambientLighting * 0.1;

	PS_OUTPUT output;
	//최종 색상은 환경광과 직접광 2개의 합으로 이루어진다.
	//output.pixelColor = float4(ambientLighting + directLighting, 1.0);
	output.pixelColor = float4(directLighting, 1.0);
	//output.pixelColor = clamp(output.pixelColor, 0.0, 1000.0);
	float3 normalV = normalize(mul(normalW, (float3x3)gmtxView));
	output.nomalColor = float4(normalV, 1.0);
	output.ambientLight = float4(ambientLighting, 1.0);
	//output.nomalColor = float4(normalW, 1.0);
	//output.pixelColor = float4(albedo,1.0);


	return(output);
}