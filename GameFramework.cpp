#include "stdafx.h"
#include"ServerPacketHandler.h"
#include "GameFramework.h"


CGameFramework::CGameFramework()
{
	//예제에는 null값을 다 집어넣는데 나는 ComPtr을 썼으므로 자동 초기화 된다.

	//m_nFenceValue = 0;

	for (int i = 0; i < m_nSwapChainBuffers; i++)	//펜스의 값을 0으로 초반의 초기화
		m_nFenceValue[i] = 0;
	m_hFenceEvent = nullptr;

	m_pScene = nullptr;

	//이전에는 이걸 추가 안했었는데 그 이유는 간단하다 아직 무언가 그릴 화면 View를 구성하지 않았고
	//스왑체인 버퍼를 생성하고 윈도우 창을 띄우고 이벤트 설계만 하는 과정이 었으니까
	/*m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };*/


	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	_tcscpy_s(m_pszFrameRate, _T("LapProject ("));
}

CGameFramework::~CGameFramework()
{

}

//응용 프로그램이 실행되고 주 윈도우가 생성되면 호출되는 함수이다.
bool CGameFramework::OnCreate(HINSTANCE hInstantce, HWND hMainWnd)
{

	m_hInstance = hInstantce;	//윈도우 인스턴스
	m_hWnd = hMainWnd;	//윈도우 핸들

	//-----서버 도입부 연결------
	m_ClientNetwork = std::make_unique<ClientNetwork>();
	if (!m_ClientNetwork->Start(L"127.0.0.1", 7777))
	{

		std::wcout << L"Failed" << std::endl;
		// 여기서 false 리턴할지, 그냥 오프라인으로 갈지는 네 선택
		return false;
	}
	else
	{
		std::wcout << L"Connected Server" << std::endl;
		Protocol::C_LOGIN pkt;
		pkt.set_accountid(1);
		pkt.set_password(2);
		auto sendbuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		m_ClientNetwork->SendToServer(sendbuffer);

	}
	//-----서버 도입부 연결 끝------

	CreateDirect3dDevice();	//Device를 만들고
	CreateCommandQueueAndList();	// CommandList는 Device로 만들 수 있으니 이후에 선언
	CreateRtvAndDsvDescriptorHeaps();	//RTV와 DSV를 위한 DescriptorHeap을 구성한다 이것도 Device를 써야 함으로 이후에 선언
	CreateSwapChain();	//DXGI 와 Device 그리고 CommandList를 이용해 스왑체인을 구성해야 한다.
	CreateRenderTargetViews();	//DescriptorHeap이 존재해야 RTV를 생성하고 연결할 수 있다.
	CreateDepthStencilView();	//이하 동문

	BuildObject(); // Device와 SwapChain 등 모든 설정이 완료된 이후 게임 객체를 만들어야 정상적으로 만들어진다.

	InitImGui(); //ImGui를 초기화 한다



	return (true);

}


void CGameFramework::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	m_io = ImGui::GetIO();
	m_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 키보드 입력 허용
	
	ImGui::StyleColorsDark();

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	::ZeroMemory(&desc,sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	desc.NumDescriptors = 1; //Imgui용 한개
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_pd3dDevice->CreateDescriptorHeap(&desc, __uuidof(ID3D12DescriptorHeap),
		(void**)m_ImGuiSRVDescriptorHeap.GetAddressOf());

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX12_Init(
		m_pd3dDevice.Get(),
		m_nSwapChainBuffers,
		DXGI_FORMAT_R8G8B8A8_UNORM,    // 스왑체인 백버퍼 포맷과 동일하게
		m_ImGuiSRVDescriptorHeap.Get(),
		m_ImGuiSRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_ImGuiSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
	);
}

void CGameFramework::OnDestroy()
{
	WaitForGpuComplete(); // ==> 만일 멀티 쓰레딩을 이용해서 동시 렌더링 작업을 처리하는 구조로 바꾼다면, 좀 더 복잡해지겠지 이게 일종의 Lock과 같은 구조를 가지니까
	//GPU가 모든 명령 리스트를 실행할 때 까지 기다린다. 이것이 CPU와 GPU 동기화하는 명령어

	ReleaseObjects();
	//게임 객체(게임 월드 객체를) 소멸시킨다.

	::CloseHandle(m_hFenceEvent);
	//ComPtr을 사용중이니 구지 선언할 필요는 없다.
	/*
	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dRenderTargetBuffers[i])
		m_ppd3dRenderTargetBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();
	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dPipelineState) m_pd3dPipelineState->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();
	if (m_pd3dFence) m_pd3dFence->Release();
	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();
	*/
#if defined(_DEBUG)
	//DXGI 디버그 인터페이스를 활용해 Direct3D 애플리케이션에서 **메모리 누수나 해제되지 않은 리소스를 탐지하기 
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
		DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

//DXGI Factory >> Adapter >> Device >> Fance >> Viewport
void CGameFramework::CreateDirect3dDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ComPtr<ID3D12Debug> pd3dDebugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pd3dDebugController))))
	{
		pd3dDebugController->EnableDebugLayer();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = CreateDXGIFactory2(nDXGIFactoryFlags, IID_PPV_ARGS(&m_pdxgiFactory));

	ComPtr<IDXGIAdapter1> bestAdapter;
	SIZE_T maxDedicatedVideoMemory = 0;

	for (UINT i = 0; ; ++i)
	{
		ComPtr<IDXGIAdapter1> adapter;
		if (FAILED(m_pdxgiFactory->EnumAdapters1(i, &adapter)))
			break;

		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		// 1. 소프트웨어 어댑터 제외
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		// 2. D3D12 지원 여부 체크
		if (FAILED(D3D12CreateDevice(
			adapter.Get(),
			D3D_FEATURE_LEVEL_12_0,
			_uuidof(ID3D12Device),
			nullptr)))
		{
			continue;
		}

		// 3. VRAM 가장 큰 어댑터 선택
		if (desc.DedicatedVideoMemory > maxDedicatedVideoMemory)
		{
			maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
			bestAdapter = adapter;
		}
	}

	// 4.최종 Device 생성
	if (bestAdapter)
	{
		hResult = D3D12CreateDevice(
			bestAdapter.Get(),
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&m_pd3dDevice));
	}
	else
	{
		// 하드웨어 GPU가 없다면 WARP
		ComPtr<IDXGIAdapter> warpAdapter;
		m_pdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
		hResult = D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&m_pd3dDevice));
	}


	// Fence 생성

	hResult = m_pd3dDevice->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&m_pd3dFence));

	m_nFenceValue[0] = 0;
	m_nFenceValue[1] = 0;

	m_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	// Descriptor increment size
	::gnCbvSrvDescriptorIncrementSize =
		m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize =
		m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize =
		m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}
// CommandQueue >> CommandAllocator >> CommandList
void CGameFramework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; //명령 방식을 Direct로 설정
	//DIRECT, COMPUTE, COPY 총 3가지 유형이 존재하며 
	//COPY는 복사 작업
	//COMPUTE는 Compute + Copy 가능
	//DIRECT는 그래픽 + Compute + Copy 전부 가능하다.
	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, __uuidof(ID3D12CommandQueue)
		, (void**)m_pd3dCommandQueue.GetAddressOf());

	//직접(Direct) 명령 큐를 생성한다.

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator)
		, (void**)m_pd3dCommandAllocator.GetAddressOf());
	//직접(Direct) 명령 할당자를 생성한다.

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator.Get()
		, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)m_pd3dCommandList.GetAddressOf());
	//직접(Direct) 명령 리스트를 생성한다.

	hResult = m_pd3dCommandList->Close();
	//명령 리스트는 생성되면 열린 상태이므로 닫힌 상태로 초기에 만들어줘야 한다.
	// 명령 큐, 명령 할당자, 명령 리스트 순으로 만들어야 하며
	// 명령 큐에 설정한 Type에 맞게 할당자와 리스트를 만들어 줘야 한다. 추가로 리스트는 노드마스크 또한 맞추어야 한다.
	//
}

/*
* 여기는 우선 윈도우 변환 설정 넣기전에 SwapChain 만들던 방식이 있던 함수.
//>>DXGI Factory로 CreateSwapChain 해준다
void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;
	//클라이언트 크기 받아와서 설정해주고 있다.

	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));	//메모리를 0으로 모두 초기화
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//후면버퍼 형식 지정
	//스왑체인 MSAA 설정
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;	//샘플링 갯수
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMasaa4xQualityLevels - 1) : 0;	//계산 수준 정하는거
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;	//스왑체인의 크기 조정방식을 의미 현재는 사용하지 않는다는 뜻
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//버퍼끼리 스왑하는 방식을 지정하는것, 현재 플립 방식 이전 내용은 폐기한다(Discard)에 담겨있다.
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;	//알파 채널을 어떻게 사용할지? 추가하는 부분 어디다 쓰는지는
	//아직 잘 모르겠다 현재는 사용안하는 상태
	//dxgiSwapChainDesc.Flags = 0;	//추가 동작을 정의하는 플래그로 여러 추가 옵션들 존재한다. ==> 이건 아무것도 정의 안하는거

	//전체화면 디스플레이 모드 서로 전환 허용
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH: 전체 화면 모드에서 디스플레이 모드를 전환할 수 있도록 허용.
	//DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT: 스왑 체인에서 프레임 대기 동작을 지원.
	//DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 디스플레이 테어링(화면 찢김)을 허용.

	//플스크린으로 사용했을때의 대한 설정
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	//스왑체인 생성

	m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue.Get(), m_hWnd, &dxgiSwapChainDesc,
		&dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)m_pdxgiSwapChain.GetAddressOf());

	m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	//ALT + ENTER 키의 동작을 비활성화

	m_nSwapChainBufferIndex = m_pdxgiSwapChain.Get()->GetCurrentBackBufferIndex();
	//스왑체인의 현재 후면버퍼 인덱스를 저장

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}
*/

//Swapchain을 만드는 방법이 총 2가지가 존재하며 방식에 따라서 OnCreate의 항목이 변할 수 있다.
void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;
	//클라이언트 크기 받아와서 설정해주고 있다.

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));	//메모리를 0으로 모두 초기화

	//스왑체인 MSAA 설정
	//dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;	//샘플링 갯수
	//dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMasaa4xQualityLevels - 1) : 0;	//계산 수준 정하는거
	dxgiSwapChainDesc.SampleDesc.Count = 1;	//샘플링 갯수
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//버퍼끼리 스왑하는 방식을 지정하는것, 현재

	//전체화면 디스플레이 모드 서로 전환 허용
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH: 전체 화면 모드에서 디스플레이 모드를 전환할 수 있도록 허용.
	//DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT: 스왑 체인에서 프레임 대기 동작을 지원.
	//DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 디스플레이 테어링(화면 찢김)을 허용.

	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //HDR이 제대로 표현안된다고해서 SRGB를 붙혀봤음
	//스왑체인을 생성할때는 SRGB 붙히면 안된다고 함 오류뜸 RTV에서만 SRGB로 해야함
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	//스왑체인 생성

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue.Get(), &dxgiSwapChainDesc,
		(IDXGISwapChain**)m_pdxgiSwapChain.GetAddressOf());

	m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	//ALT + ENTER 키의 동작을 비활성화

	m_nSwapChainBufferIndex = m_pdxgiSwapChain.Get()->GetCurrentBackBufferIndex();
	//스왑체인의 현재 후면버퍼 인덱스를 저장

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

//렌더타겟 view와 깊이버퍼 view의 heap을 생성
void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers + 1 + 1 + 1;	//스왑 체인의 갯수와 동일하게 설정
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	//RTV로 DescriptorHeap을 사용한다는 의미
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap)
		, (void**)m_pd3dRtvDescriptorHeap.GetAddressOf());
	//렌더 타겟 서술자 힙(서술자의 개수는 스왑체인 버퍼의 개수로)을 생성한다.

	//::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//DESCRIPTORHEAP에다가 실제로 값을 집어 넣을려면 서술자의 속성에 맞추어 원소의 크기를 설정해줘야 한다.

	d3dDescriptorHeapDesc.NumDescriptors = 1 + 5; //물론 쉐도우 맵을 만들거나 할 경우 조명 갯수만큼 만들어질 수 있다.
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//RTV로 DescriptorHeap을 사용한다는 의미
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap)
		, (void**)m_pd3dDsvDescriptorHeap.GetAddressOf());
	//렌더 타겟 서술자 힙(서술자의 개수는 스왑체인 버퍼의 개수로)을 생성한다.

	//::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

}

//굉장히 간단하게 만들지만 만약 디퍼드 렌더링같이 G버퍼를 활용하는 경우에는 일반적인 버퍼처럼 만들어서 따로 저장해줘야 한다.
void CGameFramework::CreateRenderTargetViews()
{
	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //LDR == >HDR이 제대로 표현안된다고해서 SRGB를 붙혀봤음
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =	//View를 만들때 Heap 연결을 해줘야 하고
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();	//Heap에 시작 주소를 가져와야 한다.
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)	//Swap Chain 버퍼 갯수 만큼 반복
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)m_ppd3dRenderTargetBuffers[i].GetAddressOf());
		// SwapCahin을 만들면서 생성했던 버퍼를 GetBuffer 리소스에 연결.
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i].Get(), &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dRTVCPUHandles[i] = d3dRtvCPUDescriptorHandle;
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
		//연결된 리소스 백버퍼에다가 View Create 해준다. 이때 힙의 CPU 주소가 필요하다.
		//나중에 CommandList에서 버퍼들을 연결해줄때는 GPU 주소가 필요하다. Swap Chain은 G버퍼를 활용하는 상황이 아니면
		//따로 Heap의 GPU 주소를 가져와 저장하거나 사용할 일이 거의 없다. 물론 디퍼드 렌더링에서는 필수일 것이다.
	}
}

void CGameFramework::CreateDepthStencilView()
{
	//위에서 순서대로 리소스이 DESC, 리소스를 어떤 TYPE으로 사용할지 (DEFAULT, UPLOAD, READ), 해당 리소스 초기화 값
	//지정해주는 CLEAR_VALUE 값 지정
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMasaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));

	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; //CPU가 해당 힙을 어떤 방식으로 접근할 수 있는지 정의

	/*
	* D3D12_CPU_PAGE_PROPERTY_UNKNOWN : 힙 타입이 기본이면 DirectX가 자동으로 설정
	* D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE :  CPU는 이 힙에 접근 불가
	* D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE : CPU -> GPU 쓰기 전용 (순차적 쓰기, 캐시 미사용)
	* D3D12_CPU_PAGE_PROPERTY_WRITE_BACK : CPU에서 캐시된 쓰기 허용 (일반적 읽기/ 쓰기 허용)
	*/

	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; //힙을 만들때 어떤 물리 메모리풀에 배치할지를 제공
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;
	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue,
		__uuidof(ID3D12Resource), (void**)m_pd3dDepthStencilBuffer.GetAddressOf());
	//깊이 스텐실 버퍼 생성

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle
		= m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer.Get(), NULL, d3dDsvCPUDescriptorHandle);
	//깊이 스텐실 버퍼 View를 생성

}

//Scene 에 오브젝트를 초기에 생성해주는 과정 ==> 런타임 중 생성되는건 따로 구성해줘야 한다.
void CGameFramework::BuildObject()
{
	//PSO 모두 초기화
	CGraphicsCommon::GetInstance().InitCommonStates(m_pd3dDevice.Get());

	m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);

	CScene* pStartScene = new CStartScene(m_hWnd, &m_GameTimer , std::move(m_ClientNetwork));
	CSceneManager::GetInstance().PushScene(pStartScene);
	m_pScene = CSceneManager::GetInstance().GetTopScene();

	//카메라 객체를 생성하고 뷰포트, 씨저렉트, 투영 변환 행렬, 카메라 변환 행렬을 설정한다.
	m_pCamera = new CCamera();
	m_pCamera->SetViewport(0, 0, m_nWndClientWidth, m_nWndClientHeight, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, m_nWndClientWidth, m_nWndClientHeight);
	m_pCamera->GenerateProjectionMatrix(1.0f, 500.0f, float(m_nWndClientWidth) /
		float(m_nWndClientHeight), 90.0f);
	m_pCamera->GenerateLightsProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f, 1280, 720);
	m_pCamera->GenerateViewMatrix(XMFLOAT3(0.0f, 0.0f, -50.0f), XMFLOAT3(0.0f, 0.0f,
		0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	m_pCamera->CreateShaderVariables(m_pd3dDevice.Get(), m_pd3dCommandList.Get());
	//if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice.Get());	// Scene에 오브젝트들을 생성하는 과정.

	//씬이 존재하면 씬 내부에 오브젝트들을 생성한다.
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice.Get(), m_pd3dCommandList.Get());

	/*CAirplanePlayer* pAirplanePlayer = new CAirplanePlayer(m_pd3dDevice.Get(),
		m_pd3dCommandList.Get(), m_pScene->GetGraphicsRootSignature());
	m_pPlayer = pAirplanePlayer;
	m_pCamera = m_pPlayer->GetCamera();*/

	//모든 생성 명령어 관련해서 실행시키기전에 항상 CommandList를 Close 해줘야 한ㄷ다.
	m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList.Get() };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	//그래픽 명령 리스트들이 모두 실행될 때까지 기다린다.
	WaitForGpuComplete();
	//그래픽 리소스들을 생성하는 과정에 생성된 업로드 버퍼들을 소멸시킨다.
	if (m_pScene) m_pScene->ReleaseUploadBuffers();

	m_GameTimer.Reset();	//BuildObject가 실행되는 상황이면 우선 게임 타이머를 리셋해주는게 기본이다.

}

void CGameFramework::ReBuildObject()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);

	m_pScene = CSceneManager::GetInstance().GetTopScene();

	//씬이 존재하면 씬 내부에 오브젝트들을 생성한다.
	if (m_pScene)
	{
		m_pScene->BuildObjects(m_pd3dDevice.Get(), m_pd3dCommandList.Get());
		int n = CSceneManager::GetInstance().GetSceneNumber();
		switch (n)
		{
		case 0 :
			{

			break;
			}
		case 1: //MainScene에 경우
			{
				//후처리를 위한 설정을 추가 중.
				DXGI_FORMAT pdxgiResourceFormats[1] = { DXGI_FORMAT_R16G16B16A16_FLOAT  };
				D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
				d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * 2);
				D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
				d3dDsvCPUDescriptorHandle.ptr += (::gnDsvDescriptorIncrementSize * 1);
				m_pScene->CreateShaderResourceViewRtvs(m_pd3dDevice.Get(), m_pd3dCommandList.Get(), 1, pdxgiResourceFormats, d3dRtvCPUDescriptorHandle);
				m_pScene->CreateShaderResourceViewDsvs(m_pd3dDevice.Get(), m_pd3dCommandList.Get(), 5, d3dDsvCPUDescriptorHandle, m_pd3dDepthStencilBuffer.Get());
				m_pScene->CreateShaderResourceViewAndUAVStructuredBuffer(m_pd3dDevice.Get(),m_pd3dCommandList.Get() );
				break;
			}
		case 2:
			{
			break;
			}
		default:
			{
			break;
			}
		}
	}

	//모든 생성 명령어 관련해서 실행시키기전에 항상 CommandList를 Close 해줘야 한ㄷ다.
	m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList.Get() };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	//그래픽 명령 리스트들이 모두 실행될 때까지 기다린다.
	WaitForGpuComplete();
	//그래픽 리소스들을 생성하는 과정에 생성된 업로드 버퍼들을 소멸시킨다.
	if (m_pScene)
	{
		m_pScene->ReleaseUploadBuffers();
	}

	m_GameTimer.Reset();	//BuildObject가 실행되는 상황이면 우선 게임 타이머를 리셋해주는게 기본이다.

}

//Build 해서 리소스를 만들었으면 항상 짝꿍 처럼 필요한 존재.
void CGameFramework::ReleaseObjects()
{
	if (m_pScene)
	{
		m_pScene->ReleaseObjects();
		delete m_pScene;
	}

}

//윈도우 API 마우스 입력 이벤트 처리
void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//마우스 캡쳐를 하고 현재 마우스 위치를 가져온다
		//::SetCapture(hWnd);
		//::GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		//마우스 캡쳐를 해제한다.
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

//윈도우 API 키보드 입력 이벤트 처리
void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
	case WM_KEYDOWN:
		if (m_pScene)
		{
			m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		}
		switch (wParam)
		{
			/*‘F1’ 키를 누르면 1인칭 카메라, ‘F2’ 키를 누르면 스페이스-쉽 카메라로 변경한다, ‘F3’ 키를 누르면 3인칭 카메라
로 변경한다.*/
		case VK_F1:
		case VK_F2:
		case VK_F3:

			break;
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F8:
			break;
			//'F9' 키가 눌리면 윈도으 모드와 전체화면 모드의 전환을 처리한다
		case VK_F9:
			ChangeSwapChainState();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

//윈도우 API 프로시저이게 여기서 저 위에 2개의 이벤트를 호출해준다.
LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_SIZE:
	{
		m_nWndClientWidth = LOWORD(lParam);
		m_nWndClientHeight = HIWORD(lParam);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

void CGameFramework::PreviousTransform()
{
	m_pScene->PreviousTransform();
}

void CGameFramework::ProcessInput()
{
	m_pScene->ProcessInput();
}

void CGameFramework::AnimateObjects()
{
	//프레임 간격 시간을 Animate 안에서 넘겨주고 있는 이유는
	//오브젝트의 Animate를 시간으로 조절해줘야 하며 인 게임내의 흘러가는 시간 또한
	//이 TimeElapsed로 계산하고 적용할 수 있으며
	//프레임 레이트가 달라져도 현실세계 기준 동일한 움직임 결과를 보장할 수 있게
	//시간을 계산하기 위해서 필요한 사항이다.
	if (m_pScene) m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
}

//GPU 동기화를 위한 함수 주의사항** Command 명령어를 모두 넣은 다음에 해당 함수를 호출해야한다.
void CGameFramework::WaitForGpuComplete()
{

	//m_nFenceValue++;	//CPU 펜스의 값을 증가한다.
	//const UINT64 nFence = m_nFenceValue;	//변경되기 이전

	const UINT64 nFence = ++m_nFenceValue[m_nSwapChainBufferIndex];	//변경된 이후 (따라하기 06 이후)
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence.Get(), nFence);
	//GPU가 펜스의 값을 설정하는 명령을 명령 큐에 추가한다.

	if (m_pd3dFence->GetCompletedValue() < nFence)
	{
		//펜스의 현재 값이 설정한 값보다 작으면 펜스의 현재 값이 설정한 값이 될 때까지 기다린다.
		hResult = m_pd3dFence->SetEventOnCompletion(nFence, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}

	/*
	* 1. m_nFencValue 값을 1 증가 시킨다.
	* 2. CommandQueue의 Signal 명령어를 추가한다 이 명령어는 가장 마지막 명령어로 GPU의 모든 명령어가 끝나고나서 호출
	* 3. 그때 Fence의 Value 값을 m_nFencValue 값으로 변경할 것이다.
	* 4. 조건문으로 현재 Value 값이 동일하지 않으면 :: WaitForSingleObject로 CPU의 블록을 걸어둔다.
	* 5. SetEventOnCompletion으로 목표 펜스 값에 도달하면 m_hFenceEvent 값을 변형시켜서 끝났음을 알리며 CPU 블록을 해제
	*/
}

//위에 WaitForGPU와 거의 동일하지만 다음 프레임 동작에 다른 후면버퍼를 사용할 경우에 대하여 대비해두는 것.
void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	const UINT64 nFenceValue = ++m_nFenceValue[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence.Get(), nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}

}

//GUI 입력
void CGameFramework::GUIProcess()
{
	
	// 공통 ImGui 프레임 시작
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Scene이 존재한다면 Scene의 GUI 호출
	if (m_pScene)
	{
		m_pScene->OnGUI(&m_io); 
	}

}

//Frame 동작 처리해주는 함수
void CGameFramework::FrameAdvance()
{
	//프레임 레이트 제한을 거는 것
	m_GameTimer.Tick(120.0f);

	if (CSceneManager::GetInstance().GetChangeNum() != 0) //0이 아니면 ReBuild
	{
		ReBuildObject();
		CSceneManager::GetInstance().NotifySceneStuck();
	}

	PreviousTransform(); //이전 위치값등을 저장

	ProcessInput(); //입력 제어

	AnimateObjects(); // 움직임 제어

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);
	// 명령어를 프레임 시작시 모두 초기화 시켜준다. 오류를 방지하기 위해서

	::SynchronizeResourceTransition(m_pd3dCommandList.Get(), m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex].Get()
	, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	//m_pd3dCommandList->RSSetViewports(1,&m_d3dViewport);	//Viewport를 RS 단계에 전달
	//m_pd3dCommandList->RSSetScissorRects(1,&m_d3dScissorRect);	// 시져 사각형 또한 전달

	/*
	* TODO:: 
	* 여기 아래에 있는 저 RTV는 삭제해도 무방. 사용안함 아니 겹쳐.
	*/
	//D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvDescriptorHandle =
	//	m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//d3dRtvDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * m_nSwapChainBufferIndex);
	//현재 사용할 렌더 타겟에 해당하는 서술자 핸들을 계산

	//float pfClearColor[4] = { 0.0f,0.0f,0.0f,1.0f };
	//m_pd3dCommandList->ClearRenderTargetView(d3dRtvDescriptorHandle, pfClearColor,
	//	0, NULL);
	//원하는 색상으로 렌더 타겟(뷰)를 지운다.

	//깊이 스텐실 버퍼를 초기화 해준다.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 0, 0, NULL);

	//OM 단계 최종적으로 후면버퍼에 그려주는 단계 여기서 Blending과 깊이 테스트를 처리하지만 셰이더 코드에서
	//PS 단계에서 OM 단계 이전에 깊이 테스트를 처리하는 경우 또한 존재한다.
	//m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvDescriptorHandle, TRUE, &d3dDsvDescriptorHandle);

	/*
	* 이쪽 사이에 Render 처리를 실행할 것이다.
	*/
	//if (m_pScene) m_pScene->Render(m_pd3dCommandList.Get());	//Render를 처리한다
	if (m_pScene)
	{
		
		m_pScene->OnPreRender(m_pd3dCommandList.Get(), 1, &m_pd3dRTVCPUHandles[m_nSwapChainBufferIndex], &d3dDsvDescriptorHandle, m_pCamera);
		m_pScene->Render(m_pd3dCommandList.Get(), m_pCamera);	//Render를 처리한다
		m_pScene->OnPostRender(m_pd3dCommandList.Get(), 1, &m_pd3dRTVCPUHandles[m_nSwapChainBufferIndex], &d3dDsvDescriptorHandle, m_pCamera);
	}

	GUIProcess(); //

	ImGui::Render();
	ID3D12DescriptorHeap* heaps[] = { m_ImGuiSRVDescriptorHeap.Get() };
	m_pd3dCommandList->SetDescriptorHeaps(_countof(heaps), heaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pd3dCommandList.Get());



	//3인칭 카메라일 때 플레이어가 항상 보이도록 렌더링한다.
#ifdef _WITH_PLAYER_TOP
//렌더 타겟은 그대로 두고 깊이 버퍼를 1.0으로 지우고 플레이어를 렌더링하면 플레이어는 무조건 그려질 것이다.
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif
	//3인칭 카메라일 때 플레이어를 렌더링한다.
	//if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList.Get(), m_pCamera);

	//Render가 끝나면 화면에 SwapChain을 Present해야 함으로 상태를 다시 PRESNET로 전환
	
	::SynchronizeResourceTransition(m_pd3dCommandList.Get(), m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex].Get()
		, D3D12_RESOURCE_STATE_RENDER_TARGET,D3D12_RESOURCE_STATE_PRESENT);

	hResult = m_pd3dCommandList->Close();	//이제 실제 모든 명령어가 작동할려면 Close 상태여야 한다.


	ComPtr<ID3D12CommandList> ppd3dCommandLists[] = { m_pd3dCommandList };//여러개의 CommandList를 연결할 수 도 있기에
	//이렇게 연결하는 것이다.
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists->GetAddressOf());//이제 GPU에게 모든 명령어를
	//처리하라고 실행시킨다.

	WaitForGpuComplete();	//우리가 알고있는 GPU 동기화를 위해서

	//아래 주석처리 된 것이 이전에 Present 처리를 한 것 Preset와 Present1의 차이가 뭔지 구분을 지어야 할듯?
	/*
	DXGI_PRESENT_PARAMETERS dxgiPresnetParameters;
	dxgiPresnetParameters.DirtyRectsCount = 0;
	dxgiPresnetParameters.pDirtyRects = NULL;
	dxgiPresnetParameters.pScrollOffset = NULL;
	dxgiPresnetParameters.pScrollRect = NULL;
	m_pdxgiSwapChain->Present1(1,0, &dxgiPresnetParameters);	//최종적으로 SwapChain을 통해서 후면버퍼를 실제 화면에 보여준다.

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	*/
	m_pdxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);

}

//////// Set이나 추가적인 기능들을 아래에 추가해줄 예정

//윈도우 화면 전체 or 윈도우 창 변환 모드
void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();	//Swap_Chain 해상도를 변환시키는 거니까 후면 버퍼 리소스에 변환이 이루어지니 우선
	//모든 GPU 명령어가 끝났는지 여부를 확인해줄 필요가 있다. 동기화 필요.
	//항상 리소스를 변환시키거나 삭제하거나 할 일이 있을때는 무조건 동기화를 해줘야 한다.

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);	//현재 Swap_CHian 모드를 가져오고
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);	//언제든 윈도우 창 혹은 전체모드로 반대 모드로 변환

	//그렇다면 변환되었을때 리소스들 상태를 또 설정해줘야한다

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		if (m_ppd3dRenderTargetBuffers[i]) m_ppd3dRenderTargetBuffers[i]->Release();

		DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
		m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
		m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth,
			m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

		m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

		CreateRenderTargetViews();
	}
}


//
