#include "stdafx.h"
#include "Shader.h"
#include "DDSTextureLoader12.h"
#include "Scene.h"


CShader::CShader()
{
	
}

CShader::~CShader()
{
	/*if (m_ppd3dPipelineStates)
	{
		for (int i = 0; i < m_nPipelineStates; i++)
		{
			if (m_ppd3dPipelineStates[i])
			{
				m_ppd3dPipelineStates[i]->Release();
			}
			delete[] m_ppd3dPipelineStates;
		}
	}*/
}

void CShader::BuildObjects(ID3D12Device* pd3dDeivce, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{


}

void CShader::ReleaseObjects()
{

}
void CShader::AnimateObjects(float fTimeElapsed)
{

}

void CShader::ReleaseUploadBuffers()
{

}

void CShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_bIsShadowMap)
	{
		pd3dCommandList->SetPipelineState(m_cpd3dShadowPSO.Get());
	}
	else
	{
		if (!m_bIsWire)
		{
			pd3dCommandList->SetPipelineState(m_cpd3dMainPSO.Get());
		}
		else
		{
			pd3dCommandList->SetPipelineState(m_cpd3dWirePSO.Get());
		}
	}


	UpdateShaderVariables(pd3dCommandList);
}

void CShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender(pd3dCommandList);
}

void CShader::NormalRender(ID3D12GraphicsCommandList* pd3dCommandList)
{

	pd3dCommandList->SetPipelineState(m_cpd3dNormalPSO.Get());

}




void CShader::OnChangePipelineStates()
{

}

void CShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList)
{
}
void CShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4
	* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);
}
void CShader::ReleaseShaderVariables()
{
}

CObjectsShader::CObjectsShader( ) 
{
	int xObjects = 5, yObjects = 5, zObjects = 5, i = 0;
	//x-축, y-축, z-축으로 21개씩 총 21 x 21 x 21 = 9261개의 정육면체를 생성하고 배치한다. 
	m_nObjects = (xObjects * 2 + 1) * (yObjects * 2 + 1) * (zObjects * 2 + 1);
}
CObjectsShader::~CObjectsShader()
{
}



void CObjectsShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * m_nObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObjects->Map(0, NULL, (void**)&m_pcbMappedGameObjects);
}

void CObjectsShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	for (int j = 0; j < m_nObjects; j++)
	{
		CB_GAMEOBJECT_INFO* pbMappedcbGameObject = (CB_GAMEOBJECT_INFO*)((UINT8*)m_pcbMappedGameObjects + (j * ncbElementBytes));
		XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));
	}

	if (m_pMaterial) //값이 NULL 이 아니면 Shader가 Material 갖고있는것.
	{
		m_pMaterial->UpdateShaderVariables(pd3dCommandList);
	}

}
void CObjectsShader::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObjects)
	{
		m_pd3dcbGameObjects->Unmap(0, NULL);
		m_pd3dcbGameObjects->Release();
	}

	//CTexturedShader::ReleaseShaderVariables();
}

void CObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList)
{

	CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Stones.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, 0, 4);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorNextHandle = CScene::GetGPUCbvDescriptorNextHandle();
	CScene::CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects.Get(), ncbElementBytes);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);

	//가로x세로x높이가 12x12x12인 정육면체 메쉬를 생성한다.
	CCubeMeshMain* pCubeMesh = new CCubeMeshMain(pd3dDevice, pd3dCommandList,
		12.0f, 12.0f, 12.0f);
	/*x-축, y-축, z-축 양의 방향의 객체 개수이다. 각 값을 1씩 늘리거나 줄이면서 실행할 때 프레임 레이트가 어떻게
   변하는 가를 살펴보기 바란다.*/
	int xObjects = 5, yObjects = 5, zObjects = 5, i = 0;
	//x-축, y-축, z-축으로 21개씩 총 21 x 21 x 21 = 9261개의 정육면체를 생성하고 배치한다. 
	m_nObjects = (xObjects * 2 + 1) * (yObjects * 2 + 1) * (zObjects * 2 + 1);
	m_ppObjects = new CGameObject * [m_nObjects];
	float fxPitch = 12.0f * 2.5f;
	float fyPitch = 12.0f * 2.5f;
	float fzPitch = 12.0f * 2.5f;

	CRotatingObject* pRotatingObject = NULL;
	for (int x = -xObjects; x <= xObjects; x++)
	{
		for (int y = -yObjects; y <= yObjects; y++)
		{
			for (int z = -zObjects; z <= zObjects; z++)
			{
				pRotatingObject = new CRotatingObject();
				pRotatingObject->SetMesh(pCubeMesh);

				//각 정육면체 객체의 위치를 설정한다.
				pRotatingObject->SetPosition(fxPitch * x, fyPitch * y, fzPitch * z);
				pRotatingObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
				pRotatingObject->SetRotationSpeed(10.0f * (i % 10) + 3.0f);

				//루트 파라미터 설정에 맞게 GameObject의 CBV View를 연결할 View를 연결중
				pRotatingObject->SetCbvGPUDescriptorHandlePtr(d3dCbvGPUDescriptorNextHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i)); //
				pRotatingObject->GetCbvGPUDescriptorHandlePtr();
				m_ppObjects[i++] = pRotatingObject;
			}
		}
	}

}

void CObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++)
		{
			if (m_ppObjects[j]) delete m_ppObjects[j];
		}
		delete[] m_ppObjects;
	}
}

void CObjectsShader::AnimateObjects(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);
	}
}

void CObjectsShader::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) m_ppObjects[j]->ReleaseUploadBuffers();
	}
}

void CObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);
	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Render(pd3dCommandList, pCamera,false , false, false,false);
		}
	}
}

bool CObjectsShader::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
			/*‘F1’ 키를 누르면 1인칭 카메라, ‘F2’ 키를 누르면 스페이스-쉽 카메라로 변경한다, ‘F3’ 키를 누르면 3인칭 카메라
로 변경한다.*/
		case VK_F1:
		case VK_F2:
		case VK_F3:
			break;
			/*case '[' :
				CSceneManager::GetInstance().PopScene();
				break;*/

		case 'A':
			std::cout << "Shader에 성공적으로 입력값을 전달함"<<endl;
			m_ppObjects[m_nObjects-1]->SetPosition(0, 0, 0);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//

 CPlayerShader::CPlayerShader() 
{
	 m_cpd3dMainPSO = CGraphicsCommon::GetInstance().m_cpd3dPlayerPSO;
}


void CPlayerShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	
}

void CPlayerShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);
	/*for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CDefaultShader::CDefaultShader() 
{
	m_cpd3dMainPSO = CGraphicsCommon::GetInstance().m_cpd3dDefaultPSO;
	m_cpd3dNormalPSO = CGraphicsCommon::GetInstance().m_cpd3dDefaultNormalPSO;
	m_cpd3dWirePSO = CGraphicsCommon::GetInstance().m_cpd3dDefaultWirePSO;
	m_cpd3dShadowPSO = CGraphicsCommon::GetInstance().m_cpd3dDefaultShadowPSO;
}

CDefaultShader::~CDefaultShader()
{
}

void CDefaultShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

}
void CDefaultShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CAnimatedShader::CAnimatedShader() 
{
	m_cpd3dMainPSO = CGraphicsCommon::GetInstance().m_cpd3dSkinnedPSO;
	m_cpd3dNormalPSO = CGraphicsCommon::GetInstance().m_cpd3dSkinnedNormalPSO;
	m_cpd3dWirePSO = CGraphicsCommon::GetInstance().m_cpd3dSkinnedWirePSO;
	m_cpd3dShadowPSO = CGraphicsCommon::GetInstance().m_cpd3dSkinnedShadowPSO;
}

CAnimatedShader::~CAnimatedShader()
{
	
}


void CAnimatedShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CAnimatedShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkyBoxShader::CSkyBoxShader( )
{
	m_cpd3dMainPSO = CGraphicsCommon::GetInstance().m_cpd3dSkyBoxPSO;
}

CSkyBoxShader::~CSkyBoxShader()
{

}

void CSkyBoxShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CSkyBoxShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CPlanShader::CPlanShader( )
{
	m_cpd3dMainPSO = CGraphicsCommon::GetInstance().m_cpd3dBasicPSO;
	m_cpd3dNormalPSO = CGraphicsCommon::GetInstance().m_cpd3dBasicNormalPSO;
	m_cpd3dWirePSO = CGraphicsCommon::GetInstance().m_cpd3dBasicWirePSO;
	m_cpd3dShadowPSO = CGraphicsCommon::GetInstance().m_cpd3dBasicShadowPSO;
}

CPlanShader::~CPlanShader()
{

}

void CPlanShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CPlanShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTerrainShader::CTerrainShader()
{
	m_cpd3dMainPSO = CGraphicsCommon::GetInstance().m_cpd3dTerrainPSO;
	m_cpd3dNormalPSO = CGraphicsCommon::GetInstance().m_cpd3dTerrainNormalPSO;
	m_cpd3dWirePSO = CGraphicsCommon::GetInstance().m_cpd3dTerrainWirePSO;
	m_cpd3dShadowPSO = CGraphicsCommon::GetInstance().m_cpd3dTerrainShadowPSO;
}

CTerrainShader::~CTerrainShader()
{

}

void CTerrainShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CTerrainShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
COBBShader::COBBShader()
{
	//혹시 몰라 PSO 통일 시켜두기
	m_cpd3dMainPSO = CGraphicsCommon::GetInstance().m_cpd3dOBBPSO;
	m_cpd3dNormalPSO = CGraphicsCommon::GetInstance().m_cpd3dOBBPSO;
	m_cpd3dWirePSO = CGraphicsCommon::GetInstance().m_cpd3dOBBPSO;
}

COBBShader::~COBBShader()
{

}

void COBBShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void COBBShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CPostProcessingShader::CPostProcessingShader()
{
	m_cpd3dMainPSO = CGraphicsCommon::GetInstance().m_cpd3dPostProcessingPSO;
	//m_cpd3dNormalPSO = CGraphicsCommon::GetInstance().m_cpd3dBasicNormalPSO;
	//m_cpd3dWirePSO = CGraphicsCommon::GetInstance().m_cpd3dBasicWirePSO;
}

CPostProcessingShader::~CPostProcessingShader()
{

}

void CPostProcessingShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CPostProcessingShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CComputeShader::CComputeShader()
{
	m_cpd3dPostPorcessingPSO = CGraphicsCommon::GetInstance().m_cpd3dComputePSO;
	m_cpd3dBlurXPSO = CGraphicsCommon::GetInstance().m_cpd3dComputeBlurXPSO;
	m_cpd3dBlurYPSO = CGraphicsCommon::GetInstance().m_cpd3dComputeBlurYPSO;
	m_cpd3dSSAOPSO = CGraphicsCommon::GetInstance().m_cpd3dComputeSSAOPSO;
	m_cpd3dBloomPSO = CGraphicsCommon::GetInstance().m_cpd3dComputeBLoomPSO;
	m_cpd3dCombinePSO = CGraphicsCommon::GetInstance().m_cpd3dComputeCombinePSO;
	m_cpd3dClusterPSO = CGraphicsCommon::GetInstance().m_cpd3dComputeClusterPSO;
}

void CComputeShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CComputeShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender(pd3dCommandList);
}

void CComputeShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{

	pd3dCommandList->SetPipelineState(m_cpd3dPostPorcessingPSO.Get());
}

void CComputeShader::OnPreparePSO(ID3D12GraphicsCommandList* pd3dCommandList, UINT num)
{
	switch (num)
	{
	case 0:
	{
		pd3dCommandList->SetPipelineState(m_cpd3dPostPorcessingPSO.Get());
		break;
	}
	case 1:
	{
		pd3dCommandList->SetPipelineState(m_cpd3dBlurXPSO.Get());
		break;
	}
	case 2:
	{
		pd3dCommandList->SetPipelineState(m_cpd3dBlurYPSO.Get());
		break;
	}
	case 3:
	{
		pd3dCommandList->SetPipelineState(m_cpd3dSSAOPSO.Get());
		break;
	}

	case 4:
	{
		pd3dCommandList->SetPipelineState(m_cpd3dBloomPSO.Get());
		break;
	}

	case 5:
	{
		pd3dCommandList->SetPipelineState(m_cpd3dCombinePSO.Get());
		break;
	}
	case 6:
	{
		pd3dCommandList->SetPipelineState(m_cpd3dClusterPSO.Get());
		break;
	}
	default :
	{
		break;
	}

	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CParticleShader::CParticleShader()
{
	m_cpd3dMainPSO = CGraphicsCommon::GetInstance().m_cpd3dParticlePSO;
	//m_cpd3dNormalPSO = CGraphicsCommon::GetInstance().m_cpd3dBasicNormalPSO;
	//m_cpd3dWirePSO = CGraphicsCommon::GetInstance().m_cpd3dBasicWirePSO;
	//m_cpd3dShadowPSO = CGraphicsCommon::GetInstance().m_cpd3dBasicShadowPSO;
}

CParticleShader::~CParticleShader()
{

}

void CParticleShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CParticleShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);
}
