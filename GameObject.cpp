#include "stdafx.h"
#include "GameObject.h"
#include "Shader.h"
#include "Scene.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//CTexture 
CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers, int nRootParameters)
{
	m_nTextureType = nTextureType; //Type 지정

	m_nTextures = nTextures; //
	if (m_nTextures > 0)
	{
		m_ppd3dTextureUploadBuffers = new ComPtr<ID3D12Resource>  [m_nTextures];
		m_ppd3dTextures = new ComPtr<ID3D12Resource>[m_nTextures];
		for (int i = 0; i < m_nTextures; i++)
		{
			//m_ppd3dTextureUploadBuffers[i]= m_ppd3dTextures[i] = NULL;
		}
		//Texture 이름 지정
		m_ppstrTextureNames = new _TCHAR[m_nTextures][64];

		for (int i = 0; i < m_nTextures; i++)
		{
			m_ppstrTextureNames[i][0] = '\0';
		}
		m_pd3dSrvGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nTextures];
		m_pd3dUavGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nTextures];
		for (int i = 0; i < m_nTextures; i++)
		{
			m_pd3dSrvGpuDescriptorHandles[i].ptr = NULL;
		}
		m_pnResourceTypes = new UINT[m_nTextures];
		m_pdxgiBufferFormats = new DXGI_FORMAT[m_nTextures];
		m_pnBufferElements = new int[m_nTextures];
	}
	m_nRootParameters = nRootParameters;
	if (nRootParameters > 0)
	{
		m_pnRootParameterIndices = new int[nRootParameters];
		m_pnComputeRootParameterIndices = new int[nRootParameters];
	}
	for (int i = 0; i < m_nRootParameters; i++)
	{
		m_pnRootParameterIndices[i] = -1;
		m_pnComputeRootParameterIndices[i] = -1;
	}

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0)
	{
		m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
	}	
}

CTexture::~CTexture()
{
	if (m_ppd3dTextures)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextures[i]) m_ppd3dTextures[i]->Release();
		delete[] m_ppd3dTextures;
	}

	if (m_ppstrTextureNames) delete[] m_ppstrTextureNames;

	if (m_pnResourceTypes) delete[] m_pnResourceTypes;
	if (m_pdxgiBufferFormats) delete[] m_pdxgiBufferFormats;
	if (m_pnBufferElements) delete[] m_pnBufferElements;

	if (m_pnRootParameterIndices) delete[] m_pnRootParameterIndices;
	if (m_pd3dSrvGpuDescriptorHandles) delete[] m_pd3dSrvGpuDescriptorHandles;

	if (m_pd3dSamplerGpuDescriptorHandles) delete[] m_pd3dSamplerGpuDescriptorHandles;
}

ComPtr<ID3D12Resource> CTexture::CreateTexture(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue, UINT nResourceType, UINT nIndex)
{
	m_pnResourceTypes[nIndex] = nResourceType;
	m_ppd3dTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, nWidth, nHeight, 1, 0, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_ppd3dTextures[nIndex]);
}

//어느 루트 파라미터 인덱스에 Texture를 연결할지 지정.
void CTexture::SetRootParameterIndex(int nIndex, UINT nRootParameterIndex)
{
	m_pnRootParameterIndices[nIndex] = nRootParameterIndex;
}
void CTexture::SetComputeRootParameterIndex(int nIndex, UINT nRootParameterIndex)
{
	m_pnComputeRootParameterIndices[nIndex] = nRootParameterIndex;
}
//디스크립터 힙에 어느 위치에 Texture가 저장되어있는지 지정.
void CTexture::SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pd3dSrvGpuDescriptorHandles[nIndex] = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetUavGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dUavGpuDescriptorHandle)
{
	m_pd3dUavGpuDescriptorHandles[nIndex] = d3dUavGpuDescriptorHandle;
}

void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_pd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

//Array로 관리되거나 여러개의 Texture를 사용하는 GameObject에 경우
//구조와 순서가 사전 정의된 순서로 나열되어있을 것임으로 그러한 경우에 사용하는 update 함수
void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//SRV를  View바인딩 방식으로 루트 파라미터에 직접 바인딩하는 구조와 거의 흡사
	//SetGraphicsRootDescriptorTable은 디스크립터 내부에 정의되어있는 View에 따라
	//한 번에 여러개 접근이 가능한데 if 조건문은 루트 파라미터에 디스크립터 테이블 정의가
	//정확히 1개의 view로 설정되어있는 경우에 한해서 작동하는 것이기 때문.
	if (m_nRootParameters == m_nTextures) 
	{
		for (int i = 0; i < m_nRootParameters; i++)
		{
			if (m_pd3dSrvGpuDescriptorHandles[i].ptr && (m_pnRootParameterIndices[i] != -1))
			{
				pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[i], m_pd3dSrvGpuDescriptorHandles[i]);
			}
		}
	}
	else
	{
		if (m_pd3dSrvGpuDescriptorHandles[0].ptr) // 
		{
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[0], m_pd3dSrvGpuDescriptorHandles[0]);
		}
	}
}

void CTexture::UpdateComputeShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList , bool bIsSrvActive)
{
	if (bIsSrvActive)
	{
		if (m_nRootParameters == m_nTextures)
		{
			for (int i = 0; i < m_nRootParameters; i++)
			{
				if (m_pd3dSrvGpuDescriptorHandles[i].ptr &&
					(m_pnComputeRootParameterIndices[i] != -1))
				{
					pd3dCommandList->SetComputeRootDescriptorTable(
						m_pnRootParameterIndices[i],
						m_pd3dSrvGpuDescriptorHandles[i]
					);
				}
			}
		}
		else
		{
			if (m_pd3dSrvGpuDescriptorHandles[0].ptr)
			{
				pd3dCommandList->SetComputeRootDescriptorTable(
					m_pnRootParameterIndices[0],
					m_pd3dSrvGpuDescriptorHandles[0]
				);
			}
		}
	}
	// UAV로 바인딩하는 경우
	else
	{
		if (m_nRootParameters == m_nTextures)
		{
			for (int i = 0; i < m_nRootParameters; i++)
			{
				if (m_pd3dUavGpuDescriptorHandles[i].ptr &&
					(m_pnComputeRootParameterIndices[i] != -1))
				{
					pd3dCommandList->SetComputeRootDescriptorTable(
						m_pnComputeRootParameterIndices[i],
						m_pd3dUavGpuDescriptorHandles[i]
					);
				}
			}
		}
		else
		{
			if (m_pd3dUavGpuDescriptorHandles[0].ptr)
			{
				pd3dCommandList->SetComputeRootDescriptorTable(
					m_pnComputeRootParameterIndices[0],
					m_pd3dUavGpuDescriptorHandles[0]
				);
			}
		}
	}
}

//사용하는 GameObject가 사용하기위한 목적대로 Texture 배열이 나열이 되어있지 않다면
//직접 인덱스에 접근해서 바인딩해줘야하며 직접 하나하나 접근하기 위한 부분
//세밀한 조정은 이 함수를 통해서 사용해야한다.
void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex)
{
	/*
	pd3dCommandList->SetGraphicsRoot32BitConstant(5, nTextureIndex, 0);
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[nParameterIndex], m_pd3dSrvGpuDescriptorHandles[nTextureIndex]);

	if (nTextureIndex == 1)
		pd3dCommandList->SetGraphicsRootDescriptorTable(4, m_pd3dSrvGpuDescriptorHandles[3]);
		*/
}

void CTexture::ReleaseShaderVariables()
{
}

void CTexture::ReleaseUploadBuffers()
{
	if (m_ppd3dTextureUploadBuffers)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextureUploadBuffers[i]) m_ppd3dTextureUploadBuffers[i]->Release();
		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = NULL;
	}
}

void CTexture::LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex)
{
	m_pnResourceTypes[nIndex] = nResourceType;
	m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromDDSFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_GENERIC_READ/*D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE*/);
}

void CTexture::LoadTextureFromWICFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex)
{
	m_pnResourceTypes[nIndex] = nResourceType;
	m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromWICFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_GENERIC_READ/*D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE*/);
}

//ShaderResourceView의 Desc를 받아오는 함수
D3D12_SHADER_RESOURCE_VIEW_DESC CTexture::GetShaderResourceViewDesc(int nIndex)
{
	ComPtr<ID3D12Resource> pShaderResource = GetResource(nIndex);
	D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	int nTextureType = GetTextureType(nIndex);
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.Format = m_pdxgiBufferFormats[nIndex];
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = m_pnBufferElements[nIndex];
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////
//CMaterial
CMaterial::CMaterial()
{
}

CMaterial::CMaterial(int nTextures)
{
	m_nTextures = nTextures; 
	m_ppTextures = new CTexture * [m_nTextures];
	m_ppstrTextureNames = new _TCHAR[m_nTextures][64];

	for (int i = 0; i < m_nTextures; i++)
	{
		m_ppTextures[i] = NULL;
	}

	for (int i = 0; i < m_nTextures; i++)
	{
		m_ppstrTextureNames[i][0] = '\0';
	}
}

CMaterial::~CMaterial()
{
	if (m_pTexture)
	{
		m_pTexture->Release();
	}
	if (m_pShader)
	{
		m_pShader->Release();
	}
}

int CMaterial::LoadTextureDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, CTexture** ppTexture, CGameObject* pParent, FILE* pInFile, CShader* pShader)
{
	int count = 0;

	char pstrTextureName[64] = { '\0' };

	BYTE nStrLength = 64;
	UINT nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrTextureName, sizeof(char), nStrLength, pInFile);
	pstrTextureName[nStrLength] = '\0';

	bool bDuplicated = false;
	if (strcmp(pstrTextureName, "null")) //null 아닌 경우에
	{
		SetMaterialType(nType);

		char pstrFilePath[64] = { '\0' };
		strcpy_s(pstrFilePath, 64, "Model/Textures/");

		//중복 텍스쳐 확인
		bDuplicated = (pstrTextureName[0] == '@');
		strcpy_s(pstrFilePath + 15, 64 - 15, (bDuplicated) ? (pstrTextureName + 1) : pstrTextureName);
		strcpy_s(pstrFilePath + 15 + ((bDuplicated) ? (nStrLength - 1) : nStrLength), 64 - 15 - ((bDuplicated) ? (nStrLength - 1) : nStrLength), ".dds");

		size_t nConverted = 0;
		mbstowcs_s(&nConverted, pwstrTextureName, 64, pstrFilePath, _TRUNCATE);

		//#define _WITH_DISPLAY_TEXTURE_NAME

#ifdef _WITH_DISPLAY_TEXTURE_NAME
		static int nTextures = 0, nRepeatedTextures = 0;
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("Texture Name: %d %c %s\n"), (pstrTextureName[0] == '@') ? nRepeatedTextures++ : nTextures++, (pstrTextureName[0] == '@') ? '@' : ' ', pwstrTextureName);
		OutputDebugString(pstrDebug);
#endif

		if (!bDuplicated)
		{
			count++;
		}
		else
		{
			//중복된 경우
			if (pParent)
			{
				while (pParent)
				{
					if (!pParent->m_pParent) break;
					pParent = pParent->m_pParent;
				}
			
			}
		}
	}
	return count;
}


void CMaterial::LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, CTexture** ppTexture, CGameObject* pParent, FILE* pInFile, CShader* pShader)
{
	char pstrTextureName[64] = { '\0' };

	BYTE nStrLength = 64;
	UINT nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrTextureName, sizeof(char), nStrLength, pInFile);
	pstrTextureName[nStrLength] = '\0';

	bool bDuplicated = false;
	if (strcmp(pstrTextureName, "null")) //null 아닌 경우에
	{
		SetMaterialType(nType);

		char pstrFilePath[64] = { '\0' };
		strcpy_s(pstrFilePath, 64, "Model/Textures/");

		//중복 텍스쳐 확인
		bDuplicated = (pstrTextureName[0] == '@');
		strcpy_s(pstrFilePath + 15, 64 - 15, (bDuplicated) ? (pstrTextureName + 1) : pstrTextureName);
		strcpy_s(pstrFilePath + 15 + ((bDuplicated) ? (nStrLength - 1) : nStrLength), 64 - 15 - ((bDuplicated) ? (nStrLength - 1) : nStrLength), ".dds");

		size_t nConverted = 0;
		mbstowcs_s(&nConverted, pwstrTextureName, 64, pstrFilePath, _TRUNCATE);

		//#define _WITH_DISPLAY_TEXTURE_NAME

#ifdef _WITH_DISPLAY_TEXTURE_NAME
		static int nTextures = 0, nRepeatedTextures = 0;
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("Texture Name: %d %c %s\n"), (pstrTextureName[0] == '@') ? nRepeatedTextures++ : nTextures++, (pstrTextureName[0] == '@') ? '@' : ' ', pwstrTextureName);
		OutputDebugString(pstrDebug);
#endif
	
		if (!bDuplicated)
		{
			//std::wcout << L"Texture Path: " << pwstrTextureName << std::endl;
			*ppTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
			(*ppTexture)->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, pwstrTextureName, RESOURCE_TEXTURE2D, 0);
			if (*ppTexture) (*ppTexture)->AddRef();

			CScene::CreateShaderResourceViews(pd3dDevice, *ppTexture, 0, nRootParameter);
		}
		else
		{
			//중복된 경우
			if (pParent)
			{
				while (pParent)
				{
					if (!pParent->m_pParent) break;
					pParent = pParent->m_pParent;
				}
				CGameObject* pRootGameObject = pParent; //
				*ppTexture = pRootGameObject->FindReplicatedTexture(pwstrTextureName);
				if (*ppTexture) (*ppTexture)->AddRef();
			}
		}
	}
}


void CMaterial::SetTexture(CTexture* pTexture)
{
	if (m_pTexture)
	{
		m_pTexture->Release();
	}
	m_pTexture = pTexture;
	if (m_pTexture)
	{
		m_pTexture->AddRef();
	}
}

void CMaterial::SetShader(CShader* pShader)
{
	if (m_pShader)
	{
		m_pShader->Release();
	}
	m_pShader = pShader;
	if (m_pShader)
	{
		m_pShader->AddRef();
	}
}

CShader* CMaterial::m_pSkinnedAnimationShader = NULL;
CShader* CMaterial::m_pStandardShader = NULL;

void CMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_pStandardShader = new CDefaultShader();
	//m_pStandardShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pSkinnedAnimationShader = new CAnimatedShader();
	//m_pSkinnedAnimationShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}


void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pTexture) //이건 이전 계층 구조가 아닌 경우에 대한 Texture
	{
		//m_pTexture 하나에 여러 Texture가 존재 할 순 있따
		m_pTexture->UpdateShaderVariables(pd3dCommandList);
	}

//
//////////////////////////////////////////////////////////////////////////////////////////
//

	//pd3dCommandList->SetGraphicsRoot32BitConstants(0, 4, &m_xmf4AmbientColor, 16);
	//pd3dCommandList->SetGraphicsRoot32BitConstants(0, 4, &m_xmf4AlbedoColor, 20);
	//pd3dCommandList->SetGraphicsRoot32BitConstants(0, 4, &m_xmf4SpecularColor, 24);
	//pd3dCommandList->SetGraphicsRoot32BitConstants(0, 4, &m_xmf4EmissiveColor, 28);
	//pd3dCommandList->SetGraphicsRoot32BitConstants(0, 1, &m_fRoughness, 32);
	//pd3dCommandList->SetGraphicsRoot32BitConstants(0, 1, &m_fSmoothness, 33);
	//pd3dCommandList->SetGraphicsRoot32BitConstants(0, 1, &m_fMetallic, 34);
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 1, &m_nType, 16);


	for (int i = 0; i < m_nTextures; i++)
	{
		//개별 Texture 혹은 계층 구조에 따른 여러 종류 전부다 구별해서 넣어줄 수 있다.
		//우선 개별 관리로 가고있다.
		//이렇게 하면 위에 것과 달리 Texture가 하나가 없거나 비어있어도 따로 관리
		//해줄 수 있어 좀 더 변환이 용이하다.
		if (m_ppTextures[i])
		{
			m_ppTextures[i]->UpdateShaderVariables(pd3dCommandList);
		}
	}

}

void CMaterial::ReleaseShaderVariables()
{
	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
	}
	if (m_pTexture)
	{
		m_pTexture->ReleaseShaderVariables();
	}
}

void CMaterial::ReleaseUploadBuffers()
{
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CLoadedModelInfo::~CLoadedModelInfo()
{
	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;
}

void CLoadedModelInfo::PrepareSkinning()
{
	int nSkinnedMesh = 0;
	m_ppSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];
	m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes, &nSkinnedMesh);

	for (int i = 0; i < m_nSkinnedMeshes; i++) m_ppSkinnedMeshes[i]->PrepareSkinning(m_pModelRootObject);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject()
{
	/*
	* 와.....진짜 까먹으면 안된다 초기화
	*/
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity()); // DirectX Math를 이용해 m_xmf4x4World를 기본행렬로 초기화 해주는 중. 
	XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixIdentity());

	//아무 계산도 못하는 상태로 처음엔 초기화
	m_obbBox.Center = XMFLOAT3(0, 0, 0);
	m_obbBox.Extents = XMFLOAT3(0, 0, 0);
	m_obbBox.Orientation = XMFLOAT4(0, 0, 0, 0);


	//우선 obb가 있든 없든 출력해주는 Shader니까 설정해준다. 나중에 개선 해야할듯하다
	m_pOBBShader = new COBBShader();

	m_d3dCbvGPUDescriptorHandle.ptr = NULL;
}

CGameObject::CGameObject(int nMaterials) : CGameObject()
{
	m_nMaterials = nMaterials;
	if (m_nMaterials > 0)
	{
		m_ppMaterials = new CMaterial * [m_nMaterials];
		for (int i = 0; i < m_nMaterials; i++)
		{
			m_ppMaterials[i] = NULL;
		}
	}
}


CGameObject::~CGameObject()
{
	if (m_pMesh)
	{
		m_pMesh->Release();
	}

	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
		m_pShader->Release();
	}
}

 
void CGameObject::ResetState()
{
	m_bIsActive = false;
	m_Tag = EObjectTag::None;
	m_nID = 0;

	m_nAnimation = 0;
	m_fAnimationTime = 0.f;

	XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixIdentity());
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());

	m_pParent = nullptr;
	m_pChild = nullptr;
	m_pSibling = nullptr;

	m_nReferences = 1; // 재사용을 위한 초기화
}


void CGameObject::SetShader(CShader* pShader) //이건 오브젝트가 Shader를 갖는구조에서 사용하는거고 현재 07/09 기준으로는 Shader가 Object를 갖고있는 구조라서 SetShader 를 쓰지 않았다
{
	if (m_pShader)
	{
		m_pShader->Release();
	}
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CGameObject::SetMesh(CMesh* pMesh)
{
	if (m_pMesh)
	{
		m_pMesh->Release();
	}

	m_pMesh = pMesh;

	if (m_pMesh)
	{
		m_pMesh->AddRef();
	}

}

void CGameObject::SetMaterial(CMaterial* pMaterial)
{
	if (m_pMaterial)
	{
		m_pMaterial->Release();
	}
	m_pMaterial = pMaterial;

	if (m_pMaterial)
	{
		m_pMaterial->AddRef();
	}
}

void CGameObject::SetMaterial(int nMaterial, CMaterial* pMaterial)
{
	if (m_ppMaterials[nMaterial])
	{
		m_ppMaterials[nMaterial]->Release();
	}
	m_ppMaterials[nMaterial] = pMaterial;

	if (m_ppMaterials[nMaterial])
	{
		m_ppMaterials[nMaterial]->AddRef();
	}
}

void CGameObject::ReleaseUploadBuffers()
{
	//정점 버퍼를 실제로 갖고있는 객체는 Mesh임으로 여기선 Mesh의 ReleaseUploadBuffers를 호출해 제거해주는 것.
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
}
/*
* 움직임 제어
*/

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4ToParent._41 = x;
	m_xmf4x4ToParent._42 = y;
	m_xmf4x4ToParent._43 = z;

	UpdateTransform(NULL);
}
void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}


void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetToParentPosition()
{
	return(XMFLOAT3(m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43));
}

//게임 객체의 로컬 z-축 벡터를 반환한다.
XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32,
		m_xmf4x4World._33)));
}
//게임 객체의 로컬 y-축 벡터를 반환한다.
XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22,
		m_xmf4x4World._23)));
}
//게임 객체의 로컬 x-축 벡터를 반환한다.
XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12,
		m_xmf4x4World._13)));
}
//게임 객체를 로컬 x-축 방향으로 이동한다.
void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
//게임 객체를 로컬 y-축 방향으로 이동한다.
void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
//게임 객체를 로컬 z-축 방향으로 이동한다.
void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
//게임 객체를 월드 좌표 기준 방향으로 움직이게 한다.
void CGameObject::Move(XMFLOAT3 xmf3Offset)
{
	m_xmf4x4ToParent._41 += xmf3Offset.x;
	m_xmf4x4ToParent._42 += xmf3Offset.y;
	m_xmf4x4ToParent._43 += xmf3Offset.z;

	UpdateTransform(NULL);
}
//게임 객체를 주어진 각도로 회전한다.
void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch),
		XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void CGameObject::SetRotation(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX cur = XMLoadFloat4x4(&m_xmf4x4ToParent);

	XMVECTOR pos = cur.r[3];

	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(fPitch),
		XMConvertToRadians(fYaw),
		XMConvertToRadians(fRoll));
	mtxRotate.r[3] = pos;

	XMStoreFloat4x4(&m_xmf4x4ToParent, mtxRotate);

	UpdateTransform(NULL);
}

void CGameObject::SetChild(CGameObject* pChild, bool bReferenceUpdate)
{
	if (pChild) //값이 존재한다면 자식으로 추가
	{
		pChild->m_pParent = this; 
		if (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild) //현재 자식이 이미 존재 한다면
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling; //형제로 추가해준다.
		m_pChild->m_pSibling = pChild; //재귀 순회가 간단하고 가변으로 설정이 용이함으로
		//Link와 같은 컨테이너 방식을 채택하는게 게임에선 좋은 접근법에 속한다.
	}
	else
	{
		m_pChild = pChild;
	}
}


int CGameObject::LoadCountTexturesFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader)
{

	int count = 0;

	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	CLoadedModelInfo* pLoadedModel = new CLoadedModelInfo();

	char pstrToken[256] = { '\0' };

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:")) // 계층 구조가 존재하면
			{
				count = CGameObject::LoadFrameHierarchyDummyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader, &pLoadedModel->m_nSkinnedMeshes);
				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
			}
			else if (!strcmp(pstrToken, "<Animation>:")) //Animation이 있다면?
			{
				////애니메이션 정보를 읽어온다.
				//CGameObject::LoadAnimationFromFile(pInFile, pLoadedModel);
				//pLoadedModel->PrepareSkinning(); //SkinnedMesh를 적용할 준비
			}
			else if (!strcmp(pstrToken, "</Animation>:"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	delete pLoadedModel;
	return count;
}


//Mesh 정보와 Animation Clip 정보를 파일로 부터 읽어온다
CLoadedModelInfo* CGameObject::LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader, EObjectModelType objectType)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	//읽어온 정보를 저장하고 관리해줄 수 있는 객체
	CLoadedModelInfo* pLoadedModel = new CLoadedModelInfo();
	
	pLoadedModel->m_ObjectType = objectType;

	char pstrToken[256] = { '\0' };

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:")) // 계층 구조가 존재하면
			{
				//계층 구조로 이루어진 정보를 읽어내고 해당 오브젝트를 Root에 저장.
				pLoadedModel->m_pModelRootObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader, &pLoadedModel->m_nSkinnedMeshes);
				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
			}
			else if (!strcmp(pstrToken, "<Animation>:")) //Animation이 있다면?
			{
				//애니메이션 정보를 읽어온다.
				CGameObject::LoadAnimationFromFile(pInFile, pLoadedModel);
				pLoadedModel->PrepareSkinning(); //SkinnedMesh를 적용할 준비
			}
			else if (!strcmp(pstrToken, "</Animation>:"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, "Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pLoadedModel);
}

int CGameObject::LoadFrameHierarchyDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes)
{
	int count = 0;
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	CGameObject* pGameObject = new CGameObject();

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:")) //오브젝트 객체의 이름이라고 보면 된다.
		{
			nFrame = ::ReadIntegerFromFile(pInFile);
			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:")) //위치, 회전, 크기 ,쿼터니언 값
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:")) //계층 구조에 핵심 부모의 Transform
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:")) //스킨닝을 쓰지 않는 경우에 Mesh 정보
		{
			CDefualtMesh* pMesh = new CDefualtMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshDummyFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			CSkinnedMesh* pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
			//스키닝을 위한 정보 Boon에 대한 정보들이 들어있다.
			pSkinnedMesh->LoadSkinInfoDummyFromFile(pd3dDevice, pd3dCommandList, pInFile);
			//pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			::ReadStringFromFile(pInFile, pstrToken); //<Mesh>: 애니메이션이 적용된 mehs에 대한 Mesh
			//스키닝을 쓰는 모델에서는 SkinningInfo가 mesh 보다 위에있다.
			if (!strcmp(pstrToken, "<Mesh>:")) pSkinnedMesh->LoadMeshDummyFromFile(pd3dDevice, pd3dCommandList, pInFile);

			//Skinning이 적용된 Mesh
			pGameObject->SetMesh(pSkinnedMesh);
		}
		//재질 정보로 주로 모델에 사용된 Texture에 대한 정보가 들어있다.
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			count = pGameObject->LoadMaterialsDummyFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader);
		}
		//계층 구조에 핵심 자식. 재귀함수 구조로 이루어져 트리 구조를 형성.
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					count += CGameObject::LoadFrameHierarchyDummyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader, pnSkinnedMeshes);
		
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, "(Frame: %p) (Parent: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	//pGameObject->Release();
	//pGameObject->ReleaseUploadBuffers();
	//pGameObject->ReleaseShaderVariables();
	delete pGameObject;
	return count;
}

CGameObject* CGameObject::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	CGameObject* pGameObject = new CGameObject();

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:")) //오브젝트 객체의 이름이라고 보면 된다.
		{
			nFrame = ::ReadIntegerFromFile(pInFile);
			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:")) //위치, 회전, 크기 ,쿼터니언 값
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:")) //계층 구조에 핵심 부모의 Transform
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:")) //스킨닝을 쓰지 않는 경우에 Mesh 정보
		{
			CDefualtMesh* pMesh = new CDefualtMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			CSkinnedMesh* pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
			//스키닝을 위한 정보 Boon에 대한 정보들이 들어있다.
			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
			//pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			::ReadStringFromFile(pInFile, pstrToken); //<Mesh>: 애니메이션이 적용된 mehs에 대한 Mesh
			//스키닝을 쓰는 모델에서는 SkinningInfo가 mesh 보다 위에있다.
			if (!strcmp(pstrToken, "<Mesh>:")) pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);

			//Skinning이 적용된 Mesh
			pGameObject->SetMesh(pSkinnedMesh);
		}
		//재질 정보로 주로 모델에 사용된 Texture에 대한 정보가 들어있다.
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader);
		}
		//계층 구조에 핵심 자식. 재귀함수 구조로 이루어져 트리 구조를 형성.
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					CGameObject* pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader, pnSkinnedMeshes);
					if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, "(Frame: %p) (Parent: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

int CGameObject::LoadMaterialsDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader)
{

	int count = 0;
	char pstrToken[64] = { '\0' };
	int nMaterial = 0;
	UINT nReads = 0;

	m_nMaterials = ReadIntegerFromFile(pInFile);

	m_ppMaterials = new CMaterial * [m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;

	CMaterial* pMaterial = NULL;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Material>:"))
		{
			nMaterial = ReadIntegerFromFile(pInFile);

			pMaterial = new CMaterial(7);
			//0:Albedo, 1:Normal, 2:Roughness, 3:Metalness, 4:AO, 5:SpecularF0, 6:Emission

			if (!pShader)
			{
				UINT nMeshType = GetMeshType();
				if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE)
				{
					if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT)
					{
						pMaterial->SetSkinnedAnimationShader();
					}
					else
					{
						pMaterial->SetStandardShader();
					}
				}
			}
			SetMaterial(nMaterial, pMaterial);
		}
		else if (!strcmp(pstrToken, "<AlbedoColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4AlbedoColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4EmissiveColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4SpecularColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<Roughness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fRoughness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Smoothness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSmoothness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Metallic>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fMetallic), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularHighlight>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSpecularHighlight), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<GlossyReflection>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossyReflection), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<AlbedoMap>:"))
		{
			count += m_ppMaterials[nMaterial]->LoadTextureDummyFromFile(pd3dDevice, pd3dCommandList, MATERIAL_ALBEDO_MAP, 4, pMaterial->m_ppstrTextureNames[0], &(pMaterial->m_ppTextures[0]), pParent, pInFile, pShader);
			
		}
		else if (!strcmp(pstrToken, "<NormalMap>:"))
		{
			count += m_ppMaterials[nMaterial]->LoadTextureDummyFromFile(pd3dDevice, pd3dCommandList, MATERIAL_NORMAL_MAP, 5, pMaterial->m_ppstrTextureNames[1], &(pMaterial->m_ppTextures[1]), pParent, pInFile, pShader);
			
		}
		else if (!strcmp(pstrToken, "<RoughnessMap>:"))
		{
			count += m_ppMaterials[nMaterial]->LoadTextureDummyFromFile(pd3dDevice, pd3dCommandList, MATERIAL_ROUGHNESS_MAP, 6, pMaterial->m_ppstrTextureNames[2], &(pMaterial->m_ppTextures[2]), pParent, pInFile, pShader);
		
		}
		else if (!strcmp(pstrToken, "<MetalnessMap>:"))
		{
			count += m_ppMaterials[nMaterial]->LoadTextureDummyFromFile(pd3dDevice, pd3dCommandList, MATERIAL_METALNESS_MAP, 7, pMaterial->m_ppstrTextureNames[3], &(pMaterial->m_ppTextures[3]), pParent, pInFile, pShader);
			
		}
		else if (!strcmp(pstrToken, "<OcclusionMap>:"))
		{
			count += m_ppMaterials[nMaterial]->LoadTextureDummyFromFile(pd3dDevice, pd3dCommandList, MATERIAL_AO_MAP, 8, pMaterial->m_ppstrTextureNames[4], &(pMaterial->m_ppTextures[4]), pParent, pInFile, pShader);

		}
		else if (!strcmp(pstrToken, "<SpecularF0Map>:"))
		{
			count += m_ppMaterials[nMaterial]->LoadTextureDummyFromFile(pd3dDevice, pd3dCommandList, MATERIAL_SPECULARF0_MAP, 9, pMaterial->m_ppstrTextureNames[5], &(pMaterial->m_ppTextures[5]), pParent, pInFile, pShader);
	
		}
		else if (!strcmp(pstrToken, "<EmissionMap>:"))
		{
			 count += m_ppMaterials[nMaterial]->LoadTextureDummyFromFile(pd3dDevice, pd3dCommandList, MATERIAL_EMISSION_MAP, 10, pMaterial->m_ppstrTextureNames[6], &(pMaterial->m_ppTextures[6]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "</Materials>"))
		{
			return count;
			break;
		}
	}

}


/*
* 여기 내용은 개선해야할 것이 많다.
* 당장에 모델 애니메이션이랑 읽혀진다면 이 부분은 주석 처리하고 임의 값 넣고 테스트
* 진행 이후 PBR 연계와 함께 개선해야하는 부분
* 먼저 AO 설정이 없는게 짜친다.
* 그리고 Shader(VS, PS와 최대한 간단한 정보만 남겨 둘 예정. ObjectShader도 개선 or 삭제 예정) 
* 그때 Material을 대대적으로 손보면서 PBR, IBL 관련 연구과제를 해결할 계획.
*/
void CGameObject::LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader)
{
	
	char pstrToken[64] = { '\0' };
	int nMaterial = 0;
	UINT nReads = 0;

	m_nMaterials = ReadIntegerFromFile(pInFile);

	m_ppMaterials = new CMaterial * [m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;

	CMaterial* pMaterial = NULL;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Material>:"))
		{
			nMaterial = ReadIntegerFromFile(pInFile);

			pMaterial = new CMaterial(7); 
			//0:Albedo, 1:Normal, 2:Roughness, 3:Metalness, 4:AO, 5:SpecularF0, 6:Emission

			if (!pShader)
			{
				UINT nMeshType = GetMeshType();
				if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE)
				{
					if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT)
					{
						pMaterial->SetSkinnedAnimationShader();
					}
					else
					{
						pMaterial->SetStandardShader();
					}
				}
			}
			SetMaterial(nMaterial, pMaterial);
		}
		else if (!strcmp(pstrToken, "<AlbedoColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4AlbedoColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4EmissiveColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4SpecularColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<Roughness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fRoughness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Smoothness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSmoothness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Metallic>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fMetallic), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularHighlight>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSpecularHighlight), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<GlossyReflection>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossyReflection), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<AlbedoMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_ALBEDO_MAP, 4, pMaterial->m_ppstrTextureNames[0], &(pMaterial->m_ppTextures[0]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<NormalMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_NORMAL_MAP, 5, pMaterial->m_ppstrTextureNames[1], &(pMaterial->m_ppTextures[1]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<RoughnessMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_ROUGHNESS_MAP, 6, pMaterial->m_ppstrTextureNames[2], &(pMaterial->m_ppTextures[2]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<MetalnessMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_METALNESS_MAP, 7, pMaterial->m_ppstrTextureNames[3], &(pMaterial->m_ppTextures[3]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<OcclusionMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_AO_MAP, 8, pMaterial->m_ppstrTextureNames[4], &(pMaterial->m_ppTextures[4]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<SpecularF0Map>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_SPECULARF0_MAP, 9, pMaterial->m_ppstrTextureNames[5], &(pMaterial->m_ppTextures[5]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<EmissionMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_EMISSION_MAP, 10, pMaterial->m_ppstrTextureNames[6], &(pMaterial->m_ppTextures[6]), pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "</Materials>"))
		{
			if (pMaterial->m_fSmoothness > 0.0f)
			{
				pMaterial->m_fRoughness = 1.0f - pMaterial->m_fSmoothness;
			}
			break;
		}
	}
	
}

void CGameObject::LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel)
{
	char pstrToken[256] = { '\0' };
	UINT nReads = 0;

	int nAnimationSets = 0;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			nAnimationSets = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationClips = new CAnimationClips(nAnimationSets);
		}
		else if (!strcmp(pstrToken, "<FrameNames>:"))
		{
			pLoadedModel->m_pAnimationClips->m_nBoneFrames = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationClips->m_ppBoneFrameCaches = new CGameObject * [pLoadedModel->m_pAnimationClips->m_nBoneFrames];

			for (int j = 0; j < pLoadedModel->m_pAnimationClips->m_nBoneFrames; j++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				pLoadedModel->m_pAnimationClips->m_ppBoneFrameCaches[j] = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);

#ifdef _WITH_DEBUG_SKINNING_BONE
				TCHAR pstrDebug[256] = { 0 };
				TCHAR pwstrAnimationBoneName[64] = { 0 };
				TCHAR pwstrBoneCacheName[64] = { 0 };
				size_t nConverted = 0;
				mbstowcs_s(&nConverted, pwstrAnimationBoneName, 64, pstrToken, _TRUNCATE);
				mbstowcs_s(&nConverted, pwstrBoneCacheName, 64, pLoadedModel->m_ppBoneFrameCaches[j]->m_pstrFrameName, _TRUNCATE);
				_stprintf_s(pstrDebug, 256, _T("AnimationBoneFrame:: Cache(%s) AnimationBone(%s)\n"), pwstrBoneCacheName, pwstrAnimationBoneName);
				OutputDebugString(pstrDebug);
#endif
			}
		}
		else if (!strcmp(pstrToken, "<AnimationSet>:"))
		{
			int nAnimationSet = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pstrToken); //Animation Set Name

			float fLength = ::ReadFloatFromFile(pInFile);
			int nFramesPerSecond = ::ReadIntegerFromFile(pInFile);
			int nKeyFrames = ::ReadIntegerFromFile(pInFile);

			pLoadedModel->m_pAnimationClips->m_pAnimationClips[nAnimationSet] = new CAnimationClip(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationClips->m_nBoneFrames, pstrToken);

			for (int i = 0; i < nKeyFrames; i++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				if (!strcmp(pstrToken, "<Transforms>:"))
				{
					CAnimationClip* pAnimationSet = pLoadedModel->m_pAnimationClips->m_pAnimationClips[nAnimationSet];

					int nKey = ::ReadIntegerFromFile(pInFile); //i
					float fKeyTime = ::ReadFloatFromFile(pInFile);

#ifdef _WITH_ANIMATION_SRT
					m_pfKeyFrameScaleTimes[i] = fKeyTime;
					m_pfKeyFrameRotationTimes[i] = fKeyTime;
					m_pfKeyFrameTranslationTimes[i] = fKeyTime;
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameScales[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationClips->m_nBoneFrames, pInFile);
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4KeyFrameRotations[i], sizeof(XMFLOAT4), pLoadedModel->m_pAnimationClips->m_nBoneFrames, pInFile);
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameTranslations[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationClips->m_nBoneFrames, pInFile);
#else
					pAnimationSet->m_pfKeyFrameTimes[i] = fKeyTime;
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i], sizeof(XMFLOAT4X4), pLoadedModel->m_pAnimationClips->m_nBoneFrames, pInFile);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}
}

//
void CGameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	//누적 곱을 통해서 서로 이어진 상태에 움직임을 보일 수 있다.
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	//obbMesh과 존재하는것과 동시에 충돌 처리가 활성화 되어있어야 obb박스에 위치를 갱신한다
	//if (m_pOBBMesh && m_bIsBoxCollision)

		XMMATRIX world = XMLoadFloat4x4(&m_xmf4x4World);

		XMVECTOR rotationQ = XMQuaternionRotationMatrix(world);
		XMStoreFloat4(&m_obbBox.Orientation, rotationQ);

		XMFLOAT3 worldPos;
		XMStoreFloat3(&worldPos, world.r[3]);
		m_obbBox.Center = Vector3::Add(worldPos, m_xmf3OBBOffset);

		//m_obbBox.Center = Vector3::Add(worldPos, XMFLOAT3(0,2,0));
	

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);


}

void CGameObject::SetTrackAnimationClip(int nAnimationTrack, int nAnimationClip)
{
	if (m_pSkinnedAnimationController) //AnimationContorller가 초기화 되어 있어야 작동.
	{
		m_pSkinnedAnimationController->SetTrackAnimationClip(nAnimationTrack, nAnimationClip);
	}
}

void CGameObject::SetTrackAnimationPosition(int nAnimationTrack, float fPosition)
{
	if (m_pSkinnedAnimationController)
	{
		m_pSkinnedAnimationController->SetTrackPosition(nAnimationTrack, fPosition);
	}
}

void CGameObject::SetRootMotion(bool bRootMotion)
{
	if (m_pSkinnedAnimationController)
	{
		m_pSkinnedAnimationController->SetRootMotion(bRootMotion);
	}
}
void CGameObject::FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh)
{
	//Mesh가 존재, Type이 BONE_INDEX_WEIGHT라면
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT))
	{
		//현재 Object에 설정된 Mesh가 SkinnedMesh
		ppSkinnedMeshes[(*pnSkinnedMesh)++] = (CSkinnedMesh*)m_pMesh;
	}
	if (m_pSibling)
	{
		m_pSibling->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
	}
	if (m_pChild)
	{
		m_pChild->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
	}
}
CTexture* CGameObject::FindReplicatedTexture(_TCHAR* pstrTextureName)
{
	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i])
		{
			for (int j = 0; j < m_ppMaterials[i]->m_nTextures; j++)
			{
				if (m_ppMaterials[i]->m_ppTextures[j])
				{
					if (!_tcsncmp(m_ppMaterials[i]->m_ppstrTextureNames[j], pstrTextureName, _tcslen(pstrTextureName))) return(m_ppMaterials[i]->m_ppTextures[j]);
				}
			}
		}
	}
	CTexture* pTexture = NULL;
	if (m_pSibling) if (pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName)) return(pTexture);
	if (m_pChild) if (pTexture = m_pChild->FindReplicatedTexture(pstrTextureName)) return(pTexture);

	return(NULL);
}

CGameObject* CGameObject::FindFrame(char* pstrFrameName)
{
	CGameObject* pFrameObject = NULL;
	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

void CGameObject::PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent)
{
	TCHAR pstrDebug[256] = { 0 };

	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	//std::cout << "(Frame: " << pGameObject
	//	<< ") (Parent: " << pParent
	//	<< ")" << std::endl;

	if (pGameObject->m_pSibling) CGameObject::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) CGameObject::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}

void CGameObject::Animate(float fTimeElapsed)
{

	OnPrepareRender();

	if (m_pSkinnedAnimationController) //skinnedAnimationController가 있는 경우
	{
		m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);
	}

	if (m_pSibling)
	{
		m_pSibling->Animate(fTimeElapsed);
	}
	if (m_pChild)
	{
		m_pChild->Animate(fTimeElapsed);
	}
}

void CGameObject::OnPrepareRender()
{

}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbObject_Info = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbObject_Info->Map(0, NULL, (void**)&m_pcbMappedObject_Info);
}

void CGameObject::ReleaseShaderVariables()
{

}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList )
{

	if (m_pcbMappedObject_Info)
	{
		XMStoreFloat4x4(&m_pcbMappedObject_Info->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	}

	if (m_d3dCbvGPUDescriptorHandle.ptr)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(3, m_d3dCbvGPUDescriptorHandle);
	}

}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{

	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);

	////건물 설치 관련해서
	//pd3dCommandList->SetGraphicsRoot32BitConstants(0, 1, &m_nIsBuildingObject, 36);

}


void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList , CCamera* pCamera)
{

}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bIsHir , bool bIsNormal, bool bIsWire  , bool bIsShadow)
{
	//Normal에 대해서는 우선 계층 구조로 그려진 애들에 한정해서 넣은 상태
	if (bIsHir)
	{
		//이 부분이 onlyShadowRender, 기본 Render, NormalRender에 Bone 상태를 공유해주는 핵심 부분
		/*
		* 1. GameObject는 개별로 AnimationController를 갖고있다
		* 2. 동일한 Mesh, Animation을 갖는 GameObject들은 이 값을 참조하고 있다.
		* 3. for(i) 안에서 같은 순번째에 AdvanceTime과 UpdateTransform으로 그 i 시점에 Mesh 와 Bone의 Transform을 특정 Track에 상태로 고정한다
		* 4. 그 고정되었던 결과를 이 아래 Controller에 UpdateShaderVariables로 값을 저장하고 전달한다
		* 5. 같은 CommandList에 Excute가 이루어진 시점 메모리에서 값을 읽어와 실질적으로 바인딩하기에
		* 6. 가장 마지막에 저장된 CB에 값을 바탕으로 모든 결과가 그려지게 되어있다.
		* 7. 결과적으로 OnlyShadow에서도 UpdateShaderVariables를 하지만 Render가 더 늦게 실행되기에 메모리엔 이 때의 BoneTransform이 전달된다.
		*/
		if (m_pSkinnedAnimationController && !bIsNormal)
		{
			m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pMesh)
		{
			UpdateShaderVariables(pd3dCommandList, &m_xmf4x4World);
			if (m_nMaterials > 0)
			{
				for (int i = 0; i < m_nMaterials; i++)
				{
					if (m_ppMaterials[i])
					{
						if (m_ppMaterials[i]->m_pShader)
						{
							m_ppMaterials[i]->m_pShader->SetWireRender(bIsWire);
							m_ppMaterials[i]->m_pShader->SetShadowMapRender(bIsShadow);
							if (!bIsNormal)
							{
								m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
							}
							else
							{
								m_ppMaterials[i]->m_pShader->NormalRender(pd3dCommandList);
							}
						}
						m_ppMaterials[i]->UpdateShaderVariables(pd3dCommandList);
					}

					if (!bIsNormal)
					{
						m_pMesh->Render(pd3dCommandList, i);
					}
					else
					{
						m_pMesh->NormalRender(pd3dCommandList, i);
					}
				}
			}
		}
		if (m_pSibling)
		{
			m_pSibling->Render(pd3dCommandList, pCamera, bIsHir, bIsNormal, bIsWire, bIsShadow);
		}
		if (m_pChild)
		{
			m_pChild->Render(pd3dCommandList, pCamera, bIsHir, bIsNormal, bIsWire, bIsShadow);
		}
	}
	else
	{
		OnPrepareRender();

		UpdateShaderVariables(pd3dCommandList);

		//GameObject가 Shader를 갖는 경우 ==> 이제 Material이 갖는 경우로만
		//사용해서 정의할 계획이라 없어질 부분이다.
		if (m_pShader)
		{
			//m_pShader->UpdateShaderVariables(pd3dCommandList, &m_xmf4x4World); //월드 변환 행렬을 VS에 넘긴다. ==> 이렇게 바꾸는 이유는 1. Shader에서 저 위치값을 넘기는것보단 위치값은 게임 오브젝트의 위치니까 게임 오브젝트 자체가 갖고있는게 더욱 용이하다 판단해서
			m_pShader->UpdateShaderVariables(pd3dCommandList);
			m_pShader->Render(pd3dCommandList, pCamera);
		}

		//이전 방식인데 아직 이전 거가 살아있는 상태라 빼지 않음
		if (m_pMaterial)
		{
			if (m_pMaterial->m_pTexture)
			{
				//m_pMaterial->UpdateShaderVariables(pd3dCommandList); 안에 있음
				m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
			}
		}

		if (m_pMesh)
		{
			m_pMesh->Render(pd3dCommandList, 0); //임시로 0을 넣은것 나중에 고쳐야함
		}
		if (m_pSibling)
		{
			m_pSibling->Render(pd3dCommandList, pCamera, bIsHir, bIsNormal, bIsWire , bIsShadow);
		}
		if (m_pChild)
		{
			m_pChild->Render(pd3dCommandList, pCamera, bIsHir, bIsNormal, bIsWire , bIsShadow);
		}
	}
}

//디버그를 위한 Render 
void CGameObject::DebugRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	/*if (m_pOBBMesh)
	{
		XMFLOAT4X4 world = m_xmf4x4World;

		XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
		XMFLOAT3 origin = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 translation = m_xmf3OBBOffset;

		XMFLOAT4X4 offsetMatrix =
			Matrix4x4::AffineTransformation(scale, origin, rotation, translation);

		XMFLOAT4X4 obbWorldMatrix = Matrix4x4::Multiply(world, offsetMatrix);

		UpdateShaderVariables(pd3dCommandList, &obbWorldMatrix);
		m_pOBBShader->Render(pd3dCommandList, pCamera);
		m_pOBBMesh->Render(pd3dCommandList, 0);
	}*/
}


void CGameObject::PrintMatrixWorld() const
{
	/*std::cout << "===== WORLD MATRIX =====\n";
	std::cout << m_xmf4x4World._11 << "  " << m_xmf4x4World._12 << "  "
		<< m_xmf4x4World._13 << "  " << m_xmf4x4World._14 << "\n";
	std::cout << m_xmf4x4World._21 << "  " << m_xmf4x4World._22 << "  "
		<< m_xmf4x4World._23 << "  " << m_xmf4x4World._24 << "\n";
	std::cout << m_xmf4x4World._31 << "  " << m_xmf4x4World._32 << "  "
		<< m_xmf4x4World._33 << "  " << m_xmf4x4World._34 << "\n";
	std::cout << m_xmf4x4World._41 << "  " << m_xmf4x4World._42 << "  "
		<< m_xmf4x4World._43 << "  " << m_xmf4x4World._44 << "\n\n";*/
}

void CGameObject::PrintOBB() const
{
	std::cout << "===== OBB INFO =====\n";

	// Center
	std::cout << "Center: ( "
		<< m_obbBox.Center.x << ", "
		<< m_obbBox.Center.y << ", "
		<< m_obbBox.Center.z << " )\n";

	// Extents
	std::cout << "Extents: ( "
		<< m_obbBox.Extents.x << ", "
		<< m_obbBox.Extents.y << ", "
		<< m_obbBox.Extents.z << " )\n";

	// Orientation Quaternion
	std::cout << "Orientation (Quaternion): ( "
		<< m_obbBox.Orientation.x << ", "
		<< m_obbBox.Orientation.y << ", "
		<< m_obbBox.Orientation.z << ", "
		<< m_obbBox.Orientation.w << " )\n";

	std::cout << "\n";
}

void CGameObject::PrintMatrixToParent() const
{
	std::cout << "===== TO-PARENT MATRIX =====\n";


	std::cout << m_xmf4x4ToParent._11 << "  " << m_xmf4x4ToParent._12 << "  "
		<< m_xmf4x4ToParent._13 << "  " << m_xmf4x4ToParent._14 << "\n";
	std::cout << m_xmf4x4ToParent._21 << "  " << m_xmf4x4ToParent._22 << "  "
		<< m_xmf4x4ToParent._23 << "  " << m_xmf4x4ToParent._24 << "\n";
	std::cout << m_xmf4x4ToParent._31 << "  " << m_xmf4x4ToParent._32 << "  "
		<< m_xmf4x4ToParent._33 << "  " << m_xmf4x4ToParent._34 << "\n";
	std::cout << m_xmf4x4ToParent._41 << "  " << m_xmf4x4ToParent._42 << "  "
		<< m_xmf4x4ToParent._43 << "  " << m_xmf4x4ToParent._44 << "\n\n";
}

///////////////////////////////////////////////////////////////////////////////////////
/////

CRotatingObject::CRotatingObject()
{
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 90.0f;
}

CRotatingObject::~CRotatingObject()
{

}

void CRotatingObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle)); //해당 방향축을 기준으로 fAngle만큼 회전 시키는 행렬을 계산해주는 함수
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World); //행렬곱

}

void CRotatingObject::Animate(float fTimeElapsed)
{
	Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
}

//----------------------------------------------------------------------------------------

CDefaultGameObject::CDefaultGameObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CDefaultGameObject::~CDefaultGameObject()
{

}

void CDefaultGameObject::Init(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, true);
}


//----------------------------------------------------------------------------------------

CAnimatedObject::CAnimatedObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel, nAnimationTracks);
}

CAnimatedObject::~CAnimatedObject()
{

}

void CAnimatedObject::Init(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	CLoadedModelInfo* pAnimatedModel = pModel;

	SetChild(pAnimatedModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pAnimatedModel);
	
	//기본적으로 0번은 Idle 상태가 들어가 있을 예정이기 때문.
	for (int i = 0; i < nAnimationTracks; i++)
	{
		m_pSkinnedAnimationController->SetTrackAnimationClip(i, i);
		if (i == 0)
		{
			m_pSkinnedAnimationController->SetTrackEnable(i, true);
		}
		else
		{
			m_pSkinnedAnimationController->SetTrackEnable(i, false);
		}
	}

	CAnimationCallbackHandler* pAnimationCallbackHandler = new CDefulatHumanAnimationCallbackHandler(m_pSkinnedAnimationController);
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	m_pSkinnedAnimationController->SetCallbackKeys(1, 1);
	m_pSkinnedAnimationController->SetTrackType(1, ANIMATION_TYPE_ONCE);

	Rotate(90, 00, 0);
}

void CAnimatedObject::SetAnimationState(EAnimationState State)
{
	switch (State)
	{
	case Idle:
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, true);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackSpeed(1, 1.0f);
		m_pSkinnedAnimationController->SetTrackAnimationClip(0, 0);
		break;
	}
	case Walk:
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, true);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackSpeed(1, 1.0f);
		m_pSkinnedAnimationController->SetTrackAnimationClip(0, 1);
		break;
	}
	case pickup:
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_pSkinnedAnimationController->SetTrackSpeed(1, 4.0f);
		m_pSkinnedAnimationController->SetTrackAnimationClip(1, 2);
		CAnimationClip* pAnimationClip = m_pSkinnedAnimationController->m_pAnimationClips->m_pAnimationClips[m_pSkinnedAnimationController->m_pAnimationTracks[1].m_nAnimationClip];
		m_pSkinnedAnimationController->SetCallbackKey(1, 0, pAnimationClip->m_fLength, (void*)(uintptr_t)State);
		break;
	}
	case punch:
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_pSkinnedAnimationController->SetTrackSpeed(1, 1.0f);
		m_pSkinnedAnimationController->SetTrackAnimationClip(1, 3);
		CAnimationClip* pAnimationClip = m_pSkinnedAnimationController->m_pAnimationClips->m_pAnimationClips[m_pSkinnedAnimationController->m_pAnimationTracks[1].m_nAnimationClip];
		m_pSkinnedAnimationController->SetCallbackKey(1, 0, pAnimationClip->m_fLength, (void*)(uintptr_t)State);
		break;
	}
	case watering:
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_pSkinnedAnimationController->SetTrackSpeed(1, 1.0f);
		m_pSkinnedAnimationController->SetTrackAnimationClip(1, 4);
		CAnimationClip* pAnimationClip = m_pSkinnedAnimationController->m_pAnimationClips->m_pAnimationClips[m_pSkinnedAnimationController->m_pAnimationTracks[1].m_nAnimationClip];
		m_pSkinnedAnimationController->SetCallbackKey(1, 0, pAnimationClip->m_fLength, (void*)(uintptr_t)State);
		break;
	}
	default:
		break;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CWoodManObject::CWoodManObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	SetOBB(pd3dDevice, pd3dCommandList);
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel, nAnimationTracks);
}

CWoodManObject::~CWoodManObject()
{

}

void CWoodManObject::Init(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	CLoadedModelInfo* pAnimatedModel = pModel;

	SetChild(pAnimatedModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pAnimatedModel);

	//기본적으로 0번은 Idle 상태가 들어가 있을 예정이기 때문.
	for (int i = 0; i < nAnimationTracks; i++)
	{
		m_pSkinnedAnimationController->SetTrackAnimationClip(i, i);
		if (i == 0)
		{
			m_pSkinnedAnimationController->SetTrackEnable(i, true);
		}
		else
		{
			m_pSkinnedAnimationController->SetTrackEnable(i, false);
		}
	}

	CAnimationCallbackHandler* pAnimationCallbackHandler = new CDefulatHumanAnimationCallbackHandler(m_pSkinnedAnimationController);
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	m_pSkinnedAnimationController->SetCallbackKeys(1, 1);
	m_pSkinnedAnimationController->SetTrackType(1, ANIMATION_TYPE_ONCE);
	
	Rotate(90, 00, 0);
}

//이건 생성자에만 넣어야한 이유는 오브젝트 풀링으로 init에서 계속 초기화해주는데 SetOBB는
//OBB에 Mesh를 계속해서 생성하기 때문에 계속 메모리를 차지하는 버퍼가 생성됨.
void CWoodManObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(0.3f, 0.25f, 0.85f); //
	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠
	
	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;

	m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.85f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBoxObject::CSkyBoxObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, wchar_t* pwstrSkyBoxHDRPath) : CGameObject(1)
{
	CSkyBoxMesh* pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 2000.0f, 2000.0f, 20.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	pSkyBoxTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, pwstrSkyBoxHDRPath, RESOURCE_TEXTURE_CUBE, 0);

	CSkyBoxShader* pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, 0, 15);

	CMaterial* pSkyBoxMaterial = new CMaterial(1);
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);
	pSkyBoxMaterial->SetShader(pSkyBoxShader);

	SetMaterial(0, pSkyBoxMaterial);
}

CSkyBoxObject::~CSkyBoxObject()
{
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CPlanObject::CPlanObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
	//CGameObject의 생성자가 2 종류가 있으며 해당 오브젝트는 Material을 몇개 가질지를 지정하는 것이 1개 존재 한다.
	CPlanMesh* pPlanMesh = new CPlanMesh(pd3dDevice, pd3dCommandList, 100,100,100,100);
	SetMesh(pPlanMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	//CTexture* pPlanAlbedoTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	//pPlanAlbedoTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/ground_03.dds", RESOURCE_TEXTURE2D, 0);

	//CTexture* pPlanNormalTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	//pPlanNormalTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/ground_03_NRM.dds", RESOURCE_TEXTURE2D, 0);


	CPlanShader* pPlanShader = new CPlanShader();
	pPlanShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);


	//2개의 Texture를 갖는 Material
	CMaterial* pPlanMaterial = new CMaterial(2);

	for (int i = 0; i < pPlanMaterial->m_nTextures; i++)
	{
		pPlanMaterial->m_ppTextures[i] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	}

	pPlanMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/ground_03.dds", RESOURCE_TEXTURE2D, 0);
	pPlanMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);

	pPlanMaterial->m_ppTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/ground_03_NRM.dds", RESOURCE_TEXTURE2D, 0);
	pPlanMaterial->SetMaterialType(MATERIAL_NORMAL_MAP);


	CScene::CreateShaderResourceViews(pd3dDevice, pPlanMaterial->m_ppTextures[0], 0, 4);
	CScene::CreateShaderResourceViews(pd3dDevice, pPlanMaterial->m_ppTextures[1], 0, 5);

	//pPlanMaterial->SetTexture(pPlanTexture);
	//pPlanMaterial->SetTexture(pPlanTexture);

	pPlanMaterial->SetShader(pPlanShader);

	//1번째 재질 연결.
	SetMaterial(0, pPlanMaterial);
}

CPlanObject::~CPlanObject()
{
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale) : CGameObject(3)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	CHeightMapGridMesh* pHeightMapGridMesh = NULL;

	
	pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, 0, 0, m_nWidth, m_nLength, nBlockWidth, nBlockLength, xmf3Scale, m_pHeightMapImage);
	SetMesh( pHeightMapGridMesh);
	
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTerrainShader* pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);


	CMaterial* pTerrainMaterial = new CMaterial(1);
	CMaterial* pTerrainSummerMaterial = new CMaterial(1);
	CMaterial* pTerrainWinterMaterial = new CMaterial(1);


	pTerrainMaterial->m_ppTextures[0] = new CTexture(11, RESOURCE_TEXTURE2D, 0, 1);
	pTerrainSummerMaterial->m_ppTextures[0] = new CTexture(3, RESOURCE_TEXTURE2D, 0, 1);
	pTerrainWinterMaterial->m_ppTextures[0] = new CTexture(3, RESOURCE_TEXTURE2D, 0, 1);

	pTerrainMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Pebbles_C_BaseColor.dds", RESOURCE_TEXTURE2D, 0);
	pTerrainMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Pebbles_C_Normal.dds", RESOURCE_TEXTURE2D, 1);
	pTerrainMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Pebbles_C_MaskMap.dds", RESOURCE_TEXTURE2D, 2);
	
	pTerrainMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Grass_A_BaseColor.dds", RESOURCE_TEXTURE2D, 3);
	pTerrainMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Grass_A_Normal.dds", RESOURCE_TEXTURE2D, 4);
	pTerrainMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Grass_A_MaskMap.dds", RESOURCE_TEXTURE2D, 5);
	
	pTerrainMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Rock_BaseColor.dds", RESOURCE_TEXTURE2D, 6);
	pTerrainMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Rock_Normal.dds", RESOURCE_TEXTURE2D, 7);
	pTerrainMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Rock_MaskMap.dds", RESOURCE_TEXTURE2D, 8);

	pTerrainMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Terrain_DetailMask_Grass.dds", RESOURCE_TEXTURE2D, 9);
	pTerrainMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Terrain_DetailMask_Rock.dds", RESOURCE_TEXTURE2D, 10);

	//겨울과 여름 관련 Texture를 위한 
	pTerrainSummerMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Sand_BaseColor.dds", RESOURCE_TEXTURE2D, 0);
	pTerrainSummerMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Sand_Normal.dds", RESOURCE_TEXTURE2D, 1);
	pTerrainSummerMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Sand_MaskMap.dds", RESOURCE_TEXTURE2D, 2);

	pTerrainWinterMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Snow_BaseColor.dds", RESOURCE_TEXTURE2D, 0);
	pTerrainWinterMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Snow_Normal.dds", RESOURCE_TEXTURE2D, 1);
	pTerrainWinterMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/HeightMapTexture/Snow_MaskMap.dds", RESOURCE_TEXTURE2D, 2);

	
	pTerrainMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pTerrainMaterial->SetMaterialType(MATERIAL_NORMAL_MAP);
	pTerrainMaterial->SetMaterialType(MATERIAL_AO_MAP);
	pTerrainMaterial->SetMaterialType(MATERIAL_ROUGHNESS_MAP);

	pTerrainSummerMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pTerrainSummerMaterial->SetMaterialType(MATERIAL_NORMAL_MAP);
	pTerrainSummerMaterial->SetMaterialType(MATERIAL_AO_MAP);
	pTerrainSummerMaterial->SetMaterialType(MATERIAL_ROUGHNESS_MAP);

	pTerrainWinterMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pTerrainWinterMaterial->SetMaterialType(MATERIAL_NORMAL_MAP);
	pTerrainWinterMaterial->SetMaterialType(MATERIAL_AO_MAP);
	pTerrainWinterMaterial->SetMaterialType(MATERIAL_ROUGHNESS_MAP);

	CScene::CreateShaderResourceViews(pd3dDevice, pTerrainMaterial->m_ppTextures[0], 0, 22);
	CScene::CreateShaderResourceViews(pd3dDevice, pTerrainSummerMaterial->m_ppTextures[0], 0, 23);
	CScene::CreateShaderResourceViews(pd3dDevice, pTerrainWinterMaterial->m_ppTextures[0], 0, 23);

	pTerrainMaterial->SetShader(pTerrainShader);
	//pTerrainSummerMaterial->SetShader(pTerrainShader);
	//pTerrainWinterMaterial->SetShader(pTerrainShader);
	//1번째 재질 연결.
	SetMaterial(0, pTerrainMaterial); //이건 항상 연결
	//2번째 재질 연결.
	SetMaterial(1, pTerrainSummerMaterial); // 이 둘은 분기하도록 해결해야함
	//3번째 재질 연결.
	SetMaterial(2, pTerrainWinterMaterial);

}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}

void CHeightMapTerrain::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bIsHir, bool bIsNormal, bool bIsWire, bool bIsShadow)
{
	//Normal에 대해서는 우선 계층 구조로 그려진 애들에 한정해서 넣은 상태
	if (bIsHir)
	{
		if (m_pMesh)
		{
			UpdateShaderVariables(pd3dCommandList, &m_xmf4x4World);
			if (m_nMaterials > 0)
			{
				for (int i = 0; i < m_nMaterials; i++)
				{
					if (m_ppMaterials[i])
					{
						if (m_ppMaterials[i]->m_pShader)
						{
							m_ppMaterials[i]->m_pShader->SetWireRender(bIsWire);
							m_ppMaterials[i]->m_pShader->SetShadowMapRender(bIsShadow);
							if (!bIsNormal)
							{
								m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
							}
							else
							{
								m_ppMaterials[i]->m_pShader->NormalRender(pd3dCommandList);
							}
						}

						if (m_nSeasonIndex == 1 && i != 2)
						{
							m_ppMaterials[0]->UpdateShaderVariables(pd3dCommandList);
							m_ppMaterials[i]->UpdateShaderVariables(pd3dCommandList);
						}
						else if (m_nSeasonIndex == 3 && i != 1)
						{
							m_ppMaterials[0]->UpdateShaderVariables(pd3dCommandList);
							m_ppMaterials[i]->UpdateShaderVariables(pd3dCommandList);
						}
						else if((m_nSeasonIndex == 0 || m_nSeasonIndex == 2)  && i == 0)
						{
							m_ppMaterials[i]->UpdateShaderVariables(pd3dCommandList);
						}

					}
			
				}
			}
			if (!bIsNormal)
			{
				m_pMesh->Render(pd3dCommandList, 0);
			}
			else
			{
				m_pMesh->NormalRender(pd3dCommandList, 0);
			}
		}
		//if (m_pSibling)
		//{
		//	m_pSibling->Render(pd3dCommandList, pCamera, bIsHir, bIsNormal, bIsWire, bIsShadow);
		//}
		//if (m_pChild)
		//{
		//	m_pChild->Render(pd3dCommandList, pCamera, bIsHir, bIsNormal, bIsWire, bIsShadow);
		//}
	}
	
}



///////////////////////////////////////////////////
///
CCornObject::CCornObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
	: CCropObject()
{
	SetOBB(pd3dDevice, pd3dCommandList);
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CCornObject::~CCornObject()
{
}

void CCornObject::Init(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, false);
	m_nHP = 1;
	SetGrowth(0.0f);
}

void CCornObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(0.1f, 0.1f, 0.1f); //
	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;


	m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.6f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}
///////////////////////////////////////////////////
///

CPotatoObject::CPotatoObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
	: CCropObject()
{
	SetOBB(pd3dDevice, pd3dCommandList);
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CPotatoObject::~CPotatoObject()
{
}

void CPotatoObject::Init(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, false);
	m_nHP = 1;
	SetGrowth(0.0f);
}

void CPotatoObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(0.1f, 0.1f, 0.1f); //
	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;


	m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.6f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}

///////////////////////////////////////////////////
///

CPumpkinObject::CPumpkinObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
	: CCropObject()
{
	SetOBB(pd3dDevice, pd3dCommandList);
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CPumpkinObject::~CPumpkinObject()
{
}

void CPumpkinObject::Init(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, false);
	m_nHP = 1;
	SetGrowth(0.0f);
}

void CPumpkinObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(0.1f, 0.1f, 0.1f); //
	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;


	m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.6f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}
///////////////////////////////////////////////////
///

CTomatoObject::CTomatoObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
	: CCropObject()
{
	SetOBB(pd3dDevice, pd3dCommandList);
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CTomatoObject::~CTomatoObject()
{
}

void CTomatoObject::Init(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, false);
	m_nHP = 1;
	SetGrowth(0.0f);
}

void CTomatoObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(0.1f, 0.1f, 0.1f); //
	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;


	m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.6f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}

///////////////////////////////////////////////////
///
CLogPlnaterObject::CLogPlnaterObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
	: CDefaultGameObject()
{
	SetOBB(pd3dDevice, pd3dCommandList);
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CLogPlnaterObject::~CLogPlnaterObject()
{
}

void CLogPlnaterObject::Init(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, true);
}

void CLogPlnaterObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(0.6f, 0.3f, 0.6f); //
	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;

	m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.3f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}
///////////////////////////////////////////////////
///

CBarnObject::CBarnObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
	: CDefaultGameObject()
{
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CBarnObject::~CBarnObject()
{
}

void CBarnObject::Init(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, true);
	SetOBB(pd3dDevice, pd3dCommandList);


}
void CBarnObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(7.5f, 5.0f, 15.0f); //
	m_bIsBoxCollision = false; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠
	m_bIsBoxTriggerCollision = true;

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;

	m_xmf3OBBOffset = XMFLOAT3(0.0f, 5.0f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}


///////////////////////////////////////////////////
///

CRockObject::CRockObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
	: CDefaultGameObject()
{
	SetOBB(pd3dDevice, pd3dCommandList);
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CRockObject::~CRockObject()
{
}

void CRockObject::Init(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, true);
}

void CRockObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(0.4f, 0.3f, 0.3f); //
	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;

	m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.3f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}

///////////////////////////////////////////////////
///

CWallObject::CWallObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
	: CDefaultGameObject()
{
	SetOBB(pd3dDevice, pd3dCommandList);
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CWallObject::~CWallObject()
{
}

void CWallObject::Init(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, true);
	m_npPlaceable = new UINT[4];
	for (int i = 0; i < 4; i++)
	{
		m_npPlaceable[i] = 0;
	}
}

void CWallObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(0.145f, 1.5f, 2.5f);//
	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;

	m_xmf3OBBOffset = XMFLOAT3(0.0f, 1.5f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}

///////////////////////////////////////////////////
///

CCeilingObject::CCeilingObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
	: CDefaultGameObject()
{
	SetOBB(pd3dDevice, pd3dCommandList);
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CCeilingObject::~CCeilingObject()
{
}

void CCeilingObject::Init(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, true);
	m_npPlaceable = new UINT[4];
	for (int i = 0; i < 4; i++)
	{
		m_npPlaceable[i] = 0;
	}
}

void CCeilingObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(2.5f, 0.145f, 2.5f); //
	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;

	m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.125f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}

///////////////////////////////////////////////////
///

CFloorObject::CFloorObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
	: CDefaultGameObject()
{
	SetOBB(pd3dDevice, pd3dCommandList);
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CFloorObject::~CFloorObject()
{
}

void CFloorObject::Init(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, true);
	m_npPlaceable = new UINT[4];
	for (int i = 0; i < 4; i++)
	{
		m_npPlaceable[i] = 0;
	}
}

void CFloorObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(2.5f, 0.25f, 2.5f);//
	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;

	m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.25f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}
///////////////////////////////////////////////////
///

CTreeObject::CTreeObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
	: CDefaultGameObject()
{
	SetOBB(pd3dDevice, pd3dCommandList);
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CTreeObject::~CTreeObject()
{
}

void CTreeObject::Init(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, true);

	m_nHP = 5;
}

void CTreeObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(0.25f, 0.6f, 0.25f); //
	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;

	m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.6f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}

///////////////////////////////////////////////////
///

CGrassObject::CGrassObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
	: CDefaultGameObject()
{
	SetOBB(pd3dDevice, pd3dCommandList);
	Init(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pModel);
}

CGrassObject::~CGrassObject()
{
}

void CGrassObject::Init(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature,
	CLoadedModelInfo* pModel)
{
	CLoadedModelInfo* pDefaultModel = pModel;
	SetChild(pDefaultModel->m_pModelRootObject, true);
}

void CGrassObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_obbBox.Extents = XMFLOAT3(0.25f, 0.25f, 0.25f); //
	m_bIsBoxCollision = false; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠
	m_bIsBoxTriggerCollision = true;

	float fWidth = m_obbBox.Extents.x * 2.0f;
	float fHeight = m_obbBox.Extents.y * 2.0f;
	float fDepth = m_obbBox.Extents.z * 2.0f;

	m_xmf3OBBOffset = XMFLOAT3(0.0f, 0.3f, 0.0f);

	//m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
}

///////////////////////////////////////////////////
///
CBuildingObject::CBuildingObject(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CGameObject()
{
	SetOBB(pd3dDevice, pd3dCommandList);
}

CBuildingObject::~CBuildingObject()
{
	if (m_pOBBData)
	{
		// OBB Mesh 해제
		for (UINT i = 0; i < m_nOBBData; ++i)
		{
			if (m_pOBBData[i].m_pOBBMesh)
			{
				m_pOBBData[i].m_pOBBMesh->ReleaseUploadBuffers();
				m_pOBBData[i].m_pOBBMesh->Release();
				delete m_pOBBData[i].m_pOBBMesh;
				m_pOBBData[i].m_pOBBMesh = nullptr;
			}
		}
		// PreOBBData 자체 해제
		delete m_pOBBData;
		m_pOBBData = nullptr;
	}

	if (m_pOBBMesh)
	{
		m_pOBBMesh->ReleaseUploadBuffers();
		m_pOBBMesh->Release();
		delete m_pOBBMesh;
	}

	if (m_pMesh)
	{
		m_pMesh->ReleaseUploadBuffers();
		m_pMesh->Release();
		delete m_pMesh;
	}
}

//이 친구는 나중에 그림자 관련해서 없애는게 나을듯? 이게 내 기억으론 파랗게 지을 물건에 형태를 보여주는 기능을하는
//오브젝트로 기억
void CBuildingObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bIsHir, bool bIsNormal, bool bIsWire, bool bIsShadow)
{
	//건물 설치 관련해서 ==> 이게 최상위 부모 노드에서 갖고있는 값을 반영해야해서

	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 1, &m_nIsBuildingObject, 17);
	CGameObject::Render(pd3dCommandList, pCamera, bIsHir, bIsNormal, bIsWire, bIsShadow);
	//이걸 한 번 해줘야 동일한 Shader PSO를 갖고있는 오브젝트들에 영향을 주지 않음 
	//Direct12는 상태머신이기 때문.
	UINT n = 0;
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 1, &n, 17);
}

void CBuildingObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

	m_bIsBoxCollision = true; //충돌 계산을 활성화 한다 ==> 이걸 해야 우선 update해주고 그런거 다 활성화 시켜둠

	float fWidth = 0.0f;
	float fHeight = 0.0f;
	float fDepth = 0.0f;

	m_nOBBData = 9; //WoodMan, LogPlanter, Cone , Wall, Ceiling, Floor

	m_pOBBData = new PreOBBData[m_nOBBData];

	//WoodMan
	m_pOBBData[0].m_objectType = EObjectModelType::WoodMan;
	m_pOBBData[0].m_OBBBox.Center = XMFLOAT3(0, 0, 0);
	m_pOBBData[0].m_OBBBox.Extents = XMFLOAT3(0.3f, 0.25f, 0.85f);
	fWidth = m_pOBBData[0].m_OBBBox.Extents.x * 2.0f;
	fHeight = m_pOBBData[0].m_OBBBox.Extents.y * 2.0f;
	fDepth = m_pOBBData[0].m_OBBBox.Extents.z * 2.0f;
	m_pOBBData[0].m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
	m_pOBBData[0].m_xmf3Offset = XMFLOAT3(0.0f, 0.85f, 0.0f);
	
	//LogPlanter
	m_pOBBData[1].m_objectType = EObjectModelType::LogPlanter;
	m_pOBBData[1].m_OBBBox.Center = XMFLOAT3(0, 0, 0);
	m_pOBBData[1].m_OBBBox.Extents = XMFLOAT3(0.6f, 0.3f, 0.6f); //
	fWidth = m_pOBBData[1].m_OBBBox.Extents.x * 2.0f;
	fHeight = m_pOBBData[1].m_OBBBox.Extents.y * 2.0f;
	fDepth = m_pOBBData[1].m_OBBBox.Extents.z * 2.0f;
	m_pOBBData[1].m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
	m_pOBBData[1].m_xmf3Offset = XMFLOAT3(0.0f, 0.3f, 0.0f);

	//Cone
	m_pOBBData[2].m_objectType = EObjectModelType::CornSeed;
	m_pOBBData[2].m_OBBBox.Center = XMFLOAT3(0, 0, 0);
	m_pOBBData[2].m_OBBBox.Extents = XMFLOAT3(0.1f, 0.1f, 0.1f); //
	fWidth = m_pOBBData[2].m_OBBBox.Extents.x * 2.0f;
	fHeight = m_pOBBData[2].m_OBBBox.Extents.y * 2.0f;
	fDepth = m_pOBBData[2].m_OBBBox.Extents.z * 2.0f;
	m_pOBBData[2].m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
	m_pOBBData[2].m_xmf3Offset = XMFLOAT3(0.0f, 0.6f, 0.0f);

	//Potato
	m_pOBBData[3].m_objectType = EObjectModelType::PotatoSeed;
	m_pOBBData[3].m_OBBBox.Center = XMFLOAT3(0, 0, 0);
	m_pOBBData[3].m_OBBBox.Extents = XMFLOAT3(0.1f, 0.1f, 0.1f); //
	fWidth = m_pOBBData[3].m_OBBBox.Extents.x * 2.0f;
	fHeight = m_pOBBData[3].m_OBBBox.Extents.y * 2.0f;
	fDepth = m_pOBBData[3].m_OBBBox.Extents.z * 2.0f;
	m_pOBBData[3].m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
	m_pOBBData[3].m_xmf3Offset = XMFLOAT3(0.0f, 0.6f, 0.0f);

	//Pumpkin
	m_pOBBData[4].m_objectType = EObjectModelType::PumpkinSeed;
	m_pOBBData[4].m_OBBBox.Center = XMFLOAT3(0, 0, 0);
	m_pOBBData[4].m_OBBBox.Extents = XMFLOAT3(0.1f, 0.1f, 0.1f); //
	fWidth = m_pOBBData[4].m_OBBBox.Extents.x * 2.0f;
	fHeight = m_pOBBData[4].m_OBBBox.Extents.y * 2.0f;
	fDepth = m_pOBBData[4].m_OBBBox.Extents.z * 2.0f;
	m_pOBBData[4].m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
	m_pOBBData[4].m_xmf3Offset = XMFLOAT3(0.0f, 0.6f, 0.0f);

	//Tomato
	m_pOBBData[5].m_objectType = EObjectModelType::TomatoSeed;
	m_pOBBData[5].m_OBBBox.Center = XMFLOAT3(0, 0, 0);
	m_pOBBData[5].m_OBBBox.Extents = XMFLOAT3(0.1f, 0.1f, 0.1f); //
	fWidth = m_pOBBData[5].m_OBBBox.Extents.x * 2.0f;
	fHeight = m_pOBBData[5].m_OBBBox.Extents.y * 2.0f;
	fDepth = m_pOBBData[5].m_OBBBox.Extents.z * 2.0f;
	m_pOBBData[5].m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
	m_pOBBData[5].m_xmf3Offset = XMFLOAT3(0.0f, 0.6f, 0.0f);

	// Wall (CWallObject 기준)
	m_pOBBData[6].m_objectType = EObjectModelType::Wall;
	m_pOBBData[6].m_OBBBox.Center = XMFLOAT3(0, 0, 0);
	m_pOBBData[6].m_OBBBox.Extents = XMFLOAT3(0.145f, 1.5f, 2.5f);
	fWidth = m_pOBBData[6].m_OBBBox.Extents.x * 2.0f;
	fHeight = m_pOBBData[6].m_OBBBox.Extents.y * 2.0f;
	fDepth = m_pOBBData[6].m_OBBBox.Extents.z * 2.0f;
	m_pOBBData[6].m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
	m_pOBBData[6].m_xmf3Offset = XMFLOAT3(0.0f, 1.5f, 0.0f);

	// Floor (CFloorObject 기준)
	m_pOBBData[7].m_objectType = EObjectModelType::Floor;
	m_pOBBData[7].m_OBBBox.Center = XMFLOAT3(0, 0, 0);
	m_pOBBData[7].m_OBBBox.Extents = XMFLOAT3(2.5f, 0.25f, 2.5f);
	fWidth = m_pOBBData[7].m_OBBBox.Extents.x * 2.0f;
	fHeight = m_pOBBData[7].m_OBBBox.Extents.y * 2.0f;
	fDepth = m_pOBBData[7].m_OBBBox.Extents.z * 2.0f;
	m_pOBBData[7].m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
	m_pOBBData[7].m_xmf3Offset = XMFLOAT3(0.0f, 0.25f, 0.0f);

	// Ceiling (CCeilingObject 기준)
	m_pOBBData[8].m_objectType = EObjectModelType::Ceiling;
	m_pOBBData[8].m_OBBBox.Center = XMFLOAT3(0, 0, 0);
	m_pOBBData[8].m_OBBBox.Extents = XMFLOAT3(2.5f, 0.145f, 2.5f);
	fWidth = m_pOBBData[8].m_OBBBox.Extents.x * 2.0f;
	fHeight = m_pOBBData[8].m_OBBBox.Extents.y * 2.0f;
	fDepth = m_pOBBData[8].m_OBBBox.Extents.z * 2.0f;
	m_pOBBData[8].m_pOBBMesh = new CBoundingCubeMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, fDepth);
	m_pOBBData[8].m_xmf3Offset = XMFLOAT3(0.0f, 0.125f, 0.0f);

}

void CBuildingObject::ResetOBB(PreOBBData m_OBBData)
{
	//검색한 내용으로 바꿔준다.
	//m_pOBBMesh = m_OBBData.m_pOBBMesh;

	m_obbBox = m_OBBData.m_OBBBox;

	m_xmf3OBBOffset = m_OBBData.m_xmf3Offset;

	m_objType = m_OBBData.m_objectType;

	//TODO: 여기 이 switch문 자체를 줄이거나 할 방법을 좀 찾아봐야할듯 그니까 논리구조를 바꿔봐야할듯
	switch (m_objType)
	{
			// Characters
		case EObjectModelType::WoodMan:
		case EObjectModelType::WorkMan:
		{
			m_objTypeTag = EObjectTag::Player;
			break;
		}
			// Corn / Potato / Pumpkin / Tomato / Seed (200~510)
		case EObjectModelType::Corn01: case EObjectModelType::Corn02: case EObjectModelType::Corn03: case EObjectModelType::Corn04:
		case EObjectModelType::CornSeed:

		case EObjectModelType::Potato01: case EObjectModelType::Potato02: case EObjectModelType::Potato03: case EObjectModelType::Potato04:
		case EObjectModelType::PotatoSeed:

		case EObjectModelType::Pumpkin01: case EObjectModelType::Pumpkin02: case EObjectModelType::Pumpkin03: case EObjectModelType::Pumpkin04:
		case EObjectModelType::PumpkinSeed:

		case EObjectModelType::Tomato01: case EObjectModelType::Tomato02: case EObjectModelType::Tomato03: case EObjectModelType::Tomato04:
		case EObjectModelType::TomatoSeed:
		{
			m_objTypeTag = EObjectTag::Crop;
			break;
		}
			// Environment 오브젝트들 (600번대)
		case EObjectModelType::LogPlanter:
		case EObjectModelType::Barn:
		case EObjectModelType::Rock01:
		case EObjectModelType::Rock02:
		case EObjectModelType::Tree01:
		case EObjectModelType::Tree02:
		case EObjectModelType::Grass01:
		case EObjectModelType::Grass02:
		{
			m_objTypeTag = EObjectTag::Environment;
			break;
		}
		//구조물
		case EObjectModelType::Floor:
		{
			m_objTypeTag = EObjectTag::Floor;
			break;
		}
		case EObjectModelType::Wall:
		{
			m_objTypeTag = EObjectTag::Wall;
			break;
		}
		case EObjectModelType::Ceiling:
		{
			m_objTypeTag = EObjectTag::Ceiling;
			break;
		}

		default:
		{
			m_objTypeTag = EObjectTag::None;
			break;
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CPostProcessingObject::CPostProcessingObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{

	CScreenMesh* pScreenMesh = new CScreenMesh(pd3dDevice, pd3dCommandList);
	SetMesh(pScreenMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CPostProcessingShader* pPostProcessingShader = new CPostProcessingShader();
	pPostProcessingShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);


	//1개의 Texture를 갖는 Material
	CMaterial* pPostProcessingMaterial = new CMaterial(1);

	for (int i = 0; i < pPostProcessingMaterial->m_nTextures; i++)
	{
		pPostProcessingMaterial->m_ppTextures[i] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	}

	//pPostProcessingMaterial->m_ppTextures[0]->
	//pPlanMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/ground_03.dds", RESOURCE_TEXTURE2D, 0);
	//pPlanMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	//pPlanMaterial->m_ppTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/ground_03_NRM.dds", RESOURCE_TEXTURE2D, 0);
	//pPlanMaterial->SetMaterialType(MATERIAL_NORMAL_MAP);
	/*CScene::CreateShaderResourceViews(pd3dDevice, pPostProcessingMaterial->m_ppTextures[0], 0, 4);
	CScene::CreateShaderResourceViews(pd3dDevice, pPostProcessingMaterial->m_ppTextures[1], 0, 5);*/
	//pPlanMaterial->SetTexture(pPlanTexture);
	//pPlanMaterial->SetTexture(pPlanTexture);

	pPostProcessingMaterial->SetShader(pPostProcessingShader);

	//1번째 재질 연결.
	SetMaterial(0, pPostProcessingMaterial);
}

CPostProcessingObject::~CPostProcessingObject()
{
}

//
void CPostProcessingObject::Render(ID3D12GraphicsCommandList* pd3dCommandList , CCamera* pCamera)
{
	if (m_pMesh)
	{
		//PostProcessing을 위한 오브젝트는 항상 스크린좌표상의 좌표를 고정으로 갖고있는 Plan을 갖고있을 것이기 떄문에
		//따로 월드변환행렬을 넘겨줄 이유가 없다.
		//UpdateShaderVariables(pd3dCommandList, &m_xmf4x4World);
		if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					if (m_ppMaterials[i]->m_pShader)
					{
						m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
					}
					m_ppMaterials[i]->UpdateShaderVariables(pd3dCommandList);
				}
				m_pMesh->Render(pd3dCommandList, i);
			
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

void CGameObject::FillProtoTransform(Protocol::Transform* pTrans)
{
	if (pTrans == nullptr) return;


	pTrans->set_qx(GetQuaternion().x);
	pTrans->set_qy(GetQuaternion().y);
	pTrans->set_qz(GetQuaternion().z);
	pTrans->set_qw(GetQuaternion().w);
	// Position (px, py, pz)

	pTrans->set_px(m_xmf4x4World._41);
	pTrans->set_py(m_xmf4x4World._42);
	pTrans->set_pz(m_xmf4x4World._43);
}

XMFLOAT4 CGameObject::GetQuaternion() const
{
	// 로컬 행렬 로드
	XMMATRIX m = XMLoadFloat4x4(&m_xmf4x4ToParent);

	// translation 제거 (회전만 추출)
	m.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);

	// 회전 행렬 → quaternion
	XMVECTOR q = XMQuaternionRotationMatrix(m);
	q = XMQuaternionNormalize(q);

	XMFLOAT4 out;
	XMStoreFloat4(&out, q);
	return out;
	//return XMFLOAT4();
}

void CGameObject::SetQuaternion(const XMFLOAT4& quat)
{
	// 현재 로컬 행렬
	XMMATRIX cur = XMLoadFloat4x4(&m_xmf4x4ToParent);

	// 위치 보존
	XMVECTOR pos = cur.r[3];

	// quaternion → rotation matrix
	XMVECTOR q = XMLoadFloat4(&quat);
	q = XMQuaternionNormalize(q);

	XMMATRIX rot = XMMatrixRotationQuaternion(q);

	// 위치 복원
	rot.r[3] = pos;

	XMStoreFloat4x4(&m_xmf4x4ToParent, rot);

	//  NULL 허용되는 UpdateTransform이어야 함
	UpdateTransform(NULL);
}

CParticleObject::CParticleObject() : CGameObject(1)
{
}

CParticleObject::~CParticleObject()
{
	if (m_cpcbParticle)
	{
		m_cpcbParticle->Unmap(0, nullptr);
	}
	m_pcbParticle = nullptr;
}

void CParticleObject::UpdateShaderVariables(
	ID3D12GraphicsCommandList* pd3dCommandList,
	XMFLOAT4X4* pxmf4x4World)
{
	CGameObject::UpdateShaderVariables(pd3dCommandList, pxmf4x4World);

	pd3dCommandList->SetGraphicsRootConstantBufferView(
		24, // 네가 쓰던 Particle CB 슬롯
		m_cpcbParticle->GetGPUVirtualAddress());
}

void CParticleObject::SetParticleState(
	float fTime,
	float fTimeElapsed,
	float fSpawnRadius,
	float fAlphaScale,
	UINT  nParticleFlags,
	XMFLOAT3 gParticleColor,
	UINT nBlockCount,
	WeatherBlock* blocks)
{
	if (!m_pcbParticle) return;

	m_pcbParticle->fTime = fTime;
	m_pcbParticle->fDeltaTime = fTimeElapsed;
	m_pcbParticle->fSpawnRadius = fSpawnRadius;
	m_pcbParticle->fAlphaScale = fAlphaScale;

	m_pcbParticle->nParticleFlags = nParticleFlags;
	m_pcbParticle->gParticleColor = gParticleColor;

	UINT copyCount = min(nBlockCount, (UINT)MAX_BLOCK);
	m_pcbParticle->blockCount = copyCount;

	for (UINT i = 0; i < copyCount; ++i)
		m_pcbParticle->blocks[i] = blocks[i];

	for (UINT i = copyCount; i < MAX_BLOCK; ++i)
	{
		m_pcbParticle->blocks[i].center = XMFLOAT3(0, 0, 0);
		m_pcbParticle->blocks[i].extent = XMFLOAT3(0, 0, 0);
	}
}

void CParticleObject::SetPreviousPosition(XMFLOAT3 pos)
{
	m_xmf3PreviousPosition = pos;
}

XMFLOAT3 CParticleObject::GetPreviousPosition()
{
	return m_xmf3PreviousPosition;
}


///////////////////////////////////////////////////
///
CRainParticleObject::CRainParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nCount)
{

	ParticleMesh* pParticleMesh = new ParticleMesh(pd3dDevice, pd3dCommandList, nCount, PF_RAIN);
	SetMesh(pParticleMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pRainTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pRainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/rain.dds", RESOURCE_TEXTURE2D, 0);

	CParticleShader* pParticleShader = new CParticleShader();
	pParticleShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pRainTexture, 0, 4);

	CMaterial* pParticleMaterial = new CMaterial(1);
	pParticleMaterial->SetTexture(pRainTexture);
	pParticleMaterial->SetShader(pParticleShader);

	SetMaterial(0, pParticleMaterial);

	UINT ncbElementBytes = ((sizeof(cbParticle) + 255) & ~255); //256의 배수
	m_cpcbParticle = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpcbParticle->Map(0, NULL, (void**)&m_pcbParticle);
}

CRainParticleObject::~CRainParticleObject()
{
	if (m_cpcbParticle)
	{
		m_cpcbParticle->Unmap(0, nullptr);
	}

	m_pcbParticle = nullptr;
}

///////////////////////////////////////////////////
///
CSnowParticleObject::CSnowParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nCount)
{
	ParticleMesh* pParticleMesh = new ParticleMesh(pd3dDevice, pd3dCommandList, nCount, PF_SNOW);
	SetMesh(pParticleMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pRainTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pRainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/snow.dds", RESOURCE_TEXTURE2D, 0);

	CParticleShader* pParticleShader = new CParticleShader();
	pParticleShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pRainTexture, 0, 4);

	CMaterial* pParticleMaterial = new CMaterial(1);
	pParticleMaterial->SetTexture(pRainTexture);
	pParticleMaterial->SetShader(pParticleShader);

	SetMaterial(0, pParticleMaterial);

	UINT ncbElementBytes = ((sizeof(cbParticle) + 255) & ~255); //256의 배수
	m_cpcbParticle = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpcbParticle->Map(0, NULL, (void**)&m_pcbParticle);
}

CSnowParticleObject::~CSnowParticleObject()
{
	if (m_cpcbParticle)
	{
		m_cpcbParticle->Unmap(0, nullptr);
	}

	m_pcbParticle = nullptr;
}


///////////////////////////////////////////////////
///

CDustParticleObject::CDustParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nCount)
{
	ParticleMesh* pParticleMesh = new ParticleMesh(pd3dDevice, pd3dCommandList, nCount, PF_DUST);
	SetMesh(pParticleMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pRainTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pRainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/dust.dds", RESOURCE_TEXTURE2D, 0);

	CParticleShader* pParticleShader = new CParticleShader();
	pParticleShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pRainTexture, 0, 4);

	CMaterial* pParticleMaterial = new CMaterial(1);
	pParticleMaterial->SetTexture(pRainTexture);
	pParticleMaterial->SetShader(pParticleShader);

	SetMaterial(0, pParticleMaterial);

	UINT ncbElementBytes = ((sizeof(cbParticle) + 255) & ~255); //256의 배수
	m_cpcbParticle = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpcbParticle->Map(0, NULL, (void**)&m_pcbParticle);
}

CDustParticleObject::~CDustParticleObject()
{
	if (m_cpcbParticle)
	{
		m_cpcbParticle->Unmap(0, nullptr);
	}

	m_pcbParticle = nullptr;
}
///////////////////////////////////////////////////
///

CPunchParticleObject::CPunchParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nCount)
{

	ParticleMesh* pParticleMesh = new ParticleMesh(pd3dDevice, pd3dCommandList, nCount, PF_PUNCH);
	SetMesh(pParticleMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//CTexture* pRainTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	//pRainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/rain.dds", RESOURCE_TEXTURE2D, 0);

	CParticleShader* pParticleShader = new CParticleShader();
	pParticleShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//CScene::CreateShaderResourceViews(pd3dDevice, pRainTexture, 0, 4);

	CMaterial* pParticleMaterial = new CMaterial(0);
	pParticleMaterial->SetTexture(NULL);
	pParticleMaterial->SetShader(pParticleShader);

	SetMaterial(0, pParticleMaterial);

	UINT ncbElementBytes = ((sizeof(cbParticle) + 255) & ~255); //256의 배수
	m_cpcbParticle = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpcbParticle->Map(0, NULL, (void**)&m_pcbParticle);
}

CPunchParticleObject::~CPunchParticleObject()
{
	if (m_cpcbParticle)
	{
		m_cpcbParticle->Unmap(0, nullptr);
	}

	m_pcbParticle = nullptr;
}

void CPunchParticleObject::Animate(float fTimeElapsed, float fTime)
{

	m_fCircleTime += fTimeElapsed;

	if (m_fCircleTime >= 1.0f)
	{
		m_fCircleTime = 0.0f;
		Activate(false);
		return;
	}
	//m_pcbParticle->fStartTime += fTimeElapsed;

}

void CPunchParticleObject::PrintStartTIme()
{

	std::cout << "초기 시작 시간 : " << m_pcbParticle->fStartTime << endl;
	std::cout << "현재 시간 : " << m_pcbParticle->fTime << endl;
}

void CPunchParticleObject::SetParticleState(
	float fTime,
	float fTimeElapsed,
	float fSpawnRadius,
	float fAlphaScale,
	UINT  nParticleFlags,
	XMFLOAT3 gParticleColor,
	UINT nBlockCount,
	WeatherBlock* blocks)
{
	if (!m_pcbParticle) return;

	CParticleObject::SetParticleState(fTime, fTimeElapsed, fSpawnRadius, fAlphaScale, nParticleFlags, gParticleColor, nBlockCount, blocks);
	m_pcbParticle->fStartTime = m_fCircleTime;

}

//TODO : 오류 생기면 점검 필요
void CCropObject::ChangeModel(CLoadedModelInfo* pNewModel)
{
	if (!pNewModel)
		return;

	// 이미 같은 모델이면 스킵
	if (m_pChild == pNewModel->m_pModelRootObject)
		return;

	// 기존 모델 제거
	if (m_pChild)
	{
		//m_pChild->Release();
		m_pChild = nullptr;
	}

	// 새 모델 장착
	SetChild(pNewModel->m_pModelRootObject, false);

	//모델이 들고 있는 타입으로 오브젝트 타입 갱신
	SetType(pNewModel->m_ObjectType);
}
/// ///////////////////////////////////////////////////////////////////////////////////////////////////

void CDefulatHumanAnimationCallbackHandler::HandleCallback(void* pCallbackData, float fTrackPosition)
{
	EAnimationState state =
		(EAnimationState)(uintptr_t)pCallbackData;

	switch (state)
	{
	case EAnimationState::punch:
	{
		m_pController->SetTrackPosition(1, -ANIMATION_CALLBACK_EPSILON);
		m_pController->SetTrackEnable(0, true);
		m_pController->SetTrackEnable(1, false);
		return;
		break;
	}

	case EAnimationState::pickup:
	{
		// pickup 처리
		m_pController->SetTrackPosition(1, -ANIMATION_CALLBACK_EPSILON);
		m_pController->SetTrackEnable(0, true);
		m_pController->SetTrackEnable(1, false);
		return;
		break;
	}
	case EAnimationState::watering:
	{
		// watering 처리
		return;
		break;
	}
	default:
	{
		break;
	}
	}
}
