#pragma once
#include "stdafx.h"
#include "GameTimer.h"
#include "SceneManager.h"
#include "Scene.h"
#include "MainScene.h"
#include "StartScene.h"
#include "Camera.h"
#include "Player.h"
#include "GraphicsCommon.h";
#include "ClientNetwork.h";

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();
	/*CCamera* m_pCamera = NULL;
	CPlayer* m_pPlayer = NULL;*/

	//마우스 클릭할 때의 마우스 커서의 위치
	POINT m_ptOldCursorPos;

private:
	std::unique_ptr<ClientNetwork> m_ClientNetwork;
private: //시간관련된 클래스의 선언자
	//시간 관리 클래스 인스턴스 선언
	CGameTimer m_GameTimer;

	CCamera* m_pCamera = NULL; //카메라

	CScene* m_pScene;	//여러개의 씬을 만들거나 혹은 그럴것을 염두해두고 있기에
	//포인터로 지정해서 리소스 해제를 중요시 해야 한다. 그리고 스마트 포인터를 쓰지 않는 이유는
	//직접적인 관리가 필수인 리소스이니까 좀 더 신경 써주기 위해서이다.
	//추가로 나중에 Scene_manager란걸 만들어 Scene 템플릿을 공유하는 여러 Scene들을 넘겨가면서
	//조절하는 구조를 구성할 계획이다. 이 구조는 Unity와 언리얼 엔진의 코드를 참고해서 구성할 계획

	


	//윈도우 창에 띄우기 위하여 유니코드로 변환 시키는 것.
	_TCHAR m_pszFrameRate[50];
private:
	HINSTANCE m_hInstance;	//
	HWND m_hWnd;	//

	int m_nWndClientWidth;	//클라이언트 크기
	int m_nWndClientHeight;	//클라이언트 크기

	ComPtr<IDXGIFactory4> m_pdxgiFactory;	//DXGI 팩토리 인터페이스에 대한 포인터
	ComPtr<IDXGISwapChain3> m_pdxgiSwapChain;	//스왑 체인 인터페이스에 대한 포인터. 디스플레이 제어를 위해 필요
	ComPtr<ID3D12Device>	m_pd3dDevice;	//Direct3D 디바이스 인터페이스에 대한 포인터, 주로 리소스를 생성하기 위하여 필요

	bool m_bMsaa4xEnable = false;	//Msaa 멀티 샘플링 On/Off 여부
	UINT m_nMasaa4xQualityLevels = 0;	//Msaa에 사용하는 수준 레벨 조절

	static const UINT m_nSwapChainBuffers = 2;	// 후면 버퍼 갯수는 2개
	UINT m_nSwapChainBufferIndex;	// 현재 Present 혹은 Render하는 후면 버퍼의 인덱스 정보

	ComPtr<ID3D12Resource>	m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ComPtr<ID3D12DescriptorHeap>	m_pd3dRtvDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dRTVCPUHandles[m_nSwapChainBuffers];
	//UINT	m_nRtvDescriptorIncrementSize;
	//렌더 타겟 버퍼, 서술자 힙 인터페이스 포인터, 렌더 타겟 서술자 원소의 크기.

	ComPtr<ID3D12Resource> m_pd3dDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap>	m_pd3dDsvDescriptorHeap;
	//UINT m_nDsvDescriptorIncrementSize;
	//깊이-스텐실 버퍼, 서술자 힙 인터페이스 포인터, 깊이-스텐실 서술자 원소의 크기.

	ComPtr<ID3D12CommandQueue>	m_pd3dCommandQueue;	//GPU 명령어를 저장할 Queue
	ComPtr<ID3D12CommandAllocator>	m_pd3dCommandAllocator;		//GPU 명령어를 할당하고 관리해주는 친구
	ComPtr<ID3D12GraphicsCommandList>	m_pd3dCommandList;	//GPU 명령어들을 저장한 리스트
	//명령 큐, 명령 할당자, 명령 리스트 인터페이스 포인터

	ComPtr<ID3D12PipelineState> m_pd3dPipelineState;
	//그래픽스 파이프라인 상태 객체에 대한 인터페이스 포인터

	ComPtr<ID3D12Fence>	m_pd3dFence;	//CommandList 관련 동기화에 사용하는거지
	//UINT64 	m_nFenceValue;
	UINT64  m_nFenceValue[m_nSwapChainBuffers];	// 후면 버퍼마다 펜스를 따로 지정하기 위해서 이렇게 변경.
	HANDLE	m_hFenceEvent;
	//펜스 인터페이스 포인터, 펜스의 값, 이벤트 핸들

	//이건 이제 카메라 클래스 내부에 존재함으로 여기엔 필요 없어서 지운다.
	//D3D12_VIEWPORT m_d3dViewport;
	//D3D12_RECT m_d3dScissorRect;
	//Direct12의 뷰포트와 씨져 사각형


	ComPtr<ID3D12DescriptorHeap> m_ImGuiSRVDescriptorHeap;

	ImGuiIO m_io;

public:
	bool	OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	//프레임 워크를 초기화해주는 함수(주 윈도우가 생성되면 한 번 호출)

	void OnDestroy();
	//프레임 워크가 종료될때 모든 객체를 삭제해주는 부분.

	//ImGui 초기화 정리 함수
	void InitImGui();

	

	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateDirect3dDevice();
	void CreateCommandQueueAndList();
	//스왑 체인, RTV, DSV 서술자 힙 설정, 디바이스, 명령 큐/할당자/리스트를 초기화 시켜주는 함수

	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	//렌더 타겟  뷰와 깊이-스텐실 뷰를 생성하는 함수.

	void BuildObject();
	void ReleaseObjects();
	//렌더링할 메쉬와 게임 객체를 생성하고 소멸하는 함수.

	//Scene이 넘어갈때 호출되는 함수
	void ReBuildObject();

	void PreviousTransform(); //이전 프레임에 동작등을 저장처리할 함수
	void ProcessInput();	// 사용자 입력 처리 (마우스, 키보드)
	void AnimateObjects(); // 게임 객체 애니메이션 (동작 처리 여기서 충돌 처리, 애니메이션 전부 처리)
	void FrameAdvance();	// 프레임 단위로 모든 동작을 처리하는데 사용할 부분 Animation 다음 Render를 처리한다.
	void GUIProcess();

	void WaitForGpuComplete();
	// CPU와 GPU 를 동기화 해주는 함수 ==> CommandList는 Queue에 저장되서 순차적으로 GPU에서 리소스를 사용한다.
	// 아직 CommandList가 다 끝나지 않은 상태에서 CPU명령(프레임 단위로 이루어지는 동작 처리 문을 우리가 코드로 짜서 넘기는거 의미)
	// 새롭게 리소스를 삭제하거나 생성하거나 변경하는 일이 발생하면 문제가 생긴다. 그래서 GPU 와 CPU 간의 동기화가 필수적이다.

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	// 윈도우 프로그래밍에서 사용자 처리를 담당하는 윈도우 메시지처리 등등

	void ChangeSwapChainState();

public:
	void MoveToNextFrame();

};



