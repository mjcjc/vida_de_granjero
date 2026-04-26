#include "stdafx.h"
#include "Scene.h"
#include"Protocol.pb.h"
#include"ServerPacketHandler.h"
CDescriptorHeap* CScene::m_pDescriptorHeap = NULL;

CDescriptorHeap::CDescriptorHeap()
{
	m_d3dSrvCPUDescriptorStartHandle.ptr = NULL;
	m_d3dSrvGPUDescriptorStartHandle.ptr = NULL;
	m_d3dUavCPUDescriptorStartHandle.ptr = NULL;
	m_d3dUavGPUDescriptorStartHandle.ptr = NULL;
}

CDescriptorHeap::~CDescriptorHeap()
{
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();
}


CScene::CScene(HWND hWnd, CGameTimer* timer, unique_ptr<ClientNetwork> clienteNetwork)
{
	m_hWnd = hWnd;
	m_pGameTimer = timer;
	m_upClientNetwork = std::move(clienteNetwork);
	
}

CScene::~CScene()
{

}

//루트 시그니쳐를 생성 ==> 초기에 간단하게 루트 시그니쳐를 만들고 적용하는 정도만 사용한 예제
//void CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
//{
//	//루트 시그니쳐를 생성 
//	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
//	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
//	d3dRootSignatureDesc.NumParameters = 0;	//루트 파라미터 연결 갯수
//	d3dRootSignatureDesc.NumStaticSamplers = 0;	//정적 샘플러 연결 갯수
//	d3dRootSignatureDesc.pParameters = NULL;	//연결할 루트 파라미터의 포인터
//	d3dRootSignatureDesc.pStaticSamplers = NULL;	//연결할 정적 샘플러의 포인터
//	d3dRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
//
//	ComPtr<ID3DBlob> pd3dSignatureBlob;	//
//	ComPtr<ID3DBlob> pd3dErrorBlob;			//
//	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pd3dSignatureBlob.GetAddressOf()
//		, pd3dErrorBlob.GetAddressOf());
//
//	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(),
//		__uuidof(ID3D12RootSignature), (void**)m_pd3dRootSignature.GetAddressOf());
//
//	if (pd3dSignatureBlob) pd3dSignatureBlob.Reset();
//	if (pd3dErrorBlob) pd3dErrorBlob.Reset();
//
//	//루트 시그니쳐란?
//	/*
//	* 렌더 파이프라인이 GPU 속에서 어떻게 작동할지 등을 설정하는 코딩인 Shader 코딩에 필요한
//	* 글로벌 변수등의 연결 및 구조를 정의내리는 것.
//	* GPU는 우리 생각보다 단순하고 연결이 까다롭기에 루트 시그니쳐에서 
//	* BYTE 크기 및 사용처 등을 전부 까다롭게 정의내려주어야 한다.
//	*/
//}

//ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
//{
//	/*
//	* 루트 파라미터 한 번에 64개의 슬롯을 설정 가능
//	* 상수, 디스크립터, 디스크립터 테이블 3개를 연결할 수 있다.
//	* 용도에 따라서 3가지 중 선택해서 넘겨야 한다.
//	*/
//	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;
//
//
//	D3D12_ROOT_PARAMETER pd3dRootParameters[2];
//	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS; //상수를 그대로 연결하는 경우
//	pd3dRootParameters[0].Constants.Num32BitValues = 16; // 16바이트의 상수버퍼의 상수 값을 넘긴다는 의미
//	pd3dRootParameters[0].Constants.ShaderRegister = 0; //Shader에 레지스트 번호 0번 사용
//	pd3dRootParameters[0].Constants.RegisterSpace = 0;
//	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VS에서 사용 가능하다는 의미
//	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
//	pd3dRootParameters[1].Constants.Num32BitValues = 32; // 32바이트
//	pd3dRootParameters[1].Constants.ShaderRegister = 1; //0번을 0번 슬롯에서 쓰니까 1번으로 지정
//	pd3dRootParameters[1].Constants.RegisterSpace = 0;
//	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
//
//	//플래그를 이렇게 변수처럼 따로 만들어 DESC에 연결 가능
//	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
//		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
//		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
//		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
//		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
//		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
//
//	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
//	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
//	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
//	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
//	d3dRootSignatureDesc.NumStaticSamplers = 0;
//	d3dRootSignatureDesc.pStaticSamplers = NULL;
//	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;
//
//	ID3DBlob* pd3dSignatureBlob = NULL;
//	ID3DBlob* pd3dErrorBlob = NULL;
//	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, //구조체를 GPU가 읽을 수 있는 바이너리 형식으로 직렬화 시켜주고 있다.
//		&pd3dSignatureBlob, &pd3dErrorBlob);
//	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), //직렬화된 바이트 코드를 읽어와 실제로 Create 해준다
//		pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void
//			**)&pd3dGraphicsRootSignature);
//	if (pd3dSignatureBlob) pd3dSignatureBlob->Release(); // 더 이상 필요 없으니까 Release 해준다
//	if (pd3dErrorBlob) pd3dErrorBlob->Release();
//	return(pd3dGraphicsRootSignature);
//
//}


//그래픽 파이프라인 상태 PSO를 생성 여기서 Shader도 생성한다.
// 이 PSO는 어떤 렌더링이냐에 따라 많이 달라지니까 나중에 여러개 PSO를 한 번에 관리하는
//그런 클래스를 따로 만들거나 따른 설정에다가 빼둘 예정.
void CScene::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice)
{
	//정점 셰이더와 픽셀 셰이더를 생성한다.
	ComPtr<ID3DBlob> pd3dVertexShaderBlob;
	ComPtr<ID3DBlob> pd3dPixelShaderBlob;

	UINT nCompileFlags = 0;
#if defiend(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	D3DCompileFromFile(L"Shader/Shaders.hlsl", NULL, NULL, "VSMain", "vs_5_1", nCompileFlags, 0,
		pd3dVertexShaderBlob.GetAddressOf(), NULL);

	D3DCompileFromFile(L"Shader/Shaders.hlsl", NULL, NULL, "PSMain", "ps_5_1", nCompileFlags, 0,
		pd3dPixelShaderBlob.GetAddressOf(), NULL);
	//Shader를 컴파일 해주는 함수. Shaders.hlsl 이라는 이름에 셰이더 파일에 진입 함수 이름 VSMain과 PSMain 2개의 
	//Shader 단계를 설정하고 각각의 버전을 선택해주고 있다.

	//래스터라이저 상태를 설정
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;	//어떻게 칠할지를 지정 RS 단계에서 설정하는 이유는 
	//정점 사이에 선형 보간을 통해서 나머지 PS에 넘길 점들의 좌표들을 만들어내니까 이 단계에서 설정하는 거다.
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK; //은면 처리
	d3dRasterizerDesc.FrontCounterClockwise = FALSE; //은면 기준을 시계 방향으로 처리한다 TRUE이면 반시계이다
	d3dRasterizerDesc.DepthBias = 0;	// z-fighting 문제를 해결하기 위한 깊이 바이어스 설정
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;	//깊이 바이어스 설정에 오차 범위 지정
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;	//
	d3dRasterizerDesc.DepthClipEnable = TRUE;	//NDC로 변환후 깊이 클리핑을 할지 말지 지정
	d3dRasterizerDesc.MultisampleEnable = FALSE;	//멀티샘플링 할지말지
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;	//안티앨리어싱 할지 말지
	d3dRasterizerDesc.ForcedSampleCount = 0;	//멀티샘플링을 강제로 적용할때 사용하는 옵션 0이면 기본 동작이자 샘플링 없음이다.
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	//레스터화 단계의 픽셀 계산 범위를 정할때 사용하는 방식에 차이를 설정하는 옵션


	//블렌드 상태를 설정 ==> 블렌딩 옵션을 설정하는 부분
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;	//알파투커버리지를 활성화할지
	d3dBlendDesc.IndependentBlendEnable = FALSE;	// 다중 렌더 타겟 개별 블렌딩 활성화 여부
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;	// 블렌딩을 사용할 것인지 여부
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;	//	논리 연산 블렌딩 활성화 여부 ==> 어떠한 방식으로 블렌딩 할지 등.
	//아래 설정들은 이제 2개 섞을 블렌딩을 어떤 비율 값으로 어떤 방식으로 설정할지 등을 설정해주는 부분.
	//색상 블렌딩과 알파 블렌딩 2개의 방식을 설정하고 있다.
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;	//
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;	//
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;	//
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;	//
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	//어떤 색상 채널을 쓸것인지 결정하는 부분. 
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//OM 단계에 연결되는 설정이며 OM 단계에는 최대 8개의 후면버퍼가 연결가능하므로 RenderTarget또한 8개가 존재할 것이다.

	//그래픽 파이프라인 상태 DESC를 설정
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = m_pd3dRootSignature.Get();
	d3dPipelineStateDesc.VS.pShaderBytecode = pd3dVertexShaderBlob->GetBufferPointer();	//VS 단계 연결
	d3dPipelineStateDesc.VS.BytecodeLength = pd3dVertexShaderBlob->GetBufferSize();
	d3dPipelineStateDesc.PS.pShaderBytecode = pd3dPixelShaderBlob->GetBufferPointer(); //PS 단계 연결
	d3dPipelineStateDesc.PS.BytecodeLength = pd3dPixelShaderBlob->GetBufferSize();
	d3dPipelineStateDesc.RasterizerState = d3dRasterizerDesc;	//RS 설정
	d3dPipelineStateDesc.BlendState = d3dBlendDesc;	//여기서 부터
	d3dPipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	d3dPipelineStateDesc.DepthStencilState.StencilEnable = FALSE;	//여기까지가 OM 단계에 필요한 설정
	d3dPipelineStateDesc.InputLayout.NumElements = 0;	//IA 단계에 필요한 InputLayout 설정 정점에 정보를 어떻게 연결할지
	d3dPipelineStateDesc.InputLayout.pInputElementDescs = NULL;	//설정하는게 IA 단계
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	//토폴로지 형태를 어떻게 지정할지 IA 연관되어있다.
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.SampleDesc.Quality = 0;
	pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, _uuidof(ID3D12PipelineState), (void**)m_pd3dPipelineState.GetAddressOf());

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob.Reset();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob.Reset();

}

void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews , int nUnorderedAccessViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews + nUnorderedAccessViews; //CBVs + SRVs + UAVs
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	
	HRESULT hr = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)m_pDescriptorHeap-> m_pd3dCbvSrvDescriptorHeap.GetAddressOf());

	if (FAILED(hr)) {
		_com_error err(hr);
		OutputDebugStringW(L"[CreateDescriptorHeap] FAILED: ");
		OutputDebugStringW(err.ErrorMessage());   // ← 여기서 사람이 읽을 수 있는 메시지 출력
		OutputDebugStringW(L"\n");
		return;
	}

	m_pDescriptorHeap->m_d3dCbvCPUDescriptorStartHandle = m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pDescriptorHeap->m_d3dCbvGPUDescriptorStartHandle = m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorStartHandle.ptr = m_pDescriptorHeap->m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorStartHandle.ptr = m_pDescriptorHeap->m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_pDescriptorHeap->m_d3dUavCPUDescriptorStartHandle.ptr = m_pDescriptorHeap->m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * (nConstantBufferViews +nShaderResourceViews));
	m_pDescriptorHeap->m_d3dUavGPUDescriptorStartHandle.ptr = m_pDescriptorHeap->m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * (nConstantBufferViews + nShaderResourceViews));

	m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dCbvCPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dCbvGPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dSrvCPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dSrvGPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dUavCPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dUavGPUDescriptorStartHandle;
}

void CScene::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	/*
	* 아 왜 이렇게 작성했지;;; CPU 핸들 값이 변하지 않는 이상한걸로 계속 하나만 채우고있었네;;
	*/
	/*D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCbvCPUDescriptorHandle;
		d3dCbvCPUDescriptorHandle.ptr = m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle);
	}*/

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVdesc = {};
	d3dCBVdesc.SizeInBytes = nStride;

	for (int j = 0; j < nConstantBufferViews; ++j)
	{
		d3dCBVdesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVdesc,
			m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle);

		m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dConstantBuffer, UINT nStride)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	d3dCBVDesc.BufferLocation = pd3dConstantBuffer->GetGPUVirtualAddress();
	pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle;
	m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr = m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr = m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;

	return(d3dCbvGPUDescriptorHandle);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferView(ID3D12Device* pd3dDevice, D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress, UINT nStride)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress;
	pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle;
	m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr = m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr = m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;

	return(d3dCbvGPUDescriptorHandle);
}

/*
* TextureResourceView를 DescriptorHeap에 생성과 함께 저장해주는 함수로
* Texture 정보를 읽어온 CTexture를 받아오며
* 디스크립터 힙의 몇 번째 부터 시작할껀지 지정 3 번쨰 파라미터 값이다
* 현재 루트 파라미터의 몇번째에 등록해서 사용할지가 4번째 파라미터에 있다.
*/
void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();
	for (int i = 0; i < nTextures; i++)
	{
		ComPtr<ID3D12Resource> pShaderResource = pTexture->GetResource(i);
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
		pd3dDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		pTexture->SetGpuDescriptorHandle(i, m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int i = 0; i < nRootParameters; i++)
	{
		pTexture->SetRootParameterIndex(i, nRootParameterStartIndex + i);
	}
}

void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, DXGI_FORMAT* pdxgiFormat,  UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();
	for (int i = 0; i < nTextures; i++)
	{
		ComPtr<ID3D12Resource> pShaderResource = pTexture->GetResource(i);
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
		d3dShaderResourceViewDesc.Format = pdxgiFormat[i];
		pd3dDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		pTexture->SetGpuDescriptorHandle(i, m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int i = 0; i < nRootParameters; i++)
	{
		pTexture->SetRootParameterIndex(i, nRootParameterStartIndex + i);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateStructureBufferSRV(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource,UINT nElementCount, UINT nStructureStride)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; //StructuredBuffer 필수
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = nElementCount;
	srvDesc.Buffer.StructureByteStride = nStructureStride;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle =
		m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle;

	pd3dDevice->CreateShaderResourceView(
		pd3dResource,
		&srvDesc,
		m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle
	);

	m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr +=
		::gnCbvSrvDescriptorIncrementSize;

	m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr +=
		::gnCbvSrvDescriptorIncrementSize;

	return gpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, DXGI_FORMAT dxgiSrvFormat)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	d3dShaderResourceViewDesc.Format = dxgiSrvFormat;
	d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
	d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
	d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle;
	pd3dDevice->CreateShaderResourceView(pd3dResource, &d3dShaderResourceViewDesc, m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle);
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

	return(d3dSrvGPUDescriptorHandle);
}

void CScene::CreateUnorderedAccessView(ID3D12Device* pd3dDevice, CTexture* pTexture, DXGI_FORMAT* pdxgiFormat , UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	// UAV Heap 시작 위치 이동
	m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle.ptr +=
		(::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle.ptr +=
		(::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();

	for (int i = 0; i < nTextures; i++)
	{
		ComPtr<ID3D12Resource> pUavResource = pTexture->GetResource(i);

		// UAV Desc (Texture2D 기준)
		D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc = {};
		d3dUnorderedAccessViewDesc.Format = pdxgiFormat[i];
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		d3dUnorderedAccessViewDesc.Texture2D.MipSlice = 0;
		d3dUnorderedAccessViewDesc.Texture2D.PlaneSlice = 0;

		pd3dDevice->CreateUnorderedAccessView(
			pUavResource.Get(),
			nullptr, // Texture UAV는 Counter 없음
			&d3dUnorderedAccessViewDesc,
			m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle
		);

		// GPU Handle 저장
		pTexture->SetUavGpuDescriptorHandle(i,
			m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle);

		// 다음 UAV 슬롯으로 이동
		m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle.ptr +=
			::gnCbvSrvDescriptorIncrementSize;

		m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle.ptr +=
			::gnCbvSrvDescriptorIncrementSize;
	}

	// Root Parameter 인덱스 설정 (u0부터)
	int nRootParameters = pTexture->GetRootParameters();
	for (int i = 0; i < nRootParameters; i++)
	{
		pTexture->SetComputeRootParameterIndex(i, nRootParameterStartIndex + i);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateStructureBufferUAV(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, UINT nElementCount, UINT nStructureStride)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;           // StructuredBuffer 필수
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = nElementCount;
	uavDesc.Buffer.StructureByteStride = nStructureStride;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle =
		m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle;

	pd3dDevice->CreateUnorderedAccessView(
		pd3dResource,
		nullptr,
		&uavDesc,
		m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle
	);

	m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle.ptr +=
		::gnCbvSrvDescriptorIncrementSize;

	m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle.ptr +=
		::gnCbvSrvDescriptorIncrementSize;

	return gpuHandle;
}




void CScene::CreateShaderResourceViewRtvs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nRenderTargets, DXGI_FORMAT* pdxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle)
{
	//Normal 값을 저장할 G-buffer 하나를 더 만든다
	//PostPorcessing 과 HDRI 결과를 저장할 RTV와  Normal 값을 저장할 G-buffer를 하나 더 만든다

	/*
	* TODO: 리팩토링할떄 저 nRenderTargets 값이랑 Compute를 위한것과 Graphics를 위한거 2개로 쪼개는 과정 필요
	*/

	//Texture를 선언 및 초기화
	m_pRenderTexture = new CTexture(nRenderTargets, RESOURCE_TEXTURE2D, 0, 1);
	m_pComputeRenderTexture = new CTexture( 4, RESOURCE_TEXTURE2D, 0, 1);

	//HDR 파이프라인으로 바꾸기 위해서
	D3D12_CLEAR_VALUE d3dClearValue = { DXGI_FORMAT_R16G16B16A16_FLOAT, { 0.0f, 0.0f, 0.0f, 1.0f } };
	//RenderTarget 갯수만큼 만들고
	for (UINT i = 0; i < nRenderTargets; i++)
	{
		d3dClearValue.Format = pdxgiFormats[i];
		m_pRenderTexture->CreateTexture(pd3dDevice, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, pdxgiFormats[i], D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS , D3D12_RESOURCE_STATE_COMMON, &d3dClearValue, RESOURCE_TEXTURE2D, i);
	}

	DXGI_FORMAT dxgiFormats[4] = { DXGI_FORMAT_R16G16B16A16_FLOAT  , DXGI_FORMAT_R16G16B16A16_FLOAT  ,
		DXGI_FORMAT_R16G16B16A16_FLOAT , DXGI_FORMAT_R16G16B16A16_FLOAT };

	for (UINT i = 0; i < 4; i++)
	{
		d3dClearValue.Format = dxgiFormats[i];
		m_pComputeRenderTexture->CreateTexture(pd3dDevice, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, dxgiFormats[i], D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, &d3dClearValue, RESOURCE_TEXTURE2D, i);
	}


	//다른 Texture리소스도 만든다.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	DXGI_FORMAT uavFormats[1] =
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR Color
	};

	DXGI_FORMAT uavFormats2[4] =
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR Color
		DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR Color
		DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR Color
		DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR Color
		
	};

	//UAV 만들어주고
	CreateUnorderedAccessView( //Render에서 결과를 담아올 RTV를 UAV로 만드는 경우
		pd3dDevice,
		m_pRenderTexture,   
		uavFormats,
		0,                  // UAV Heap index
		4                   // Compute RootParameter 시작 인덱스 (u0)
	);

	CreateUnorderedAccessView( //CS에서 사용할 목적으로 생성된 G-Buffer들과 블러결과를 swap 해줄 2개의 Texture UAV
		pd3dDevice,
		m_pComputeRenderTexture,
		uavFormats2,
		0,                  // UAV Heap index
		4                   // Compute RootParameter 시작 인덱스 (u0)
	);
	//SRV도 만들고
	CScene::CreateShaderResourceViews(pd3dDevice, m_pRenderTexture, 0, 19);
	CScene::CreateShaderResourceViews(pd3dDevice, m_pComputeRenderTexture, 0, 3); //Compute에 연결할꺼니까

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;


	m_pd3dRtvCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[nRenderTargets];
	m_pd3dG_bufferRtvCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[2];
	//RTV 만들고
	for (UINT i = 0; i < nRenderTargets; i++)
	{
		//PostProcessing에 전반적인 기본 Texture
		d3dRenderTargetViewDesc.Format = pdxgiFormats[i];
		ComPtr<ID3D12Resource> pd3dTextureResource = m_pRenderTexture->GetResource(i);
		pd3dDevice->CreateRenderTargetView(pd3dTextureResource.Get(), &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dRtvCPUDescriptorHandles[i] = d3dRtvCPUDescriptorHandle; //RTV랑 DSV는 CPU Address가 필요하다
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}

	for (UINT i = 0; i < 2; i++)
	{
		//PostProcessing에 전반적인 기본 Texture
		d3dRenderTargetViewDesc.Format = dxgiFormats[i];

		//SSAO에 사용할 G_Buffer를 저장할 RenderTexture
		ComPtr<ID3D12Resource> pd3dTextureResource = m_pComputeRenderTexture->GetResource(i);
		pd3dDevice->CreateRenderTargetView(pd3dTextureResource.Get(), &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dG_bufferRtvCPUDescriptorHandles[i] = d3dRtvCPUDescriptorHandle; //RTV랑 DSV는 CPU Address가 필요하다
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CScene::CreateShaderResourceViewDsvs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nDepthTextures, D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle, ID3D12Resource* DefaultDepthStncilBuffer)
{
	//Texture를 선언 및 초기화
	m_pShadowDepthTexture = new CTexture(nDepthTextures, RESOURCE_TEXTURE2D, 0, 1);

	D3D12_CLEAR_VALUE d3dClearValue = {};
	d3dClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;
	
	DXGI_FORMAT dxgiFormatResource = DXGI_FORMAT_R32_TYPELESS; // Resource를 위한것
	DXGI_FORMAT* pdxgiFormatSRV = new DXGI_FORMAT[nDepthTextures]; // SRV를 위한것

	//nDepthTextures 갯수만큼 만들고
	for (UINT i = 0; i < nDepthTextures; i++)
	{
		pdxgiFormatSRV[i] = DXGI_FORMAT_R32_FLOAT;
		if (i == 0)
		{
			m_pShadowDepthTexture->CreateTexture(pd3dDevice, _DEPTH_BUFFER_WIDTH, _DEPTH_BUFFER_HEIGHT, dxgiFormatResource, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, &d3dClearValue, RESOURCE_TEXTURE2D, i);
		}
		else
		{
			m_pShadowDepthTexture->CreateTexture(pd3dDevice, _SPOT_DEPTH_BUFFER_WIDTH, _SPOT_DEPTH_BUFFER_HEIGHT, dxgiFormatResource, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, &d3dClearValue, RESOURCE_TEXTURE2D, i);
		}
	}

	//다른 Texture리소스도 만든다.
	//CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//SRV도 만들고
	CScene::CreateShaderResourceViews(pd3dDevice, m_pShadowDepthTexture, pdxgiFormatSRV,  0, 20);

	//기본으로 쓰이는 깊이맵
	DXGI_FORMAT dxgiFormatSRV = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	m_pd3dDefaultDepthMapSrvDescrptorHandles = CScene::CreateShaderResourceView(pd3dDevice, DefaultDepthStncilBuffer,  dxgiFormatSRV);
	m_pDefaultDepthStancilResource = DefaultDepthStncilBuffer;

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	d3dDepthStencilViewDesc.Texture2D.MipSlice = 0;

	m_pd3dSahdowMapDsvCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[nDepthTextures];

	//DSV 만들고

	ComPtr<ID3D12Resource> pd3dTextureResource;

	for (UINT i = 0; i < nDepthTextures; i++)
	{
		pd3dTextureResource = m_pShadowDepthTexture->GetResource(i);
		pd3dDevice->CreateDepthStencilView(pd3dTextureResource.Get(), &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
		m_pd3dSahdowMapDsvCPUDescriptorHandles[i] = d3dDsvCPUDescriptorHandle; //RTV랑 DSV는 CPU Address가 필요하다
		d3dDsvCPUDescriptorHandle.ptr += ::gnDsvDescriptorIncrementSize;
	}
	if (pdxgiFormatSRV)
	{
		delete[] pdxgiFormatSRV;
	}
}

void CScene::CreateShaderResourceViewAndUAVStructuredBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	//
	UINT ncbElementBytes = ((sizeof(ClusterCB) + 255) & ~255); //256의 배수
	m_cpcbCluster = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpcbCluster->Map(0, NULL, (void**)&m_pcbCluster);

	ncbElementBytes = ((sizeof(LightingData) + 255) & ~255); //256의 배수
	m_cpcbLightData = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpcbLightData->Map(0, NULL, (void**)&m_pcbLightData);

	//UINT clusterCount =
	//	m_ClusterDim.x * m_ClusterDim.y * m_ClusterDim.z;
	UINT clusterCount =
		m_ClusterDim.x * m_ClusterDim .y* m_ClusterDim.z;


	UINT bufferSize =
		sizeof(ClusterLightList) * clusterCount;

	m_cpsbClusterLightList =
		::CreateBufferResource(
			pd3dDevice,
			pd3dCommandList,
			nullptr,
			bufferSize,
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			nullptr
		);

	// ClusterLightList UAV (CS에서 쓰기)
	m_hClusterLightListUav =
		CreateStructureBufferUAV(
			pd3dDevice,
			m_cpsbClusterLightList.Get(),
			clusterCount,
			sizeof(ClusterLightList)
		);

	// ClusterLightList SRV (PS에서 읽기)
	m_hClusterLightListSrv =
		CreateStructureBufferSRV(
			pd3dDevice,
			m_cpsbClusterLightList.Get(),
			clusterCount,
			sizeof(ClusterLightList)
		);
	

}


void CScene::BuildObjects(ID3D12Device* pd3dDevice)
{
	//CreateGraphicsRootSignature(pd3dDevice);	//루트 시그니쳐와 PSO를 여기서 초기화해주고 있다. 
	//CreateGraphicsPipelineState(pd3dDevice);	//이건 구지 BuildObjects에 넣어줄 필요까진 없고 설계를 이렇게 했을 뿐
	//초기화 함수를 따로 만들든해서 제어 가능하다.

	//다만 GameFramework에서 BuildObjects를 초기화 개념을 적용해서 사용하는 구조로 로직이 짜여있어서 여기다가 넣는게
	//편리하게 보인다.

}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dRootSignature = CGraphicsCommon::GetInstance().m_cpd3dMainSceneRootSignature;

	m_cpd3dDevice = pd3dDevice;
	m_cpd3dCommandList = pd3dCommandList;
}

void CScene::InitLights()
{
	
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CScene::ReleaseShaderVariables()
{
	if (m_cpd3dcbLights)
	{
		m_cpd3dcbLights->Unmap(0, NULL);
		m_cpd3dcbLights->Release();
	}

	if (m_cpd3dcbImGui_Info)
	{
		m_cpd3dcbImGui_Info->Unmap(0, NULL);
		m_cpd3dcbImGui_Info->Release();
	}
}

void CScene::ReleaseObjects()
{
	if (m_pd3dRootSignature) m_pd3dRootSignature.Reset();	//ComPtr을 썻기에 구지 필요하진 않은데

	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].ReleaseShaderVariables();
		m_pShaders[i].ReleaseObjects();
	}
	if (m_pShaders)
	{
		delete[] m_pShaders;
	}
	if (m_pd3dPipelineState) m_pd3dPipelineState.Reset();	//항상 Release로 메모리 관리가 필요하단걸 잊지말자.


}

void CScene::ReleaseUploadBuffers()
{

	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j])
		{
			m_ppObjects[j]->ReleaseUploadBuffers();
		}
	}

	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].ReleaseUploadBuffers();
	}
	//if (m_ppShader)
	//{
	//	for (int j = 0; j < m_nShaders; j++) if (m_ppShader[j])
	//		m_ppShader[j]->ReleaseUploadBuffers();
	//}
}

ID3D12RootSignature* CScene::GetGraphicsRootSignature()
{
	return(m_pd3dRootSignature.Get());
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

void CScene::PreviousTransform()
{

}

bool CScene::ProcessInput()
{
	return false;
}

//오브젝트의 애니메이션
void CScene::AnimateObjects(float fTimeElapsed)
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++)
		{
			m_ppObjects[j]->Animate(fTimeElapsed);
		}
	}

	if (m_pShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_pShaders[i].AnimateObjects(fTimeElapsed);
		}
	}

}

//Render 하기전 우선 이건 안쓰고 CShader에서 설정하고 있다
void CScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//루트 시그니쳐 연결
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get());

	//PSO 연결 ==> PSO에 루트 시그니쳐를 연결했으며 서로 다른 게 연결되어 있다면 런타임 
	pd3dCommandList->SetPipelineState(m_pd3dPipelineState.Get());

	//IA의 토폴로지를 트라이앵글리스트로 설정
	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

//본 Render를 시작하기 전에 사전작업
void CScene::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, CCamera* pCamera)
{
	//pCamera->SetViewportsAndScissorRects(pd3dCommandList); //뷰포트와 시저렉트를 실제 GPU 명령어로 Set해주는 거

	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get()); //루트 시그니쳐를 GPU에게 넘기는거
	pd3dCommandList->SetComputeRootSignature(m_cpd3dComputeRootSignature.Get());
	//if (pCamera)
	//{
	//	for (int i = 0; i < MAX_LIGHTS; i++)
	//	{
	//		//TODO 여기다가 조명 Shader 처리를 한번 에 쫙 해야한다 
	//	}
	//}
}

//Scene내에 오브젝트들을 그리는 동작 ==> Camara가 하나이고 움직이지 않는다는 전제하에서
void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//PrepareRender(pd3dCommandList);

	//pd3dCommandList->DrawInstanced(6,1,0,0);	//Draw 명령 여긴 임의로 그냥 그리는거라고 보면된다.

	 //그래픽 루트 시그너쳐를 파이프라인에 연결(설정)한다.
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get());

	//씬을 렌더링하는 것은 씬을 구성하는 셰이더(셰이더가 포함하는 객체)들을 렌더링하는 것이다.
	//for (int i = 0; i < m_nShaders; i++)
	//{
	//	m_ppShader[i]->Render(pd3dCommandList);
	//}
}

void CScene::OnlyShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
}

void CScene::PrePareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CScene::OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles
	, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, CCamera* pCamera)
{

}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_nWeather == 0)
	{
		if (m_pIBLBRDFTexture) //환경맵이 존재를 한다면
		{
			m_pIBLBRDFTexture->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pIBLSpecluarTexture) //환경맵이 존재를 한다면
		{
			m_pIBLSpecluarTexture->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pIBLIrradianceTexture) //환경맵이 존재를 한다면
		{
			m_pIBLIrradianceTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}
	else
	{
		if (m_pIBLBRDFTextureCloud) //환경맵이 존재를 한다면
		{
			m_pIBLBRDFTextureCloud->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pIBLSpecluarTextureCloud) //환경맵이 존재를 한다면
		{
			m_pIBLSpecluarTextureCloud->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pIBLIrradianceTextureCloud) //환경맵이 존재를 한다면
		{
			m_pIBLIrradianceTextureCloud->UpdateShaderVariables(pd3dCommandList);
		}
	}
}


void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	//pCamera->SetViewportsAndScissorRects(pd3dCommandList); //뷰포트와 시저렉트를 실제 GPU 명령어로 Set해주는 거

	//pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get()); //루트 시그니쳐를 GPU에게 넘기는거
	
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	
	//ImGui에 설정된 플래그를 전달하는 역할 중
	UpdateShaderVariables(pd3dCommandList);

	if (pCamera)
	{
		pCamera->UpdateShaderVariables(pd3dCommandList); //카메라의 상수 버퍼나 여러 GPU나 Shader에 넘겨야할 명령어등을 Update해주는 부분
	}



	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_pShaders)
		{
			m_pShaders[i].Render(pd3dCommandList, pCamera);
		}
	}
}

void CScene::OnGUI(ImGuiIO* pGuiIO)
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();

	ImGui::SetNextWindowSize(ImVec2(320, 200), ImGuiCond_Always);
	ImGui::SetNextWindowPos(
		center,
		ImGuiCond_Always,
		ImVec2(0.5f, 0.5f)
	);

	ImGui::Begin("Login", nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoMove);

	ImGui::Text("Please log in");
	ImGui::Separator();

	ImGui::InputInt("ID", &m_szLoginID);
		ImGui::Spacing();

	// 로그인 요청 중이면 버튼 비활성화
	if (m_bLoginRequesting)
	{
		ImGui::BeginDisabled();
		ImGui::EndDisabled();
	}

	if (ImGui::Button("Login", ImVec2(120, 0)))
	{
		if (m_szLoginID == 0)
		{
			m_bLoginFailed = true;
		}
		else
		{
			Protocol::C_LOGIN pkt;
			pkt.set_accountid(m_szLoginID);
			cout << "Login ID: " << m_szLoginID << endl;
			auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
			m_upClientNetwork->SendToServer(sendBuffer);
			m_bLoginFailed = false;
			m_bLoginRequesting = true;

			
		}
	}
	

	ImGui::SameLine();

	if (ImGui::Button("Exit", ImVec2(120, 0)))
	{
		// PostQuitMessage(0);
	}
	ImGui::End();
}

//파일로 부터 몇개의 Texture를 사용하는지 읽어오는 함수
int CScene::CountTexturesFromFiles(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const std::vector<const char*>& fileList)
{
	int total = 0;
	for (auto file : fileList)
	{
		if (file && strlen(file) > 0)
			total += CGameObject::LoadCountTexturesFromFile(pd3dDevice, pd3dCommandList,pd3dGraphicsRootSignature, (char*)file, NULL);
	}
	return total;
}

XMVECTOR CScene::ClosetPointOnOBB(BoundingOrientedBox& obb, XMVECTOR point)
{
	
	// OBB 정보
	XMVECTOR vCenter = XMLoadFloat3(&obb.Center);
	XMVECTOR vExtents = XMLoadFloat3(&obb.Extents);
	XMVECTOR vOrientation = XMLoadFloat4(&obb.Orientation);

	// point를 OBB 로컬 공간으로 변환
	XMVECTOR vLocal = XMVectorSubtract(point, vCenter);

	// 회전 역변환 (world -> obb local)
	XMVECTOR vInvRot = XMQuaternionInverse(vOrientation);
	vLocal = XMVector3Rotate(vLocal, vInvRot);

	// 로컬 AABB에서 clamp
	XMVECTOR vClamped = XMVectorClamp(
		vLocal,
		XMVectorNegate(vExtents),
		vExtents
	);

	// 다시 월드 공간으로
	XMVECTOR vWorld = XMVector3Rotate(vClamped, vOrientation);
	vWorld = XMVectorAdd(vWorld, vCenter);

	return vWorld;
}

void CScene::CreateSSAOKernel()
{
}




LightControl::LightControl()
{
	// Light 기본 초기화
	m_Lights.radiance = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Lights.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_Lights.position = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_Lights.fallOffStart = 0.0f;
	m_Lights.fallOffEnd = 0.0f;
	m_Lights.spotPower = 1.0f;

	m_Lights.radius = 0;
	m_Lights.bIsShadowSet = 0;
	m_Lights.bIsUse = 0;
	m_Lights.type = 0; // NONE or INVALID

	m_Lights.viewProj = Matrix4x4::Identity();
	m_Lights.invProj = Matrix4x4::Identity();

	// Spot Light BoundingSphere 초기화
	spotSphere.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	spotSphere.Radius = 0.0f;
}

LightControl::~LightControl()
{
}

void LightControl::SetLight(UINT nType, XMFLOAT3 radiance, XMFLOAT3 direction, XMFLOAT3 position, UINT nbIsUse)
{
	m_Lights.type = nType;
	m_Lights.bIsUse = nbIsUse;

	m_Lights.radiance = radiance;
	m_Lights.direction = direction;
	m_Lights.position = position;

	m_Lights.bIsShadowSet = 1;

	m_Lights.radius = 0;
}

void LightControl::SetLight(UINT nType, XMFLOAT3 radiance, XMFLOAT3 direction, XMFLOAT3 position, float fallOffStart, float fallOffEnd, float spotPower, UINT nbIsUse, UINT nbIsShadowUse)
{
	m_Lights.type = nType;
	m_Lights.bIsUse = nbIsUse;
	m_Lights.bIsShadowSet = nbIsShadowUse;

	m_Lights.radiance = radiance;
	m_Lights.direction = direction;
	m_Lights.position = position;

	m_Lights.fallOffStart = fallOffStart;
	m_Lights.fallOffEnd = fallOffEnd;
	m_Lights.spotPower = spotPower;

	m_Lights.radius = 0;

	// Spot Light용 BoundingSphere
	spotSphere.Center = position;
	spotSphere.Radius = fallOffEnd;
}

void LightControl::SetPosition(XMFLOAT3 xmf3Position)
{
	m_Lights.position = xmf3Position;
	spotSphere.Center = xmf3Position;
}

void LightControl::SetDirection(XMFLOAT3 xmf3Direction)
{
	m_Lights.direction = xmf3Direction;
}

void LightControl::SetShadowUse(UINT nbIsUse)
{
	m_Lights.bIsShadowSet = nbIsUse;
}

Light LightControl::GetLight()
{
	return m_Lights;
}

BoundingSphere LightControl::GetBoundingSphere()
{
	return spotSphere;
}


void CScene::SendObjectMoveToServer(CGameObject* pObj, Protocol::PlayerState state)
{

	//std::cout << "[SendObjectMoveToServer] called" << std::endl;
	if (pObj == nullptr)
	{
		//std::cout << "  -> pObj == nullptr, return" << std::endl;
		return;
	}

	if (m_upClientNetwork == nullptr)
	{
		//std::cout << "  -> net == nullptr, return" << std::endl;
		return;
	}

	Protocol::C_MOVE pkt;
	Protocol::Transform* trans = pkt.mutable_player()->mutable_transform();
	pObj->FillProtoTransform(trans);
	pkt.mutable_player()->set_state(state);

	//std::cout << "STate" << pkt.mutable_player()->state() << std::endl;
	// Fix for E1086: Ensure the correct usage of mutable_transform()  

	/*std::cout << "  -> C_MOVE pos=("
		<< trans->px() << ", "
		<< trans->py() << ", "
		<< trans->pz() << ")\n";*/

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	m_upClientNetwork->SendToServer(sendBuffer);

	//std::cout << "  -> net->Broadcast() called\n";
}