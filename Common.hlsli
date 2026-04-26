#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

#define FRAME_BUFFER_WIDTH 640.0f
#define FRAME_BUFFER_HEIGHT 480.0f

#define MAX_LIGHTS_PER_CLUSTER 128

#define MAX_LIGHTS 9 //이거 이제 안씀 클러스터드 렌더링에선 필요 x
#define POINT_LIGHT 1
#define SPOT_LIGHT 8
#define DIRECTIONAL_LIGHT 1

#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256


//오브젝트 TextureMAP
#define MATERIAL_ALBEDO_MAP        0x01    // 0000 0001
#define MATERIAL_NORMAL_MAP        0x02    // 0000 0010
#define MATERIAL_ROUGHNESS_MAP     0x04    // 0000 0100
#define MATERIAL_METALNESS_MAP     0x08    // 0000 1000
#define MATERIAL_AO_MAP            0x10    // 0001 0000
#define MATERIAL_SPECULARF0_MAP    0x20    // 0010 0000
#define MATERIAL_EMISSION_MAP      0x40    // 0100 0000
#define MATERIAL_SMOOTHNESS_MAP 0x80 // 1000 0000

// On/Off용 플래그 _MAP_USE
#define MATERIAL_ALBEDO_MAP_USE        0x01
#define MATERIAL_NORMAL_MAP_USE        0x02
#define MATERIAL_ROUGHNESS_MAP_USE     0x04
#define MATERIAL_METALNESS_MAP_USE     0x08
#define MATERIAL_AO_MAP_USE            0x10
#define MATERIAL_SPECULARF0_MAP_USE    0x20
#define MATERIAL_EMISSION_MAP_USE      0x40

#define SEASON_SPRING 0x01
#define SEASON_SUMMER 0x02
#define SEASON_FALL 0x04
#define SEASON_WINTER 0x08

// On/Off용 플래그  PBR, Direct, Ambient 활성화 여부
#define DIRECT_LIGHT_USE        0x01
#define AMBIENT_LIGHT_USE        0x02
#define DIRECTION_LIGHT_USE    0x04 //Direction Light를 꺼서 클러스터드 렌더링에 성능 테스트를 보여주기 위해

//Direction Light인지 Spot Light인지 
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_SPOT        0x02

#define WEATHER_CLEAR   0x01   // 맑음
#define WEATHER_RAIN    0x02   // 비
#define WEATHER_SNOW    0x04   // 눈
#define WEATHER_DUST    0x08   // 황사 / 먼지
#define WEATHER_FOG     0x10   // 안개

// 재질
struct Material  //우선 블린퐁 모델 기준으로 Materail 정의 나중에 변경 가능
{
    float3 ambient;
    float shininess;
    float3 diffuse;
    float dummy1; // 16 bytes 맞춰주기 위해 추가
    float3 specular;
    float dummy2;
};

static const Material g_DebugMaterial =
{
    /* ambient   */
    float3(0.1f, 0.1f, 0.1f),
    /* shininess */ 32.0f,

    /* diffuse   */ float3(0.8f, 0.8f, 0.8f),
    /* dummy1    */ 0.0f,

    /* specular  */ float3(1.0f, 1.0f, 1.0f),
    /* dummy2    */ 0.0f
};

// 조명
struct Light
{
    float3 radiance; //강도
    float fallOffStart; // 범위 시작 
    float3 direction; // 방향
    float fallOffEnd; //범위 끝
    float3 position; //위치
    float spotPower; //spot 조명에 경우 모이는 힘
    
    float radius;
    uint bIsShadowSet; //0 : bool, 1 : true
    uint bIsUse; //0 : bool, 1 : true
    uint type; //조명 type
    
    Matrix viewProj; //Shadow Map 연산에사용해야함
    Matrix invProj;
    
    
    
};

struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cAlbedo;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
    float m_cRoughness;
    float m_cSmoothness;
    float m_Metallic;
};

//Particle 설정에 필요
#define PF_RAIN         0x01    
#define PF_SNOW         0x02   
#define PF_DUST         0x04   
#define PF_PUNCH         0x08   //무언가 캘때 나오는 파티클을 생각중

// Particle Behavior Flags (상위 비트)
#define PF_USE_COLOR    0x0100  // (1 << 8)  색상 적용 (황사/연기)
#define PF_USE_NOISE    0x0200  // (1 << 9)  노이즈 이동
#define PF_USE_WIND     0x0400  // (1 << 10) 바람 영향

#define MAX_BLOCK 4  // 

struct WeatherBlock
{
    float3 center;
    float _pad0; // 16-byte align

    float3 extent;
    float _pad1; // 16-byte align
};

cbuffer cbParticle : register(b15)
{
    float fTime : packoffset(c0);
    float fDeltaTime : packoffset(c0.y);
    float fStartTime : packoffset(c0.z);
    float fSpawnRadius : packoffset(c0.w);
    
    float fAlphaScale : packoffset(c1.x);
    float3 _pad1 : packoffset(c1.y); // cbuffer alignment
    
    uint nParticleFlags : packoffset(c2.x);
    float3 gParticleColor : packoffset(c2.y);

    uint blockCount : packoffset(c3.x);
    float3 _pad2 : packoffset(c3.y); // cbuffer alignment

    WeatherBlock blocks[MAX_BLOCK] : packoffset(c4.x);
};

static const MATERIAL gMaterial =
{
    /* m_cAmbient   */
    float4(0.03, 0.03, 0.03, 1.0),
    /* m_cAlbedo    */ float4(1.0, 1.0, 1.0, 1.0),
    /* m_cSpecular  */ float4(1.0, 1.0, 1.0, 32.0),
    /* m_cEmissive  */ float4(0.0, 0.0, 0.0, 1.0),
    /* m_cRoughness */ 0.5,
    /* m_cSmoothness*/ 0.5,
    /* m_Metallic   */ 0.0
};

SamplerState glinearSamplerState : register(s0);
SamplerState gpointSamplerState : register(s1);
SamplerComparisonState shadowCompareSampler : register(s2);
SamplerState shadowPointSampler : register(s3);
SamplerState gLinearmipPointSampler : register(s4);


/*
CBV를 직접 연결하는건 
Camera b0
Player의 좌표 b1
조명은 b10
게임 오브젝트는 b2
BoneOffset b11
BoneTransforms b12
*/

cbuffer cbGameObjectInfo : register(b2)
{
    matrix gmtxWorld : packoffset(c0);
   // MATERIAL gMaterial : packoffset(c4); //개별로 갖고있는 경우에 대해서
   // uint gnTexturesMask : packoffset(c8);
};

/*
cbuffer cbGameObjectMaterial : register(b3)
{
    matrix gmtxWorldCb : packoffset(c0); //
    MATERIAL gMaterial : packoffset(c4);
    uint gnTexturesMask : packoffset(c8.w);
    uint gnIsBuildingObj : packoffset(c9);
};*/

cbuffer cbGameObjectMaterial : register(b3)
{
    matrix gmtxWorldCb : packoffset(c0); //
    uint gnTexturesMask : packoffset(c4.x);
    uint gnIsBuildingObj : packoffset(c4.y);
};

cbuffer cbImGuiMaterialFlages : register(b4)
{
    uint gnTextureUseMask : packoffset(c0);
    float3 gf3Dummy : packoffset(c0.y);
    uint gnLightTypeMask : packoffset(c1.x);
    float3 gf3Dummy2 : packoffset(c1.y);
    float gf1Exposure : packoffset(c2.x);
    float gf1Gamma : packoffset(c2.y);
    float gf1Roughness : packoffset(c2.z);
    float gf1Metalness : packoffset(c2.w);
    uint gnShadowOnOFF : packoffset(c3.x);
    uint gnSeason : packoffset(c3.y);
    uint gnWeather : packoffset(c3.z);
};

cbuffer cbCameraInfo : register(b0)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProj : packoffset(c4);
    matrix gmtxinvProj : packoffset(c8);
    float3 gf3EyeWorld : packoffset(c12); //카메라의 월드상 좌표
};

cbuffer cbPlayerInfo : register(b1)
{
    matrix gmtxplayerWorld : packoffset(c0);
};



cbuffer cbBoneOffsets : register(b11)
{
    float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b12)
{
    float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

cbuffer cbLighht : register(b10)
{
    Light light[MAX_LIGHTS];
};

struct ClusterLightList
{
    uint lightCount; // 이 클러스터에 들어온 조명 수
    uint lightIndices[MAX_LIGHTS_PER_CLUSTER]; // gLights의 인덱스
};

StructuredBuffer<ClusterLightList> gClusterLightLists : register(t15);

cbuffer ClusterCB : register(b16)
{
    uint3 clusterDim; // (X, Y, Z)
    uint totalClusters; // X * Y * Z

    float nearZ;
    float farZ;

    float2 screenSize;
    float2 invScreenSize;
};

cbuffer cbShadowMapIndex : register(b5)
{
    uint nShadowIndex : packoffset(c0);
};

#define NEAR_PLANE 2
// #define LIGHT_WORLD_RADIUS 0.001
//#define LIGHT_FRUSTUM_WIDTH 0.34641 // <- 계산해서 찾은 값
#define LIGHT_FRUSTUM_WIDTH 0.11547
//그림자 맵
Texture2D shadowMaps[5] : register(t30);



static const float2 poissonDisk[16] =
{
    float2(-0.94201624, -0.39906216), float2(0.94558609, -0.76890725),
            float2(-0.094184101, -0.92938870), float2(0.34495938, 0.29387760),
            float2(-0.91588581, 0.45771432), float2(-0.81544232, -0.87912464),
            float2(-0.38277543, 0.27676845), float2(0.97484398, 0.75648379),
            float2(0.44323325, -0.97511554), float2(0.53742981, -0.47373420),
            float2(-0.26496911, -0.41893023), float2(0.79197514, 0.19090188),
            float2(-0.24188840, 0.99706507), float2(-0.81409955, 0.91437590),
            float2(0.19984126, 0.78641367), float2(0.14383161, -0.14100790)
};

static const float2 offsets[9] =
{
    float2(-1, -1), float2(0, -1), float2(1, -1),
            float2(-1, 0), float2(0, 0), float2(1, 0),
            float2(-1, +1), float2(0, +1), float2(1, +1)
};

static const float2 diskSamples64[64] =
{
    float2(0.0, 0.0),
            float2(-0.12499844227275288, 0.000624042775189866), float2(0.1297518688031755, -0.12006020382326336),
            float2(-0.017851253586055427, 0.21576916541852392), float2(-0.1530983013115895, -0.19763833164521946),
            float2(0.27547541035593626, 0.0473106572479027), float2(-0.257522587854559, 0.16562643733622642),
            float2(0.0842605283808073, -0.3198048832600703), float2(0.1645196099088727, 0.3129429627830483),
            float2(-0.3528833088400373, -0.12687935349026194), float2(0.36462214742013344, -0.1526456341030772),
            float2(-0.17384046457324884, 0.37637015407303087), float2(-0.1316547617859344, -0.4125130588224921),
            float2(0.3910687393754993, 0.2240317858770442), float2(-0.45629121277761536, 0.10270505898899496),
            float2(0.27645268679640483, -0.3974278701387824), float2(0.06673001731984558, 0.49552709793561556),
            float2(-0.39574431915605623, -0.33016879600548193), float2(0.5297612167716342, -0.024557141621887494),
            float2(-0.3842909284448636, 0.3862583103507092), float2(0.0230336562454131, -0.5585422550532486),
            float2(0.36920334463249477, 0.43796562686149154), float2(-0.5814490172413539, -0.07527974727019048),
            float2(0.4903718680780365, -0.3448339179919178), float2(-0.13142003698572613, 0.5981043168868373),
            float2(-0.31344141845114937, -0.540721256470773), float2(0.608184438565748, 0.19068741092811003),
            float2(-0.5882602609696388, 0.27536315179038107), float2(0.25230610046544444, -0.6114259003901626),
            float2(0.23098706800827415, 0.6322736546883326), float2(-0.6076303951666067, -0.31549215975943595),
            float2(0.6720886334230931, -0.1807536135834609), float2(-0.37945598830371974, 0.5966683776943834),
            float2(-0.1251555455510758, -0.7070792667147104), float2(0.5784815570900413, 0.44340623372555477),
            float2(-0.7366710399837763, 0.0647362251696953), float2(0.50655463562529, -0.553084443034271),
            float2(8.672987356252326e-05, 0.760345311340794), float2(-0.5205650355786364, -0.5681215043747359),
            float2(0.7776435491294021, 0.06815798190547596), float2(-0.6273416101921778, 0.48108471615868836),
            float2(0.1393236805531513, -0.7881712453757264), float2(0.4348773806743975, 0.6834703093608201),
            float2(-0.7916014213464706, -0.21270211499241704), float2(0.7357897682897174, -0.38224784745000717),
            float2(-0.2875567908732709, 0.7876776574352392), float2(-0.3235695699691864, -0.7836151691933712),
            float2(0.7762165924462436, 0.3631291803355136), float2(-0.8263007976064866, 0.2592816844184794),
            float2(0.4386452756167397, -0.7571098481588484), float2(0.18988542402304126, 0.8632459242554175),
            float2(-0.7303253445407815, -0.5133224046555819), float2(0.8939004035324556, -0.11593993515830946),
            float2(-0.5863762307291154, 0.6959079795748251), float2(-0.03805753378232556, -0.9177699189461416),
            float2(0.653979655650389, 0.657027860897389), float2(-0.9344208130797295, -0.04310155546401203),
            float2(0.7245109901504777, -0.6047386420191574), float2(-0.12683493131695708, 0.9434844461875473),
            float2(-0.5484582700240663, -0.7880790100251422), float2(0.9446610338564589, 0.2124041692463835),
            float2(-0.8470120123194587, 0.48548496473788055), float2(0.29904134279525085, -0.9377229203230629),
            float2(0.41623562331748715, 0.9006236205438447),
};

// NdcDepthToViewDepth
float N2V(float ndcDepth, matrix invProj)
{
    float4 pointView = mul(float4(0, 0, ndcDepth, 1), invProj);
    return pointView.z / pointView.w;
}

//PCF 연산
float PCF_Filter(float2 uv, float zReceiverNdc, float filterRadiusUV, Texture2D shadowMap)
{
    float sum = 0.0f;
    for (int i = 0; i < 9; ++i)
    {
        float2 offset = offsets[i] * filterRadiusUV;
        sum += shadowMap.SampleCmpLevelZero(shadowCompareSampler, uv + offset, zReceiverNdc);
    }
    return sum / 9.0;
}

// void Func(out float a) <- c++의 void Func(float& a) 처럼 출력값 저장 가능

void FindBlocker(out float avgBlockerDepthView, out float numBlockers, float2 uv,
                 float zReceiverView, Texture2D shadowMap, matrix invProj, float lightRadiusWorld)
{
    float lightRadiusUV = lightRadiusWorld / LIGHT_FRUSTUM_WIDTH; //둘 다 월드 좌표계 기준 반지름과 넓이 => Near Plane 월드 기준 크기 대비 Light의 반지름이 얼마나 큰지 알 수 있다.
    
    float searchRadius = lightRadiusUV * (zReceiverView - NEAR_PLANE) / zReceiverView;

    float blockerSum = 0;
    numBlockers = 0;
    for (int i = 0; i < 16; ++i)
    {
        float shadowMapDepth =
            shadowMap.SampleLevel(shadowPointSampler, float2(uv + poissonDisk[i] * searchRadius), 0).r;

        shadowMapDepth = N2V(shadowMapDepth, invProj);
        
        if (shadowMapDepth < zReceiverView)
        {
            blockerSum += shadowMapDepth;
            numBlockers++;
        }
    }
    avgBlockerDepthView = blockerSum / numBlockers;
}

float PCSS(float2 uv, float zReceiverNdc, Texture2D shadowMap, matrix invProj, float lightRadiusWorld)
{
    float lightRadiusUV = lightRadiusWorld / LIGHT_FRUSTUM_WIDTH;
    
    float zReceiverView = N2V(zReceiverNdc, invProj);
    
    // STEP 1: blocker search
    float avgBlockerDepthView = 0;
    float numBlockers = 0;

    FindBlocker(avgBlockerDepthView, numBlockers, uv, zReceiverView, shadowMap, invProj, lightRadiusWorld);

    if (numBlockers < 1)
    {
        // There are no occluders so early out(this saves filtering)
        return 1.0f;
    }
    else
    {
        // STEP 2: penumbra size
        float penumbraRatio = (zReceiverView - avgBlockerDepthView) / avgBlockerDepthView;
        float filterRadiusUV = penumbraRatio * lightRadiusUV * NEAR_PLANE / zReceiverView;

        // STEP 3: filtering
        return PCF_Filter(uv, zReceiverNdc, filterRadiusUV, shadowMap);
    }
}

//그림자 맵을 염두해두고 계산하는 함수
float3 LightRadiance(Light light, float3 posWorld, float3 normalWorld, Texture2D shadowMap)
{
    // Directional light Direction Light라면
    float3 lightVec = light.type & LIGHT_DIRECTIONAL
                      ? -light.direction
                      : light.position - posWorld;
        
    float lightDist = length(lightVec);
    lightVec /= lightDist;

    // Spot light ==> 감쇠 모델이 정욕되어야 함으로
    float spotFator = light.type & LIGHT_SPOT
                     ? pow(max(-dot(lightVec, light.direction), 0.0f), light.spotPower)
                      : 1.0f;
        
    // Distance attenuation
    float att = light.type & LIGHT_SPOT ? saturate((light.fallOffEnd - lightDist)
                         / (light.fallOffEnd - light.fallOffStart)) : 1.0f;

    // Shadow map
    float shadowFactor = 1.0;

    
    const float nearZ = 0.1; // 카메라 설정과 동일
        
     // 1. Project posWorld to light screen    
    float4 lightScreen = mul(float4(posWorld, 1.0), light.viewProj);
    lightScreen.xyz /= lightScreen.w;
        
     // 2. 카메라(광원)에서 볼 때의 텍스춰 좌표 계산
    float2 lightTexcoord = float2(lightScreen.x, -lightScreen.y);
    lightTexcoord += 1.0;
    lightTexcoord *= 0.5;
        
    uint width, height, numMips;
    shadowMap.GetDimensions(0, width, height, numMips);
        
    // Texel size
    float dx = 2.0 / (float) width;
    
    if (lightTexcoord.x < 0.0 || lightTexcoord.x > 1.0 ||
        lightTexcoord.y < 0.0 || lightTexcoord.y > 1.0)
    {
        shadowFactor = 1.0;
    }
    else
    {
   
        shadowFactor = PCF_Filter(lightTexcoord.xy, lightScreen.z - 0.001, dx, shadowMap);
        //shadowFactor = PCF_Filter(lightTexcoord.xy, lightScreen.z - 0.0000001, dx, shadowMap);
        //shadowFactor = PCSS(lightTexcoord, lightScreen.z - 0.01, shadowMap, light.invProj, light.radius);
    }

    float3 radiance = light.radiance * spotFator * att * shadowFactor;

    return radiance;
}

//그림자 맵 자체가 부여되지 않은 조명에 대한 강도 함수
float3 LightRadiance(in Light light, in float3 posWorld, in float3 normalWorld)
{
    // Directional light
    float3 lightVec = light.type & LIGHT_DIRECTIONAL
                      ? -light.direction
                      : light.position - posWorld;
        
    float lightDist = length(lightVec);
    lightVec /= lightDist;

    // Spot light
    float spotFator = light.type & LIGHT_SPOT
                     ? pow(max(-dot(lightVec, light.direction), 0.0f), light.spotPower)
                      : 1.0f;
        
    // Distance attenuation
    float att = light.type & LIGHT_SPOT ? saturate((light.fallOffEnd - lightDist)
                         / (light.fallOffEnd - light.fallOffStart)) : 1.0f;

    // Shadow map
    float shadowFactor = 1.0;
    float3 radiance = light.radiance * spotFator * att * shadowFactor;

    return radiance;
}

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal,
                   float3 toEye, Material mat)
{
    float3 halfway = normalize(toEye + lightVec);
    float hdotn = dot(halfway, normal);
    float3 specular = mat.specular * pow(max(hdotn, 0.0f), mat.shininess);

    return mat.ambient + (mat.diffuse + specular) * lightStrength;
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal,
                                float3 toEye)
{
    float3 lightVec = -L.direction;

    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.radiance * ndotl;

    // Luna DX12 책에서는 Specular 계산에도
    // Lambert's law가 적용된 lightStrength를 사용합니다.
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal,
                         float3 toEye)
{
    float3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        lightVec /= d;

        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.radiance * ndotl;

        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= att;

        float spotFactor = pow(max(-dot(lightVec, L.direction), 0.0f), L.spotPower);
        lightStrength *= spotFactor;

        return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
    }
    
    // if에 else가 없을 경우 경고 발생
    // warning X4000: use of potentially uninitialized variable
}

uint GetClusterIndex_PS(float3 posView)
{
	// Screen space (View → Clip → NDC → [0,1])
    float4 clip = mul(float4(posView, 1.0), gmtxProj);
    float2 ndc = clip.xy / clip.w;
    ndc = ndc * 0.5f + 0.5f;
    ndc = saturate(ndc);

    uint x = min(clusterDim.x - 1, uint(ndc.x * clusterDim.x));
    uint y = min(clusterDim.y - 1, uint(ndc.y * clusterDim.y));

	// Z slice (View space 기준)
    float z = posView.z; // LH, forward +
    float zNorm = saturate((z - nearZ) / (farZ - nearZ));
    uint zSlice = min(clusterDim.z - 1, uint(zNorm * clusterDim.z));

    return x
		+ y * clusterDim.x
		+ zSlice * clusterDim.x * clusterDim.y;
}

#endif 