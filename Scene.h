#pragma once
#include "GameTimer.h"
#include "Shader.h"


class CScene
{
public:
	CScene();
	virtual ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);


	//void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);	//루트 시그니쳐를 생성해주는 함수 ==> 이 둘은 나중에

	void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice);	//PSO를 만들어주는 함수. ==> 외부 클래스 혹은
	//stdafx로 빼내거나 아니면 Scene Manager를 만들어 별개로 빼둘 계획이다. 여러 Scene을 Load하고 관리할려면
	//생성관련된 함수는 공용으로 설계하는게 가장 좋으니까.
	//아니면 이 Scene 클래스를 Scene Manager 처럼 사용하고 다른 Scene 클래스를 만들어서 설계해도 좋을 듯 하다.
	//GameFramework에서 모두 한 번에 처리하도록 설계하는 것도 나쁘지 않을듯??
	//물론 Scene에 사용되는 리소스에 량에 따라서 혹은 다른 루트 PSO 가 필요한 경우가 있으니 이건 좀 더 고려해 봐야 한다.
	//이 문구는 항상 읽어봐라 2025.02.15 작성
	// Scene구조를 스택 구조로 바꾸어 사용할 예정으로 변경.
	// 루트 시그니쳐와 루트 파라미터와 같은것들은 별계의 유틸 클래스로 구별하여 static으로 만들어 상황에따라 Scene에서 선택하도록 변경예정
	// 이 문구는 2025.05.31 작성

	void BuildObjects(ID3D12Device* pd3dDevice);
	void BuildObjects(ID3D12Device* pd3dDeivce, ID3D12GraphicsCommandList* pd3dCommandList);

	void ReleaseObjects(); //항상 생성하는 오브젝트가 존재한다면, 삭제도 당연히 존재해야한다.

	bool ProcessInput();	//
	void AnimateObjects(float fTimeElapsed);	//

	void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);	//Render를 하기전 PSO나 루트 시그니쳐 등을
	//연결을 해줘야 하는데 이러한 공통 설정등을 미리 연결해주는걸 항상 해야 한다.
	void Render(ID3D12GraphicsCommandList* pd3dCommandList);		//

	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);	//Render를 할때 여러 카메라가 존재할 수 있고, 카메라에 따라 값이 달라질 수 있음으로 Camera를 파라미터로 받는다.

	void ReleaseUploadBuffers();

	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice); //루트 시그니쳐를 생성해주고 반환
	ID3D12RootSignature* GetGraphicsRootSignature(); //저장하고 있는 루트 시그니쳐를 반환

	ComPtr<ID3D12RootSignature> m_pd3dRootSignature;	//루트 시그니쳐
	ComPtr<ID3D12PipelineState> m_pd3dPipelineState;	//그래픽 파이프라인 만일 Compute 파이프라인이면 같은 변수지만 별계로
	//내부를 설정해줘야 한다.

protected:
	//CShader안에도 오브젝트가 있는데 밑에 오브젝트를 사용하는 동안에는 쓰지 않아야 한다.
	CShader** m_ppShader = NULL;
	//int m_nShaders = 0;

	//배치(Batch) 처리를 하기 위한 Shader
	CObjectsShader* m_pShaders = NULL;
	int m_nShaders = 0;

	//게임 오브젝트를 씬이 직접 가지고 있는 구조로 바꿔서 테스트하기 위해 추가된 부분.
	CGameObject** m_ppObjects = NULL;
	int m_nObjects = 0;

	CGameObject* m_pTestObject = NULL;


};

