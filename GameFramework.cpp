#include "stdafx.h"
#include"ServerPacketHandler.h"
#include "GameFramework.h"


CGameFramework::CGameFramework()
{
	//�������� null���� �� ����ִµ� ���� ComPtr�� �����Ƿ� �ڵ� �ʱ�ȭ �ȴ�.

	//m_nFenceValue = 0;

	for (int i = 0; i < m_nSwapChainBuffers; i++)	//�潺�� ���� 0���� �ʹ��� �ʱ�ȭ
		m_nFenceValue[i] = 0;
	m_hFenceEvent = nullptr;

	m_pScene = nullptr;

	//�������� �̰� �߰� ���߾��µ� �� ������ �����ϴ� ���� ���� �׸� ȭ�� View�� �������� �ʾҰ�
	//����ü�� ���۸� �����ϰ� ������ â�� ���� �̺�Ʈ ���踸 �ϴ� ������ �����ϱ�
	/*m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };*/


	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	_tcscpy_s(m_pszFrameRate, _T("LapProject ("));
}

CGameFramework::~CGameFramework()
{

}

//���� ���α׷��� ����ǰ� �� �����찡 �����Ǹ� ȣ��Ǵ� �Լ��̴�.
bool CGameFramework::OnCreate(HINSTANCE hInstantce, HWND hMainWnd)
{

	m_hInstance = hInstantce;	//������ �ν��Ͻ�
	m_hWnd = hMainWnd;	//������ �ڵ�

	//-----���� ���Ժ� ����------
	m_ClientNetwork = std::make_unique<ClientNetwork>();
	if (!m_ClientNetwork->Start(L"127.0.0.1", 7777))
	{

		std::wcout << L"Failed" << std::endl;
		// ���⼭ false ��������, �׳� ������������ ������ �� ����
		return false;
	}
	else
	{
		std::wcout << L"Connected Server" << std::endl;
		Protocol::C_LOGIN pkt;
		pkt.set_accountid("1");
		pkt.set_password("2");
		auto sendbuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		m_ClientNetwork->SendToServer(sendbuffer);

	}
	//-----���� ���Ժ� ���� ��------

	CreateDirect3dDevice();	//Device�� �����
	CreateCommandQueueAndList();	// CommandList�� Device�� ���� �� ������ ���Ŀ� ����
	CreateRtvAndDsvDescriptorHeaps();	//RTV�� DSV�� ���� DescriptorHeap�� �����Ѵ� �̰͵� Device�� ��� ������ ���Ŀ� ����
	CreateSwapChain();	//DXGI �� Device �׸��� CommandList�� �̿��� ����ü���� �����ؾ� �Ѵ�.
	CreateRenderTargetViews();	//DescriptorHeap�� �����ؾ� RTV�� �����ϰ� ������ �� �ִ�.
	CreateDepthStencilView();	//���� ����

	BuildObject(); // Device�� SwapChain �� ��� ������ �Ϸ�� ���� ���� ��ü�� ������ ���������� ���������.

	InitImGui(); //ImGui�� �ʱ�ȭ �Ѵ�



	return (true);

}


void CGameFramework::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	m_io = ImGui::GetIO();
	m_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Ű���� �Է� ���
	
	ImGui::StyleColorsDark();

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	::ZeroMemory(&desc,sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	desc.NumDescriptors = 1; //Imgui�� �Ѱ�
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_pd3dDevice->CreateDescriptorHeap(&desc, __uuidof(ID3D12DescriptorHeap),
		(void**)m_ImGuiSRVDescriptorHeap.GetAddressOf());

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX12_Init(
		m_pd3dDevice.Get(),
		m_nSwapChainBuffers,
		DXGI_FORMAT_R8G8B8A8_UNORM,    // ����ü�� ����� ���˰� �����ϰ�
		m_ImGuiSRVDescriptorHeap.Get(),
		m_ImGuiSRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_ImGuiSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
	);
}

void CGameFramework::OnDestroy()
{
	WaitForGpuComplete(); // ==> ���� ��Ƽ �������� �̿��ؼ� ���� ������ �۾��� ó���ϴ� ������ �ٲ۴ٸ�, �� �� ������������ �̰� ������ Lock�� ���� ������ �����ϱ�
	//GPU�� ��� ���� ����Ʈ�� ������ �� ���� ��ٸ���. �̰��� CPU�� GPU ����ȭ�ϴ� ���ɾ�

	ReleaseObjects();
	//���� ��ü(���� ���� ��ü��) �Ҹ��Ų��.

	::CloseHandle(m_hFenceEvent);
	//ComPtr�� ������̴� ���� ������ �ʿ�� ����.
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
	//DXGI ����� �������̽��� Ȱ���� Direct3D ���ø����̼ǿ��� **�޸� ������ �������� ���� ���ҽ��� Ž���ϱ� 
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

		// 1. ����Ʈ���� ����� ����
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		// 2. D3D12 ���� ���� üũ
		if (FAILED(D3D12CreateDevice(
			adapter.Get(),
			D3D_FEATURE_LEVEL_12_0,
			_uuidof(ID3D12Device),
			nullptr)))
		{
			continue;
		}

		// 3. VRAM ���� ū ����� ����
		if (desc.DedicatedVideoMemory > maxDedicatedVideoMemory)
		{
			maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
			bestAdapter = adapter;
		}
	}

	// 4.���� Device ����
	if (bestAdapter)
	{
		hResult = D3D12CreateDevice(
			bestAdapter.Get(),
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&m_pd3dDevice));
	}
	else
	{
		// �ϵ���� GPU�� ���ٸ� WARP
		ComPtr<IDXGIAdapter> warpAdapter;
		m_pdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
		hResult = D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&m_pd3dDevice));
	}


	// Fence ����

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
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; //���� ����� Direct�� ����
	//DIRECT, COMPUTE, COPY �� 3���� ������ �����ϸ� 
	//COPY�� ���� �۾�
	//COMPUTE�� Compute + Copy ����
	//DIRECT�� �׷��� + Compute + Copy ���� �����ϴ�.
	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, __uuidof(ID3D12CommandQueue)
		, (void**)m_pd3dCommandQueue.GetAddressOf());

	//����(Direct) ���� ť�� �����Ѵ�.

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator)
		, (void**)m_pd3dCommandAllocator.GetAddressOf());
	//����(Direct) ���� �Ҵ��ڸ� �����Ѵ�.

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator.Get()
		, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)m_pd3dCommandList.GetAddressOf());
	//����(Direct) ���� ����Ʈ�� �����Ѵ�.

	hResult = m_pd3dCommandList->Close();
	//���� ����Ʈ�� �����Ǹ� ���� �����̹Ƿ� ���� ���·� �ʱ⿡ �������� �Ѵ�.
	// ���� ť, ���� �Ҵ���, ���� ����Ʈ ������ ������ �ϸ�
	// ���� ť�� ������ Type�� �°� �Ҵ��ڿ� ����Ʈ�� ����� ��� �Ѵ�. �߰��� ����Ʈ�� ��帶��ũ ���� ���߾�� �Ѵ�.
	//
}

/*
* ����� �켱 ������ ��ȯ ���� �ֱ����� SwapChain ����� ����� �ִ� �Լ�.
//>>DXGI Factory�� CreateSwapChain ���ش�
void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;
	//Ŭ���̾�Ʈ ũ�� �޾ƿͼ� �������ְ� �ִ�.

	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));	//�޸𸮸� 0���� ��� �ʱ�ȭ
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//�ĸ���� ���� ����
	//����ü�� MSAA ����
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;	//���ø� ����
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMasaa4xQualityLevels - 1) : 0;	//��� ���� ���ϴ°�
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;	//����ü���� ũ�� ��������� �ǹ� ����� ������� �ʴ´ٴ� ��
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//���۳��� �����ϴ� ����� �����ϴ°�, ���� �ø� ��� ���� ������ ����Ѵ�(Discard)�� ����ִ�.
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;	//���� ä���� ��� �������? �߰��ϴ� �κ� ���� ��������
	//���� �� �𸣰ڴ� ����� �����ϴ� ����
	//dxgiSwapChainDesc.Flags = 0;	//�߰� ������ �����ϴ� �÷��׷� ���� �߰� �ɼǵ� �����Ѵ�. ==> �̰� �ƹ��͵� ���� ���ϴ°�

	//��üȭ�� ���÷��� ��� ���� ��ȯ ���
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH: ��ü ȭ�� ��忡�� ���÷��� ��带 ��ȯ�� �� �ֵ��� ���.
	//DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT: ���� ü�ο��� ������ ��� ������ ����.
	//DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : ���÷��� �׾(ȭ�� ����)�� ���.

	//�ý�ũ������ ����������� ���� ����
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	//����ü�� ����

	m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue.Get(), m_hWnd, &dxgiSwapChainDesc,
		&dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)m_pdxgiSwapChain.GetAddressOf());

	m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	//ALT + ENTER Ű�� ������ ��Ȱ��ȭ

	m_nSwapChainBufferIndex = m_pdxgiSwapChain.Get()->GetCurrentBackBufferIndex();
	//����ü���� ���� �ĸ���� �ε����� ����

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}
*/

//Swapchain�� ����� ����� �� 2������ �����ϸ� ��Ŀ� ���� OnCreate�� �׸��� ���� �� �ִ�.
void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;
	//Ŭ���̾�Ʈ ũ�� �޾ƿͼ� �������ְ� �ִ�.

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));	//�޸𸮸� 0���� ��� �ʱ�ȭ

	//����ü�� MSAA ����
	//dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;	//���ø� ����
	//dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMasaa4xQualityLevels - 1) : 0;	//��� ���� ���ϴ°�
	dxgiSwapChainDesc.SampleDesc.Count = 1;	//���ø� ����
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//���۳��� �����ϴ� ����� �����ϴ°�, ����

	//��üȭ�� ���÷��� ��� ���� ��ȯ ���
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH: ��ü ȭ�� ��忡�� ���÷��� ��带 ��ȯ�� �� �ֵ��� ���.
	//DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT: ���� ü�ο��� ������ ��� ������ ����.
	//DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : ���÷��� �׾(ȭ�� ����)�� ���.

	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //HDR�� ����� ǥ���ȵȴٰ��ؼ� SRGB�� ��������
	//����ü���� �����Ҷ��� SRGB ������ �ȵȴٰ� �� ������ RTV������ SRGB�� �ؾ���
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	//����ü�� ����

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue.Get(), &dxgiSwapChainDesc,
		(IDXGISwapChain**)m_pdxgiSwapChain.GetAddressOf());

	m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	//ALT + ENTER Ű�� ������ ��Ȱ��ȭ

	m_nSwapChainBufferIndex = m_pdxgiSwapChain.Get()->GetCurrentBackBufferIndex();
	//����ü���� ���� �ĸ���� �ε����� ����

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

//����Ÿ�� view�� ���̹��� view�� heap�� ����
void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers + 1 + 1 + 1;	//���� ü���� ������ �����ϰ� ����
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	//RTV�� DescriptorHeap�� ����Ѵٴ� �ǹ�
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap)
		, (void**)m_pd3dRtvDescriptorHeap.GetAddressOf());
	//���� Ÿ�� ������ ��(�������� ������ ����ü�� ������ ������)�� �����Ѵ�.

	//::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//DESCRIPTORHEAP���ٰ� ������ ���� ���� �������� �������� �Ӽ��� ���߾� ������ ũ�⸦ ��������� �Ѵ�.

	d3dDescriptorHeapDesc.NumDescriptors = 1 + 5; //���� ������ ���� ����ų� �� ��� ���� ������ŭ ������� �� �ִ�.
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//RTV�� DescriptorHeap�� ����Ѵٴ� �ǹ�
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap)
		, (void**)m_pd3dDsvDescriptorHeap.GetAddressOf());
	//���� Ÿ�� ������ ��(�������� ������ ����ü�� ������ ������)�� �����Ѵ�.

	//::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

}

//������ �����ϰ� �������� ���� ���۵� ���������� G���۸� Ȱ���ϴ� ��쿡�� �Ϲ����� ����ó�� ���� ���� ��������� �Ѵ�.
void CGameFramework::CreateRenderTargetViews()
{
	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //LDR == >HDR�� ����� ǥ���ȵȴٰ��ؼ� SRGB�� ��������
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =	//View�� ���鶧 Heap ������ ����� �ϰ�
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();	//Heap�� ���� �ּҸ� �����;� �Ѵ�.
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)	//Swap Chain ���� ���� ��ŭ �ݺ�
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)m_ppd3dRenderTargetBuffers[i].GetAddressOf());
		// SwapCahin�� ����鼭 �����ߴ� ���۸� GetBuffer ���ҽ��� ����.
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i].Get(), &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dRTVCPUHandles[i] = d3dRtvCPUDescriptorHandle;
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
		//����� ���ҽ� ����ۿ��ٰ� View Create ���ش�. �̶� ���� CPU �ּҰ� �ʿ��ϴ�.
		//���߿� CommandList���� ���۵��� �������ٶ��� GPU �ּҰ� �ʿ��ϴ�. Swap Chain�� G���۸� Ȱ���ϴ� ��Ȳ�� �ƴϸ�
		//���� Heap�� GPU �ּҸ� ������ �����ϰų� ����� ���� ���� ����. ���� ���۵� ������������ �ʼ��� ���̴�.
	}
}

void CGameFramework::CreateDepthStencilView()
{
	//������ ������� ���ҽ��� DESC, ���ҽ��� � TYPE���� ������� (DEFAULT, UPLOAD, READ), �ش� ���ҽ� �ʱ�ȭ ��
	//�������ִ� CLEAR_VALUE �� ����
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
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; //CPU�� �ش� ���� � ������� ������ �� �ִ��� ����

	/*
	* D3D12_CPU_PAGE_PROPERTY_UNKNOWN : �� Ÿ���� �⺻�̸� DirectX�� �ڵ����� ����
	* D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE :  CPU�� �� ���� ���� �Ұ�
	* D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE : CPU -> GPU ���� ���� (������ ����, ĳ�� �̻��)
	* D3D12_CPU_PAGE_PROPERTY_WRITE_BACK : CPU���� ĳ�õ� ���� ��� (�Ϲ��� �б�/ ���� ���)
	*/

	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; //���� ���鶧 � ���� �޸�Ǯ�� ��ġ������ ����
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;
	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue,
		__uuidof(ID3D12Resource), (void**)m_pd3dDepthStencilBuffer.GetAddressOf());
	//���� ���ٽ� ���� ����

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle
		= m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer.Get(), NULL, d3dDsvCPUDescriptorHandle);
	//���� ���ٽ� ���� View�� ����

}

//Scene �� ������Ʈ�� �ʱ⿡ �������ִ� ���� ==> ��Ÿ�� �� �����Ǵ°� ���� ��������� �Ѵ�.
void CGameFramework::BuildObject()
{
	//PSO ��� �ʱ�ȭ
	CGraphicsCommon::GetInstance().InitCommonStates(m_pd3dDevice.Get());

	m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);

	CScene* pStartScene = new CStartScene(m_hWnd, &m_GameTimer , std::move(m_ClientNetwork));
	CSceneManager::GetInstance().PushScene(pStartScene);
	m_pScene = CSceneManager::GetInstance().GetTopScene();

	//ī�޶� ��ü�� �����ϰ� ����Ʈ, ������Ʈ, ���� ��ȯ ���, ī�޶� ��ȯ ����� �����Ѵ�.
	m_pCamera = new CCamera();
	m_pCamera->SetViewport(0, 0, m_nWndClientWidth, m_nWndClientHeight, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, m_nWndClientWidth, m_nWndClientHeight);
	m_pCamera->GenerateProjectionMatrix(1.0f, 500.0f, float(m_nWndClientWidth) /
		float(m_nWndClientHeight), 90.0f);
	m_pCamera->GenerateLightsProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f, 1280, 720);
	m_pCamera->GenerateViewMatrix(XMFLOAT3(0.0f, 0.0f, -50.0f), XMFLOAT3(0.0f, 0.0f,
		0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	m_pCamera->CreateShaderVariables(m_pd3dDevice.Get(), m_pd3dCommandList.Get());
	//if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice.Get());	// Scene�� ������Ʈ���� �����ϴ� ����.

	//���� �����ϸ� �� ���ο� ������Ʈ���� �����Ѵ�.
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice.Get(), m_pd3dCommandList.Get());

	/*CAirplanePlayer* pAirplanePlayer = new CAirplanePlayer(m_pd3dDevice.Get(),
		m_pd3dCommandList.Get(), m_pScene->GetGraphicsRootSignature());
	m_pPlayer = pAirplanePlayer;
	m_pCamera = m_pPlayer->GetCamera();*/

	//��� ���� ���ɾ� �����ؼ� �����Ű������ �׻� CommandList�� Close ����� �Ѥ���.
	m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList.Get() };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	//�׷��� ���� ����Ʈ���� ��� ����� ������ ��ٸ���.
	WaitForGpuComplete();
	//�׷��� ���ҽ����� �����ϴ� ������ ������ ���ε� ���۵��� �Ҹ��Ų��.
	if (m_pScene) m_pScene->ReleaseUploadBuffers();

	m_GameTimer.Reset();	//BuildObject�� ����Ǵ� ��Ȳ�̸� �켱 ���� Ÿ�̸Ӹ� �������ִ°� �⺻�̴�.

}

void CGameFramework::ReBuildObject()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);

	m_pScene = CSceneManager::GetInstance().GetTopScene();

	//���� �����ϸ� �� ���ο� ������Ʈ���� �����Ѵ�.
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
		case 1: //MainScene�� ���
			{
				//��ó���� ���� ������ �߰� ��.
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

	//��� ���� ���ɾ� �����ؼ� �����Ű������ �׻� CommandList�� Close ����� �Ѥ���.
	m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList.Get() };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	//�׷��� ���� ����Ʈ���� ��� ����� ������ ��ٸ���.
	WaitForGpuComplete();
	//�׷��� ���ҽ����� �����ϴ� ������ ������ ���ε� ���۵��� �Ҹ��Ų��.
	if (m_pScene)
	{
		m_pScene->ReleaseUploadBuffers();
	}

	m_GameTimer.Reset();	//BuildObject�� ����Ǵ� ��Ȳ�̸� �켱 ���� Ÿ�̸Ӹ� �������ִ°� �⺻�̴�.

}

//Build �ؼ� ���ҽ��� ��������� �׻� ¦�� ó�� �ʿ��� ����.
void CGameFramework::ReleaseObjects()
{
	if (m_pScene)
	{
		m_pScene->ReleaseObjects();
		delete m_pScene;
	}

}

//������ API ���콺 �Է� �̺�Ʈ ó��
void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//���콺 ĸ�ĸ� �ϰ� ���� ���콺 ��ġ�� �����´�
		//::SetCapture(hWnd);
		//::GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		//���콺 ĸ�ĸ� �����Ѵ�.
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

//������ API Ű���� �Է� �̺�Ʈ ó��
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
			/*��F1�� Ű�� ������ 1��Ī ī�޶�, ��F2�� Ű�� ������ �����̽�-�� ī�޶�� �����Ѵ�, ��F3�� Ű�� ������ 3��Ī ī�޶�
�� �����Ѵ�.*/
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
			//'F9' Ű�� ������ ������ ���� ��üȭ�� ����� ��ȯ�� ó���Ѵ�
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

//������ API ���ν����̰� ���⼭ �� ���� 2���� �̺�Ʈ�� ȣ�����ش�.
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
	//������ ���� �ð��� Animate �ȿ��� �Ѱ��ְ� �ִ� ������
	//������Ʈ�� Animate�� �ð����� ��������� �ϸ� �� ���ӳ��� �귯���� �ð� ����
	//�� TimeElapsed�� ����ϰ� ������ �� ������
	//������ ����Ʈ�� �޶����� ���Ǽ��� ���� ������ ������ ����� ������ �� �ְ�
	//�ð��� ����ϱ� ���ؼ� �ʿ��� �����̴�.
	if (m_pScene) m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
}

//GPU ����ȭ�� ���� �Լ� ���ǻ���** Command ���ɾ ��� ���� ������ �ش� �Լ��� ȣ���ؾ��Ѵ�.
void CGameFramework::WaitForGpuComplete()
{

	//m_nFenceValue++;	//CPU �潺�� ���� �����Ѵ�.
	//const UINT64 nFence = m_nFenceValue;	//����Ǳ� ����

	const UINT64 nFence = ++m_nFenceValue[m_nSwapChainBufferIndex];	//����� ���� (�����ϱ� 06 ����)
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence.Get(), nFence);
	//GPU�� �潺�� ���� �����ϴ� ������ ���� ť�� �߰��Ѵ�.

	if (m_pd3dFence->GetCompletedValue() < nFence)
	{
		//�潺�� ���� ���� ������ ������ ������ �潺�� ���� ���� ������ ���� �� ������ ��ٸ���.
		hResult = m_pd3dFence->SetEventOnCompletion(nFence, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}

	/*
	* 1. m_nFencValue ���� 1 ���� ��Ų��.
	* 2. CommandQueue�� Signal ���ɾ �߰��Ѵ� �� ���ɾ�� ���� ������ ���ɾ�� GPU�� ��� ���ɾ ���������� ȣ��
	* 3. �׶� Fence�� Value ���� m_nFencValue ������ ������ ���̴�.
	* 4. ���ǹ����� ���� Value ���� �������� ������ :: WaitForSingleObject�� CPU�� ������ �ɾ�д�.
	* 5. SetEventOnCompletion���� ��ǥ �潺 ���� �����ϸ� m_hFenceEvent ���� �������Ѽ� �������� �˸��� CPU ������ ����
	*/
}

//���� WaitForGPU�� ���� ���������� ���� ������ ���ۿ� �ٸ� �ĸ���۸� ����� ��쿡 ���Ͽ� ����صδ� ��.
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

//GUI �Է�
void CGameFramework::GUIProcess()
{
	
	// ���� ImGui ������ ����
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Scene�� �����Ѵٸ� Scene�� GUI ȣ��
	if (m_pScene)
	{
		m_pScene->OnGUI(&m_io); 
	}

}

//Frame ���� ó�����ִ� �Լ�
void CGameFramework::FrameAdvance()
{
	//������ ����Ʈ ������ �Ŵ� ��
	m_GameTimer.Tick(120.0f);

	if (CSceneManager::GetInstance().GetChangeNum() != 0) //0�� �ƴϸ� ReBuild
	{
		ReBuildObject();
		CSceneManager::GetInstance().NotifySceneStuck();
	}

	PreviousTransform(); //���� ��ġ������ ����

	ProcessInput(); //�Է� ����

	AnimateObjects(); // ������ ����

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL);
	// ���ɾ ������ ���۽� ��� �ʱ�ȭ �����ش�. ������ �����ϱ� ���ؼ�

	::SynchronizeResourceTransition(m_pd3dCommandList.Get(), m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex].Get()
	, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	//m_pd3dCommandList->RSSetViewports(1,&m_d3dViewport);	//Viewport�� RS �ܰ迡 ����
	//m_pd3dCommandList->RSSetScissorRects(1,&m_d3dScissorRect);	// ���� �簢�� ���� ����

	/*
	* TODO:: 
	* ���� �Ʒ��� �ִ� �� RTV�� �����ص� ����. ������ �ƴ� ����.
	*/
	//D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvDescriptorHandle =
	//	m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//d3dRtvDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * m_nSwapChainBufferIndex);
	//���� ����� ���� Ÿ�ٿ� �ش��ϴ� ������ �ڵ��� ���

	//float pfClearColor[4] = { 0.0f,0.0f,0.0f,1.0f };
	//m_pd3dCommandList->ClearRenderTargetView(d3dRtvDescriptorHandle, pfClearColor,
	//	0, NULL);
	//���ϴ� �������� ���� Ÿ��(��)�� �����.

	//���� ���ٽ� ���۸� �ʱ�ȭ ���ش�.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 0, 0, NULL);

	//OM �ܰ� ���������� �ĸ���ۿ� �׷��ִ� �ܰ� ���⼭ Blending�� ���� �׽�Ʈ�� ó�������� ���̴� �ڵ忡��
	//PS �ܰ迡�� OM �ܰ� ������ ���� �׽�Ʈ�� ó���ϴ� ��� ���� �����Ѵ�.
	//m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvDescriptorHandle, TRUE, &d3dDsvDescriptorHandle);

	/*
	* ���� ���̿� Render ó���� ������ ���̴�.
	*/
	//if (m_pScene) m_pScene->Render(m_pd3dCommandList.Get());	//Render�� ó���Ѵ�
	if (m_pScene)
	{
		
		m_pScene->OnPreRender(m_pd3dCommandList.Get(), 1, &m_pd3dRTVCPUHandles[m_nSwapChainBufferIndex], &d3dDsvDescriptorHandle, m_pCamera);
		m_pScene->Render(m_pd3dCommandList.Get(), m_pCamera);	//Render�� ó���Ѵ�
		m_pScene->OnPostRender(m_pd3dCommandList.Get(), 1, &m_pd3dRTVCPUHandles[m_nSwapChainBufferIndex], &d3dDsvDescriptorHandle, m_pCamera);
	}

	GUIProcess(); //

	ImGui::Render();
	ID3D12DescriptorHeap* heaps[] = { m_ImGuiSRVDescriptorHeap.Get() };
	m_pd3dCommandList->SetDescriptorHeaps(_countof(heaps), heaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pd3dCommandList.Get());



	//3��Ī ī�޶��� �� �÷��̾ �׻� ���̵��� �������Ѵ�.
#ifdef _WITH_PLAYER_TOP
//���� Ÿ���� �״�� �ΰ� ���� ���۸� 1.0���� ����� �÷��̾ �������ϸ� �÷��̾�� ������ �׷��� ���̴�.
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif
	//3��Ī ī�޶��� �� �÷��̾ �������Ѵ�.
	//if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList.Get(), m_pCamera);

	//Render�� ������ ȭ�鿡 SwapChain�� Present�ؾ� ������ ���¸� �ٽ� PRESNET�� ��ȯ
	
	::SynchronizeResourceTransition(m_pd3dCommandList.Get(), m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex].Get()
		, D3D12_RESOURCE_STATE_RENDER_TARGET,D3D12_RESOURCE_STATE_PRESENT);

	hResult = m_pd3dCommandList->Close();	//���� ���� ��� ���ɾ �۵��ҷ��� Close ���¿��� �Ѵ�.


	ComPtr<ID3D12CommandList> ppd3dCommandLists[] = { m_pd3dCommandList };//�������� CommandList�� ������ �� �� �ֱ⿡
	//�̷��� �����ϴ� ���̴�.
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists->GetAddressOf());//���� GPU���� ��� ���ɾ
	//ó���϶�� �����Ų��.

	WaitForGpuComplete();	//�츮�� �˰��ִ� GPU ����ȭ�� ���ؼ�

	//�Ʒ� �ּ�ó�� �� ���� ������ Present ó���� �� �� Preset�� Present1�� ���̰� ���� ������ ����� �ҵ�?
	/*
	DXGI_PRESENT_PARAMETERS dxgiPresnetParameters;
	dxgiPresnetParameters.DirtyRectsCount = 0;
	dxgiPresnetParameters.pDirtyRects = NULL;
	dxgiPresnetParameters.pScrollOffset = NULL;
	dxgiPresnetParameters.pScrollRect = NULL;
	m_pdxgiSwapChain->Present1(1,0, &dxgiPresnetParameters);	//���������� SwapChain�� ���ؼ� �ĸ���۸� ���� ȭ�鿡 �����ش�.

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	*/
	m_pdxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);

}

//////// Set�̳� �߰����� ��ɵ��� �Ʒ��� �߰����� ����

//������ ȭ�� ��ü or ������ â ��ȯ ���
void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();	//Swap_Chain �ػ󵵸� ��ȯ��Ű�� �Ŵϱ� �ĸ� ���� ���ҽ��� ��ȯ�� �̷������ �켱
	//��� GPU ���ɾ �������� ���θ� Ȯ������ �ʿ䰡 �ִ�. ����ȭ �ʿ�.
	//�׻� ���ҽ��� ��ȯ��Ű�ų� �����ϰų� �� ���� �������� ������ ����ȭ�� ����� �Ѵ�.

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);	//���� Swap_CHian ��带 ��������
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);	//������ ������ â Ȥ�� ��ü���� �ݴ� ���� ��ȯ

	//�׷��ٸ� ��ȯ�Ǿ����� ���ҽ��� ���¸� �� ����������Ѵ�

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


