#pragma once
#include "Mesh.h"
#include "Camera.h"

class CShader;

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

	//레퍼런스 카운팅으로 관리할꺼니까 여기도 References 변수를 만들고 AddRef와 Relese를 만들어준다. Direct12에서 기본적으로 인터페이스들을 레퍼런스 카운팅으로 관리하도록 되어 있기 때문에 동일시 해주는 것이다.
private:
	int m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

protected:
	XMFLOAT4X4 m_xmf4x4World; //월드 좌표계상의 자신의 위치 행렬
	CMesh* m_pMesh = nullptr; // 오브젝트는 자신의 형태를 가져야 함으로 Mesh를 갖는다.
	CShader* m_pShader = nullptr; // 자신을 어떻게 그릴지 어떤 Shader로 연결할지 PSO를 연결할지 등등을 결정짓는 Shader 클래스 이건 오브젝트가 가져도 되고 Shader 클래스가 오브젝트를 가져도 된다.
public:
	void ReleaseUploadBuffers();
	virtual void SetMesh(CMesh* pMesh); // Mesh를 설정
	virtual void SetShader(CShader* pShader); // Shader를 설정
	virtual void Animate(float fTimeElapsed);  // Animate로 동작, 충돌 처리 등등 물리적인 연산등을 Render 이전에 수행시킬 것이며 GameObject들은 모두 상속 받아 독자적으로 동작할꺼라 virtual 을 써준다.
	virtual void OnPrepareRender(); //Direct12에서는 CommanndList 명령어를 수행하기 이전에 명시적으로 연결해줘야하거나 상태를 지정해줘야 함으로 PrepareRender와 같은 동작을 해주는 것이 필요하다.
	//현재는 PSO나 루트 시그니쳐 연결등을 여기서 처리하는걸 감안하지 않고 의미만 두고 연결해둔 부분 같다. 2025.07.04

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList); //Animate -> Render로 실제 Draw콜을 실행하는 명령어 현재 이상태로는 Scene에 배치된 모든 GameObject 갯수만큼 DrawCall이 발생할 것이다.
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

public:
	//상수 버퍼를 생성한다.
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList);
	//상수 버퍼의 내용을 갱신한다.
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	//게임 객체의 월드 변환 행렬에서 위치 벡터와 방향(x-축, y-축, z-축) 벡터를 반환한다.
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	//게임 객체의 위치를 설정한다.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	//게임 객체를 로컬 x-축, y-축, z-축 방향으로 이동한다.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	//게임 객체를 회전(x-축, y-축, z-축)한다.
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
};


class CRotatingObject : public CGameObject //GameObject 중 회전을 하는 오브젝트
{

public:
	CRotatingObject();
	virtual ~CRotatingObject();
private:
	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed;
public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; } //Speed값을 설정
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { // 회전 각을 설정
		m_xmf3RotationAxis =
			xmf3RotationAxis;
	}

	virtual void Animate(float fTimeElapsed);

public:

	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle); //

};

