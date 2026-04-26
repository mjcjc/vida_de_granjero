#include "stdafx.h"
#include "GraphicsCommon.h"

void CGraphicsCommon::InitCommonStates(ID3D12Device* pd3dDevice)
{
	InitInputLayout(pd3dDevice);
	InitRasterizerStates(pd3dDevice);
	InitBlendStates(pd3dDevice);
	InitDepthStencilStates(pd3dDevice);
	InitRtvDsvFormat(pd3dDevice);
	InitRootSignature(pd3dDevice);
	InitPipelineStates(pd3dDevice);
	InitComputeRootSignature(pd3dDevice);
	InitComputePipelineStates(pd3dDevice);

}

void CGraphicsCommon::InitInputLayout(ID3D12Device* pd3dDevice)
{

	// Basic Input Layout (Position, Normal, UV)

	{
		UINT count = 2;
		D3D12_INPUT_ELEMENT_DESC* elems = new D3D12_INPUT_ELEMENT_DESC[count]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		m_d3dPlayerInput.pInputElementDescs = elems;
		m_d3dPlayerInput.NumElements = count;
	}

	{
		UINT count = 3;
		D3D12_INPUT_ELEMENT_DESC* elems = new D3D12_INPUT_ELEMENT_DESC[count]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		m_d3dParticleInput.pInputElementDescs = elems;
		m_d3dParticleInput.NumElements = count;
	}

	{
		UINT count = 3;
		D3D12_INPUT_ELEMENT_DESC* elems = new D3D12_INPUT_ELEMENT_DESC[count]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		m_d3dBasicInput.pInputElementDescs = elems;
		m_d3dBasicInput.NumElements = count;
	}


	{
		UINT count = 4;
		D3D12_INPUT_ELEMENT_DESC* elems = new D3D12_INPUT_ELEMENT_DESC[count]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		m_d3dTerrainInput.pInputElementDescs = elems;
		m_d3dTerrainInput.NumElements = count;
	}


	// Default Input Layout (Position, Normal, UV, Tangent)

	{
		UINT count = 5;
		D3D12_INPUT_ELEMENT_DESC* elems = new D3D12_INPUT_ELEMENT_DESC[count]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}

		};

		m_d3dDefaultInput.pInputElementDescs = elems;
		m_d3dDefaultInput.NumElements = count;
	}


	// Skinned Mesh Input Layout (Position, Normal, UV, Tangent, Weights, Indices)

	{
		UINT count = 7;
		D3D12_INPUT_ELEMENT_DESC* elems = new D3D12_INPUT_ELEMENT_DESC[count]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,2, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }

		};

		m_d3dSkinnedInput.pInputElementDescs = elems;
		m_d3dSkinnedInput.NumElements = count;
	}

	// Skybox Input Layout (Position only)

	{
		UINT count = 1;
		D3D12_INPUT_ELEMENT_DESC* elems = new D3D12_INPUT_ELEMENT_DESC[count]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		m_d3dSkyBoxInput.pInputElementDescs = elems;
		m_d3dSkyBoxInput.NumElements = count;
	}

	// PostPorcessing Input Layout
	{
		UINT count = 2;
		D3D12_INPUT_ELEMENT_DESC* elems = new D3D12_INPUT_ELEMENT_DESC[count]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		m_d3dPostPorcessingInput.pInputElementDescs = elems;
		m_d3dPostPorcessingInput.NumElements = count;
	}
}

void CGraphicsCommon::InitRasterizerStates(ID3D12Device* pd3dDevice)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = false;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = true;
	d3dRasterizerDesc.MultisampleEnable = false;
	d3dRasterizerDesc.AntialiasedLineEnable = false;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	m_d3dDefaultRasterize = d3dRasterizerDesc;

	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;

	m_d3dWireRasterize = d3dRasterizerDesc;

	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.DepthBias = 1000;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 1.0f;

	m_d3dShadowRasterize = d3dRasterizerDesc;
}

void CGraphicsCommon::InitBlendStates(ID3D12Device* pd3dDevice)
{

	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = false;
	d3dBlendDesc.IndependentBlendEnable = false;
	d3dBlendDesc.RenderTarget[0].BlendEnable = false;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = false;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	m_d3dDefaultBlend = d3dBlendDesc;

	D3D12_BLEND_DESC d3dAlphaBlend = d3dBlendDesc;

	d3dAlphaBlend.RenderTarget[0].BlendEnable = true;
	d3dAlphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dAlphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dAlphaBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dAlphaBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dAlphaBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	d3dAlphaBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

	m_d3dAlphaBlend = d3dAlphaBlend;

}
void CGraphicsCommon::InitDepthStencilStates(ID3D12Device* pd3dDevice)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = true;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = false;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	m_d3dDefaultDepth = d3dDepthStencilDesc;

	D3D12_DEPTH_STENCIL_DESC d3dNoDepthStencilDesc;
	::ZeroMemory(&d3dNoDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));

	d3dNoDepthStencilDesc.DepthEnable = false;
	d3dNoDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dNoDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	d3dNoDepthStencilDesc.StencilEnable = false;
	d3dNoDepthStencilDesc.StencilReadMask = 0x00;
	d3dNoDepthStencilDesc.StencilWriteMask = 0x00;

	d3dNoDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dNoDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dNoDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dNoDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	d3dNoDepthStencilDesc.BackFace = d3dNoDepthStencilDesc.FrontFace;

	m_d3dDoneDepth = d3dNoDepthStencilDesc;
}
void CGraphicsCommon::InitRootSignature(ID3D12Device* pd3dDevice)
{
	//Start Scene
	{
		/*
	* 루트 파라미터 한 번에 64개의 슬롯을 설정 가능
	* 상수, 디스크립터, 디스크립터 테이블 3개를 연결할 수 있다.
	* 용도에 따라서 3가지 중 선택해서 넘겨야 한다.
	*/
		ComPtr<ID3D12RootSignature> cpd3dGraphicsRootSignature;


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
				**)cpd3dGraphicsRootSignature.GetAddressOf());
		if (pd3dSignatureBlob) pd3dSignatureBlob->Release(); // 더 이상 필요 없으니까 Release 해준다
		if (pd3dErrorBlob) pd3dErrorBlob->Release();

		m_cpd3dStartSceneRootSignature = cpd3dGraphicsRootSignature;
	}

	//Main Scene
	{
		ComPtr<ID3D12RootSignature> cpd3dGraphicsRootSignature;
		D3D12_DESCRIPTOR_RANGE pd3dDescriptorRangesShadow[1];
		pd3dDescriptorRangesShadow[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRangesShadow[0].NumDescriptors = 5;
		pd3dDescriptorRangesShadow[0].BaseShaderRegister = 30; //t30 : Shadow Map
		pd3dDescriptorRangesShadow[0].RegisterSpace = 0;
		pd3dDescriptorRangesShadow[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//pd3dDescriptorRangesShadow[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		//pd3dDescriptorRangesShadow[1].NumDescriptors = 4;
		//pd3dDescriptorRangesShadow[1].BaseShaderRegister = 31; //t31 ~ t34:  Spot Light Shadow Map
		//pd3dDescriptorRangesShadow[1].RegisterSpace = 0;
		//pd3dDescriptorRangesShadow[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


		D3D12_DESCRIPTOR_RANGE pd3dDescriptorRangesTerrain[2];
		pd3dDescriptorRangesTerrain[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRangesTerrain[0].NumDescriptors = 11;
		pd3dDescriptorRangesTerrain[0].BaseShaderRegister = 40; //t40  ~ : TerrainMap
		pd3dDescriptorRangesTerrain[0].RegisterSpace = 0;
		pd3dDescriptorRangesTerrain[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRangesTerrain[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRangesTerrain[1].NumDescriptors = 3;
		pd3dDescriptorRangesTerrain[1].BaseShaderRegister = 51; //t51 ~  : Summer, Winter Map
		pd3dDescriptorRangesTerrain[1].RegisterSpace = 0;
		pd3dDescriptorRangesTerrain[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


		D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[14];

		pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		pd3dDescriptorRanges[0].NumDescriptors = 1;
		pd3dDescriptorRanges[0].BaseShaderRegister = 2; //b2: GameObject 
		pd3dDescriptorRanges[0].RegisterSpace = 0;
		pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[1].NumDescriptors = 1;
		pd3dDescriptorRanges[1].BaseShaderRegister = 1; //t1:  Albedo
		pd3dDescriptorRanges[1].RegisterSpace = 0;
		pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[2].NumDescriptors = 1;
		pd3dDescriptorRanges[2].BaseShaderRegister = 2; //t2 : NormalMap
		pd3dDescriptorRanges[2].RegisterSpace = 0;
		pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[3].NumDescriptors = 1;
		pd3dDescriptorRanges[3].BaseShaderRegister = 3; //t3: Roughnness
		pd3dDescriptorRanges[3].RegisterSpace = 0;
		pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[4].NumDescriptors = 1;
		pd3dDescriptorRanges[4].BaseShaderRegister = 4; //t4: Metal
		pd3dDescriptorRanges[4].RegisterSpace = 0;
		pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[5].NumDescriptors = 1;
		pd3dDescriptorRanges[5].BaseShaderRegister = 5; //t5: Occlusion
		pd3dDescriptorRanges[5].RegisterSpace = 0;
		pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[6].NumDescriptors = 1;
		pd3dDescriptorRanges[6].BaseShaderRegister = 6; //t6: Specluar F0
		pd3dDescriptorRanges[6].RegisterSpace = 0;
		pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[7].NumDescriptors = 1;
		pd3dDescriptorRanges[7].BaseShaderRegister = 7; //t7: emission
		pd3dDescriptorRanges[7].RegisterSpace = 0;
		pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[8].NumDescriptors = 1;
		pd3dDescriptorRanges[8].BaseShaderRegister = 10; //t10: skybox
		pd3dDescriptorRanges[8].RegisterSpace = 0;
		pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[9].NumDescriptors = 1;
		pd3dDescriptorRanges[9].BaseShaderRegister = 11; //t11: IBL BRDF LUT
		pd3dDescriptorRanges[9].RegisterSpace = 0;
		pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[10].NumDescriptors = 1;
		pd3dDescriptorRanges[10].BaseShaderRegister = 12; //t12: Specluar IBL
		pd3dDescriptorRanges[10].RegisterSpace = 0;
		pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[11].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[11].NumDescriptors = 1;
		pd3dDescriptorRanges[11].BaseShaderRegister = 13; //t13: Irradiance
		pd3dDescriptorRanges[11].RegisterSpace = 0;
		pd3dDescriptorRanges[11].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[12].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[12].NumDescriptors = 1;
		pd3dDescriptorRanges[12].BaseShaderRegister = 14; //t14: HDR RenderTarget SRV
		pd3dDescriptorRanges[12].RegisterSpace = 0;
		pd3dDescriptorRanges[12].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[13].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[13].NumDescriptors = 1;
		pd3dDescriptorRanges[13].BaseShaderRegister = 15; //t15: Clustering Light List의 구조체 버퍼
		pd3dDescriptorRanges[13].RegisterSpace = 0;
		pd3dDescriptorRanges[13].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		/*
		* 1. Camera //CBV
		* 2. Player(플레이어 위치 좌표 관련) //CBV
		* 3. Light //CBV
		* 4. Texture //여기에 기본적인 오브젝트의 월드 변환행렬도 테이블로 Table
		*/

		D3D12_ROOT_PARAMETER pd3dRootParameters[4 + 8 + 2 + 1 + 1 + 3 + 1 + 1 + 1 + 2 + 1 + 2];

		pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		pd3dRootParameters[0].Constants.Num32BitValues = 16 + 1 + 1;
		pd3dRootParameters[0].Constants.ShaderRegister = 3; //Material + GameObject
		pd3dRootParameters[0].Constants.RegisterSpace = 0;
		pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //Camera
		pd3dRootParameters[1].Descriptor.ShaderRegister = 0;
		pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //Player
		pd3dRootParameters[2].Descriptor.ShaderRegister = 1;
		pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //GameObject
		pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
		pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
		pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
		pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
		pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
		pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
		pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
		pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
		pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		//자 이건 Animation 설정이 있는 놈에 한해서 필요한 부분인데 쓰고있잖아
		//그래서 GraphicsCommon으로 Scene에 있는이 루트 시그니쳐부터해서 PSO를 
		//찢어서 추가해줄 예정이야 안그래?
		pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[11].Descriptor.ShaderRegister = 11; //b11
		pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[12].Descriptor.ShaderRegister = 12; //b12
		pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //Light
		pd3dRootParameters[13].Descriptor.ShaderRegister = 10;
		pd3dRootParameters[13].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//ImGuiFlag
		pd3dRootParameters[14].Descriptor.ShaderRegister = 4;
		pd3dRootParameters[14].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//skybox t:10
		pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[15].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[15].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
		pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pd3dRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[16].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[16].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
		pd3dRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pd3dRootParameters[17].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[17].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[17].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[10]);
		pd3dRootParameters[17].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pd3dRootParameters[18].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[18].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[18].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[11]);
		pd3dRootParameters[18].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		//Render Target Post
		pd3dRootParameters[19].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[19].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[19].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[12]);
		pd3dRootParameters[19].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		//Light Shadw Map
		pd3dRootParameters[20].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[20].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[20].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRangesShadow[0]);
		pd3dRootParameters[20].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pd3dRootParameters[21].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		pd3dRootParameters[21].Constants.Num32BitValues = 1;
		pd3dRootParameters[21].Constants.ShaderRegister = 5; //ShadowIndex
		pd3dRootParameters[21].Constants.RegisterSpace = 0;
		pd3dRootParameters[21].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Terrain
		pd3dRootParameters[22].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[22].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[22].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRangesTerrain[0]);
		pd3dRootParameters[22].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pd3dRootParameters[23].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[23].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[23].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRangesTerrain[1]);
		pd3dRootParameters[23].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		//particle
		pd3dRootParameters[24].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[24].Descriptor.ShaderRegister = 15;
		pd3dRootParameters[24].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[24].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Cluster 정보와 ClusterLightList;

		pd3dRootParameters[25].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[25].Descriptor.ShaderRegister = 16;
		pd3dRootParameters[25].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[25].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		pd3dRootParameters[26].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[26].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[26].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[13]); //t15
		pd3dRootParameters[26].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


		D3D12_STATIC_SAMPLER_DESC d3dSamplerDescs[5];
		::ZeroMemory(&d3dSamplerDescs[0], sizeof(D3D12_STATIC_SAMPLER_DESC));
		::ZeroMemory(&d3dSamplerDescs[1], sizeof(D3D12_STATIC_SAMPLER_DESC));
		::ZeroMemory(&d3dSamplerDescs[2], sizeof(D3D12_STATIC_SAMPLER_DESC));
		::ZeroMemory(&d3dSamplerDescs[3], sizeof(D3D12_STATIC_SAMPLER_DESC));
		::ZeroMemory(&d3dSamplerDescs[4], sizeof(D3D12_STATIC_SAMPLER_DESC));
		d3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		d3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		d3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		d3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		d3dSamplerDescs[0].MipLODBias = 0;
		d3dSamplerDescs[0].MaxAnisotropy = 1;
		d3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		d3dSamplerDescs[0].MinLOD = 0;
		d3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
		d3dSamplerDescs[0].ShaderRegister = 0;
		d3dSamplerDescs[0].RegisterSpace = 0;
		d3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


		d3dSamplerDescs[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		d3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		d3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		d3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		d3dSamplerDescs[1].MipLODBias = 0;
		d3dSamplerDescs[1].MaxAnisotropy = 1;
		d3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		d3dSamplerDescs[1].MinLOD = 0;
		d3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
		d3dSamplerDescs[1].ShaderRegister = 1;
		d3dSamplerDescs[1].RegisterSpace = 0;
		d3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


		d3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		d3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		d3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		d3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		//d3dSamplerDescs[3].BorderColor[0] = 100.0f;
		d3dSamplerDescs[2].MipLODBias = 0;
		d3dSamplerDescs[2].MaxAnisotropy = 1;
		d3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		d3dSamplerDescs[2].MinLOD = 0;
		d3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
		d3dSamplerDescs[2].ShaderRegister = 2;
		d3dSamplerDescs[2].RegisterSpace = 0;
		d3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


		d3dSamplerDescs[3].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		d3dSamplerDescs[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		d3dSamplerDescs[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		d3dSamplerDescs[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		d3dSamplerDescs[3].MipLODBias = 0;
		d3dSamplerDescs[3].MaxAnisotropy = 1;
		d3dSamplerDescs[3].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		d3dSamplerDescs[3].MinLOD = 0;
		d3dSamplerDescs[3].MaxLOD = D3D12_FLOAT32_MAX;
		d3dSamplerDescs[3].ShaderRegister = 3;
		d3dSamplerDescs[3].RegisterSpace = 0;
		d3dSamplerDescs[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		d3dSamplerDescs[4].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		d3dSamplerDescs[4].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		d3dSamplerDescs[4].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		d3dSamplerDescs[4].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		d3dSamplerDescs[4].MipLODBias = 0;
		d3dSamplerDescs[4].MaxAnisotropy = 1;
		d3dSamplerDescs[4].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		d3dSamplerDescs[4].MinLOD = 0;
		d3dSamplerDescs[4].MaxLOD = D3D12_FLOAT32_MAX;
		d3dSamplerDescs[4].ShaderRegister = 4;
		d3dSamplerDescs[4].RegisterSpace = 0;
		d3dSamplerDescs[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


		//플래그를 이렇게 변수처럼 따로 만들어 DESC에 연결 가능
		D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
		::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
		d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
		d3dRootSignatureDesc.pParameters = pd3dRootParameters;
		d3dRootSignatureDesc.NumStaticSamplers = _countof(d3dSamplerDescs);
		d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDescs;
		d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

		ID3DBlob* pd3dSignatureBlob = NULL;
		ID3DBlob* pd3dErrorBlob = NULL;
		::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, //구조체를 GPU가 읽을 수 있는 바이너리 형식으로 직렬화 시켜주고 있다.
			&pd3dSignatureBlob, &pd3dErrorBlob);
		pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), //직렬화된 바이트 코드를 읽어와 실제로 Create 해준다
			pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void
				**)cpd3dGraphicsRootSignature.GetAddressOf());
		if (pd3dSignatureBlob) pd3dSignatureBlob->Release(); // 더 이상 필요 없으니까 Release 해준다
		if (pd3dErrorBlob) pd3dErrorBlob->Release();

		m_cpd3dMainSceneRootSignature = cpd3dGraphicsRootSignature;
	}
}
void CGraphicsCommon::InitComputeRootSignature(ID3D12Device* pd3dDevice)
{

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[5];
	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //t0: 깊이맵  , SSAO
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 1; //t1:   노멀맵 G-버퍼
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 2; //t2: IBL
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 0; //u0:  HDRI UAV
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 1; //u1:  HDRI UAV
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	D3D12_ROOT_PARAMETER pd3dRootParameters[8];

	// b0 : Constant Buffer
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0;
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1;
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// t0~ : SRV Table
	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1];
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2];
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	// u0~ : UAV Table
	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3];
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[4];
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //Light Data
	pd3dRootParameters[7].Descriptor.ShaderRegister = 2; 
	pd3dRootParameters[7].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_STATIC_SAMPLER_DESC d3dSamplerDescs[1];
	::ZeroMemory(&d3dSamplerDescs[0], sizeof(D3D12_STATIC_SAMPLER_DESC));

	d3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	d3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3dSamplerDescs[0].MipLODBias = 0;
	d3dSamplerDescs[0].MaxAnisotropy = 1;
	d3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dSamplerDescs[0].MinLOD = 0;
	d3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDescs[0].ShaderRegister = 0;
	d3dSamplerDescs[0].RegisterSpace = 0;
	d3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//플래그를 이렇게 변수처럼 따로 만들어 DESC에 연결 가능
	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_NONE;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(d3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ComPtr<ID3DBlob> pd3dSignatureBlob = NULL;
	ComPtr<ID3DBlob> pd3dErrorBlob = NULL;

	ComPtr<ID3DBlob> blob;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);

	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&m_cpd3dMainSceneComputeRootSignature));
}
void CGraphicsCommon::InitRtvDsvFormat(ID3D12Device* pd3dDevice)
{
	m_dxgiRtvFormatHDR = DXGI_FORMAT_R16G16B16A16_FLOAT;
	m_dxgiRtvFormatLDR = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_dxgiRtvFormatUnknown = DXGI_FORMAT_UNKNOWN;

	m_dxgiDsvFormatDefualt = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_dxgiDsvFormatShadowMap = DXGI_FORMAT_D32_FLOAT;
	m_dxgiDsvFormatUnknown = DXGI_FORMAT_UNKNOWN;


}
void CGraphicsCommon::InitPipelineStates(ID3D12Device* pd3dDevice)
{


	m_cpd3dSkinnedPSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"SkinnedVS.hlsl", L"SkinnedPS.hlsl",
		m_d3dDefaultRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dSkinnedInput, m_dxgiRtvFormatHDR, m_dxgiDsvFormatDefualt, 3);
	m_cpd3dSkinnedNormalPSO = CreateNormalPSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"SkinnedVS.hlsl", L"NormalGS.hlsl", L"SimplePS.hlsl",
		m_d3dDefaultRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dSkinnedInput, m_dxgiRtvFormatHDR);
	m_cpd3dSkinnedWirePSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"SkinnedVS.hlsl", L"SkinnedPS.hlsl",
		m_d3dWireRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dSkinnedInput, m_dxgiRtvFormatHDR, m_dxgiDsvFormatDefualt, 3);
	m_cpd3dSkinnedShadowPSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"DepthOnlySkinnedVS.hlsl", NULL,
		m_d3dShadowRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dSkinnedInput, m_dxgiRtvFormatUnknown, m_dxgiDsvFormatShadowMap, 0);

	m_cpd3dDefaultPSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"DefaultVS.hlsl", L"DefaultPS.hlsl",
		m_d3dDefaultRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dDefaultInput, m_dxgiRtvFormatHDR, m_dxgiDsvFormatDefualt, 3);
	m_cpd3dDefaultWirePSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"DefaultVS.hlsl", L"DefaultPS.hlsl",
		m_d3dWireRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dDefaultInput, m_dxgiRtvFormatHDR, m_dxgiDsvFormatDefualt, 3);
	m_cpd3dDefaultNormalPSO = CreateNormalPSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"DefaultVS.hlsl", L"NormalGS.hlsl", L"SimplePS.hlsl",
		m_d3dDefaultRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dDefaultInput, m_dxgiRtvFormatHDR);
	m_cpd3dDefaultShadowPSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"DepthOnlyDefaultVS.hlsl", NULL,
		m_d3dShadowRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dDefaultInput, m_dxgiRtvFormatUnknown, m_dxgiDsvFormatShadowMap, 0);

	m_cpd3dBasicPSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"BasicVS.hlsl", L"BasicPS.hlsl",
		m_d3dDefaultRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dBasicInput, m_dxgiRtvFormatHDR, m_dxgiDsvFormatDefualt, 3);
	m_cpd3dBasicNormalPSO = CreateNormalPSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"BasicVS.hlsl", L"NormalGS.hlsl", L"SimplePS.hlsl",
		m_d3dDefaultRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dBasicInput, m_dxgiRtvFormatHDR);
	m_cpd3dBasicWirePSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"BasicVS.hlsl", L"BasicPS.hlsl",
		m_d3dWireRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dBasicInput, m_dxgiRtvFormatHDR, m_dxgiDsvFormatDefualt, 3);
	m_cpd3dBasicShadowPSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"DepthOnlyBasicVS.hlsl", NULL,
		m_d3dShadowRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dBasicInput, m_dxgiRtvFormatUnknown, m_dxgiDsvFormatShadowMap, 0);

	m_cpd3dTerrainPSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"TerrainVS.hlsl", L"TerrainPS.hlsl",
		m_d3dDefaultRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dTerrainInput, m_dxgiRtvFormatHDR, m_dxgiDsvFormatDefualt, 3);
	m_cpd3dTerrainNormalPSO = CreateNormalPSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"TerrainVS.hlsl", L"NormalGS.hlsl", L"SimplePS.hlsl",
		m_d3dDefaultRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dTerrainInput, m_dxgiRtvFormatHDR);
	m_cpd3dTerrainWirePSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"TerrainVS.hlsl", L"TerrainPS.hlsl",
		m_d3dWireRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dTerrainInput, m_dxgiRtvFormatHDR, m_dxgiDsvFormatDefualt, 3);
	m_cpd3dTerrainShadowPSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"DepthOnlyTerrainVS.hlsl", NULL,
		m_d3dShadowRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dTerrainInput, m_dxgiRtvFormatUnknown, m_dxgiDsvFormatShadowMap, 0);

	m_cpd3dSkyBoxPSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"SkyBoxVS.hlsl", L"SkyBoxPS.hlsl",
		m_d3dDefaultRasterize, m_d3dDefaultBlend, m_d3dDoneDepth, m_d3dSkyBoxInput, m_dxgiRtvFormatHDR, m_dxgiDsvFormatDefualt, 1);

	m_cpd3dPlayerPSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"PlayerVS.hlsl", L"PlayerPS.hlsl",
		m_d3dDefaultRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dPlayerInput, m_dxgiRtvFormatHDR, m_dxgiDsvFormatDefualt, 1);

	m_cpd3dOBBPSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"SkyBoxVS.hlsl", L"SimpleGreenPS.hlsl",
		m_d3dWireRasterize, m_d3dDefaultBlend, m_d3dDefaultDepth, m_d3dBasicInput, m_dxgiRtvFormatHDR, m_dxgiDsvFormatDefualt, 1);

	m_cpd3dPostProcessingPSO = CreatePSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"PostProcessingVS.hlsl", L"PostProcessingPS.hlsl",
		m_d3dDefaultRasterize, m_d3dDefaultBlend, m_d3dDoneDepth, m_d3dPostPorcessingInput, m_dxgiRtvFormatLDR, m_dxgiDsvFormatUnknown, 1);

	m_cpd3dParticlePSO = CreateNormalPSO(pd3dDevice, m_cpd3dMainSceneRootSignature.Get(), L"ParticleVS.hlsl", L"ParticleGS.hlsl", L"ParticlePS.hlsl",
		m_d3dDefaultRasterize, m_d3dAlphaBlend, m_d3dDefaultDepth, m_d3dParticleInput, m_dxgiRtvFormatHDR);
}

void CGraphicsCommon::InitComputePipelineStates(ID3D12Device* pd3dDevice)
{
	m_cpd3dComputePSO = CreateComputePSO(pd3dDevice, m_cpd3dMainSceneComputeRootSignature.Get(), L"PostProcessingCS.hlsl");
	m_cpd3dComputeBlurXPSO = CreateComputePSO(pd3dDevice, m_cpd3dMainSceneComputeRootSignature.Get(), L"BlurXCS.hlsl");
	m_cpd3dComputeBlurYPSO = CreateComputePSO(pd3dDevice, m_cpd3dMainSceneComputeRootSignature.Get(), L"BlurYCS.hlsl");
	m_cpd3dComputeSSAOPSO = CreateComputePSO(pd3dDevice, m_cpd3dMainSceneComputeRootSignature.Get(), L"SSAOCS.hlsl");
	m_cpd3dComputeCombinePSO = CreateComputePSO(pd3dDevice, m_cpd3dMainSceneComputeRootSignature.Get(), L"CombineCS.hlsl");
	m_cpd3dComputeBLoomPSO = CreateComputePSO(pd3dDevice, m_cpd3dMainSceneComputeRootSignature.Get(), L"BloomCS.hlsl");
	m_cpd3dComputeClusterPSO = CreateComputePSO(pd3dDevice, m_cpd3dMainSceneComputeRootSignature.Get(), L"ClusteredRenderingCS.hlsl");
}


ComPtr<ID3D12PipelineState> CGraphicsCommon::CreatePSO(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature,
	WCHAR* pszVSFileName, WCHAR* pszPSFileName, D3D12_RASTERIZER_DESC rasterizeState, D3D12_BLEND_DESC blendState,
	D3D12_DEPTH_STENCIL_DESC depthStencilState, D3D12_INPUT_LAYOUT_DESC inputlayout, DXGI_FORMAT dxgiRtvFormat, DXGI_FORMAT dxgiDsvFormat, UINT nRenderTargets)
{
	ComPtr<ID3D12PipelineState> cpd3dPipelineState;

	ComPtr<ID3DBlob> pd3dVertexShaderBlob = nullptr;
	ComPtr<ID3DBlob> pd3dPixelShaderBlob = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(pszVSFileName, pd3dVertexShaderBlob.GetAddressOf());
	d3dPipelineStateDesc.PS = CreatePixelShader(pszPSFileName, pd3dPixelShaderBlob.GetAddressOf());
	d3dPipelineStateDesc.RasterizerState = rasterizeState;
	d3dPipelineStateDesc.BlendState = blendState;
	d3dPipelineStateDesc.DepthStencilState = depthStencilState;
	d3dPipelineStateDesc.InputLayout = inputlayout;
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = nRenderTargets;
	//여기 들어간 RTVFormats부분 말이다

	switch (nRenderTargets)
	{
	case 1:
	{
		d3dPipelineStateDesc.RTVFormats[0] = dxgiRtvFormat;
		break;
	}
	case 2:
	{
		d3dPipelineStateDesc.RTVFormats[0] = dxgiRtvFormat;
		d3dPipelineStateDesc.RTVFormats[1] = dxgiRtvFormat;
		break;
	}
	case 3:
	{
		d3dPipelineStateDesc.RTVFormats[0] = dxgiRtvFormat;
		d3dPipelineStateDesc.RTVFormats[1] = dxgiRtvFormat;
		d3dPipelineStateDesc.RTVFormats[2] = dxgiRtvFormat;
		break;
	}
	default:
	{
		break;
	}
	}
	d3dPipelineStateDesc.DSVFormat = dxgiDsvFormat;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)cpd3dPipelineState.GetAddressOf());

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();
	//if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[]
	//	d3dPipelineStateDesc.InputLayout.pInputElementDescs;

	return(cpd3dPipelineState);
}

ComPtr<ID3D12PipelineState> CGraphicsCommon::CreateNormalPSO(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature, WCHAR* pszVSFileName, WCHAR* pszGSFileName, WCHAR* pszPSFileName,
	D3D12_RASTERIZER_DESC rasterizeState, D3D12_BLEND_DESC blendState, D3D12_DEPTH_STENCIL_DESC depthStencilState, D3D12_INPUT_LAYOUT_DESC inputlayout, DXGI_FORMAT dxgiRtvFormat)
{
	ComPtr<ID3D12PipelineState> cpd3dPipelineState;

	ComPtr<ID3DBlob> pd3dVertexShaderBlob = nullptr;
	ComPtr<ID3DBlob> pd3dPixelShaderBlob = nullptr;
	ComPtr<ID3DBlob> pd3dGeomertyShaderBlob = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(pszVSFileName, pd3dVertexShaderBlob.GetAddressOf());
	d3dPipelineStateDesc.GS = CreateGeometryShader(pszGSFileName, pd3dGeomertyShaderBlob.GetAddressOf());
	d3dPipelineStateDesc.PS = CreatePixelShader(pszPSFileName, pd3dPixelShaderBlob.GetAddressOf());
	d3dPipelineStateDesc.RasterizerState = rasterizeState;
	d3dPipelineStateDesc.BlendState = blendState;
	d3dPipelineStateDesc.DepthStencilState = depthStencilState;
	d3dPipelineStateDesc.InputLayout = inputlayout;
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = dxgiRtvFormat; DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)cpd3dPipelineState.GetAddressOf());

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();
	//if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[]
	//	d3dPipelineStateDesc.InputLayout.pInputElementDescs;
	//
	return(cpd3dPipelineState);
}

ComPtr<ID3D12PipelineState> CGraphicsCommon::CreateComputePSO(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature, WCHAR* pszCSFileName)
{
	ComPtr<ID3D12PipelineState> cpd3dPipelineState;

	ComPtr<ID3DBlob> pd3dComputeShaderBlob = nullptr;

	D3D12_COMPUTE_PIPELINE_STATE_DESC d3dComputePipelineStateDesc;
	::ZeroMemory(&d3dComputePipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	d3dComputePipelineStateDesc.pRootSignature = pd3dRootSignature;
	d3dComputePipelineStateDesc.CS =
		CreateComputeShader(pszCSFileName, pd3dComputeShaderBlob.GetAddressOf());
	d3dComputePipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	pd3dDevice->CreateComputePipelineState(
		&d3dComputePipelineStateDesc,
		__uuidof(ID3D12PipelineState),
		(void**)cpd3dPipelineState.GetAddressOf()
	);

	return cpd3dPipelineState;
}


D3D12_SHADER_BYTECODE CGraphicsCommon::CompileShaderFromFile(WCHAR* pszFileName, LPCSTR
	pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob)
{
	ID3DBlob* errorBlob = nullptr;
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	HRESULT hr = ::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, &errorBlob);

	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		// 실패했을 때는 빈 바이트코드 리턴
		D3D12_SHADER_BYTECODE empty = {};
		return empty;
	}

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();
	return(d3dShaderByteCode);
}

//정점 셰이더 바이트 코드를 생성(컴파일)한다. 
D3D12_SHADER_BYTECODE CGraphicsCommon::CreateVertexShader(WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob)
{
	return(CGraphicsCommon::CompileShaderFromFile(pszFileName, "main", "vs_5_1", ppd3dShaderBlob));
}

//픽셀 셰이더 바이트 코드를 생성(컴파일)한다. 
D3D12_SHADER_BYTECODE CGraphicsCommon::CreatePixelShader(WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob)
{
	//ex)L"SimplePS.hlsl"
	return(CGraphicsCommon::CompileShaderFromFile(pszFileName, "main", "ps_5_1", ppd3dShaderBlob));
}

//기하 셰이더 바이트 코드를 생성(컴파일)한다. 
D3D12_SHADER_BYTECODE CGraphicsCommon::CreateGeometryShader(WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob)
{

	return(CGraphicsCommon::CompileShaderFromFile(pszFileName, "main", "gs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CGraphicsCommon::CreateComputeShader(WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob)
{
	return(CGraphicsCommon::CompileShaderFromFile(pszFileName, "main", "cs_5_1", ppd3dShaderBlob));
}
