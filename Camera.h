#pragma once

#define ASPECT_RATIO (float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT)) //종횡비 ==> 카메라에는 종횡비가 좀 중요하다

//카메라의 종류(모드: Mode)를 나타내는 상수를 다음과 같이 선언한다.  ==> 이걸보면 비트로 관리하는데 이게 나중에 이것저것 관리하고 조합하는데 용이하기 때문.
#define FIRST_PERSON_CAMERA 0x01
#define SPACESHIP_CAMERA	0x02
#define THIRD_PERSON_CAMERA	0x03

class CPlayer; //이렇게 선언해주는 이유는 카메라는 결국 플레이어를 따랃다니는 필수 요소이기 때문에 여기다가 아 이거 명칭이 뭐였지.. 기억이 안나네

struct VS_CB_CAMERA_INFO //VS에 넘겨줄 상수 버퍼
{
	XMFLOAT4X4 m_xmf4x4View; //카메라 변환 행렬
	XMFLOAT4X4 m_xmf4x4Proj; //투영 변환 행렬
};

//3D 게임 프로그래밍에 가장 중요 요소인 카메라에 대한 클래스 ==> 카메라가 3D 세상을 보여줄 눈이 되어줄 것이다.
class CCamera
{
protected:
	//카메라의 위치(월드좌표계) 벡터이다.
	XMFLOAT3 m_xmf3Position;
	//카메라의 로컬 x-축(Right), y-축(Up), z-축(Look)을 나타내는 벡터이다.*/
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 	m_xmf3Up;
	XMFLOAT3	m_xmf3Look;

	//카메라가 x-축, z-축, y-축으로 얼마만큼 회전했는 가를 나타내는 각도이다.
	float m_fPitch;
	float m_fRoll;
	float 	m_fYaw;

	//카메라의 종류(1인칭 카메라, 스페이스-쉽 카메라, 3인칭 카메라)를 나타낸다.
	DWORD m_nMode; //저 위에 설정한 상수 값을 받아줄 친구.
	//플레이어가 바라볼 위치 벡터이다. 주로 3인칭 카메라에서 사용된다.
	XMFLOAT3 m_xmf3LookAtWorld;
	//플레이어와 카메라의 오프셋을 나타내는 벡터이다. 주로 3인칭 카메라에서 사용된다.
	XMFLOAT3 m_xmf3Offset;
	//플레이어가 회전할 때 얼마만큼의 시간을 지연시킨 후 카메라를 회전시킬 것인가를 나타낸다.
	float m_fTimeLag;

	XMFLOAT4X4 m_xmf4x4View; //카메라 변환 행렬
	XMFLOAT4X4 m_xmf4x4Proj; // 투영 변환 행렬

	//뷰포트랑 시져렉트
	D3D12_VIEWPORT m_d3dViewport;
	D3D12_RECT m_d3dScissorRect;

	//카메라를 가지고 있는 플레이어에 대한 포인터
	CPlayer* m_pPlayer = NULL;

public:
	CCamera();
	CCamera(CCamera* pCamera); //의도가 무엇일까?
	virtual ~CCamera();

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList //Shader에 필요한 설정등을 만드는것. CShader나 Object에서 따로 처리하지 않나? 하겠지만 카메라에 상수버퍼라던가 따로 정의해주는게 정리가 용이하기에 따로 만든다.
		* pd3dCommandList); //그리고 CCamera도 엄연히 하나의 오브젝트니까 오브젝트들은 독자적으로 Shader에 연결할 상수 버퍼나 구조체 버퍼를 갖고있을 테니까 또 생각해볼 수 있다.

	virtual void ReleaseShaderVariables(); //항상 C++, Direct12나 게임을 만들때는 Release와 같은 메모리를 위한 요소는 항상 생각하는 습관을 가져야 한다.

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList); //버퍼 동기화라던가 업데이트라던가 계속해줘야 한다.

	void GenerateViewMatrix();
	void GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up); //카메라 변환 행렬을 추가해주는 함수
	/*카메라가 여러번 회전을 하게 되면 누적된 실수 연산의 부정확성 때문에 카메라의 로컬 x-축(Right), y-축(Up), z
축(Look)이 서로 직교하지 않을 수 있다. 카메라의 로컬 x-축(Right), y-축(Up), z-축(Look)이 서로 직교하도록 만들
어준다.*/
	void RegenerateViewMatrix();

	void GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle); //투영 행렬을 추가해주는 함수

	void SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float fMinZ = //뷰포트 설정 ==> 실제 카메라가 바라보고있는 화면을 스크린 기준 얼만큼 보여줄지 영역을 설정한 Rect
		0.0f, float fMaxZ = 1.0f);

	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom); //  씨져렉트 설정==>뷰포트 내부에 어디를 얼만큼 마스킹 할지 등을 설정해준다.

	virtual void SetViewportsAndScissorRects(ID3D12GraphicsCommandList* pd3dCommandList); //GPU에 실제로 ViewPort, ScissorRect를 추가해주는 Command 명령어를 내리는 부분.


	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; } // 카메라를 갖고있는 플레이어를 지정. 이렇게 해줘야하는 이유로 생각이 드는건 게임내의 플레이어객체가 여러개일 수 있음으로 실제 내가 플레이하는 캐릭터만 카메라를 가졍야 함으로
	CPlayer* GetPlayer() { return(m_pPlayer); } // 카메라에 연결된 플레이어를 반환
	void SetMode(DWORD nMode) { m_nMode = nMode; } //카메라 모드 설정
	DWORD GetMode() { return(m_nMode); } // 현재 카메라의 모드를 반환
	void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; } //카메라의 위치 이동 
	XMFLOAT3& GetPosition() { return(m_xmf3Position); } //카메라의 현재 위치 가져오기
	void SetLookAtPosition(XMFLOAT3 xmf3LookAtWorld) { //현재 카메라가 바라보는 방향 설정을 월드 위치 기준으로 설정하거나 할때 필요한 것 이걸로 게임 중 카메라가 특정 오브젝트를 바라보도록 하는데 용이함 (카메라의 로컬 z축)
		m_xmf3LookAtWorld =
			xmf3LookAtWorld;
	}
	XMFLOAT3& GetLookAtPosition() { return(m_xmf3LookAtWorld); } //현재 카메라가 바라봐야하는 위치 
	XMFLOAT3& GetRightVector() { return(m_xmf3Right); } //로컬  x축
	XMFLOAT3& GetUpVector() { return(m_xmf3Up); } //로컬 y축
	XMFLOAT3& GetLookVector() { return(m_xmf3Look); } // 로컬 z축
	float& GetPitch() { return(m_fPitch); } //x축 회전
	float& GetRoll() { return(m_fRoll); } // z축 회전
	float& GetYaw() { return(m_fYaw); } // y축 회전
	void SetOffset(XMFLOAT3 xmf3Offset) { m_xmf3Offset = xmf3Offset; }
	XMFLOAT3& GetOffset() { return(m_xmf3Offset); }
	void SetTimeLag(float fTimeLag) { m_fTimeLag = fTimeLag; }
	float GetTimeLag() { return(m_fTimeLag); }
	XMFLOAT4X4 GetViewMatrix() { return(m_xmf4x4View); }
	XMFLOAT4X4 GetProjectionMatrix() { return(m_xmf4x4Proj); }
	D3D12_VIEWPORT GetViewport() { return(m_d3dViewport); }
	D3D12_RECT GetScissorRect() { return(m_d3dScissorRect); }
	//카메라를 xmf3Shift 만큼 이동한다.
	virtual void Move(const XMFLOAT3& xmf3Shift) {
		m_xmf3Position.x += xmf3Shift.x;
		m_xmf3Position.y += xmf3Shift.y; m_xmf3Position.z += xmf3Shift.z;
	}
	//카메라를 x-축, y-축, z-축으로 회전하는 가상함수이다.
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f) { }
	//카메라의 이동, 회전에 따라 카메라의 정보를 갱신하는 가상함수이다.
	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed) { }
	//3인칭 카메라에서 카메라가 바라보는 지점을 설정한다. 일반적으로 플레이어를 바라보도록 설정한다.
	virtual void SetLookAt(XMFLOAT3& xmf3LookAt) { }


};

class CSpaceShipCamera : public CCamera //비행기 화면 기준의 카메라 설정ㅇ
{
public:
	CSpaceShipCamera(CCamera* pCamera);
	virtual ~CSpaceShipCamera() { }
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};

class CFirstPersonCamera : public CCamera //1인칭 모드의 카메라 설정
{
public:
	CFirstPersonCamera(CCamera* pCamera);
	virtual ~CFirstPersonCamera() { }
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};

class CThirdPersonCamera : public CCamera //3인칭 모드의 카메라 설정
{
public:
	CThirdPersonCamera(CCamera* pCamera);
	virtual ~CThirdPersonCamera() { }
	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& vLookAt);
};