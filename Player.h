#pragma once

#define DIR_FORWARD 0x01 //입력 제어를 이렇게 제어하면 공동입력 제어가 용이하다 ==> 조건문에서 판정하기 좋다.
#define DIR_BACKWARD 0x02
#define DIR_LEFT 0x04
#define DIR_RIGHT 0x08
#define DIR_UP 0x10
#define DIR_DOWN 0x20
#include "GameObject.h"
#include "Camera.h"
#include <ClientNetwork.h>

struct Player_Inventory
{
	int Rock = 0;
	int wood = 0;
	int coonSeed = 0;
	int potatoSeed = 0;
	int pumpkinSeed = 0;
	int tomatoSeed = 0;
};

struct CB_PLAYER_INFO
{
	XMFLOAT4X4					m_xmf4x4World;
};

class CPlayer : public CGameObject
{
protected:
	//플레이어의 위치 벡터, x-축(Right), y-축(Up), z-축(Look) 벡터이다.
	XMFLOAT3 		m_xmf3Position;
	XMFLOAT3 	m_xmf3Right;
	XMFLOAT3 	m_xmf3Up;
	XMFLOAT3 	m_xmf3Look;
	//플레이어가 로컬 x-축(Right), y-축(Up), z-축(Look)으로 얼마만큼 회전했는가를 나타낸다.
	float 		m_fPitch;
	float 	m_fYaw;
	float 	m_fRoll;

	XMFLOAT3 m_xmf3PreviousPosition;

	XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	//플레이어의 이동 속도를 나타내는 벡터이다.
	XMFLOAT3 		m_xmf3Velocity;

	//플레이어에 작용하는 중력을 나타내는 벡터이다.
	XMFLOAT3		m_xmf3Gravity;
	//xz-평면에서 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다.
	float		m_fMaxVelocityXZ;
	//y-축 방향으로 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다.
	float	m_fMaxVelocityY;
	//플레이어에 작용하는 마찰력을 나타낸다.
	float		m_fFriction;
	//플레이어의 위치가 바뀔 때마다 호출되는 OnPlayerUpdateCallback() 함수에서 사용하는 데이터이다. //LPVOID란 Windows에서 정의한 "포인터 형식의 void"이다.
	LPVOID		m_pPlayerUpdatedContext;
	//카메라의 위치가 바뀔 때마다 호출되는 OnCameraUpdateCallback() 함수에서 사용하는 데이터이다.
	LPVOID 		 m_pCameraUpdatedContext;
	//플레이어에 현재 설정된 카메라이다.
	CCamera* m_pCamera = NULL;

	RECT m_MoveBounds; //움직임 반경 제어

public:
	CPlayer();
	virtual ~CPlayer();



	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetPreviousPosition() { return(m_xmf3PreviousPosition); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }
	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	/*플레이어의 위치를 xmf3Position 위치로 설정한다. xmf3Position 벡터에서 현재 플레이어의 위치 벡터를 빼면 현
   재 플레이어의 위치에서 xmf3Position 방향으로의 벡터가 된다. 현재 플레이어의 위치에서 이 벡터 만큼 이동한다.*/
	void SetPosition(XMFLOAT3& xmf3Position) {
		Move(XMFLOAT3(xmf3Position.x -
			m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z),
			false);
	}

	void SetPreviousPosition(XMFLOAT3& xmf3Position)
	{
		m_xmf3PreviousPosition = xmf3Position;
	}

	virtual void AddForce(XMFLOAT3 force);

	XMFLOAT3 m_xmf3InputDir = XMFLOAT3(0, 0, 0);
	float m_fCurrentSpeed = 0.0f;
	float m_fMaxSpeed = 6.0f;       // 최고 속도
	float m_fAccel = 5.0f;          // 가속 계수 (초당 4배 Lerp)
	float m_fDecel = 10.0f;          // 감속 계수
	float m_fgravity = -9.8f;

	float m_fVerticalVelocity = 0.0f;
	bool  m_bOnGround = false;
	bool	 m_bOnBuilding = false;

	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }

	XMFLOAT3& GetVelocity() { return(m_xmf3Velocity); }
	float GetYaw() { return(m_fYaw); }
	float GetPitch() { return(m_fPitch); }
	float GetRoll() { return(m_fRoll); }
	CCamera* GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	//플레이어를 이동하는 함수이다.
	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f) {};
	//플레이어를 회전하는 함수이다.
	void Rotate(float x, float y, float z);
	//플레이어의 위치와 회전 정보를 경과 시간에 따라 갱신하는 함수이다.
	void Update(float fTimeElapsed);
	//플레이어의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }
	//카메라의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	//카메라를 변경하기 위하여 호출하는 함수이다.
	CCamera* OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) {
		return(NULL);
	}
	//플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수이다.
	virtual void OnPrepareRender();
	//플레이어의 카메라가 3인칭 카메라일 때 플레이어(메쉬)를 렌더링한다.
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera =
		NULL);
	void ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera =
		NULL);

	virtual void SetMovementBound(RECT bounds);

	virtual void SetParticleObject(CParticleObject* pParticleObject) {}
	virtual CParticleObject* GetParticleObject() {
		return m_pParticleObject;
	}

	virtual void SetTriggerObject(CGameObject* pObj) { m_pTriggerObject = pObj; }
	virtual CGameObject* GetTriggerObject() { return m_pTriggerObject; }

	virtual BoundingOrientedBox GetTriggerObbBox() { return m_TriggerobbBox; }

	public: 
		Player_Inventory m_pPlayerInventory; // 플레이어 인벤토리 정보
protected:
	ComPtr<ID3D12Resource> m_pd3dcbPlayer = NULL;
	CB_PLAYER_INFO* m_pcbMappedPlayer = NULL;

	CGameObject* m_pTriggerObject = NULL; //현재 플레이어가 상호작용 가능한 오브젝트를 의미하고
	CParticleObject* m_pParticleObject = NULL;
	BoundingOrientedBox m_TriggerobbBox; //바위를 캐거나 나무를 캐거나, 풀을 캐거나 F 상호작용에서 상호작용하고있는 애를 걸러줄 친구
public:
	ClientNetwork* m_upClientNetwork;
	void setNetworkClient(ClientNetwork* netClient)
	{
		m_upClientNetwork = netClient; 
	}
};

class CAirplanePlayer : public CPlayer
{
public:
	CAirplanePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CAirplanePlayer();
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
};

class CPlayerIdleSetCallbackHandler : public CAnimationCallbackHandler
{
private:
	CAnimationController* m_pController = NULL;
	CPlayer* m_pPlayer = NULL;
	CParticleObject* m_pParticleObject = NULL;
public:
	CPlayerIdleSetCallbackHandler(CAnimationController* pController) { m_pController = pController; }
	~CPlayerIdleSetCallbackHandler() { delete m_pController; }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition);
};

class CFarmPlayer : public CPlayer
{

public:
	CFarmPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext, int nAnimationTracks );
	virtual ~CFarmPlayer();
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);

	virtual void AddForce(XMFLOAT3 force);
	virtual void Jump();

	virtual void SetMovementBound(RECT bounds); //라운드에서 움직일 수 있는 반경을 선택하는 함수
	
	virtual void SetParticleObject(CParticleObject* pParticleObject);

	virtual void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);


};