#pragma once
class CGraphicsCommon
{

private:
	CGraphicsCommon() {};
	virtual ~CGraphicsCommon() {};
public:

	CGraphicsCommon(const CGraphicsCommon&) = delete;
	CGraphicsCommon& operator=(const CGraphicsCommon&) = delete;

	static CGraphicsCommon& GetInstance()
	{
		static CGraphicsCommon instance;
		return instance;
	}

	//PSO
	ComPtr<ID3D12PipelineState> m_cpd3dBasicPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dBasicNormalPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dBasicWirePSO;
	ComPtr<ID3D12PipelineState> m_cpd3dBasicShadowPSO;

	ComPtr<ID3D12PipelineState> m_cpd3dDefaultPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dDefaultNormalPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dDefaultWirePSO;
	ComPtr<ID3D12PipelineState> m_cpd3dDefaultShadowPSO;

	ComPtr<ID3D12PipelineState> m_cpd3dSkinnedPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dSkinnedNormalPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dSkinnedWirePSO;
	ComPtr<ID3D12PipelineState> m_cpd3dSkinnedShadowPSO;

	ComPtr<ID3D12PipelineState> m_cpd3dSkyBoxPSO;

	ComPtr<ID3D12PipelineState> m_cpd3dPlayerPSO;

	ComPtr<ID3D12PipelineState> m_cpd3dOBBPSO;

	ComPtr<ID3D12PipelineState> m_cpd3dPostProcessingPSO;

	ComPtr<ID3D12PipelineState> m_cpd3dTerrainPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dTerrainWirePSO;
	ComPtr<ID3D12PipelineState> m_cpd3dTerrainNormalPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dTerrainShadowPSO;

	ComPtr<ID3D12PipelineState> m_cpd3dParticlePSO;

	//ComputePSO
	ComPtr<ID3D12PipelineState> m_cpd3dComputePSO;
	ComPtr<ID3D12PipelineState> m_cpd3dComputeBlurXPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dComputeBlurYPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dComputeSSAOPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dComputeCombinePSO;
	ComPtr<ID3D12PipelineState> m_cpd3dComputeBLoomPSO;
	ComPtr< ID3D12PipelineState> m_cpd3dComputeClusterPSO;

	//RootSignature
	ComPtr<ID3D12RootSignature> m_cpd3dStartSceneRootSignature;
	ComPtr<ID3D12RootSignature> m_cpd3dMainSceneRootSignature;
	ComPtr<ID3D12RootSignature> m_cpd3dMainSceneComputeRootSignature;

	void InitCommonStates(ID3D12Device* device);

protected:


	// 내부적으로 InitCommonStates()에서 사용
	void InitInputLayout(ID3D12Device* pd3dDevice);
	void InitRasterizerStates(ID3D12Device* pd3dDevice);
	void InitBlendStates(ID3D12Device* pd3dDevice);
	void InitDepthStencilStates(ID3D12Device* pd3dDevice);
	void InitRootSignature(ID3D12Device* pd3dDevice);
	void InitComputeRootSignature(ID3D12Device* pd3dDevice);
	void InitRtvDsvFormat(ID3D12Device* pd3dDevice);
	void InitPipelineStates(ID3D12Device* pd3dDevice);
	void InitComputePipelineStates(ID3D12Device* pd3dDevice);

	//INPUT LAYOUT
	D3D12_INPUT_LAYOUT_DESC m_d3dBasicInput;
	D3D12_INPUT_LAYOUT_DESC m_d3dDefaultInput;
	D3D12_INPUT_LAYOUT_DESC m_d3dSkinnedInput;
	D3D12_INPUT_LAYOUT_DESC m_d3dSkyBoxInput;
	D3D12_INPUT_LAYOUT_DESC m_d3dPostPorcessingInput;
	D3D12_INPUT_LAYOUT_DESC m_d3dTerrainInput;
	D3D12_INPUT_LAYOUT_DESC m_d3dParticleInput;


	D3D12_INPUT_LAYOUT_DESC m_d3dPlayerInput;

	//RASTERIZER
	D3D12_RASTERIZER_DESC m_d3dDefaultRasterize;
	D3D12_RASTERIZER_DESC m_d3dWireRasterize;
	D3D12_RASTERIZER_DESC m_d3dShadowRasterize;

	//BELEND
	D3D12_BLEND_DESC m_d3dDefaultBlend;
	D3D12_BLEND_DESC m_d3dAlphaBlend;

	//DEPTH
	D3D12_DEPTH_STENCIL_DESC m_d3dDefaultDepth;
	D3D12_DEPTH_STENCIL_DESC m_d3dDoneDepth;

	//RTV, DSV Format
	DXGI_FORMAT m_dxgiRtvFormatHDR;
	DXGI_FORMAT m_dxgiRtvFormatLDR;
	DXGI_FORMAT m_dxgiRtvFormatUnknown;

	DXGI_FORMAT m_dxgiDsvFormatShadowMap;
	DXGI_FORMAT m_dxgiDsvFormatDefualt;
	DXGI_FORMAT m_dxgiDsvFormatUnknown;

	//Scene 에 있던거
	//ComPtr<ID3D12RootSignature> CreateGraphicsRootSignature(ID3D12Device* pd3dDevice); //루트 시그니쳐를 생성해주고 반환
	//ComPtr<ID3D12RootSignature> GetGraphicsRootSignature(); //저장하고 있는 루트 시그니쳐를 반환

	//Shader 에 있던거
	D3D12_SHADER_BYTECODE CreateVertexShader(WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE CreatePixelShader(WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE CreateGeometryShader(WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE CreateComputeShader(WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob);

	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);

	virtual ComPtr<ID3D12PipelineState> CreatePSO(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature,
		WCHAR* pszVSFileName, WCHAR* pszPSFileName, D3D12_RASTERIZER_DESC raster, D3D12_BLEND_DESC blend,
		D3D12_DEPTH_STENCIL_DESC depthStencil, D3D12_INPUT_LAYOUT_DESC inputlayout, DXGI_FORMAT dxgiRtvFormat, DXGI_FORMAT dxgiDsvFormat, UINT nRenderTargets);

	//임시로 달아둔 함수
	virtual ComPtr<ID3D12PipelineState> CreateNormalPSO(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature, WCHAR* pszVSFileName, WCHAR* pszGSFileName, WCHAR* pszPSFileName,
		D3D12_RASTERIZER_DESC raster, D3D12_BLEND_DESC blend, D3D12_DEPTH_STENCIL_DESC depthStencil, D3D12_INPUT_LAYOUT_DESC inputlayout, DXGI_FORMAT dxgiRtvFormat);

	virtual ComPtr<ID3D12PipelineState> CreateComputePSO(
		ID3D12Device* pd3dDevice,
		ID3D12RootSignature* pd3dRootSignature,
		WCHAR* pszCSFileName
	);
};

