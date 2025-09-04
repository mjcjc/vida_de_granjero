#include "stdafx.h"
#include "GameObject.h"

#include "Shader.h"

CGameObject::CGameObject()
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity()); // DirectX Math를 이용해 m_xmf4x4World를 기본행렬로 초기화 해주는 중. 
}

CGameObject::~CGameObject()
{
	if (m_pMesh)
	{
		m_pMesh->Release();
	}

	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
		m_pShader->Release();
	}
}

void CGameObject::SetShader(CShader* pShader) //이건 오브젝트가 Shader를 갖는구조에서 사용하는거고 현재 07/09 기준으로는 Shader가 Object를 갖고있는 구조라서 SetShader 를 쓰지 않았다
{
	if (m_pShader)
	{
		m_pShader->Release();
	}
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CGameObject::SetMesh(CMesh* pMesh)
{
	if (m_pMesh)
	{
		m_pMesh->Release();
	}

	m_pMesh = pMesh;

	if (m_pMesh)
	{
		m_pMesh->AddRef();
	}

}

void CGameObject::ReleaseUploadBuffers()
{
	//정점 버퍼를 실제로 갖고있는 객체는 Mesh임으로 여기선 Mesh의 ReleaseUploadBuffers를 호출해 제거해주는 것.
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
}
/*
* 움직임 제어
*/

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}
void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}
XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}
//게임 객체의 로컬 z-축 벡터를 반환한다.
XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32,
		m_xmf4x4World._33)));
}
//게임 객체의 로컬 y-축 벡터를 반환한다.
XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22,
		m_xmf4x4World._23)));
}
//게임 객체의 로컬 x-축 벡터를 반환한다.
XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12,
		m_xmf4x4World._13)));
}
//게임 객체를 로컬 x-축 방향으로 이동한다.
void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
//게임 객체를 로컬 y-축 방향으로 이동한다.
void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
//게임 객체를 로컬 z-축 방향으로 이동한다.
void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
//게임 객체를 주어진 각도로 회전한다.
void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch),
		XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Animate(float fTimeElapsed)
{

}

void CGameObject::OnPrepareRender()
{

}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

}
void CGameObject::ReleaseShaderVariables()
{

}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	//객체의 월드 변환 행렬을 루트 상수(32-비트 값)를 통하여 셰이더 변수(상수 버퍼)로 복사한다.
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	/*OnPrepareRender();

	if (m_pShader)
	{
		m_pShader->Render(pd3dCommandList);
	}

	if (m_pMesh)
	{
		m_pMesh->Render(pd3dCommandList);
	}*/
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	UpdateShaderVariables(pd3dCommandList);

	if (m_pShader)
	{
		//m_pShader->UpdateShaderVariables(pd3dCommandList, &m_xmf4x4World); //월드 변환 행렬을 VS에 넘긴다. ==> 이렇게 바꾸는 이유는 1. Shader에서 저 위치값을 넘기는것보단 위치값은 게임 오브젝트의 위치니까 게임 오브젝트 자체가 갖고있는게 더욱 용이하다 판단해서
		m_pShader->Render(pd3dCommandList, pCamera);
	}

	if (m_pMesh)
	{
		m_pMesh->Render(pd3dCommandList);
	}
}

CRotatingObject::CRotatingObject()
{
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 90.0f;
}

CRotatingObject::~CRotatingObject()
{

}

void CRotatingObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle)); //해당 방향축을 기준으로 fAngle만큼 회전 시키는 행렬을 계산해주는 함수
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World); //행렬곱

}

void CRotatingObject::Animate(float fTimeElapsed)
{
	Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
}

