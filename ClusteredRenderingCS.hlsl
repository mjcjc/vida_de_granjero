#define MAX_LIGHTS_PER_CLUSTER 128
#define MAX_LIGHTS 56 // DIrection Light를 빼고서 해야함
cbuffer ClusterCB : register(b1)
{
    uint3 clusterDim;        // (X, Y, Z)
    uint  totalClusters;     // X * Y * Z

    float nearZ;
    float farZ;

    float2 screenSize;
    float2 invScreenSize;
};


cbuffer cbCameraInfo : register(b0)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProj : packoffset(c4);
    matrix gmtxinvProj : packoffset(c8);
    float3 gf3EyeWorld : packoffset(c12); //카메라의 월드상 좌표
};


struct LightingData
{
    float3 positionW;   // View Space 위치
    float  radius;       // 영향 반경

    float3 directionW;  // Spot Light 방향
    float  intensity;    // 강도

    uint   type;         // 0: Point, 1: Spot
    float3 padding;
};

struct ClusterLightList
{
    uint lightCount;                         // 이 클러스터에 들어온 조명 수
    uint lightIndices[MAX_LIGHTS_PER_CLUSTER]; // gLights의 인덱스
};

cbuffer cbLightingData : register(b2)
{
    LightingData lightingData[MAX_LIGHTS];
};

RWStructuredBuffer<ClusterLightList> gClusterLightLists : register(u0);


bool IntersectSphereAABB(
    float3 sphereCenter,
    float radius,
    float3 boxMin,
    float3 boxMax)
{
    float3 closest = clamp(sphereCenter, boxMin, boxMax);
    float3 delta = closest - sphereCenter;
    return dot(delta, delta) <= radius * radius;
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x >= clusterDim.x ||
        DTid.y >= clusterDim.y ||
        DTid.z >= clusterDim.z)
        return;
    //


    uint clusterIndex =
        DTid.x +
        DTid.y * clusterDim.x +
        DTid.z * clusterDim.x * clusterDim.y;

    //스크린 기준 x,y 분할
    float2 tileSize;
    tileSize.x = screenSize.x / clusterDim.x;
    tileSize.y = screenSize.y / clusterDim.y;

    float2 pixelMin =
        float2(DTid.x * tileSize.x,
            DTid.y * tileSize.y);

    float2 pixelMax = pixelMin + tileSize;

    //z 방향 슬라이스
    float zSliceNear =
        nearZ + (farZ - nearZ) * (DTid.z / (float)clusterDim.z);

    float zSliceFar =
        nearZ + (farZ - nearZ) * ((DTid.z + 1) / (float)clusterDim.z);

 


    //
    float2 ndcMin = pixelMin * invScreenSize * 2.0f - 1.0f;
    float2 ndcMax = pixelMax * invScreenSize * 2.0f - 1.0f;



    //NDC
    float4 cornerVS[8];

    cornerVS[0] = mul(float4(ndcMin.x, ndcMin.y, 0, 1), gmtxinvProj);
    cornerVS[1] = mul(float4(ndcMax.x, ndcMin.y, 0, 1), gmtxinvProj);
    cornerVS[2] = mul(float4(ndcMin.x, ndcMax.y, 0, 1), gmtxinvProj);
    cornerVS[3] = mul(float4(ndcMax.x, ndcMax.y, 0, 1), gmtxinvProj);

    cornerVS[4] = mul(float4(ndcMin.x, ndcMin.y, 1, 1), gmtxinvProj);
    cornerVS[5] = mul(float4(ndcMax.x, ndcMin.y, 1, 1), gmtxinvProj);
    cornerVS[6] = mul(float4(ndcMin.x, ndcMax.y, 1, 1), gmtxinvProj);
    cornerVS[7] = mul(float4(ndcMax.x, ndcMax.y, 1, 1), gmtxinvProj);

    for (int i = 0; i < 8; i++)
    {
        cornerVS[i] /= cornerVS[i].w;
    }

    //AABB
    float3 aabbMin = cornerVS[0].xyz;
    float3 aabbMax = cornerVS[0].xyz;


    for (int i = 1; i < 8; i++)
     {
       aabbMin = min(aabbMin, cornerVS[i].xyz);
       aabbMax = max(aabbMax, cornerVS[i].xyz);
     }

    aabbMin.z = zSliceNear;
    aabbMax.z = zSliceFar;

    //모든 라이트를 돌며 해당 클러스터 인덱스에 값을 저장한다.
    uint baseIndex = clusterIndex;

    gClusterLightLists[baseIndex].lightCount = 0;

    //gClusterLightLists[baseIndex].lightCount = 1;
    //gClusterLightLists[baseIndex].lightIndices[0] = 0;
    //return;

    for (uint l = 0; l < MAX_LIGHTS; l++)
    {
        LightingData light = lightingData[l];

        if (light.radius <= 0)
            continue;

        float3 lightPosVS =
            mul(float4(light.positionW, 1.0f), gmtxView).xyz;

        if (IntersectSphereAABB(
            lightPosVS,
            light.radius,
            aabbMin,
            aabbMax))
        {
            uint count = gClusterLightLists[baseIndex].lightCount;

            if (count < MAX_LIGHTS_PER_CLUSTER)
            {
                gClusterLightLists[baseIndex].lightIndices[count] = l;
                gClusterLightLists[baseIndex].lightCount = count + 1;
            }
        }
    }
}