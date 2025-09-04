#include "stdafx.h"
#include "Scene.h"


CScene::CScene()
{

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

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	/*
	* 루트 파라미터 한 번에 64개의 슬롯을 설정 가능
	* 상수, 디스크립터, 디스크립터 테이블 3개를 연결할 수 있다.
	* 용도에 따라서 3가지 중 선택해서 넘겨야 한다.
	*/
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[2];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS; //상수를 그대로 연결하는 경우
	pd3dRootParameters[0].Constants.Num32BitValues = 16; // 16바이트의 상수버퍼의 상수 값을 넘긴다는 의미
	pd3dRootParameters[0].Constants.ShaderRegister = 0; //Shader에 레지스트 번호 0번 사용
	pd3dRootParameters[0].Constants.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VS에서 사용 가능하다는 의미
	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 32; // 32바이트
	pd3dRootParameters[1].Constants.ShaderRegister = 1; //0번을 0번 슬롯에서 쓰니까 1번으로 지정
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	//플래그를 이렇게 변수처럼 따로 만들어 DESC에 연결 가능
	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, //구조체를 GPU가 읽을 수 있는 바이너리 형식으로 직렬화 시켜주고 있다.
		&pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), //직렬화된 바이트 코드를 읽어와 실제로 Create 해준다
		pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void
			**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release(); // 더 이상 필요 없으니까 Release 해준다
	if (pd3dErrorBlob) pd3dErrorBlob->Release();
	return(pd3dGraphicsRootSignature);


	//매개변수가 없는 루트 시그너쳐를 생성한다.
	/*D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = 0;
	d3dRootSignatureDesc.pParameters = NULL;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void
			**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();
	return(pd3dGraphicsRootSignature);*/
}


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

	D3DCompileFromFile(L"Shaders.hlsl", NULL, NULL, "VSMain", "vs_5_1", nCompileFlags, 0,
		pd3dVertexShaderBlob.GetAddressOf(), NULL);

	D3DCompileFromFile(L"Shaders.hlsl", NULL, NULL, "PSMain", "ps_5_1", nCompileFlags, 0,
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
	//2025.07.08에 추가한 부분 Shader.h에 그래픽 파이프라인 설정을 넘기고 나서 Scene에 BuildObject 설정을 바꾸면서 나오는 내용
	/*m_pd3dRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	m_nShaders = 1;
	m_ppShader = new CShader * [m_nShaders];

	CShader* pShader = new CShader();
	pShader->CreateShader(pd3dDevice, m_pd3dRootSignature.Get());
	pShader->BuildObjects(pd3dDevice, pd3dCommandList, NULL);

	m_ppShader[0] = pShader;*/

	//2025.07.10에 추가한 부분 게임 오브젝트를 Scene이 갖는 구조로 변경

	/* 2025.07.17이 전에 추가했던 부분 이건 Shader가 Object를 갖는 구조가 아닌 Object가 Shader를 갖는 구조
	m_pd3dRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	//CTriangleMesh* pMesh = new CTriangleMesh(pd3dDevice, pd3dCommandList);
	CCubeMeshDiffused* pMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList);
	m_nObjects = 1;
	m_ppObjects = new CGameObject * [m_nObjects];

	CRotatingObject* pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pMesh);

	CDiffusedShader* pShader = new CDiffusedShader();
	pShader->CreateShader(pd3dDevice, m_pd3dRootSignature.Get());
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pRotatingObject->SetShader(pShader);
	m_ppObjects[0] = pRotatingObject;
	*/

	m_pd3dRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	m_nShaders = 1;
	m_pShaders = new CObjectsShader[m_nShaders];
	m_pShaders[0].CreateShader(pd3dDevice, m_pd3dRootSignature.Get());
	m_pShaders[0].BuildObjects(pd3dDevice, pd3dCommandList);

	//CCubeMeshDiffused* pMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList,12.0f,12.0f,12.0f);

	//CRotatingObject* pRotatingObject = new CRotatingObject();
	//pRotatingObject->SetMesh(pMesh);

	//CDiffusedShader* pShader = new CDiffusedShader();
	//pShader->CreateShader(pd3dDevice, m_pd3dRootSignature.Get());
	//pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//pRotatingObject->SetShader(pShader);

	//m_pTestObject = pRotatingObject;
	//m_pTestObject->SetPosition(0,0,30);

}

void CScene::ReleaseObjects()
{
	if (m_pd3dRootSignature) m_pd3dRootSignature.Reset();	//ComPtr을 썻기에 구지 필요하진 않은데

	//if (m_ppShader)
	//{
	//	for (int i = 0; i < m_nShaders; i++)
	//	{
	//		m_ppShader[i]->ReleaseShaderVariables();
	//		m_ppShader[i]->ReleaseObjects();
	//		m_ppShader[i]->Release();
	//	}
	//	delete[] m_ppShader;
	//}

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
	//
	//for (int i = 0; i < m_nShaders; i++)
	//{
	//	m_ppShader[i]->AnimateObjects(fTimeElapsed);
	//}
}

//Render 하기전 C
void CScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//루트 시그니쳐 연결
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get());

	//PSO 연결 ==> PSO에 루트 시그니쳐를 연결했으며 서로 다른 게 연결되어 있다면 런타임 
	pd3dCommandList->SetPipelineState(m_pd3dPipelineState.Get());

	//IA의 토폴로지를 트라이앵글리스트로 설정
	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList); //뷰포트와 시저렉트를 실제 GPU 명령어로 Set해주는 거

	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get()); //루트 시그니쳐를 GPU에게 넘기는거

	if (pCamera)
	{
		pCamera->UpdateShaderVariables(pd3dCommandList); //카메라의 상수 버퍼나 여러 GPU나 Shader에 넘겨야할 명령어등을 Update해주는 부분
	}
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].Render(pd3dCommandList, pCamera);
	}

	//m_pTestObject->Render(pd3dCommandList, pCamera);

	/*
	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
	*/

}