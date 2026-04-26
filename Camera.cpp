#include "stdafx.h"
#include "Player.h"
#include "Camera.h"


void CCamera::GenerateFrustum()
{
	m_BoundingFrustumCamera.CreateFromMatrix(m_BoundingFrustumCamera, XMLoadFloat4x4(&m_xmf4x4Proj));
	//뷰프로스텀 월드 기준으로 초기화
	XMMATRIX invView = XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_xmf4x4View));
	m_BoundingFrustumCamera.Transform(m_BoundingFrustumCamera, invView);
}

CCamera::CCamera()
{
	m_xmf4x4View = Matrix4x4::Identity();
	m_xmf4x4Proj = Matrix4x4::Identity();
	m_xmf4x4InvProj = Matrix4x4::Identity();
	m_xmf4x4LightViews = new XMFLOAT4X4[5];
	m_xmf4x4LightProjs = new XMFLOAT4X4[5];
	m_xmf4x4LightInvProj = new XMFLOAT4X4[5];
	m_xmf3LightPosition = new XMFLOAT3[5];
	m_d3dShadowMapViewport = new D3D12_VIEWPORT[5];
	m_d3dShadowScissorRect = new D3D12_RECT[5];
	for (int i = 0; i < 5; i++)
	{
		m_xmf4x4LightViews[i] = Matrix4x4::Identity();
		m_xmf4x4LightProjs[i] = Matrix4x4::Identity();
		m_xmf4x4LightInvProj[i] = Matrix4x4::Identity();
		m_xmf3LightPosition[i] = XMFLOAT3(0,0,0);
		switch (i)
		{
		case 0:
			{
				m_d3dShadowMapViewport[i] = { 0, 0, _DEPTH_BUFFER_WIDTH , _DEPTH_BUFFER_HEIGHT, 0.0f, 1.0f };
				m_d3dShadowScissorRect[i] = { 0, 0, _DEPTH_BUFFER_WIDTH , _DEPTH_BUFFER_HEIGHT };
				break;
			}
		default:
			{
				m_d3dShadowMapViewport[i] = { 0, 0, _SPOT_DEPTH_BUFFER_WIDTH , _SPOT_DEPTH_BUFFER_HEIGHT, 0.0f, 1.0f };
				m_d3dShadowScissorRect[i] = { 0, 0, _SPOT_DEPTH_BUFFER_WIDTH , _SPOT_DEPTH_BUFFER_HEIGHT };
				break;
			}
		}
	}
	m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fTimeLag = 0.0f;
	m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_nMode = 0x00;
	m_pPlayer = NULL;
}

CCamera::CCamera(CCamera* pCamera)
{
	if (pCamera)
	{
		//카메라가 이미 있으면 기존 카메라의 정보를 새로운 카메라에 복사한다. 
		*this = *pCamera;
	}
	else
	{
		//카메라가 없으면 기본 정보를 설정한다. 
		m_xmf4x4View = Matrix4x4::Identity();
		m_xmf4x4Proj = Matrix4x4::Identity();
		m_xmf4x4InvProj = Matrix4x4::Identity();
		m_xmf4x4LightViews = new XMFLOAT4X4[5];
		m_xmf4x4LightProjs = new XMFLOAT4X4[5];
		m_xmf4x4LightInvProj = new XMFLOAT4X4[5];
		m_xmf3LightPosition = new XMFLOAT3[5];
		m_d3dShadowMapViewport = new D3D12_VIEWPORT[5];
		m_d3dShadowScissorRect = new D3D12_RECT[5];
		for (int i = 0; i < 5; i++)
		{
			m_xmf4x4LightViews[i] = Matrix4x4::Identity();
			m_xmf4x4LightProjs[i] = Matrix4x4::Identity();
			m_xmf4x4LightInvProj[i] = Matrix4x4::Identity();
			m_xmf3LightPosition[i] = XMFLOAT3(0, 0, 0);
			switch (i)
			{
			case 0:
			{
				m_d3dShadowMapViewport[i] = { 0, 0, _DEPTH_BUFFER_WIDTH , _DEPTH_BUFFER_HEIGHT, 0.0f, 1.0f };
				m_d3dShadowScissorRect[i] = { 0, 0, _DEPTH_BUFFER_WIDTH , _DEPTH_BUFFER_HEIGHT };
				break;
			}
			default:
			{
				m_d3dShadowMapViewport[i] = { 0, 0, _SPOT_DEPTH_BUFFER_WIDTH , _SPOT_DEPTH_BUFFER_HEIGHT, 0.0f, 1.0f };
				m_d3dShadowScissorRect[i] = { 0, 0, _SPOT_DEPTH_BUFFER_WIDTH , _SPOT_DEPTH_BUFFER_HEIGHT };
				break;
			}
			}
		}
		m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
		m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = 0.0f;
		m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_fTimeLag = 0.0f;
		m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_nMode = 0x00;
		m_pPlayer = NULL;

		
	}
}

CCamera::~CCamera()
{

}


void CCamera::SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float
	fMinZ, float fMaxZ)
{
	m_d3dViewport.TopLeftX = float(xTopLeft);
	m_d3dViewport.TopLeftY = float(yTopLeft);
	m_d3dViewport.Width = float(nWidth);
	m_d3dViewport.Height = float(nHeight);
	m_d3dViewport.MinDepth = fMinZ;
	m_d3dViewport.MaxDepth = fMaxZ;
}

void CCamera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom)
{
	m_d3dScissorRect.left = xLeft;
	m_d3dScissorRect.top = yTop;
	m_d3dScissorRect.right = xRight;
	m_d3dScissorRect.bottom = yBottom;
}

void CCamera::GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle)
{
	m_xmf4x4Proj = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(fFOVAngle), //왼손좌표계 기준의 투영 변환 행렬을 만드는 함수
		fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);

	m_fnearZ = fNearPlaneDistance;
	m_fFarZ = fFarPlaneDistance;

	XMMATRIX proj = XMLoadFloat4x4(&m_xmf4x4Proj);
	XMMATRIX invProj = XMMatrixInverse(nullptr, proj);
	XMStoreFloat4x4(&m_xmf4x4InvProj, invProj);
}

void CCamera::GenerateLightsProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle , float ViewWidht, float ViewHeight)
{
	//TODO:: 여기 부분 안쓸듯 ==> CaseCade ShadowMapping 도전할때 활용할듯
	for (int i = 0; i < 5; i++)
	{
		/*
		// Spot Light : position + direction
		float fFovAngle = 120.0f; // m_pLights->m_pLights[j].m_fPhi = cos(60.0f);
		//float fAspectRatio = float(_DEPTH_BUFFER_WIDTH) / float(_DEPTH_BUFFER_HEIGHT);
		float fAspectRatio = 1;
		float fNearPlaneDistance = 1.0f, fFarPlaneDistance = pLights[i].fallOffEnd;
		xmmtxLProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fFovAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
	
		//XMStoreFloat4x4(&m_xmf4x4LightViews[i], view);
//XMStoreFloat4x4(&m_xmf4x4LightProjs[i], xmmtxLProjection);

		float width = 50; //이게 작아야 훨씬 고퀄리티 그림자를 만들 수 있따
		float height = 50;
		float nearZ = 1.0f;
		float farZ = 200.0f;
		xmmtxLProjection = XMMatrixOrthographicLH(
			width,
			height,
			nearZ,
			farZ
		);

		*/

		if (i == 0)
		{
			m_xmf4x4LightProjs[i] = Matrix4x4::OrthographicLH(ViewWidht, ViewHeight, fNearPlaneDistance, fFarPlaneDistance);
		}
		else
		{
			m_xmf4x4LightProjs[i] = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(fFOVAngle), 
				fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
		}
		m_xmf4x4LightInvProj[i] = Matrix4x4::Inverse(m_xmf4x4LightProjs[i]);
	}
}

void CCamera::GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up)
{
	m_xmf3Position = xmf3Position;
	m_xmf3LookAtWorld = xmf3LookAt;
	m_xmf3Up = xmf3Up;

	GenerateViewMatrix();
}

void CCamera::GenerateViewMatrix()
{
	m_xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, m_xmf3LookAtWorld, m_xmf3Up);//왼손 좌표계 기준 카메라 변환 행렬

	GenerateFrustum();
}

void CCamera::RegenerateViewMatrix()
{
	//카메라의 z-축을 기준으로 카메라의 좌표축들이 직교하도록 카메라 변환 행렬을 갱신한다.
	//카메라의 z-축 벡터를 정규화한다.
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	//카메라의 z-축과 y-축에 수직인 벡터를 x-축으로 설정한다.
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	//카메라의 z-축과 x-축에 수직인 벡터를 y-축으로 설정한다.
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
	//이 부분은 카메라 변환 행렬을 직접 계산하는 부분 위에 GenerateViewMatrix에는 DirectX에서 제공하는 함수를 활용하는 부분
	m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 =
		m_xmf3Look.x;
	m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 =
		m_xmf3Look.y;
	m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 =
		m_xmf3Look.z;
	m_xmf4x4View._41 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Right);
	m_xmf4x4View._42 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Up);
	m_xmf4x4View._43 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Look);

	GenerateFrustum();
}

void CCamera::RegenerateLightViewProjMatrix(Light* pLights)
{

	// 0 : Directional Light
	// 1~4 : Spot Light

	for (int i = 0; i < SHADOWMAPS; i++)
	{
		if (pLights[i].bIsShadowSet == 0) //==> 기본적으로 무조건 1로 넘어오겠지만, 조명 갯수가 부족한 경우
		{
			continue;
		}
		//0번 인덱스의 그림자는 무조건 DirectionLight니까
		pLights[0].position = Vector3::Subtract(GetPosition(), Vector3::ScalarProduct(pLights[0].direction, 50));
		pLights[i].viewProj = Matrix4x4::Identity();
		pLights[i].invProj = Matrix4x4::Identity();

		XMFLOAT3 xmf3Position = pLights[i].position;
		XMFLOAT3 xmf3dir = pLights[i].direction;
		XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		
		if (abs(Vector3::DotProduct(xmf3Up, xmf3dir) + 1.0f) < 1e-5)
		{
			xmf3Up = XMFLOAT3(1.0f, 0.0f, 0.0f);
		}

		XMMATRIX view = XMMatrixLookAtLH(XMLoadFloat3(&xmf3Position), XMLoadFloat3(&Vector3::Add(xmf3Position , xmf3dir)), XMLoadFloat3(&xmf3Up));
		XMMATRIX xmmtxLProjection;
		if (i == 0) //Direction Light에 그림자
		{
			float width = 100; //이게 작아야 훨씬 고퀄리티 그림자를 만들 수 있따
			float height = 100;
			float nearZ = 1.0f;
			float farZ = 200.0f;
			xmmtxLProjection = XMMatrixOrthographicLH(
				width,
				height,
				nearZ,
				farZ
			);
		}
		else //Spot Light에 그림자
		{
			float fFovAngle = 160.0f; //
			float fAspectRatio = 1;
			float fNearPlaneDistance =  1.0f, fFarPlaneDistance = pLights[i].fallOffEnd;
			xmmtxLProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fFovAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
		}

		//XMStoreFloat4x4(&m_xmf4x4LightViews[i], view);
		//XMStoreFloat4x4(&m_xmf4x4LightProjs[i], xmmtxLProjection);

		if (i == 1 || i == 2)
		{
			view = XMMatrixLookAtLH(XMLoadFloat3(&xmf3Position), XMLoadFloat3(&Vector3::Add(xmf3Position, xmf3dir)), XMLoadFloat3(&xmf3Up));
		}
		
		XMStoreFloat4x4(&pLights[i].viewProj, XMMatrixTranspose(XMMatrixMultiply(view, xmmtxLProjection)));
		XMStoreFloat4x4(&pLights[i].invProj, XMMatrixTranspose(XMMatrixInverse(NULL, xmmtxLProjection)));
	}
}

//XMFLOAT3 CCamera::UpdateDirectionalLightPostion(XMFLOAT3 LightPosition, XMFLOAT3 LightDirection, float shadowDistance)
//{
//
//	XMFLOAT3 lightPos = Vector3::Subtract(GetPosition(), LightDirection * shadowDistance);
//	return lightPos;
//}


void CCamera::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbCamera_Info = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbCamera_Info->Map(0, NULL, (void**)&m_pcbMappedCamera_Info);
}

void CCamera::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) //Shader에다가 해당 상수 버퍼를 연결하는 중
{
	/*
	* 루트 파라미터 인덱스를 같은 곳에 넣을 수 있지만 크기 제한이 있고, 오프셋을 항상 잘 지켜주어야 한다.
	* 또한 크기를 직접 전부 알고 세심히 정해줘야하는것도 잊지 말아야 한다.
	*/

	
	XMStoreFloat4x4(&m_pcbMappedCamera_Info->m_xmf4x4View, 
		XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4View)));
	XMStoreFloat4x4(&m_pcbMappedCamera_Info->m_xmf4x4Proj,
		XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Proj)));
	XMStoreFloat4x4(&m_pcbMappedCamera_Info->m_xmf4x4invProj,
		XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4InvProj)));
	XMStoreFloat3(&m_pcbMappedCamera_Info->m_xmf3Position, 
		XMLoadFloat3(&m_xmf3Position));

	// ::memcpy(&m_pcbMappedCamera->m_xmf3Position, &m_xmf3Position, sizeof(XMFLOAT3));
	
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbCamera_Info->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(1, d3dGpuVirtualAddress);
	
	//계산 파이프라인에서 접근
	//pd3dCommandList->SetComputeRootConstantBufferView(0, d3dGpuVirtualAddress);

}
void CCamera::UpdateShaderVariablesCompute(ID3D12GraphicsCommandList* pd3dCommandList) //Shader에다가 해당 상수 버퍼를 연결하는 중
{

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbCamera_Info->GetGPUVirtualAddress();

	//계산 파이프라인에서 접근
	pd3dCommandList->SetComputeRootConstantBufferView(0, d3dGpuVirtualAddress);

}

//이거 지금보니까 그림자를 위해서 내가 만들어둔거네? 근데 이거 
void CCamera::UpdatePreShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, int index) //Shader에다가 해당 상수 버퍼를 연결하는 중
{


		XMStoreFloat4x4(&m_pcbMappedCamera_Info->m_xmf4x4View,
			XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4LightViews[index])));
		XMStoreFloat4x4(&m_pcbMappedCamera_Info->m_xmf4x4Proj,
			XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4LightProjs[index])));

		XMStoreFloat3(&m_pcbMappedCamera_Info->m_xmf3Position,
			XMLoadFloat3(&m_xmf3LightPosition[index]));


		D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbCamera_Info->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(1, d3dGpuVirtualAddress);

		//
	
	
}


void CCamera::ReleaseShaderVariables()
{

}

void CCamera::SetViewportsAndScissorRects(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//RS 단계에 시져렉트와 뷰포트를 연결해주는 것.
	//Direct12는 상태 머신이기 때문에, 상태를 명시적으로 연결해줘야 한다. 그래서 CommandList에 Set 명령어가 굉장히 많다.
	pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
	pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
}

XMFLOAT4X4 CCamera::GetInvProjectinMatrix()
{
	return m_xmf4x4InvProj;
}

XMFLOAT4X4 CCamera::GetProjectinMatrix()
{
	return m_xmf4x4Proj;
}

CSpaceShipCamera::CSpaceShipCamera(CCamera* pCamera) : CCamera(pCamera)
{
	m_nMode = SPACESHIP_CAMERA;
}

//스페이스-쉽 카메라를 플레이어의 로컬 x-축(Right), y-축(Up), z-축(Look)을 기준으로 회전하는 함수이다.
void CSpaceShipCamera::Rotate(float x, float y, float z)
{
	if (m_pPlayer && (x != 0.0f))
	{
		//플레이어의 로컬 x-축에 대한 x 각도의 회전 행렬을 계산한다.
		XMFLOAT3 xmf3Right = m_pPlayer->GetRightVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Right),
			XMConvertToRadians(x));
		//카메라의 로컬 x-축, y-축, z-축을 회전한다.
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		/*카메라의 위치 벡터에서 플레이어의 위치 벡터를 뺀다. 결과는 플레이어 위치를 기준(원점)으로 한 카메라의 위치
	   벡터이다.*/
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		//플레이어의 위치를 중심으로 카메라의 위치 벡터(플레이어를 기준으로 한)를 회전한다.
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		//회전시킨 카메라의 위치 벡터에 플레이어의 위치를 더하여 카메라의 위치 벡터를 구한다.
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
	}
	if (m_pPlayer && (y != 0.0f))
	{
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up),
			XMConvertToRadians(y));
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
	}
	if (m_pPlayer && (z != 0.0f))
	{
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look),
			XMConvertToRadians(z));
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
	}
}

CFirstPersonCamera::CFirstPersonCamera(CCamera* pCamera) : CCamera(pCamera)
{
	m_nMode = FIRST_PERSON_CAMERA;
	if (pCamera)
	{
		/*1인칭 카메라로 변경하기 이전의 카메라가 스페이스-쉽 카메라이면 카메라의 Up 벡터를 월드좌표의 y-축이 되도록
	   한다. 이것은 스페이스-쉽 카메라의 로컬 y-축 벡터가 어떤 방향이든지 1인칭 카메라(대부분 사람인 경우)의 로컬 y
	   축 벡터가 월드좌표의 y-축이 되도록 즉, 똑바로 서있는 형태로 설정한다는 의미이다. 그리고 로컬 x-축 벡터와 로컬
	   z-축 벡터의 y-좌표가 0.0f가 되도록 한다. 이것은 다음 그림과 같이 로컬 x-축 벡터와 로컬 z-축 벡터를 xz-평면(지
	   면)으로 투영하는 것을 의미한다. 즉, 1인칭 카메라의 로컬 x-축 벡터와 로컬 z-축 벡터는 xz-평면에 평행하다.*/
		if (pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xmf3Right.y = 0.0f;
			m_xmf3Look.y = 0.0f;
			m_xmf3Right = Vector3::Normalize(m_xmf3Right);
			m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		}
	}
}


void CFirstPersonCamera::Rotate(float x, float y, float z)
{
	if (x != 0.0f)
	{
		//카메라의 로컬 x-축을 기준으로 회전하는 행렬을 생성한다. 사람의 경우 고개를 끄떡이는 동작이다.
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right),
			XMConvertToRadians(x));
		//카메라의 로컬 x-축, y-축, z-축을 회전 행렬을 사용하여 회전한다.
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (m_pPlayer && (y != 0.0f))
	{
		//플레이어의 로컬 y-축을 기준으로 회전하는 행렬을 생성한다.
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up),
			XMConvertToRadians(y));
		//카메라의 로컬 x-축, y-축, z-축을 회전 행렬을 사용하여 회전한다.
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (m_pPlayer && (z != 0.0f))
	{
		//플레이어의 로컬 z-축을 기준으로 회전하는 행렬을 생성한다.
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look),
			XMConvertToRadians(z));
		//카메라의 위치 벡터를 플레이어 좌표계로 표현한다(오프셋 벡터).
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		//오프셋 벡터 벡터를 회전한다.
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		//회전한 카메라의 위치를 월드 좌표계로 표현한다.
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
		//카메라의 로컬 x-축, y-축, z-축을 회전한다.
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
}

CThirdPersonCamera::CThirdPersonCamera(CCamera* pCamera) : CCamera(pCamera)
{
	m_nMode = THIRD_PERSON_CAMERA;
	if (pCamera)
	{
		/*3인칭 카메라로 변경하기 이전의 카메라가 스페이스-쉽 카메라이면 카메라의 Up 벡터를 월드좌표의 y-축이 되도록
	   한다. 이것은 스페이스-쉽 카메라의 로컬 y-축 벡터가 어떤 방향이든지 3인칭 카메라(대부분 사람인 경우)의 로컬 y
	   축 벡터가 월드좌표의 y-축이 되도록 즉, 똑바로 서있는 형태로 설정한다는 의미이다. 그리고 로컬 x-축 벡터와 로컬
	   z-축 벡터의 y-좌표가 0.0f가 되도록 한다. 이것은 로컬 x-축 벡터와 로컬 z-축 벡터를 xz-평면(지면)으로 투영하는
	   것을 의미한다. 즉, 3인칭 카메라의 로컬 x-축 벡터와 로컬 z-축 벡터는 xz-평면에 평행하다.*/
		if (pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xmf3Right.y = 0.0f;
			m_xmf3Look.y = 0.0f;
			m_xmf3Right = Vector3::Normalize(m_xmf3Right);
			m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		}
	}
}

void CThirdPersonCamera::Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed)
{
	//플레이어가 있으면 플레이어의 회전에 따라 3인칭 카메라도 회전해야 한다.
	if (m_pPlayer)
	{
		XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();
		XMFLOAT3 xmf3Right = m_pPlayer->GetRightVector();
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		//플레이어의 로컬 x-축, y-축, z-축 벡터로부터 회전 행렬(플레이어와 같은 방향을 나타내는 행렬)을 생성한다.
		xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
		xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
		xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;
		//카메라 오프셋 벡터를 회전 행렬로 변환(회전)한다.
		XMFLOAT3 xmf3Offset = Vector3::TransformCoord(m_xmf3Offset, xmf4x4Rotate);
		//회전한 카메라의 위치는 플레이어의 위치에 회전한 카메라 오프셋 벡터를 더한 것이다.
		XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);
		//현재의 카메라의 위치에서 회전한 카메라의 위치까지의 방향과 거리를 나타내는 벡터이다.
		XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, m_xmf3Position);
		float fLength = Vector3::Length(xmf3Direction);
		xmf3Direction = Vector3::Normalize(xmf3Direction);

		/*3인칭 카메라의 래그(Lag)는 플레이어가 회전하더라도 카메라가 동시에 따라서 회전하지 않고 약간의 시차를 두고
	   회전하는 효과를 구현하기 위한 것이다. m_fTimeLag가 1보다 크면 fTimeLagScale이 작아지고 실제 회전(이동)이 적
	   게 일어날 것이다. m_fTimeLag가 0이 아닌 경우 fTimeElapsed를 곱하고 있으므로 3인칭 카메라는 1초의 시간동안
	   (1.0f / m_fTimeLag)의 비율만큼 플레이어의 회전을 따라가게 될 것이다.*/
		float fTimeLagScale = (m_fTimeLag) ? fTimeElapsed * (1.0f / m_fTimeLag) : 1.0f;
		float fDistance = fLength * fTimeLagScale;
		if (fDistance > fLength) fDistance = fLength;
		if (fLength < 0.01f) fDistance = fLength;
		if (fDistance > 0)
		{
			//실제로 카메라를 회전하지 않고 이동을 한다(회전의 각도가 작은 경우 회전 이동은 선형 이동과 거의 같다).
			m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Direction, fDistance);
			//카메라가 플레이어를 바라보도록 한다.
			SetLookAt(xmf3LookAt);
		}
	}
}

void CThirdPersonCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
	//현재 카메라의 위치에서 플레이어를 바라보기 위한 카메라 변환 행렬을 생성한다.
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt,
		m_pPlayer->GetUpVector());
	//카메라 변환 행렬에서 카메라의 x-축, y-축, z-축을 구한다.
	m_xmf3Right = XMFLOAT3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
	m_xmf3Up = XMFLOAT3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
	m_xmf3Look = XMFLOAT3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);
}