#include "stdafx.h"
#include "Player.h"
#include"Protocol.pb.h"
#include"ServerPacketHandler.h"
#include "Shader.h" //다른 오브젝트들은 Shader가 Object를 갖는 구조로 Direct12의 상태 변환를 최소화 하는 방향이지만, Player는 단독으로 존재하거나 소수로 존재함으로 자기 자신이
//Shader를 갖는 경우가 있기에 이렇게 include해서 사용도 한다.

CPlayer::CPlayer()
{
	m_pCamera = NULL;
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}
CPlayer::~CPlayer()
{
	ReleaseShaderVariables();
	if (m_pCamera) delete m_pCamera;
}

void CPlayer::AddForce(XMFLOAT3 force)
{
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	//CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList); //

	//UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	//m_pd3dcbPlayer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	//m_pd3dcbPlayer->Map(0, NULL, (void**)&m_pcbMappedPlayer);

	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::ReleaseShaderVariables()
{
	CGameObject::ReleaseShaderVariables();
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}
void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedPlayer->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbPlayer->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dGpuVirtualAddress);

	//CGameObject::UpdateShaderVariables(pd3dCommandList);
}

/*플레이어의 위치를 변경하는 함수이다. 플레이어의 위치는 기본적으로 사용자가 플레이어를 이동하기 위한 키보드를
누를 때 변경된다. 플레이어의 이동 방향(dwDirection)에 따라 플레이어를 fDistance 만큼 이동한다.*/
void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	XMFLOAT3 raw = XMFLOAT3(0, 0, 0);
	
	if(dwDirection)
	{
		if (dwDirection & DIR_FORWARD)
			raw = Vector3::Add(raw, m_xmf3Look);
		if (dwDirection & DIR_BACKWARD)
			raw = Vector3::Add(raw, Vector3::ScalarProduct(m_xmf3Look, -1, false));
		if (dwDirection & DIR_RIGHT)
			raw = Vector3::Add(raw, m_xmf3Right);
		if (dwDirection & DIR_LEFT)
			raw = Vector3::Add(raw, Vector3::ScalarProduct(m_xmf3Right, -1, false));

	}
	// 이동 방향만 저장 (속도는 Update에서 계산)
	m_xmf3InputDir = raw;   
}
void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	//XMFLOAT3 pos = GetPosition();
	//if (pos.x > m_MoveBounds.left && pos.x < m_MoveBounds.right &&
	//	pos.z > m_MoveBounds.top && pos.z < m_MoveBounds.bottom)
	//{

	//	//bUpdateVelocity가 참이면 플레이어를 이동하지 않고 속도 벡터를 변경한다.
	//	if (bUpdateVelocity)
	//	{
	//		//플레이어의 속도 벡터를 xmf3Shift 벡터만큼 변경한다.
	//		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	//	}
	//	else
	//	{
	//		//플레이어를 현재 위치 벡터에서 xmf3Shift 벡터만큼 이동한다.
	//		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);

	//		//플레이어의 위치가 변경되었으므로 카메라의 위치도 xmf3Shift 벡터만큼 이동한다.
	//		if (m_pCamera) m_pCamera->Move(xmf3Shift);
	//	}
	//}

	// Barn 이동 불가 영역 체크
	auto IsBlockedByBarn = [](const XMFLOAT3& p)
		{
			// 앞쪽 가로 막힘
			if ((p.x >= -7.50f && p.x <= -1.54f && p.z >= 14.56f && p.z <= 15.50f) ||
				(p.x >= 1.45f && p.x <= 7.50f && p.z >= 14.56f && p.z <= 15.50f))
				return true;

			// 안쪽 세로 막힘
			if ((p.x >= -7.50f && p.x <= -1.54f && p.z >= -15.93f && p.z <= -15.22f) ||
				(p.x >= 1.45f && p.x <= 7.50f && p.z >= -15.93f && p.z <= -15.22f))
				return true;

			// 좌우 외벽
			if ((p.x >= -8.0f && p.x <= -7.0f && p.z >= -15.93f && p.z <= 15.50f) ||
				(p.x >= 7.0f && p.x <= 8.0f && p.z >= -15.93f && p.z <= 15.50f))
				return true;

			return false;
		};

	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
		return;
	}

	XMFLOAT3 pos = GetPosition();
	XMFLOAT3 nextPos = Vector3::Add(pos, xmf3Shift);

	// XZ 평면 Clamp
	nextPos.x = MathUtill::ClampFloat(nextPos.x,
		(float)m_MoveBounds.left,
		(float)m_MoveBounds.right);

	nextPos.z = MathUtill::ClampFloat(nextPos.z,
		(float)m_MoveBounds.top,
		(float)m_MoveBounds.bottom);

	// Barn 충돌
	if (IsBlockedByBarn(nextPos))
	{
		nextPos = pos; // 이동 취소
	}

	XMFLOAT3 appliedShift = Vector3::Subtract(nextPos, pos);
	m_xmf3Position = nextPos;

	if (m_pCamera)
		m_pCamera->Move(appliedShift);
}
//플레이어를 로컬 x-축, y-축, z-축을 중심으로 회전한다.
void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCameraMode = m_pCamera->GetMode();
	//1인칭 카메라 또는 3인칭 카메라의 경우 플레이어의 회전은 약간의 제약이 따른다.
	if ((nCameraMode == FIRST_PERSON_CAMERA) || (nCameraMode == THIRD_PERSON_CAMERA))
	{
		/*로컬 x-축을 중심으로 회전하는 것은 고개를 앞뒤로 숙이는 동작에 해당한다. 그러므로 x-축을 중심으로 회전하는
	   각도는 -89.0~+89.0도 사이로 제한한다. x는 현재의 m_fPitch에서 실제 회전하는 각도이므로 x만큼 회전한 다음
	   Pitch가 +89도 보다 크거나 -89도 보다 작으면 m_fPitch가 +89도 또는 -89도가 되도록 회전각도(x)를 수정한다.*/
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			//로컬 y-축을 중심으로 회전하는 것은 몸통을 돌리는 것이므로 회전 각도의 제한이 없다.
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			/*로컬 z-축을 중심으로 회전하는 것은 몸통을 좌우로 기울이는 것이므로 회전 각도는 -20.0~+20.0도 사이로 제한된
		   다. z는 현재의 m_fRoll에서 실제 회전하는 각도이므로 z만큼 회전한 다음 m_fRoll이 +20도 보다 크거나 -20도보다
		   작으면 m_fRoll이 +20도 또는 -20도가 되도록 회전각도(z)를 수정한다.*/
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		//카메라를 x, y, z 만큼 회전한다. 플레이어를 회전하면 카메라가 회전하게 된다.
		m_pCamera->Rotate(x, y, z);
		/*플레이어를 회전한다. 1인칭 카메라 또는 3인칭 카메라에서 플레이어의 회전은 로컬 y-축에서만 일어난다. 플레이어
	   의 로컬 y-축(Up 벡터)을 기준으로 로컬 z-축(Look 벡터)와 로컬 x-축(Right 벡터)을 회전시킨다. 기본적으로 Up 벡
	   터를 기준으로 회전하는 것은 플레이어가 똑바로 서있는 것을 가정한다는 의미이다.*/
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
				XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCameraMode == SPACESHIP_CAMERA)
	{
		/*스페이스-쉽 카메라에서 플레이어의 회전은 회전 각도의 제한이 없다. 그리고 모든 축을 중심으로 회전을 할 수 있
	   다.*/
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right),
				XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
				XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look),
				XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	/*회전으로 인해 플레이어의 로컬 x-축, y-축, z-축이 서로 직교하지 않을 수 있으므로 z-축(Look 벡터)을 기준으로
   하여 서로 직교하고 단위벡터가 되도록 한다.*/
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

//이 함수는 매 프레임마다 호출된다. 플레이어의 속도 벡터에 중력과 마찰력 등을 적용한다.
void CPlayer::Update(float fTimeElapsed)
{

	// -----------------------------------------
	// 1) 방향 입력 정규화
	// -----------------------------------------
		//
	// 1) 입력 방향 정규화
	//
	XMFLOAT3 dir = m_xmf3InputDir;
	//std::cout << dir.x << " : " << dir.y << " : " << dir.z << endl;
	float len = Vector3::Length(dir);

	if (len > 0.001f) {
		dir.x /= len;
		dir.y /= len;
		dir.z /= len;
	}
	else {
		dir = XMFLOAT3(0, 0, 0);
	}

	//
	// 2) 속도 설정 (가속/감속 없음)
	//
	if (len > 0.001f)
		m_fCurrentSpeed = m_fMaxSpeed;   // 입력이 있으면 바로 최대 속도
	else
		m_fCurrentSpeed = 0.0f;          // 입력 없으면 즉시 정지


	if (!m_bOnGround && !m_bOnBuilding)
	{
		m_fVerticalVelocity += -9.8f * fTimeElapsed;

	}
	else
	{
		m_fVerticalVelocity = 0.0f;
	}
	//
	// 3) 실제 이동 벡터 계산
	//
	XMFLOAT3 moveVec =
	{
		dir.x * m_fCurrentSpeed * fTimeElapsed,
		 m_fVerticalVelocity* fTimeElapsed,
		dir.z * m_fCurrentSpeed * fTimeElapsed
	};

	DWORD nCameraMode = m_pCamera->GetMode();

	if (nCameraMode == SPACESHIP_CAMERA)
	{
		moveVec.y = dir.y * m_fCurrentSpeed * fTimeElapsed;
	}
	// -----------------------------------------
	// 5) 실제 이동
	// -----------------------------------------
	Move(moveVec, false);

	// 5) 카메라/콜백 갱신 

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	
	if (nCameraMode == THIRD_PERSON_CAMERA)
		m_pCamera->Update(m_xmf3Position, fTimeElapsed);

	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);

	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		XMFLOAT3 forwardOffset = Vector3::ScalarProduct(m_xmf3Look, 3.0f, false);
		XMFLOAT3 lookAtPos = Vector3::Add(m_xmf3Position, forwardOffset);
		m_pCamera->SetLookAt(lookAtPos);
	}

	m_pCamera->RegenerateViewMatrix();


	// 6) 애니메이션 

	//float finalSpeed = Vector3::Length(m_xmf3Velocity);
	if (m_fCurrentSpeed <= 0.01f && m_pSkinnedAnimationController)
	{
		m_pSkinnedAnimationController->SetTrackAnimationClip(0, 0);
	}
}
void CPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{

}
/*카메라를 변경할 때 ChangeCamera() 함수에서 호출되는 함수이다. nCurrentCameraMode는 현재 카메라의 모드
이고 nNewCameraMode는 새로 설정할 카메라 모드이다.*/
CCamera* CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	//새로운 카메라의 모드에 따라 카메라를 새로 생성한다.
	CCamera* pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = new CSpaceShipCamera(m_pCamera);
		break;
	}
	/*현재 카메라의 모드가 스페이스-쉽 모드의 카메라이고 새로운 카메라가 1인칭 또는 3인칭 카메라이면 플레이어의
   Up 벡터를 월드좌표계의 y-축 방향 벡터(0, 1, 0)이 되도록 한다. 즉, 똑바로 서도록 한다. 그리고 스페이스-쉽 카메
   라의 경우 플레이어의 이동에는 제약이 없다. 특히, y-축 방향의 움직임이 자유롭다. 그러므로 플레이어의 위치는 공
   중(위치 벡터의 y-좌표가 0보다 크다)이 될 수 있다. 이때 새로운 카메라가 1인칭 또는 3인칭 카메라이면 플레이어의
   위치는 지면이 되어야 한다. 그러므로 플레이어의 Right 벡터와 Look 벡터의 y 값을 0으로 만든다. 이제 플레이어의
   Right 벡터와 Look 벡터는 단위벡터가 아니므로 정규화한다.*/
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		/*Look 벡터와 월드좌표계의 z-축(0, 0, 1)이 이루는 각도(내적=cos)를 계산하여 플레이어의 y-축의 회전 각도
	   m_fYaw로 설정한다.*/
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		/*새로운 카메라의 모드가 스페이스-쉽 모드의 카메라이고 현재 카메라 모드가 1인칭 또는 3인칭 카메라이면 플레이
	   어의 로컬 축을 현재 카메라의 로컬 축과 같게 만든다.*/
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}
	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		//현재 카메라를 사용하는 플레이어 객체를 설정한다.
		pNewCamera->SetPlayer(this);
	}
	if (m_pCamera) delete m_pCamera;
	return(pNewCamera);
}


void CPlayer::OnPrepareRender()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);
}
void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	//카메라 모드가 3인칭이면 플레이어 객체를 렌더링한다. ==> 3인칭이 아니면 플레이어가 보일 이유가 없기에 Render 자체를 안하는것
	//카메라가 플레이어 내부에 존재하기에 클리핑 처리로 해도 될것 같지만 Render 하는것보단 비용이 들어갈테니까 
	//이런 방식이나 생각은 온라인 게임이 된다고 한다면 이때 Tag로 분류하는 식을 추가하는게 좋을듯?
	
	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		//if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);
		//TOCHANGE:
		CGameObject::Render(pd3dCommandList, pCamera, true, false, false,false);
	}
}

void CPlayer::ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;

	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		CGameObject::Render(pd3dCommandList, pCamera, true, false, false, true);
	}
}

void CPlayer::SetMovementBound(RECT bounds)
{
}


CAirplanePlayer::CAirplanePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	//비행기 메쉬를 생성한다.
	CMesh* pAirplaneMesh = new CAirplaneMeshDiffused(pd3dDevice, pd3dCommandList, 20.0f,
		20.0f, 4.0f, XMFLOAT4(0.0f, 0.5f, 0.0f, 0.0f));
	//CMesh* testMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList);
	SetMesh(pAirplaneMesh);
	//플레이어의 카메라를 스페이스-쉽 카메라로 변경(생성)한다.
	m_pCamera = ChangeCamera(SPACESHIP_CAMERA, 0.0f);
	//플레이어를 위한 셰이더 변수를 생성한다.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//플레이어의 위치를 설정한다.
	SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	//플레이어(비행기) 메쉬를 렌더링할 때 사용할 셰이더를 생성한다.
	CAnimatedShader* pShader = new CAnimatedShader();
	SetShader(pShader);
}
CAirplanePlayer::~CAirplanePlayer()
{
}

void CAirplanePlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
	//비행기 모델을 그리기 전에 x-축으로 90도 회전한다. Mesh의 로컬이 위를 바라보고있어서 한 번 회전시켜주는것
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f,
		0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

//카메라를 변경할 때 호출되는 함수이다. nNewCameraMode는 새로 설정할 카메라 모드이다. 
CCamera* CAirplanePlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		//플레이어의 특성을 1인칭 카메라 모드에 맞게 변경한다. 중력은 적용하지 않는다.
		SetFriction(200.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(100.0f);
		SetMaxVelocityY(100.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f,
			1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		//플레이어의 특성을 스페이스-쉽 카메라 모드에 맞게 변경한다. 중력은 적용하지 않는다.
		SetFriction(125.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(100.0f);
		SetMaxVelocityY(100.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f,
			1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		//플레이어의 특성을 3인칭 카메라 모드에 맞게 변경한다. 지연 효과와 카메라 오프셋을 설정한다.
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(100.0f);
		SetMaxVelocityY(100.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		//3인칭 카메라의 지연 효과를 설정한다. 값을 0.25f 대신에 0.0f와 1.0f로 설정한 결과를 비교하기 바란다.
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f,
			1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));


	//플레이어를 시간의 경과에 따라 갱신(위치와 방향을 변경: 속도, 마찰력, 중력 등을 처리)한다.
	Update(fTimeElapsed);
	return(m_pCamera);
}

////////////////////////////////////////////////////////////////////////////////////////////////
///

void CPlayerIdleSetCallbackHandler::HandleCallback(void* pCallbackData, float fTrackPosition)
{
	
	EAnimationState state =
		(EAnimationState)(uintptr_t)pCallbackData;

	switch (state)
	{
	case EAnimationState::punch:
	{
		m_pController->SetTrackPosition(1, -ANIMATION_CALLBACK_EPSILON);
		m_pController->SetTrackEnable(0, true);
		m_pController->SetTrackEnable(1, false);
		return;
		break;
	}

	case EAnimationState::pickup:
	{
		// pickup 처리
		m_pController->SetTrackPosition(1, -ANIMATION_CALLBACK_EPSILON);
		m_pController->SetTrackEnable(0, true);
		m_pController->SetTrackEnable(1, false);
		return;
		break;
	}
	case EAnimationState::watering:
	{
		// watering 처리
		return;
		break;
	}
	default:
	{
		break;
	}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CPlayer* pPlayer = reinterpret_cast<CPlayer*>(pCallbackData); //이게 없단 소리는 data 가 enum으로 넘어왔다
	if (!pPlayer)
		return;

	CGameObject* pTarget = pPlayer->GetTriggerObject();
	if (!pTarget)
		return;

	EObjectModelType objType = pTarget->GetType();
	Protocol::C_OBEJCT_HP pkt;
	bool bAllowPunchParticle = false;
	switch (objType)
	{
		// Tree / Rock
	case EObjectModelType::Tree01:
	case EObjectModelType::Tree02:
	{
		CTreeObject* pTree = reinterpret_cast<CTreeObject*>(pTarget);
		pTree->AddHP(-1);
		pkt.set_object_id(pTree->GetID());
		pkt.set_hp(pTree->GetHP());
		auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		pPlayer->m_upClientNetwork->SendToServer(sendBuffer);
		bAllowPunchParticle = true;
		break;
	}
	case EObjectModelType::Rock01:
	case EObjectModelType::Rock02:
	{
		CRockObject* pRock = reinterpret_cast<CRockObject*>(pTarget);
		pRock->AddHP(-1);
		pkt.set_object_id(pRock->GetID());
		pkt.set_hp(pRock->GetHP());
		auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		pPlayer->m_upClientNetwork->SendToServer(sendBuffer);
		bAllowPunchParticle = true;
		break;
	}
		// Grass 
	case EObjectModelType::Grass01:
	case EObjectModelType::Grass02:
	{
		CGrassObject* pGrass = reinterpret_cast<CGrassObject*>(pTarget);
		pGrass->AddHP(-1);
		pkt.set_object_id(pGrass->GetID());
		pkt.set_hp(pGrass->GetHP()); auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		pPlayer->m_upClientNetwork->SendToServer(sendBuffer);
		break;
	}
	case EObjectModelType::Corn03:
	case EObjectModelType::Potato03:
	case EObjectModelType::Pumpkin03:
	case EObjectModelType::Tomato03:
	{
		CCropObject* pCrop = reinterpret_cast<CCropObject*>(pTarget);
		pCrop->AddHP(-1);
		pkt.set_object_id(pCrop->GetID());
		pkt.set_hp(pCrop->GetHP()); auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		pPlayer->m_upClientNetwork->SendToServer(sendBuffer);
		break;
	}
		return; 

	default:
		return;
	}

	
	
	
	if (!bAllowPunchParticle)
		return;

	XMFLOAT3 playerPos = pPlayer->GetPosition();
	XMFLOAT3 forward = pPlayer->GetLookVector();
	XMFLOAT3 right = pPlayer->GetRightVector();

	XMVECTOR vForward = XMVector3Normalize(XMLoadFloat3(&forward));
	XMVECTOR vRight = XMVector3Normalize(XMLoadFloat3(&right));

	XMStoreFloat3(&forward, vForward);
	XMStoreFloat3(&right, vRight);

	XMFLOAT3 spawnPos;
	spawnPos.x = playerPos.x
		+ forward.x * 1.1f        // 앞쪽
		- right.x * 0.3f;       //왼쪽 (LH)

	spawnPos.y = playerPos.y + 1.15f;     // 위쪽

	spawnPos.z = playerPos.z
		+ forward.z * 1.1f
		- right.z * 0.3f;
      
	CPunchParticleObject* pPunchParticle = static_cast<CPunchParticleObject*>(pPlayer->GetParticleObject());
	// 파티클 실행
	if (!pPunchParticle)
	{
		return;
	}
	pPunchParticle->SetPosition(spawnPos);
	pPunchParticle->Activate(true);

}


CFarmPlayer::CFarmPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList*
	pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext,int nAnimationTracks)
{
	//플레이어의 카메라를 스페이스-쉽 카메라로 변경(생성)한다.
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
	//플레이어를 위한 셰이더 변수를 생성한다.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CLoadedModelInfo* pAnimatedModel = pModel;

	SetChild(pAnimatedModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pAnimatedModel);
	m_pSkinnedAnimationController->SetTrackAnimationClip(0, 0);
	m_pSkinnedAnimationController->SetTrackAnimationClip(1, 1);

	m_pSkinnedAnimationController->SetTrackEnable(0, true);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);

	
	m_pSkinnedAnimationController->SetCallbackKeys(1, 2); //해당 애니메이션Track에 콜백 키를 1개 만들어라
	//m_pSkinnedAnimationController->SetCallbackKey(1, 0, 1.0f, NULL);

	//핸들러를 등록한다
	CAnimationCallbackHandler* pAnimationCallbackHandler = new CPlayerIdleSetCallbackHandler(m_pSkinnedAnimationController);
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	//CAnimationClip* pAnimationClip = m_pSkinnedAnimationController->m_pAnimationClips->m_pAnimationClips[m_pSkinnedAnimationController->m_pAnimationTracks[1].m_nAnimationClip];
	//m_pSkinnedAnimationController->SetCallbackKey(1, 0, pAnimationClip->m_fLength , NULL);

	m_pSkinnedAnimationController->SetTrackType(1, ANIMATION_TYPE_ONCE);

	SetOBB(pd3dDevice, pd3dCommandList);

	SetScale(XMFLOAT3(1,1,1));
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	SetPlayerUpdatedContext(pTerrain);

	m_MoveBounds = { -62,  -62,   62,   62 }; //초반엔 봄의 영역 만큼 지정

	//Rotate(0, 90, 0);

}
CFarmPlayer::~CFarmPlayer()
{
}

void CFarmPlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
	//비행기 모델을 그리기 전에 x-축으로 90도 회전한다. Mesh의 로컬이 위를 바라보고있어서 한 번 회전시켜주는것
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f,
		0.0f);
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);
}

//카메라를 변경할 때 호출되는 함수이다. nNewCameraMode는 새로 설정할 카메라 모드이다. 
CCamera* CFarmPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 1.5f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(125.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 2.0f, -4.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

void CFarmPlayer::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_TriggerobbBox.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_TriggerobbBox.Extents = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_obbBox.Extents = XMFLOAT3(0.3f, 0.25f, 0.85f); // ==> 이것도 z축을 더 길게 한이유가 로컬이 누워있는걸 회전했기떄문
	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;

	m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.85f, 0.0f);

	m_TriggerobbBox.Extents = XMFLOAT3(0.4f, 0.4f, 0.85f); //
	m_bIsBoxTriggerCollision = true; //충돌 물리 연산은 안하지만 상호작용은 가능하도록 하는거고

	fWidth = m_TriggerobbBox.Extents.x * 2.0f;
	fHeight = m_TriggerobbBox.Extents.y * 2.0f;
	fDepth = m_TriggerobbBox.Extents.z * 2.0f;

	//m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.85f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}

void CFarmPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 pos = GetPosition();

	// HeightMap 기준 좌표 변환
	XMFLOAT3 queryPos = Vector3::Add(pos, XMFLOAT3(249, 0, 249));
	float height = pTerrain->GetHeight(queryPos.x, queryPos.z);

	if (pos.y <= height)
	{
		pos.y = height;
		SetPosition(pos);

		m_bOnGround = true;
		//m_fVerticalVelocity = 0.0f;
	}
	else
	{
		m_bOnGround = false;
	}
}

void CFarmPlayer::AddForce(XMFLOAT3 force)
{
	// 지금 구조에서는 Y만 사용 ==> 점프 구현에만 사용할 예정.
	m_fVerticalVelocity += force.y;
}


void CFarmPlayer::Jump()
{
	
	if (!m_bOnGround && !m_bOnBuilding)
	{
		return;   // 공중 점프 방지
	}

	const float jumpImpulse = 6.0f; // 튜닝값 (6~8 사이 추천)
	AddForce(XMFLOAT3(0.0f, jumpImpulse, 0.0f));

	m_bOnGround = false;
	m_bOnBuilding = false;
	
}



void CFarmPlayer::SetMovementBound(RECT bounds)
{
	m_MoveBounds = bounds;
}

void CFarmPlayer::SetParticleObject(CParticleObject* pParticleObject)
{
	m_pParticleObject = pParticleObject;
}

void CFarmPlayer::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	if (m_bIsBoxTriggerCollision) //Player는 별개로 상호작용 Collision에 위치가 앞에 존재할것임으로 따로 Transform을 해준다
		// 다른 오브젝트에 경우엔 이 Trigger가 있으면 보통 obb에 적용이 되서 따로 updateTransform을 수행해주진 않는다.
	{
		XMMATRIX world = XMLoadFloat4x4(&m_xmf4x4World);

		XMVECTOR rotationQ = XMQuaternionRotationMatrix(world);
		XMStoreFloat4(&m_TriggerobbBox.Orientation, rotationQ);

		XMFLOAT3 worldPos;
		XMStoreFloat3(&worldPos, world.r[3]);

		XMFLOAT3 forward = GetLookVector(); //Player는 오브젝트에 형태 때문에 회전한거라 GetLock으로 가져오면 회전된 결과가 옴
		// 그래서 GetLockVector로 가야함
		XMFLOAT3 triggerCenter = Vector3::Add(worldPos, forward, 1.0f);
		/*	triggerCenter.x += forward.x * 5.0f;
			triggerCenter.z += forward.z * 5.0f;*/

		m_TriggerobbBox.Center = Vector3::Add(triggerCenter, m_xmf3OBBOffset);

		//m_obbBox.Center = Vector3::Add(worldPos, XMFLOAT3(0,2,0));
	}

	CGameObject::UpdateTransform(pxmf4x4Parent);

	
}
