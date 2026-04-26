#pragma once
#include "Scene.h"
#include "MainScene.h"

class CStartScene : public CScene
{
public:

	CStartScene(HWND hWnd, CGameTimer* timer, unique_ptr<ClientNetwork> clienteNetwork) : CScene(hWnd, timer, std::move(clienteNetwork))
	{
		m_hWnd = hWnd;
		m_pGameTimer = timer;

	};
	virtual ~CStartScene() {};


	CGameTimer* m_pGameTimer;

	HWND m_hWnd;

	POINT m_ptOldCursorPos;


	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);	//루트 시그니쳐를 생성해주는 함수 ==> 이 둘은 나중에

	virtual void BuildObjects(ID3D12Device* pd3dDeivce, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void ReleaseObjects(); //항상 생성하는 오브젝트가 존재한다면, 삭제도 당연히 존재해야한다.

	virtual bool ProcessInput();	//

	//virtual void AnimateObjects(float fTimeElapsed);	//
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles,
		D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, CCamera* pCamera);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);	//Render를 할때 여러 카메라가 존재할 수 있고, 카메라에 따라 값이 달라질 수 있음으로 Camera를 파라미터로 받는다.

	virtual void OnGUI(ImGuiIO* pGuiIO);

	virtual void ReleaseUploadBuffers();

	virtual void OnEnter() { std::cout << "Scene OnEnter" << endl; };
	virtual void OnPause() { std::cout << "Scene OnPause" << endl; };
	virtual void OnResume() { std::cout << "Scene OnResume" << endl; };
	virtual void OnExit() { std::cout << "Scene OnExit" << endl; };
};

