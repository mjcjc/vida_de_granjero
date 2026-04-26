#include "Common.hlsli"

//TODO: Texture 몇개 쓸껀지 구분 생각해야함
Texture2D gtxtTextureAlbedo1 : register(t40);
Texture2D gtxtTextureNormal1 : register(t41);
Texture2D gtxtTextureMaskMap1 : register(t42);

Texture2D gtxtTextureAlbedo2 : register(t43);
Texture2D gtxtTextureNormal2 : register(t44);
Texture2D gtxtTextureMaskMap2 : register(t45);

Texture2D gtxtTextureAlbedo3 : register(t46);
Texture2D gtxtTextureNormal3 : register(t47);
Texture2D gtxtTextureMaskMap3 : register(t48);

Texture2D gtxtTextureMasking01 : register(t49); //Terrain MaskingTexture
Texture2D gtxtTextureMasking02 : register(t50);

Texture2D gtxtTextureSpecluarBRDF : register(t11); // t11 : IBL BRDF LUT
TextureCube gtxtTextureSpecluarIBL : register(t12); // t12 : Specluar IBL
TextureCube gtxtTextureDiffuseIrradianceIBL : register(t13); // t13 : Irradiance

Texture2D gtxtTextureAlbedo4 : register(t51);
Texture2D gtxtTextureNormal4 : register(t52);
Texture2D gtxtTextureMaskMap4 : register(t53);


static const float gParallaxScale = 0.03f; //시차 매핑을 위한 상수
static const float gParallaxBias = -0.015f; // 시차 매핑을 위한 상수

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
	float2 uv : TEXCOORD0;
	float2 uv2 : TEXCOORD1;
};

//노멀 매핑 함수
float3 GetNormal(VS_OUTPUT input , Texture2D normalMap)
{
	float3 normalW = input.normalW;

	//노멀맵을 사용할 것이며 노멀맵이 존재하는 경우
	if (gnTextureUseMask & MATERIAL_NORMAL_MAP_USE && gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3 normal = normalMap.SampleLevel(glinearSamplerState, input.uv, 0.0).rgb; // 샘플링
		normal = 2.0 * normal - 1.0; //범위 -1 ~ 1 변경 필요

		//만약 OpenGL용 노멀맵에 경우 y가 반대라는 것도 알아야 한다.

		float3 N = normalW;
		//float3 T = normalize(input.tangentW - dot(input.tangentW, N) * N); //Tangent가 N과 수직이 아닐 수 있기에 만드는 것
		float3 T = float3(1, 0, 0);
		float3 B = cross(N, T);

		float3x3 TBN = float3x3(T, B, N);
		normalW = normalize(mul(normal, TBN));
	}

	return normalW;
}

//프레넬 방정식을 계산하여 값을 구하는 과정
float3 SchlickFresnel(float3 F0, float NdotH)
{

	float3 F = F0 + (float3(1.0, 1.0, 1.0) - F0) * pow(2.0, (-5.55473 * NdotH - 6.98316) * NdotH);

	return F;
}


float3 DiffuseIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
	float metallic)
{
	float3 F0 = lerp(Fdielectric, albedo, metallic);    //F0의 비율이 얼만큼 넣을지를 metallic으로 조절 해주고 있다.
	float3 F = SchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
	float3 kd = lerp(1.0 - F, 0.0, metallic);

	float3 irradiance = gtxtTextureDiffuseIrradianceIBL.Sample(glinearSamplerState, normalWorld);
	irradiance *= albedo;

	return kd * irradiance;
}


float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
	float metallic, float roughness)
{
	// Environment BRDF
	float R = 1 - roughness;
	float c1 = dot(normalWorld, pixelToEye);
	//SpecularBRDF의 x = cos , y = roughness 값
	float2 specularBRDF = gtxtTextureSpecluarBRDF.Sample(gpointSamplerState, float2(c1, R)).rg;

	//러프니스를 넣음으로써 거칠기가 오를 수록 SpecularIBL에 값을 더욱 흐리게 만들어 마치 거친 것과 같은
	//효과를 연출. 저 5.0f는 해당 맵에 밉맵Level에 맞춰둔것
	float3 specularIrradiance = gtxtTextureSpecluarIBL.SampleLevel(glinearSamplerState, 
		reflect(-pixelToEye, normalWorld), roughness * 5.0f).rgb;

	const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
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

//밉맵 LOD 적용을 위한 계산 함수
float ComputeLOD(float dist)
{

	float distMin = 3.0;
	float distMax = 20.0;

	float lod = 10.0 * saturate(dist / (distMax - distMin));
	return lod;
}


struct PS_OUTPUT
{
	float4 pixelColor : SV_Target0;
	float4 nomalColor : SV_Target1;
	float4 ambientLight : SV_Target2;
};
[earlydepthstencil]
PS_OUTPUT  main(VS_OUTPUT input) : SV_TARGET
{
	//그리드 공간에 저장된 조명에 값에 인덱스를 읽어들이는 과정
	float3 posView = mul(float4(input.positionW, 1.0), gmtxView).xyz; //
	uint clusterIndex = GetClusterIndex_PS(posView); //
	ClusterLightList cluster = gClusterLightLists[clusterIndex];

	float3 albedo;
	float3 normalW;
	float  roughnessValue;
	float  metalnessValue;
	float3 emission;
	float  aoValue;
	float3 specularF0;

	//lod
	float lod = 0;

	//환경광
	float3 ambientLighting = float3(0, 0, 0);

	//직사광 ==> 직접 빛에 영향을 받는 경우
	float3 directLighting = float3(0, 0, 0);

	//눈을 향하는 방향
	float3 pixelToEyeD = gf3EyeWorld - input.positionW;

	float3 pixelToEye = normalize(pixelToEyeD);

	lod = ComputeLOD(length(pixelToEyeD));

	normalW = GetNormal(input , gtxtTextureNormal1);

	float4 unityMask1 =
		gtxtTextureMaskMap1.SampleLevel(glinearSamplerState, input.uv, lod);

	float4 unityMask2 =
		gtxtTextureMaskMap2.SampleLevel(glinearSamplerState, input.uv, lod);

	float4 unityMask3 =
		gtxtTextureMaskMap3.SampleLevel(glinearSamplerState, input.uv, lod);

	float4 unityMask4 =
		gtxtTextureMaskMap4.SampleLevel(glinearSamplerState, input.uv, lod);


	float height1 = unityMask1.b;
	float height2 = unityMask2.b;
	float height3 = unityMask3.b;
	float height4 = unityMask4.b;

	// UV 보정 (간이 Parallax)
	float2 parallaxOffset1 =
		(pixelToEye.xy / max(pixelToEye.z, 0.2f)) *
		(height1 * gParallaxScale + gParallaxBias);

	float2 parallaxOffset2 =
		(pixelToEye.xy / max(pixelToEye.z, 0.2f)) *
		(height2 * gParallaxScale + gParallaxBias);

	float2 parallaxOffset3 =
		(pixelToEye.xy / max(pixelToEye.z, 0.2f)) *
		(height3 * gParallaxScale + gParallaxBias);

	float2 parallaxOffset4 =
		(pixelToEye.xy / max(pixelToEye.z, 0.2f)) *
		(height4 * gParallaxScale + gParallaxBias);

	// UV 보정 적용
	float2 uvParallax1 = input.uv + parallaxOffset1;
	float2 uvParallax2 = input.uv + parallaxOffset2;
	float2 uvParallax3 = input.uv + parallaxOffset3;
	float2 uvParallax4 = input.uv + parallaxOffset4;

	 unityMask1 =
		gtxtTextureMaskMap1.SampleLevel(glinearSamplerState, uvParallax1, lod);

	unityMask2 =
		gtxtTextureMaskMap2.SampleLevel(glinearSamplerState, uvParallax2, lod);

	unityMask3 =
		gtxtTextureMaskMap3.SampleLevel(glinearSamplerState, uvParallax3, lod);

	unityMask4 =
		gtxtTextureMaskMap4.SampleLevel(glinearSamplerState, uvParallax4, lod);


	albedo =
		((gnTexturesMask & MATERIAL_ALBEDO_MAP) && (gnTextureUseMask & MATERIAL_ALBEDO_MAP_USE)) ?
		gtxtTextureAlbedo1.SampleLevel(glinearSamplerState, uvParallax1, lod).rgb :
		gMaterial.m_cAlbedo.rgb;

	albedo = pow(albedo, gf1Gamma);

	roughnessValue =
		((gnTexturesMask & MATERIAL_ROUGHNESS_MAP) && (gnTextureUseMask & MATERIAL_ROUGHNESS_MAP_USE)) ?
		1.0 - unityMask1.a :
		gf1Roughness;


	metalnessValue =
		((gnTexturesMask & MATERIAL_METALNESS_MAP) && (gnTextureUseMask & MATERIAL_METALNESS_MAP_USE)) ?
		unityMask1.r :
		gf1Metalness;


	// AO (Ambient Occlusion) ==> Ambient 인 환경광에만 영향을 주는 계산 값.
	aoValue =
		((gnTexturesMask & MATERIAL_AO_MAP) && (gnTextureUseMask & MATERIAL_AO_MAP_USE)) ?
		unityMask1.g :
		1.0f; // AO가 없으면 영향 없음

	// Specular F0
	specularF0 = F0return(albedo, metalnessValue);

	// Emission
	emission = gMaterial.m_cEmissive;


	float mask1 = gtxtTextureMasking01.SampleLevel(glinearSamplerState, input.uv2, 0).r;
	float mask2 = gtxtTextureMasking02.SampleLevel(glinearSamplerState, input.uv2, 0).r;

	// Layer 2
	if (mask1 > 0.0)
	{

		if (gnSeason & SEASON_SUMMER || gnSeason & SEASON_WINTER)
		{
			albedo =
				((gnTexturesMask & MATERIAL_ALBEDO_MAP) && (gnTextureUseMask & MATERIAL_ALBEDO_MAP_USE)) ?
				gtxtTextureAlbedo4.SampleLevel(glinearSamplerState, uvParallax4, lod).rgb :
				albedo;

			roughnessValue =
				((gnTexturesMask & MATERIAL_ROUGHNESS_MAP) && (gnTextureUseMask & MATERIAL_ROUGHNESS_MAP_USE)) ?
				1.0 - unityMask4.a :
				gf1Roughness;


			metalnessValue =
				((gnTexturesMask & MATERIAL_METALNESS_MAP) && (gnTextureUseMask & MATERIAL_METALNESS_MAP_USE)) ?
				unityMask4.r :
				gf1Metalness;


			// AO (Ambient Occlusion) ==> Ambient 인 환경광에만 영향을 주는 계산 값.
			aoValue =
				((gnTexturesMask & MATERIAL_AO_MAP) && (gnTextureUseMask & MATERIAL_AO_MAP_USE)) ?
				unityMask4.g :
				1.0f; // AO가 없으면 영향 없음

			normalW = GetNormal(input, gtxtTextureNormal4);
		}
		else
		{
			albedo =
				((gnTexturesMask & MATERIAL_ALBEDO_MAP) && (gnTextureUseMask & MATERIAL_ALBEDO_MAP_USE)) ?
				gtxtTextureAlbedo2.SampleLevel(glinearSamplerState, uvParallax2, lod).rgb :
				albedo;

			roughnessValue =
				((gnTexturesMask & MATERIAL_ROUGHNESS_MAP) && (gnTextureUseMask & MATERIAL_ROUGHNESS_MAP_USE)) ?
				1.0 - unityMask2.a :
				gf1Roughness;


			metalnessValue =
				((gnTexturesMask & MATERIAL_METALNESS_MAP) && (gnTextureUseMask & MATERIAL_METALNESS_MAP_USE)) ?
				unityMask2.r :
				gf1Metalness;


			// AO (Ambient Occlusion) ==> Ambient 인 환경광에만 영향을 주는 계산 값.
			aoValue =
				((gnTexturesMask & MATERIAL_AO_MAP) && (gnTextureUseMask & MATERIAL_AO_MAP_USE)) ?
				unityMask2.g :
				1.0f; // AO가 없으면 영향 없음
			normalW = GetNormal(input, gtxtTextureNormal2);
		}
	}
	// Layer 3 (우선순위 높음)
	if (mask2 > 0.0)
	{
		albedo =
			((gnTexturesMask & MATERIAL_ALBEDO_MAP) && (gnTextureUseMask & MATERIAL_ALBEDO_MAP_USE)) ?
			gtxtTextureAlbedo3.SampleLevel(glinearSamplerState, uvParallax3, lod).rgb :
			albedo;

		roughnessValue =
			((gnTexturesMask & MATERIAL_ROUGHNESS_MAP) && (gnTextureUseMask & MATERIAL_ROUGHNESS_MAP_USE)) ?
			1.0 - unityMask3.a :
			gf1Roughness;


		metalnessValue =
			((gnTexturesMask & MATERIAL_METALNESS_MAP) && (gnTextureUseMask & MATERIAL_METALNESS_MAP_USE)) ?
			unityMask3.r :
			gf1Metalness;


		// AO (Ambient Occlusion) ==> Ambient 인 환경광에만 영향을 주는 계산 값.
		aoValue =
			((gnTexturesMask & MATERIAL_AO_MAP) && (gnTextureUseMask & MATERIAL_AO_MAP_USE)) ?
			unityMask3.g :
			1.0f; // AO가 없으면 영향 없음

		normalW = GetNormal(input, gtxtTextureNormal3);
	}

	albedo = pow(albedo, gf1Gamma);
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

		float3 lightVec = L.position - input.positionW;

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

	//		//Fresnel
	//		float3 F = SchlickFresnel(specularF0, max(0.0, dot(halfway, pixelToEye)));

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
	//float4 albe2do = gtxtTextureAlbedo.Sample(glinearSamplerState, input.uv);

	ambientLighting = ambientLighting * 0.1;
	PS_OUTPUT output;
	//최종 색상은 환경광과 직접광 2개의 합으로 이루어진다.
	//output.pixelColor = float4(ambientLighting + directLighting, 1.0);
	output.pixelColor = float4(directLighting, 1.0);
	output.pixelColor = clamp(output.pixelColor, 0.0, 1000.0);
	float3 normalV = normalize(mul(normalW, (float3x3)gmtxView));
	output.nomalColor = float4(normalV, 1.0);
	output.ambientLight = float4(ambientLighting,1.0);
	//output.nomalColor = float4(normalW, 1.0);
	//out.pixelColor = color;
	return(output);

}