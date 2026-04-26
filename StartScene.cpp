#include "stdafx.h"
#include "StartScene.h"

void CStartScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CScene::BuildObjects(pd3dDevice, pd3dCommandList);
}

bool CStartScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//마우스 캡쳐를 하고 현재 마우스 위치를 가져온다
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
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

	return false;
}

bool CStartScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	CScene* pscene = nullptr;
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_F1:
		case VK_F2:
		case VK_F3:
			break;
		case 'P':
			std::cout << "CStartScene에서 MainScene으로 이동"<<endl;
			pscene = new CMainScene(m_hWnd, m_pGameTimer, std::move(m_upClientNetwork));
			CSceneManager::GetInstance().PushScene(pscene);
			CSceneManager::GetInstance().SetSceneNumber(1);
			CSceneManager::GetInstance().NotifySceneStuck(1);
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

bool CStartScene::ProcessInput()
{
	
	return false;
}

void CStartScene::ReleaseObjects()
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

void CStartScene::ReleaseUploadBuffers()
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

void CStartScene::OnGUI(ImGuiIO* pGuiIO)
{
	CScene::OnGUI(pGuiIO);
}

//그림자를 위한 처리를 할 것이다
void CStartScene::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, CCamera* pCamera)
{

	CScene::OnPreRender(pd3dCommandList, nRenderTargets, pd3dRtvCPUHandles, pd3dDsvCPUHandle, pCamera);
	pd3dCommandList->OMSetRenderTargets(1, pd3dRtvCPUHandles, TRUE, pd3dDsvCPUHandle);

	//if (pCamera)
	//{
	//	for (int i = 0; i < MAX_LIGHTS; i++)
	//	{
	//		//TODO 여기다가 조명 Shader 처리를 한번 에 쫙 해야한다 
	//	}
	//}
}
void CStartScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene::Render(pd3dCommandList, pCamera);
}