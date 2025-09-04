// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define _WITH_SWAPCHAIN_FULLSCREEN_STATE 

#define MAX_LIGHTS 8
#define MAX_MATERIALS 8
#define POINT_LIGHT 1
#define SPOT_LIGHT 2
#define DIRECTIONAL_LIGHT 3

// Windows 헤더 파일
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <shellapi.h>

#include <string>
#include <wrl.h>

#include <fstream>
#include <vector>
#include <Mmsystem.h>

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

#pragma comment(lib, "dxguid.lib")

#pragma comment(lib, "winmm.lib")

#define FRAME_BUFFER_WIDTH	1280
#define FRAME_BUFFER_HEIGHT	720

#define RANDOM_COLOR XMFLOAT4(rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX)) //정점의 색상을 무작위로 설정하기 위해 사용하는.
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




/*
* Direct12의 Math는 기본적으로 SIMD를 활용하기 위한 자료형 XMVECTOR를 지원한다
* XMVECTOR는 클래스 멤버 변수로 사용못하는 제약이 있어, XMFLOAT3로 반환하거나 저장해야한다.
* 그래서 보통 연산을 가속하기 위해서 XMLoadFloat3로 XMFLOAT3를 XMVECTOR로 변환한 다음 연산처리를 하고
* XMStoreFloat3로 그 XMVECTOR를 XMFLOAT3로 다시 변환시키고 반환하는 구조를 취하는 것으로 보다 빠른 연산처리를 하는 것이다.
*/

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
	inline XMFLOAT4X4 Inverse(XMFLOAT4X4& xmmtx4x4Matrix)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixInverse(NULL,
			XMLoadFloat4x4(&xmmtx4x4Matrix)));
		return(xmmtx4x4Result);
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
	inline XMFLOAT4X4 LookAtLH(XMFLOAT3& xmf3EyePosition, XMFLOAT3& xmf3LookAtPosition,
		XMFLOAT3& xmf3UpDirection)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixLookAtLH(XMLoadFloat3(&xmf3EyePosition),
			XMLoadFloat3(&xmf3LookAtPosition), XMLoadFloat3(&xmf3UpDirection)));
		return(xmmtx4x4Result);
	}
}