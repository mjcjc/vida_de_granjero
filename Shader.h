#pragma once

#include "GameObject.h"
#include "Camera.h"
#include "GraphicsCommon.h";

//게임 객체의 정보를 Shader 에 넘겨주기 위한 구조체(상수 버퍼)이다. 
//struct CB_GAMEOBJECT_INFO
//{
//	XMFLOAT4X4 m_xmf4x4World;
//};

class CShader
{
public:
	CShader();
	virtual ~CShader();
private:
	int m_nReferences = 0;

public:
	bool m_bIsWire = false;
	bool m_bIsShadowMap = false;
public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }


	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext = NULL);

	virtual  void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void NormalRender(ID3D12GraphicsCommandList* pd3dCommandList); //

	virtual void OnChangePipelineStates();

	virtual void SetMainPSO(ComPtr<ID3D12PipelineState> pso) { m_cpd3dMainPSO = pso; };
	virtual void SetNormalPSO(ComPtr<ID3D12PipelineState> pso) { m_cpd3dNormalPSO = pso; };
	virtual void SetWirePSO(ComPtr<ID3D12PipelineState> pso) { m_cpd3dWirePSO = pso; };

	void SetWireRender(bool bIs) { m_bIsWire = bIs; };
	void SetShadowMapRender(bool bIS) { m_bIsShadowMap = bIS; }
	//테스트 용도로 추가
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) { return false; };
protected:

	ComPtr<ID3D12PipelineState> m_cpd3dMainPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dNormalPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dWirePSO;
	ComPtr<ID3D12PipelineState> m_cpd3dShadowPSO;

};

class CComputeShader : public CShader
{
public:
	CComputeShader();
	virtual ~CComputeShader() {};

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	void OnPreparePSO(ID3D12GraphicsCommandList* pd3dCommandList,UINT num);


protected:
	ComPtr<ID3D12PipelineState> m_cpd3dPostPorcessingPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dBlurXPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dBlurYPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dSSAOPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dBloomPSO;
	ComPtr<ID3D12PipelineState> m_cpd3dCombinePSO;
	ComPtr<ID3D12PipelineState> m_cpd3dClusterPSO;

};
class CObjectsShader : public CShader
{
public:
	CObjectsShader();
	virtual ~CObjectsShader();
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();


	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	virtual int GetNumberOfObjects() { return m_nObjects; };

	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
protected:
	CGameObject** m_ppObjects = NULL;
	int m_nObjects = 0;

	CMaterial* m_pMaterial = NULL; //임시로 넣어둔거

	ComPtr<ID3D12Resource> m_pd3dcbGameObjects = NULL;
	CB_GAMEOBJECT_INFO* m_pcbMappedGameObjects = NULL;
};



class CPlayerShader : public CObjectsShader
{
public:
	CPlayerShader();
	virtual ~CPlayerShader() {};

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

};

class CDefaultShader : public CShader
{
public:
	CDefaultShader();
	virtual ~CDefaultShader();

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

class CAnimatedShader : public CDefaultShader
{

public:
	CAnimatedShader();
	virtual ~CAnimatedShader();

public:

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
private:

};

class CSkyBoxShader : public CShader
{

public:
	CSkyBoxShader();
	virtual ~CSkyBoxShader();

public:


	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
private:

};

class CPlanShader : public CShader
{

public:
	CPlanShader();
	virtual ~CPlanShader(); 

public:
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

private:

};

class CTerrainShader : public CShader
{

public:
	CTerrainShader();
	virtual ~CTerrainShader();

public:
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

private:

};

class COBBShader : public CShader
{
public:
	COBBShader();
	virtual ~COBBShader();

public:
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

/////////////////////////////////////////////////////////////

class CPostProcessingShader : public CShader
{

public:
	CPostProcessingShader();
	virtual ~CPostProcessingShader();

public:
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

private:

};

/////////////////////////////////////////////////////////////

class CParticleShader : public CShader
{
public:
	CParticleShader();
	virtual ~CParticleShader();

public:
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

private:

};