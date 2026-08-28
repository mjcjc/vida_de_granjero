#include "stdafx.h"
#include "Scene.h"
#include"Protocol.pb.h"
#include"ServerPacketHandler.h"
#include "Player.h"
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

//��Ʈ �ñ״��ĸ� ���� ==> �ʱ⿡ �����ϰ� ��Ʈ �ñ״��ĸ� ����� �����ϴ� ������ ����� ����
//void CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
//{
//	//��Ʈ �ñ״��ĸ� ���� 
//	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
//	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
//	d3dRootSignatureDesc.NumParameters = 0;	//��Ʈ �Ķ���� ���� ����
//	d3dRootSignatureDesc.NumStaticSamplers = 0;	//���� ���÷� ���� ����
//	d3dRootSignatureDesc.pParameters = NULL;	//������ ��Ʈ �Ķ������ ������
//	d3dRootSignatureDesc.pStaticSamplers = NULL;	//������ ���� ���÷��� ������
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
//	//��Ʈ �ñ״��Ķ�?
//	/*
//	* ���� ������������ GPU �ӿ��� ��� �۵����� ���� �����ϴ� �ڵ��� Shader �ڵ��� �ʿ���
//	* �۷ι� �������� ���� �� ������ ���ǳ����� ��.
//	* GPU�� �츮 �������� �ܼ��ϰ� ������ ��ٷӱ⿡ ��Ʈ �ñ״��Ŀ��� 
//	* BYTE ũ�� �� ���ó ���� ���� ��ٷӰ� ���ǳ����־�� �Ѵ�.
//	*/
//}

//ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
//{
//	/*
//	* ��Ʈ �Ķ���� �� ���� 64���� ������ ���� ����
//	* ���, ��ũ����, ��ũ���� ���̺� 3���� ������ �� �ִ�.
//	* �뵵�� ���� 3���� �� �����ؼ� �Ѱܾ� �Ѵ�.
//	*/
//	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;
//
//
//	D3D12_ROOT_PARAMETER pd3dRootParameters[2];
//	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS; //����� �״�� �����ϴ� ���
//	pd3dRootParameters[0].Constants.Num32BitValues = 16; // 16����Ʈ�� ��������� ��� ���� �ѱ�ٴ� �ǹ�
//	pd3dRootParameters[0].Constants.ShaderRegister = 0; //Shader�� ������Ʈ ��ȣ 0�� ���
//	pd3dRootParameters[0].Constants.RegisterSpace = 0;
//	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VS���� ��� �����ϴٴ� �ǹ�
//	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
//	pd3dRootParameters[1].Constants.Num32BitValues = 32; // 32����Ʈ
//	pd3dRootParameters[1].Constants.ShaderRegister = 1; //0���� 0�� ���Կ��� ���ϱ� 1������ ����
//	pd3dRootParameters[1].Constants.RegisterSpace = 0;
//	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
//
//	//�÷��׸� �̷��� ����ó�� ���� ����� DESC�� ���� ����
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
//	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, //����ü�� GPU�� ���� �� �ִ� ���̳ʸ� �������� ����ȭ �����ְ� �ִ�.
//		&pd3dSignatureBlob, &pd3dErrorBlob);
//	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), //����ȭ�� ����Ʈ �ڵ带 �о�� ������ Create ���ش�
//		pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void
//			**)&pd3dGraphicsRootSignature);
//	if (pd3dSignatureBlob) pd3dSignatureBlob->Release(); // �� �̻� �ʿ� �����ϱ� Release ���ش�
//	if (pd3dErrorBlob) pd3dErrorBlob->Release();
//	return(pd3dGraphicsRootSignature);
//
//}


//�׷��� ���������� ���� PSO�� ���� ���⼭ Shader�� �����Ѵ�.
// �� PSO�� � �������̳Ŀ� ���� ���� �޶����ϱ� ���߿� ������ PSO�� �� ���� �����ϴ�
//�׷� Ŭ������ ���� ����ų� ���� �������ٰ� ���� ����.
void CScene::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice)
{
	//���� ���̴��� �ȼ� ���̴��� �����Ѵ�.
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
	//Shader�� ������ ���ִ� �Լ�. Shaders.hlsl �̶�� �̸��� ���̴� ���Ͽ� ���� �Լ� �̸� VSMain�� PSMain 2���� 
	//Shader �ܰ踦 �����ϰ� ������ ������ �������ְ� �ִ�.

	//�����Ͷ����� ���¸� ����
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;	//��� ĥ������ ���� RS �ܰ迡�� �����ϴ� ������ 
	//���� ���̿� ���� ������ ���ؼ� ������ PS�� �ѱ� ������ ��ǥ���� �����ϱ� �� �ܰ迡�� �����ϴ� �Ŵ�.
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK; //���� ó��
	d3dRasterizerDesc.FrontCounterClockwise = FALSE; //���� ������ �ð� �������� ó���Ѵ� TRUE�̸� �ݽð��̴�
	d3dRasterizerDesc.DepthBias = 0;	// z-fighting ������ �ذ��ϱ� ���� ���� ���̾ ����
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;	//���� ���̾ ������ ���� ���� ����
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;	//
	d3dRasterizerDesc.DepthClipEnable = TRUE;	//NDC�� ��ȯ�� ���� Ŭ������ ���� ���� ����
	d3dRasterizerDesc.MultisampleEnable = FALSE;	//��Ƽ���ø� ��������
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;	//��Ƽ�ٸ���� ���� ����
	d3dRasterizerDesc.ForcedSampleCount = 0;	//��Ƽ���ø��� ������ �����Ҷ� ����ϴ� �ɼ� 0�̸� �⺻ �������� ���ø� �����̴�.
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	//������ȭ �ܰ��� �ȼ� ��� ������ ���Ҷ� ����ϴ� ��Ŀ� ���̸� �����ϴ� �ɼ�


	//������ ���¸� ���� ==> ������ �ɼ��� �����ϴ� �κ�
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;	//������Ŀ�������� Ȱ��ȭ����
	d3dBlendDesc.IndependentBlendEnable = FALSE;	// ���� ���� Ÿ�� ���� ������ Ȱ��ȭ ����
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;	// �������� ����� ������ ����
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;	//	���� ���� ������ Ȱ��ȭ ���� ==> ��� ������� ������ ���� ��.
	//�Ʒ� �������� ���� 2�� ���� �������� � ���� ������ � ������� �������� ���� �������ִ� �κ�.
	//���� �������� ���� ������ 2���� ����� �����ϰ� �ִ�.
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;	//
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;	//
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;	//
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;	//
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	//� ���� ä���� �������� �����ϴ� �κ�. 
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//OM �ܰ迡 ����Ǵ� �����̸� OM �ܰ迡�� �ִ� 8���� �ĸ���۰� ���ᰡ���ϹǷ� RenderTarget���� 8���� ������ ���̴�.

	//�׷��� ���������� ���� DESC�� ����
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = m_pd3dRootSignature.Get();
	d3dPipelineStateDesc.VS.pShaderBytecode = pd3dVertexShaderBlob->GetBufferPointer();	//VS �ܰ� ����
	d3dPipelineStateDesc.VS.BytecodeLength = pd3dVertexShaderBlob->GetBufferSize();
	d3dPipelineStateDesc.PS.pShaderBytecode = pd3dPixelShaderBlob->GetBufferPointer(); //PS �ܰ� ����
	d3dPipelineStateDesc.PS.BytecodeLength = pd3dPixelShaderBlob->GetBufferSize();
	d3dPipelineStateDesc.RasterizerState = d3dRasterizerDesc;	//RS ����
	d3dPipelineStateDesc.BlendState = d3dBlendDesc;	//���⼭ ����
	d3dPipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	d3dPipelineStateDesc.DepthStencilState.StencilEnable = FALSE;	//��������� OM �ܰ迡 �ʿ��� ����
	d3dPipelineStateDesc.InputLayout.NumElements = 0;	//IA �ܰ迡 �ʿ��� InputLayout ���� ������ ������ ��� ��������
	d3dPipelineStateDesc.InputLayout.pInputElementDescs = NULL;	//�����ϴ°� IA �ܰ�
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	//�������� ���¸� ��� �������� IA �����Ǿ��ִ�.
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
		OutputDebugStringW(err.ErrorMessage());   // �� ���⼭ ����� ���� �� �ִ� �޽��� ���
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
	* �� �� �̷��� �ۼ�����;;; CPU �ڵ� ���� ������ �ʴ� �̻��Ѱɷ� ��� �ϳ��� ä����־���;;
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
* TextureResourceView�� DescriptorHeap�� ������ �Բ� �������ִ� �Լ���
* Texture ������ �о�� CTexture�� �޾ƿ���
* ��ũ���� ���� �� ��° ���� �����Ҳ��� ���� 3 ���� �Ķ���� ���̴�
* ���� ��Ʈ �Ķ������ ���°�� ����ؼ� ��������� 4��° �Ķ���Ϳ� �ִ�.
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
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; //StructuredBuffer �ʼ�
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
	// UAV Heap ���� ��ġ �̵�
	m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle.ptr +=
		(::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle.ptr +=
		(::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();

	for (int i = 0; i < nTextures; i++)
	{
		ComPtr<ID3D12Resource> pUavResource = pTexture->GetResource(i);

		// UAV Desc (Texture2D ����)
		D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc = {};
		d3dUnorderedAccessViewDesc.Format = pdxgiFormat[i];
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		d3dUnorderedAccessViewDesc.Texture2D.MipSlice = 0;
		d3dUnorderedAccessViewDesc.Texture2D.PlaneSlice = 0;

		pd3dDevice->CreateUnorderedAccessView(
			pUavResource.Get(),
			nullptr, // Texture UAV�� Counter ����
			&d3dUnorderedAccessViewDesc,
			m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle
		);

		// GPU Handle ����
		pTexture->SetUavGpuDescriptorHandle(i,
			m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle);

		// ���� UAV �������� �̵�
		m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle.ptr +=
			::gnCbvSrvDescriptorIncrementSize;

		m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle.ptr +=
			::gnCbvSrvDescriptorIncrementSize;
	}

	// Root Parameter �ε��� ���� (u0����)
	int nRootParameters = pTexture->GetRootParameters();
	for (int i = 0; i < nRootParameters; i++)
	{
		pTexture->SetComputeRootParameterIndex(i, nRootParameterStartIndex + i);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateStructureBufferUAV(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, UINT nElementCount, UINT nStructureStride)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;           // StructuredBuffer �ʼ�
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
	//Normal ���� ������ G-buffer �ϳ��� �� �����
	//PostPorcessing �� HDRI ����� ������ RTV��  Normal ���� ������ G-buffer�� �ϳ� �� �����

	/*
	* TODO: �����丵�ҋ� �� nRenderTargets ���̶� Compute�� ���ѰͰ� Graphics�� ���Ѱ� 2���� �ɰ��� ���� �ʿ�
	*/

	//Texture�� ���� �� �ʱ�ȭ
	m_pRenderTexture = new CTexture(nRenderTargets, RESOURCE_TEXTURE2D, 0, 1);
	m_pComputeRenderTexture = new CTexture( 4, RESOURCE_TEXTURE2D, 0, 1);

	//HDR �������������� �ٲٱ� ���ؼ�
	D3D12_CLEAR_VALUE d3dClearValue = { DXGI_FORMAT_R16G16B16A16_FLOAT, { 0.0f, 0.0f, 0.0f, 1.0f } };
	//RenderTarget ������ŭ �����
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


	//�ٸ� Texture���ҽ��� �����.
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

	//UAV ������ְ�
	CreateUnorderedAccessView( //Render���� ����� ��ƿ� RTV�� UAV�� ����� ���
		pd3dDevice,
		m_pRenderTexture,   
		uavFormats,
		0,                  // UAV Heap index
		4                   // Compute RootParameter ���� �ε��� (u0)
	);

	CreateUnorderedAccessView( //CS���� ����� �������� ������ G-Buffer��� ��������� swap ���� 2���� Texture UAV
		pd3dDevice,
		m_pComputeRenderTexture,
		uavFormats2,
		0,                  // UAV Heap index
		4                   // Compute RootParameter ���� �ε��� (u0)
	);
	//SRV�� �����
	CScene::CreateShaderResourceViews(pd3dDevice, m_pRenderTexture, 0, 19);
	CScene::CreateShaderResourceViews(pd3dDevice, m_pComputeRenderTexture, 0, 3); //Compute�� �����Ҳ��ϱ�

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;


	m_pd3dRtvCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[nRenderTargets];
	m_pd3dG_bufferRtvCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[2];
	//RTV �����
	for (UINT i = 0; i < nRenderTargets; i++)
	{
		//PostProcessing�� �������� �⺻ Texture
		d3dRenderTargetViewDesc.Format = pdxgiFormats[i];
		ComPtr<ID3D12Resource> pd3dTextureResource = m_pRenderTexture->GetResource(i);
		pd3dDevice->CreateRenderTargetView(pd3dTextureResource.Get(), &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dRtvCPUDescriptorHandles[i] = d3dRtvCPUDescriptorHandle; //RTV�� DSV�� CPU Address�� �ʿ��ϴ�
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}

	for (UINT i = 0; i < 2; i++)
	{
		//PostProcessing�� �������� �⺻ Texture
		d3dRenderTargetViewDesc.Format = dxgiFormats[i];

		//SSAO�� ����� G_Buffer�� ������ RenderTexture
		ComPtr<ID3D12Resource> pd3dTextureResource = m_pComputeRenderTexture->GetResource(i);
		pd3dDevice->CreateRenderTargetView(pd3dTextureResource.Get(), &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dG_bufferRtvCPUDescriptorHandles[i] = d3dRtvCPUDescriptorHandle; //RTV�� DSV�� CPU Address�� �ʿ��ϴ�
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CScene::CreateShaderResourceViewDsvs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nDepthTextures, D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle, ID3D12Resource* DefaultDepthStncilBuffer)
{
	//Texture�� ���� �� �ʱ�ȭ
	m_pShadowDepthTexture = new CTexture(nDepthTextures, RESOURCE_TEXTURE2D, 0, 1);

	D3D12_CLEAR_VALUE d3dClearValue = {};
	d3dClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;
	
	DXGI_FORMAT dxgiFormatResource = DXGI_FORMAT_R32_TYPELESS; // Resource�� ���Ѱ�
	DXGI_FORMAT* pdxgiFormatSRV = new DXGI_FORMAT[nDepthTextures]; // SRV�� ���Ѱ�

	//nDepthTextures ������ŭ �����
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

	//�ٸ� Texture���ҽ��� �����.
	//CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//SRV�� �����
	CScene::CreateShaderResourceViews(pd3dDevice, m_pShadowDepthTexture, pdxgiFormatSRV,  0, 20);

	//�⺻���� ���̴� ���̸�
	DXGI_FORMAT dxgiFormatSRV = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	m_pd3dDefaultDepthMapSrvDescrptorHandles = CScene::CreateShaderResourceView(pd3dDevice, DefaultDepthStncilBuffer,  dxgiFormatSRV);
	m_pDefaultDepthStancilResource = DefaultDepthStncilBuffer;

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	d3dDepthStencilViewDesc.Texture2D.MipSlice = 0;

	m_pd3dSahdowMapDsvCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[nDepthTextures];

	//DSV �����

	ComPtr<ID3D12Resource> pd3dTextureResource;

	for (UINT i = 0; i < nDepthTextures; i++)
	{
		pd3dTextureResource = m_pShadowDepthTexture->GetResource(i);
		pd3dDevice->CreateDepthStencilView(pd3dTextureResource.Get(), &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
		m_pd3dSahdowMapDsvCPUDescriptorHandles[i] = d3dDsvCPUDescriptorHandle; //RTV�� DSV�� CPU Address�� �ʿ��ϴ�
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
	UINT ncbElementBytes = ((sizeof(ClusterCB) + 255) & ~255); //256�� ���
	m_cpcbCluster = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpcbCluster->Map(0, NULL, (void**)&m_pcbCluster);

	ncbElementBytes = ((sizeof(LightingData) + 255) & ~255); //256�� ���
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

	// ClusterLightList UAV (CS���� ����)
	m_hClusterLightListUav =
		CreateStructureBufferUAV(
			pd3dDevice,
			m_cpsbClusterLightList.Get(),
			clusterCount,
			sizeof(ClusterLightList)
		);

	// ClusterLightList SRV (PS���� �б�)
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
	//CreateGraphicsRootSignature(pd3dDevice);	//��Ʈ �ñ״��Ŀ� PSO�� ���⼭ �ʱ�ȭ���ְ� �ִ�. 
	//CreateGraphicsPipelineState(pd3dDevice);	//�̰� ���� BuildObjects�� �־��� �ʿ���� ���� ���踦 �̷��� ���� ��
	//�ʱ�ȭ �Լ��� ���� ������ؼ� ���� �����ϴ�.

	//�ٸ� GameFramework���� BuildObjects�� �ʱ�ȭ ������ �����ؼ� ����ϴ� ������ ������ ¥���־ ����ٰ� �ִ°�
	//�����ϰ� ���δ�.

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
	if (m_pd3dRootSignature) m_pd3dRootSignature.Reset();	//ComPtr�� ���⿡ ���� �ʿ����� ������

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
	if (m_pd3dPipelineState) m_pd3dPipelineState.Reset();	//�׻� Release�� �޸� ������ �ʿ��ϴܰ� ��������.


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

//������Ʈ�� �ִϸ��̼�
void CScene::AnimateObjects(float fTimeElapsed)
{
	{
		std::lock_guard<std::mutex> guard(m_movementReconciliationMutex);
		if (m_pMovementCorrectionTarget != nullptr)
		{
			const float correctionLengthSq =
				m_movementCorrection.x * m_movementCorrection.x +
				m_movementCorrection.y * m_movementCorrection.y +
				m_movementCorrection.z * m_movementCorrection.z;
			if (correctionLengthSq > 0.000001f)
			{
				const float rawBlend = 1.0f - std::exp(-12.0f * fTimeElapsed);
				const float blend = rawBlend < 0.0f ? 0.0f : (rawBlend > 1.0f ? 1.0f : rawBlend);
				XMFLOAT3 applied = {
					m_movementCorrection.x * blend,
					m_movementCorrection.y * blend,
					m_movementCorrection.z * blend };
				if (correctionLengthSq < 0.0001f)
					applied = m_movementCorrection;

				XMFLOAT3 position = m_pMovementCorrectionTarget->GetPosition();
				position.x += applied.x;
				position.y += applied.y;
				position.z += applied.z;
				m_pMovementCorrectionTarget->SetPosition(position.x, position.y, position.z);
				m_movementCorrection.x -= applied.x;
				m_movementCorrection.y -= applied.y;
				m_movementCorrection.z -= applied.z;
				m_lastPredictedMovePosition.x += applied.x;
				m_lastPredictedMovePosition.y += applied.y;
				m_lastPredictedMovePosition.z += applied.z;
			}
		}
	}

	CGameObject* airborneMoveObject = nullptr;
	Protocol::PlayerState airborneMoveState = Protocol::PlayerState::IDLE;
	float airbornePacketDelta = 0.0f;
	{
		std::lock_guard<std::mutex> guard(m_movementReconciliationMutex);
		CFarmPlayer* farmPlayer = dynamic_cast<CFarmPlayer*>(m_pLastMovePacketObject);
		if (farmPlayer != nullptr)
		{
			const bool isAirborne = !farmPlayer->m_bOnGround && !farmPlayer->m_bOnBuilding;
			const uint64 nowMs = ::GetTickCount64();
			const uint64 elapsedMs = m_lastMovePacketTimeMs == 0 ? 0 : nowMs - m_lastMovePacketTimeMs;
			const bool landedSinceLastFrame = m_wasAirborneForNetwork && !isAirborne;
			if ((isAirborne && elapsedMs >= 50) || landedSinceLastFrame)
			{
				airborneMoveObject = farmPlayer;
				airborneMoveState = m_lastMovePacketState;
				airbornePacketDelta = elapsedMs > 100 ? 0.1f : static_cast<float>(elapsedMs) / 1000.0f;
			}
			m_wasAirborneForNetwork = isAirborne;
		}
	}
	if (airborneMoveObject != nullptr)
	{
		CPlayer* player = static_cast<CPlayer*>(airborneMoveObject);
		XMFLOAT3 input = player->m_xmf3InputDir;
		const float inputLength = std::sqrt(input.x * input.x + input.z * input.z);
		if (inputLength > 0.001f)
		{
			input.x /= inputLength;
			input.z /= inputLength;
		}
		else
		{
			input.x = 0.0f;
			input.z = 0.0f;
		}
		SendObjectMoveToServer(airborneMoveObject, airborneMoveState,
			input.x, input.z, airbornePacketDelta);
	}

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

//Render �ϱ��� �켱 �̰� �Ⱦ��� CShader���� �����ϰ� �ִ�
void CScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//��Ʈ �ñ״��� ����
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get());

	//PSO ���� ==> PSO�� ��Ʈ �ñ״��ĸ� ���������� ���� �ٸ� �� ����Ǿ� �ִٸ� ��Ÿ�� 
	pd3dCommandList->SetPipelineState(m_pd3dPipelineState.Get());

	//IA�� ���������� Ʈ���̾ޱ۸���Ʈ�� ����
	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

//�� Render�� �����ϱ� ���� �����۾�
void CScene::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, CCamera* pCamera)
{
	//pCamera->SetViewportsAndScissorRects(pd3dCommandList); //����Ʈ�� ������Ʈ�� ���� GPU ���ɾ�� Set���ִ� ��

	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get()); //��Ʈ �ñ״��ĸ� GPU���� �ѱ�°�
	pd3dCommandList->SetComputeRootSignature(m_cpd3dComputeRootSignature.Get());
	//if (pCamera)
	//{
	//	for (int i = 0; i < MAX_LIGHTS; i++)
	//	{
	//		//TODO ����ٰ� ���� Shader ó���� �ѹ� �� �� �ؾ��Ѵ� 
	//	}
	//}
}

//Scene���� ������Ʈ���� �׸��� ���� ==> Camara�� �ϳ��̰� �������� �ʴ´ٴ� �����Ͽ���
void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//PrepareRender(pd3dCommandList);

	//pd3dCommandList->DrawInstanced(6,1,0,0);	//Draw ���� ���� ���Ƿ� �׳� �׸��°Ŷ�� ����ȴ�.

	 //�׷��� ��Ʈ �ñ׳��ĸ� ���������ο� ����(����)�Ѵ�.
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get());

	//���� �������ϴ� ���� ���� �����ϴ� ���̴�(���̴��� �����ϴ� ��ü)���� �������ϴ� ���̴�.
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
		if (m_pIBLBRDFTexture) //ȯ����� ���縦 �Ѵٸ�
		{
			m_pIBLBRDFTexture->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pIBLSpecluarTexture) //ȯ����� ���縦 �Ѵٸ�
		{
			m_pIBLSpecluarTexture->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pIBLIrradianceTexture) //ȯ����� ���縦 �Ѵٸ�
		{
			m_pIBLIrradianceTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}
	else
	{
		if (m_pIBLBRDFTextureCloud) //ȯ����� ���縦 �Ѵٸ�
		{
			m_pIBLBRDFTextureCloud->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pIBLSpecluarTextureCloud) //ȯ����� ���縦 �Ѵٸ�
		{
			m_pIBLSpecluarTextureCloud->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pIBLIrradianceTextureCloud) //ȯ����� ���縦 �Ѵٸ�
		{
			m_pIBLIrradianceTextureCloud->UpdateShaderVariables(pd3dCommandList);
		}
	}
}


void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	//pCamera->SetViewportsAndScissorRects(pd3dCommandList); //����Ʈ�� ������Ʈ�� ���� GPU ���ɾ�� Set���ִ� ��

	//pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get()); //��Ʈ �ñ״��ĸ� GPU���� �ѱ�°�
	
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	
	//ImGui�� ������ �÷��׸� �����ϴ� ���� ��
	UpdateShaderVariables(pd3dCommandList);

	if (pCamera)
	{
		pCamera->UpdateShaderVariables(pd3dCommandList); //ī�޶��� ��� ���۳� ���� GPU�� Shader�� �Ѱܾ��� ���ɾ���� Update���ִ� �κ�
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

	ImGui::InputText("ID", m_szLoginID, IM_ARRAYSIZE(m_szLoginID));
	ImGui::InputText("Password", m_szLoginPW, IM_ARRAYSIZE(m_szLoginPW), ImGuiInputTextFlags_Password);
	ImGui::Spacing();

	// �α��� ��û ���̸� ��ư ��Ȱ��ȭ
	if (m_bLoginRequesting)
	{
		ImGui::BeginDisabled();
		ImGui::EndDisabled();
	}

	if (ImGui::Button("Login", ImVec2(120, 0)))
	{
		if (m_szLoginID[0] == '\0' || m_szLoginPW[0] == '\0')
		{
			m_bLoginFailed = true;
		}
		else
		{
			Protocol::C_LOGIN pkt;
			pkt.set_accountid(m_szLoginID);
			pkt.set_password(m_szLoginPW);
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

//���Ϸ� ���� ��� Texture�� ����ϴ��� �о���� �Լ�
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
	
	// OBB ����
	XMVECTOR vCenter = XMLoadFloat3(&obb.Center);
	XMVECTOR vExtents = XMLoadFloat3(&obb.Extents);
	XMVECTOR vOrientation = XMLoadFloat4(&obb.Orientation);

	// point�� OBB ���� �������� ��ȯ
	XMVECTOR vLocal = XMVectorSubtract(point, vCenter);

	// ȸ�� ����ȯ (world -> obb local)
	XMVECTOR vInvRot = XMQuaternionInverse(vOrientation);
	vLocal = XMVector3Rotate(vLocal, vInvRot);

	// ���� AABB���� clamp
	XMVECTOR vClamped = XMVectorClamp(
		vLocal,
		XMVectorNegate(vExtents),
		vExtents
	);

	// �ٽ� ���� ��������
	XMVECTOR vWorld = XMVector3Rotate(vClamped, vOrientation);
	vWorld = XMVectorAdd(vWorld, vCenter);

	return vWorld;
}

void CScene::CreateSSAOKernel()
{
}




LightControl::LightControl()
{
	// Light �⺻ �ʱ�ȭ
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

	// Spot Light BoundingSphere �ʱ�ȭ
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

	// Spot Light�� BoundingSphere
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


void CScene::SendObjectMoveToServer(CGameObject* pObj, Protocol::PlayerState state,
	float inputX, float inputZ, float deltaTime)
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
	std::lock_guard<std::mutex> guard(m_movementReconciliationMutex);
	m_pLastMovePacketObject = pObj;
	m_lastMovePacketState = state;
	m_lastMovePacketTimeMs = ::GetTickCount64();

	Protocol::C_MOVE pkt;
	const uint32 sequence = m_nextMoveSequence++;
	const XMFLOAT3 predictedPosition = pObj->GetPosition();
	XMFLOAT3 predictedDelta = { 0.0f, 0.0f, 0.0f };
	if (m_hasPredictedMovePosition)
	{
		predictedDelta.x = predictedPosition.x - m_lastPredictedMovePosition.x;
		predictedDelta.y = predictedPosition.y - m_lastPredictedMovePosition.y;
		predictedDelta.z = predictedPosition.z - m_lastPredictedMovePosition.z;
	}
	m_lastPredictedMovePosition = predictedPosition;
	m_hasPredictedMovePosition = true;
	m_pendingMoveInputs.push_back({ sequence, predictedDelta });

	Protocol::Transform* trans = pkt.mutable_player()->mutable_transform();
	pObj->FillProtoTransform(trans);
	pkt.mutable_player()->set_state(state);
	pkt.set_sequence(sequence);
	pkt.set_client_tick(::GetTickCount());
	pkt.set_input_x(inputX);
	pkt.set_input_z(inputZ);
	const float safeDeltaTime = deltaTime < 0.0f ? 0.0f : (deltaTime > 0.1f ? 0.1f : deltaTime);
	pkt.set_delta_time(safeDeltaTime);

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

void CScene::ReconcileLocalPlayer(CGameObject* pObj, const XMFLOAT3& serverPosition,
	uint32 lastProcessedSequence)
{
	if (pObj == nullptr)
		return;
	std::lock_guard<std::mutex> guard(m_movementReconciliationMutex);

	m_pendingMoveInputs.erase(
		std::remove_if(m_pendingMoveInputs.begin(), m_pendingMoveInputs.end(),
			[lastProcessedSequence](const PendingMoveInput& input)
			{
				return input.sequence <= lastProcessedSequence;
			}),
		m_pendingMoveInputs.end());

	XMFLOAT3 reconciled = serverPosition;
	for (const PendingMoveInput& input : m_pendingMoveInputs)
	{
		reconciled.x += input.predictedDelta.x;
		reconciled.y += input.predictedDelta.y;
		reconciled.z += input.predictedDelta.z;
	}

	const XMFLOAT3 currentPosition = pObj->GetPosition();
	const XMFLOAT3 error = {
		reconciled.x - currentPosition.x,
		reconciled.y - currentPosition.y,
		reconciled.z - currentPosition.z };
	const float horizontalErrorSq = error.x * error.x + error.z * error.z;
	const bool needsHardCorrection = horizontalErrorSq > 4.0f || std::fabs(error.y) > 2.0f;
	if (needsHardCorrection)
	{
		if (CPlayer* player = dynamic_cast<CPlayer*>(pObj))
			player->SetPosition(reconciled);
		else
			pObj->SetPosition(reconciled.x, reconciled.y, reconciled.z);
		m_movementCorrection = { 0.0f, 0.0f, 0.0f };
		m_lastPredictedMovePosition = reconciled;
	}
	else
	{
		m_pMovementCorrectionTarget = pObj;
		m_movementCorrection = error;
		m_lastPredictedMovePosition = currentPosition;
	}
	m_hasPredictedMovePosition = true;
}


