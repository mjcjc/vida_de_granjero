// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define _WITH_SWAPCHAIN_FULLSCREEN_STATE 

//계산에 들어가는 최대 조명의 갯수를 의미
#define MAX_LIGHTS 57 //이거 현재 sorting을 위한 전체에 접근하기 위해서
#define SPOT_LIGHT 8
#define DIRECTIONAL_LIGHT 1
#define SHADOWMAPS 5

//그림자 맵의 최대량이고 항상 이갯수를 유지하며 이 이상 그림자 맵을 만들지 않으며 줄지도 않는다.
#define DIRECTIONAL_LIGHT_SHADOWMAP 1
#define SPOT_LIGHT_SHADOWMAP 4

#define EPSILON					1.0e-10f //부동소수점

//어떤 종류에 조명인지
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_SPOT        0x02


//애니메이션 플래그 설정
#define ANIMATION_TYPE_ONCE				0
#define ANIMATION_TYPE_LOOP				1
#define ANIMATION_TYPE_PINGPONG			2

#define ANIMATION_CALLBACK_EPSILON		0.00165f

// Windows 헤더 파일
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <shellapi.h>
#include <algorithm>

#include <string>
#include <wrl.h>

#include <fstream>
#include <vector>
#include <Mmsystem.h>

#include <iostream>
#include <io.h>
#include <fcntl.h>

#include <imgui.h> //imguil를 위한 헤더
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
//

#include <random>

//extern "C" DWORD WINAPI timeGetTime(void);

// Direct12를 사용하기 위해 필요한 것들
#include <d3d12.h>
#include <dxgi1_4.h>	//그래픽 디바이스 장치를 위한 마이크로소프트웨에서 제공하는 저수준 함수들을 다루기 위한 헤더

#include <D3Dcompiler.h>	

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <DXGIDebug.h>
#include <comdef.h> 
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

//STL과 별계로 C++의 기본 기능이 아닌 Window API에서 Direct와 같은 Com 객체를 사용하는 API들을 위한 전용 스마트 포인터
//ComPtr<자료형> c ;
// c.Get(); c.GetAdressof();, c.Reset(); 각각 포인터, 이중 포인터, Release 수동 제어 이렇게 있다. 본질은 스마트 포인터이니까 Reset을 명시적으로 사용할 일은 적을 것이다.
using Microsoft::WRL::ComPtr;

//임포트할 라이브러리는 추가하는 부분. 이렇게 하면 링커 속성에서 입력하지 않고 임포트할 수 있다.
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib, "DirectXTex.lib")

#pragma comment(lib, "dxguid.lib")

#pragma comment(lib, "winmm.lib")

#define FRAME_BUFFER_WIDTH	1280
#define FRAME_BUFFER_HEIGHT	720

#define _DEPTH_BUFFER_WIDTH		(FRAME_BUFFER_WIDTH * 4)
#define _DEPTH_BUFFER_HEIGHT	(FRAME_BUFFER_HEIGHT * 4 )

#define _SPOT_DEPTH_BUFFER_WIDTH	2000
#define _SPOT_DEPTH_BUFFER_HEIGHT	2000

// On/Off용 플래그  _MAP_USE
#define MATERIAL_ALBEDO_MAP_USE        0x01
#define MATERIAL_NORMAL_MAP_USE        0x02
#define MATERIAL_ROUGHNESS_MAP_USE     0x04
#define MATERIAL_METALNESS_MAP_USE     0x08
#define MATERIAL_AO_MAP_USE            0x10
#define MATERIAL_SPECULARF0_MAP_USE    0x20
#define MATERIAL_EMISSION_MAP_USE      0x40

// On/Off용 플래그 SSAO, BLoom USE

#define MATERIAL_SSAO_USE        0x01
#define MATERIAL_BLOOM_USE        0x02

// On/Off용 플래그  PBR, Direct, Ambient 활성화 여부
#define DIRECT_LIGHT_USE        0x01
#define AMBIENT_LIGHT_USE        0x02
#define DIRECTION_LIGHT_USE    0x04 

//계절
#define SEASON_SPRING 0x01
#define SEASON_SUMMER 0x02
#define SEASON_FALL 0x04
#define SEASON_WINTER 0x08

//날씨
#define WEATHER_CLEAR   0x01   // 맑음
#define WEATHER_RAIN    0x02   // 비
#define WEATHER_SNOW    0x04   // 눈
#define WEATHER_DUST    0x08   // 황사 / 먼지
#define WEATHER_FOG     0x10   // 안개

#define RANDOM_COLOR XMFLOAT4(rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX)) //정점의 색상을 무작위로 설정하기 위해 사용하는.

inline bool IsZero(float fValue) { return((fabsf(fValue) < EPSILON)); }
inline bool IsEqual(float fA, float fB) { return(::IsZero(fA - fB)); }
inline bool IsZero(float fValue, float fEpsilon) { return((fabsf(fValue) < fEpsilon)); }
inline bool IsEqual(float fA, float fB, float fEpsilon) { return(::IsZero(fA - fB, fEpsilon)); }
inline float InverseSqrt(float fValue) { return 1.0f / sqrtf(fValue); }
inline void Swap(float* pfS, float* pfT) { float fTemp = *pfS; *pfS = *pfT; *pfT = fTemp; }

extern UINT	gnCbvSrvDescriptorIncrementSize; //CBVSRV DescirpotrHeap내에 들어가는
extern UINT gnRtvDescriptorIncrementSize; //Rtv DescripotrHeap에 들어가는 사이즈
extern UINT gnDsvDescriptorIncrementSize; // Dsv 
//view의 크기

/*
* Buffer를 만들어주는 유틸 함수.
* Buffer를 만들기 위해서는 기본적으로 Device, CommandList가 필요하다
* 필요한 이유는 Device가 해당 기기의 그래픽카드에 알맞게 Com객체 인터페이스 함수등등을 모두 제공해주는 도구.
* CommandList를 통해 상태 전이 및 HEAP 할당등을 제어해야 하기 때문
* 이제 버퍼를 빈껍데기로 만들지 않는 이상 pData에 원하는 값을 내가만들 버퍼에 형식에 맞추어 넘겨줘야한다.
* 그리고 해당 크기를 예상하고 길이를 직접 넘겨주어야 정상작동함으로 nBytes 또한 넘겨준다.
* 그리고 리소스를 어떤 Type의 Heap 공간에 할당해서 GPU에서 활용할건지를 지정해줘야하며
* 리소스를 생성하고 사용하는데 상태를 지정해주어야 리소스 베리어가 정상작동할 수 있음으로 ResourceStates또한 넘겨줘야하며 Default 리소스에 경우 UploadBuffer로 Type이 정의된
* 리소스를 사용해서 값을 넘겨주어야 함으로 UploadBuffer 또한 있는것.
*
* extern은 해당 함수의 정의는 다른 cpp에 존재한다는 명시적 의미
*/
extern ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	void* pData, UINT nBytes, D3D12_HEAP_TYPE d3dHeapType,
	D3D12_RESOURCE_STATES d3dResoucreStates, ID3D12Resource** ppd3dUploadBuffer);
extern ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	void* pData, UINT nBytes, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, 
	D3D12_RESOURCE_FLAGS pd3dResourceFlag, ID3D12Resource** ppd3dUploadBuffer);

//Texture를 직접 만드는 함수
extern ID3D12Resource* CreateTexture2DResource(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue);


//DDS에서 읽어오는 방법
extern ComPtr<ID3D12Resource> CreateTextureResourceFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
	wchar_t* pszFileName, ID3D12Resource** ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
//DDS 제외 여러 이미지 종류에서 리소스 읽어오는 방법
extern ComPtr<ID3D12Resource> CreateTextureResourceFromWICFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
	wchar_t* pszFileName, ID3D12Resource** ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

//imgui 사용할려면 필요한거  imgui_impl_win32.cpp의 있는데 이건 내 프로젝트에
//추가해서 사용할것이 아니기에 extern으로 정의를 해줘야 입력제어가 가능해진다.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/*
* Direct12의 Math는 기본적으로 SIMD를 활용하기 위한 자료형 XMVECTOR를 지원한다
* XMVECTOR는 클래스 멤버 변수로 사용못하는 제약이 있어, XMFLOAT3로 반환하거나 저장해야한다.
* 그래서 보통 연산을 가속하기 위해서 XMLoadFloat3로 XMFLOAT3를 XMVECTOR로 변환한 다음 연산처리를 하고
* XMStoreFloat3로 그 XMVECTOR를 XMFLOAT3로 다시 변환시키고 반환하는 구조를 취하는 것으로 보다 빠른 연산처리를 하는 것이다.
*/

//리소스 베리어 설정 함수
extern void SynchronizeResourceTransition(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* pd3dResource, D3D12_RESOURCE_STATES d3dStateBefore, D3D12_RESOURCE_STATES d3dStateAfter);
extern void SynchronizeUAVBarrier(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* pd3dResource);
extern BYTE ReadStringFromFile(FILE* pInFile, char* pstrToken);
extern int ReadIntegerFromFile(FILE* pInFile);
extern float ReadFloatFromFile(FILE* pInFile);

// 애니메이션 상태 ==>
enum EAnimationState
{
	Idle = 0,
	Walk = 1,
	pickup = 2,
	punch = 3,
	watering = 4,
};



//재질과 조명 정의 ==> 우선 임시 나중에 변경 가능
struct Material
{
	XMFLOAT3 ambient; //환경광에 따른 ==> 이게 빛이 없는 기본 색감을 담당
	float shininess; //빛번짐 강도
	XMFLOAT3 diffuse; //난반사
	float dummy1; 
	XMFLOAT3 specular; //정반사
	float dummy2;
};

struct  Light
{
	XMFLOAT3 radiance; //강도
	float fallOffStart; //시작
	XMFLOAT3 direction;
	float fallOffEnd ;
	XMFLOAT3 position ;
	float spotPower;

	UINT radius;
	UINT bIsShadowSet; //0 : bool, 1 : true
	UINT bIsUse; //0 : bool, 1 : true
	UINT type;

	XMFLOAT4X4 viewProj; //
	XMFLOAT4X4 invProj; //
};

///////////////////////////////////////////////// 클러스터드 렌더링에 필요한 자료
#define MAX_LIGHTS_PER_CLUSTER 128

struct LightingData
{
	XMFLOAT3 positionW;
	float radius;

	XMFLOAT3 directionW;
	float intensity;
	
	UINT Type;
	XMFLOAT3 padding;
};

struct ClusterCB
{
	XMUINT3 clusterDim;
	UINT		totalClusters;

	float nearZ;
	float farZ;
	XMFLOAT2 screenSize;

	XMFLOAT2 invScreenSize;
	XMFLOAT2 padding;
};

struct ClusterLightList
{
	UINT lightCount;
	UINT lightIndices[MAX_LIGHTS_PER_CLUSTER];
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 파티클
// Weather / Particle Flags
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
	XMFLOAT3 center;   // 월드 좌표
	float _pad0;
	XMFLOAT3 extent;   // half size (AABB)
	float _pad1;
};


struct cbParticle
{
	float fTime; //누적시간
	float fDeltaTime; //프레임 간격
	float fStartTime; //시작 시간
	float fSpawnRadius; //스폰 범위

	float fAlphaScale; //
	XMFLOAT3 _pad;

	UINT  nParticleFlags; //어느 종류의 파티클인지   
	XMFLOAT3 gParticleColor;  // 황사/연기용

	UINT blockCount;
	XMFLOAT3 _pad2;

	WeatherBlock blocks[MAX_BLOCK];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SSAO_KERNEL_SIZE 16

struct SSAOKernelCB
{
	XMFLOAT3 kernel[SSAO_KERNEL_SIZE];
	float    padding; // 16바이트 정렬용
};

inline float RandomFloat(float minValue, float maxValue)
{
	static std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> dist(minValue, maxValue);
	return dist(rng);
}

namespace MathUtill
{
	inline float ClampFloat(float v, float minV, float maxV)
	{
		if (v < minV) return minV;
		if (v > maxV) return maxV;
		return v;
	}
}
//3차원 벡터 연산 관련된 처리 도구를 만든다.
namespace Vector3
{
	inline XMFLOAT3 XMVectorToFloat3(XMVECTOR& xmvVector) //XMVECTOR를 XMFLOAT3로 변환 시켜주는거
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, xmvVector);
		return(xmf3Result);
	}

	inline XMFLOAT3 ScalarProduct(XMFLOAT3& xmf3Vector, float fScalar, bool bNormalize =   //스칼라 곱
		true)
	{
		XMFLOAT3 xmf3Result;
		if (bNormalize)
			XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)) *
				fScalar);
		else
			XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector) * fScalar);
		return(xmf3Result);
	}
	inline XMFLOAT3 Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2) //벡터 덧셈
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) +
			XMLoadFloat3(&xmf3Vector2));
		return(xmf3Result);
	}
	inline XMFLOAT3 Add(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, float fScalar) //벡터 덧셈 + 스칼라 곱
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + (XMLoadFloat3(&xmf3Vector2)
			* fScalar));
		return(xmf3Result);
	}
	inline XMFLOAT3 Subtract(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2) //벡터 뺄셈
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) -
			XMLoadFloat3(&xmf3Vector2));
		return(xmf3Result);
	}
	inline float DotProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2) // 내적
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Dot(XMLoadFloat3(&xmf3Vector1),
			XMLoadFloat3(&xmf3Vector2)));
		return(xmf3Result.x);
	}
	inline XMFLOAT3 CrossProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, bool // 외적
		bNormalize = true)
	{
		XMFLOAT3 xmf3Result;
		if (bNormalize)
			XMStoreFloat3(&xmf3Result,
				XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&xmf3Vector1),
					XMLoadFloat3(&xmf3Vector2))));
		else
			XMStoreFloat3(&xmf3Result, XMVector3Cross(XMLoadFloat3(&xmf3Vector1),
				XMLoadFloat3(&xmf3Vector2)));
		return(xmf3Result);
	}
	inline XMFLOAT3 Normalize(XMFLOAT3& xmf3Vector) //노멀라이즈
	{
		XMFLOAT3 m_xmf3Normal;
		XMStoreFloat3(&m_xmf3Normal, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)));
		return(m_xmf3Normal);
	}
	inline float Length(XMFLOAT3& xmf3Vector) // 길이, 크기 반환
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Length(XMLoadFloat3(&xmf3Vector)));
		return(xmf3Result.x);
	}
	inline float Angle(XMVECTOR& xmvVector1, XMVECTOR& xmvVector2) //두 벡터 사이각 반환 ==> 도(degree) 단위로 반환
	{
		XMVECTOR xmvAngle = XMVector3AngleBetweenNormals(xmvVector1, xmvVector2);
		return(XMConvertToDegrees(acosf(XMVectorGetX(xmvAngle)))); //XxmvAngle의 x값에 라디안 값으로 들어가있다
	}

	inline float Angle(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2) //XMFLOAT3 로 반환한 경우 사이각 구하는거
	{
		return(Angle(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
	}
	inline XMFLOAT3 TransformNormal(XMFLOAT3& xmf3Vector, XMMATRIX& xmmtxTransform) //법선 벡터 행 변환 ==> 방향만을 갖고있는 법선 벡터에 대한 행렬 변환
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3TransformNormal(XMLoadFloat3(&xmf3Vector),
			xmmtxTransform));
		return(xmf3Result);
	}
	inline XMFLOAT3 TransformCoord(XMFLOAT3& xmf3Vector, XMMATRIX& xmmtxTransform) // 위치 벡터에 대한 행 변환 ==> 좌표 변환을 행하고 있다
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3TransformCoord(XMLoadFloat3(&xmf3Vector),
			xmmtxTransform));
		return(xmf3Result);
	}
	inline XMFLOAT3 TransformCoord(XMFLOAT3& xmf3Vector, XMFLOAT4X4& xmmtx4x4Matrix)
	{
		return(TransformCoord(xmf3Vector, XMLoadFloat4x4(&xmmtx4x4Matrix)));
	}
}


//4차원 벡터의 연산
namespace Vector4
{
	inline XMFLOAT4 Add(XMFLOAT4& xmf4Vector1, XMFLOAT4& xmf4Vector2)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) +
			XMLoadFloat4(&xmf4Vector2));
		return(xmf4Result);
	}
	inline XMFLOAT4 Multiply(XMFLOAT4& xmf4Vector1, XMFLOAT4& xmf4Vector2)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) *
			XMLoadFloat4(&xmf4Vector2));
		return(xmf4Result);
	}
	inline XMFLOAT4 Multiply(float fScalar, XMFLOAT4& xmf4Vector)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, fScalar * XMLoadFloat4(&xmf4Vector));
		return(xmf4Result);
	}
}

//행렬의 연산
namespace Matrix4x4
{
	inline XMFLOAT4X4 Identity()
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixIdentity());
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 Zero()
	{
		XMFLOAT4X4 xmf4x4Result;
		XMStoreFloat4x4(&xmf4x4Result, XMMatrixSet(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
		return(xmf4x4Result);
	}

	inline XMFLOAT4X4 Scale(XMFLOAT4X4& xmf4x4Matrix, float fScale)
	{
		XMFLOAT4X4 xmf4x4Result;
		XMStoreFloat4x4(&xmf4x4Result, XMLoadFloat4x4(&xmf4x4Matrix) * fScale);
		return(xmf4x4Result);
	}

	inline XMFLOAT4X4 Add(XMFLOAT4X4& xmmtx4x4Matrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
	{
		XMFLOAT4X4 xmf4x4Result;
		XMStoreFloat4x4(&xmf4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) + XMLoadFloat4x4(&xmmtx4x4Matrix2));
		return(xmf4x4Result);
	}

	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) *
			XMLoadFloat4x4(&xmmtx4x4Matrix2));
		return(xmmtx4x4Result);
	}
	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMMATRIX& xmmtxMatrix2)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) * xmmtxMatrix2);
		return(xmmtx4x4Result);
	}
	inline XMFLOAT4X4 Multiply(XMMATRIX& xmmtxMatrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, xmmtxMatrix1 * XMLoadFloat4x4(&xmmtx4x4Matrix2));
		return(xmmtx4x4Result);
	}


	inline XMFLOAT4X4 RotateAxis(XMFLOAT3& xmf3Axis, float fAngle)
	{
		XMFLOAT4X4 xmf4x4Result;
		XMStoreFloat4x4(&xmf4x4Result, XMMatrixRotationAxis(XMLoadFloat3(&xmf3Axis), XMConvertToRadians(fAngle)));
		return(xmf4x4Result);
	}

	inline XMFLOAT4X4 Rotate(float x, float y, float z)
	{
		XMFLOAT4X4 xmf4x4Result;
		XMStoreFloat4x4(&xmf4x4Result, XMMatrixRotationRollPitchYaw(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z)));
		return(xmf4x4Result);
	}

	inline XMFLOAT4X4 AffineTransformation(XMFLOAT3& xmf3Scaling, XMFLOAT3& xmf3RotateOrigin, XMFLOAT3& xmf3Rotation, XMFLOAT3& xmf3Translation)
	{
		XMFLOAT4X4 xmf4x4Result;
		XMStoreFloat4x4(&xmf4x4Result, XMMatrixAffineTransformation(XMLoadFloat3(&xmf3Scaling), XMLoadFloat3(&xmf3RotateOrigin), XMQuaternionRotationRollPitchYaw(XMConvertToRadians(xmf3Rotation.x), XMConvertToRadians(xmf3Rotation.y), XMConvertToRadians(xmf3Rotation.z)), XMLoadFloat3(&xmf3Translation)));
		return(xmf4x4Result);
	}

	inline XMFLOAT4X4 Inverse(XMFLOAT4X4& xmmtx4x4Matrix)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixInverse(NULL,
			XMLoadFloat4x4(&xmmtx4x4Matrix)));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 Interpolate(XMFLOAT4X4& xmf4x4Matrix1, XMFLOAT4X4& xmf4x4Matrix2, float t)
	{
		XMFLOAT4X4 xmf4x4Result;
		XMVECTOR S0, R0, T0, S1, R1, T1;
		XMMatrixDecompose(&S0, &R0, &T0, XMLoadFloat4x4(&xmf4x4Matrix1));
		XMMatrixDecompose(&S1, &R1, &T1, XMLoadFloat4x4(&xmf4x4Matrix2));
		XMVECTOR S = XMVectorLerp(S0, S1, t);
		XMVECTOR T = XMVectorLerp(T0, T1, t);
		XMVECTOR R = XMQuaternionSlerp(R0, R1, t);
		XMStoreFloat4x4(&xmf4x4Result, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));
		return(xmf4x4Result);
	}

	inline XMFLOAT4X4 Transpose(XMFLOAT4X4& xmmtx4x4Matrix)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result,
			XMMatrixTranspose(XMLoadFloat4x4(&xmmtx4x4Matrix)));
		return(xmmtx4x4Result);
	}
	inline XMFLOAT4X4 PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ,
		float FarZ)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio,
			NearZ, FarZ));
		return(xmmtx4x4Result);
	}
	inline XMFLOAT4X4 OrthographicLH(float ViewWidth, float ViewHeight,
		float NearZ, float FarZ)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(
			&xmmtx4x4Result,
			XMMatrixOrthographicLH(ViewWidth, ViewHeight, NearZ, FarZ)
		);
		return xmmtx4x4Result;
	}
	inline XMFLOAT4X4 LookAtLH(XMFLOAT3& xmf3EyePosition, XMFLOAT3& xmf3LookAtPosition,
		XMFLOAT3& xmf3UpDirection)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixLookAtLH(XMLoadFloat3(&xmf3EyePosition),
			XMLoadFloat3(&xmf3LookAtPosition), XMLoadFloat3(&xmf3UpDirection)));
		return(xmmtx4x4Result);
	}
}

namespace Triangle
{
	inline bool Intersect(XMFLOAT3& xmf3RayPosition, XMFLOAT3& xmf3RayDirection, XMFLOAT3& v0, XMFLOAT3& v1, XMFLOAT3& v2, float& fHitDistance)
	{
		return(TriangleTests::Intersects(XMLoadFloat3(&xmf3RayPosition), XMLoadFloat3(&xmf3RayDirection), XMLoadFloat3(&v0), XMLoadFloat3(&v1), XMLoadFloat3(&v2), fHitDistance));
	}
}

namespace Plane
{
	inline XMFLOAT4 Normalize(XMFLOAT4& xmf4Plane)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMPlaneNormalize(XMLoadFloat4(&xmf4Plane)));
		return(xmf4Result);
	}
}

extern UINT GMyServerPlayerId;

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")

#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#endif