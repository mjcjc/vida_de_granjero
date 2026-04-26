#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

#define FRAME_BUFFER_WIDTH 640.0f
#define FRAME_BUFFER_HEIGHT 480.0f

#define MAX_LIGHTS 3
#define POINT_LIGHT 1
#define SPOT_LIGHT 1
#define DIRECTIONAL_LIGHT 1
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

// 조명
struct Light
{
    float3 strength; //강도
    float fallOffStart; // 범위 시작 
    float3 direction; // 방향
    float fallOffEnd; //범위 끝
    float3 position; //위치
    float spotPower; //spot 조명에 경우 모이는 힘
};

/*
CBV를 직접 연결하는건 
Camera b0
Player의 좌표 b1
조명은 b10
게임 오브젝트는 b2

*/

cbuffer cbGameObjectInfo : register(b0)
{
    matrix gmtxWorld : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProj : packoffset(c4);
};

#endif // __COMMON_HLSLI__