#include "stdafx.h"

ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	void* pData, UINT nBytes, D3D12_HEAP_TYPE d3dHeapType,
	D3D12_RESOURCE_STATES d3dResoucreStates, ID3D12Resource** ppd3dUploadBuffer)
{
	//생성해서 반환해줄 실제 버퍼리소스
	ComPtr<ID3D12Resource> pd3dBuffer = nullptr;
	/*
	* 리소스를 생성하기 위해선 기본적으로 Heap을 어떤 Type으로 어떻게 할당할건지와 해당 리소스를 어떤 종류로 만들건지 2가지를 정해줘야한다.
	* D3D12_HEAP_PROPERTIES가 리소스가 저장된 HEAP을 정의하는거
	* D3D12_RESOURCE_DESC가 리소스를 어떤 종류로 만들지를 정의하는거
	*/
	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = d3dHeapType;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //일반적인 정점 정보 같은 버펄를 넘기거나 상수 버퍼를 넘기거나할때는 BUFFER로 만든다. 만일 ShaderResource같은건 TEXTURE2D, TEXTURE3D 등등 TEXTURE 종류로 만들어준다.
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = nBytes; //Width는 일반 BUFFER나 TEXTURE 상관없이 둘 다 이용
	d3dResourceDesc.Height = 1; //내 기억으로는 일반 BUFFER는 Height를 1로 고정 서브리소스가 있는경우 제외하고?? 그리고 TEXTURE의 경우에는 해상도의 Height를 넘겨줄 필요가 있다.
	d3dResourceDesc.DepthOrArraySize = 1; //TEXTURE3D에 경우는 숫자가 올라갈 수 있다. 아니면 배열을 갖고있는?? 그러한 경우에 설정이 필요한걸로 암
	d3dResourceDesc.MipLevels = 1; // MipMAP을 지원하는 TEXTUREE의 경우에는 1이 아닌 이외 값을 쓰는경우가 있으며 0을 쓸 경우 최대 Mip 수준을 다 사용할 수 있게 한다는 의미이다.
	d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN; //TEXTUREE의 경우 RGBA등등에 크기를 지정해줘야해서 UNKNOWN을 쓰지 않는다.
	d3dResourceDesc.SampleDesc.Count = 1; //MSAA의 수준을 정하는것.
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; //GPU 메모리 상 배치 방식을 명시하는 필드로 총 4가지 종류가 존재 한다. 이것은 행 기반 메모리 배치를 의미한다.
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE; //이건 Shader에 여러 단계에서 어느 단계에서 리소스에 접근을 허용해줄지를 지정해주는부분 이었던걸로 기억.



	D3D12_RESOURCE_STATES d3dResourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
	if (d3dHeapType == D3D12_HEAP_TYPE_UPLOAD)
	{
		d3dResourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
	else if (d3dHeapType == D3D12_HEAP_TYPE_READBACK)
	{
		d3dResourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
	}
	//CreateCommittedResource를 사용해 리소스를 기본적으로 생성하며 이 함수는 내부적으로 HEAP에 공간을 자동으로 생성해주기에 편리한 함수이다.
	HRESULT hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, d3dResourceInitialStates, NULL, __uuidof(ID3D12Resource), (void**)pd3dBuffer.GetAddressOf());

	if (pData)
	{
		switch (d3dHeapType)
		{
		case D3D12_HEAP_TYPE_DEFAULT: //DEFAULT 타입으로 지정된 경우
		{
			//DEFUALT타입에 경우 GPU에서만 접근이 가능함으로 값을 집어넣기 위해서는 같은 GPU 상에 존재하는 버퍼에서 CommandList를 통한 Copy를 해야지만 값을 넣을 수 있다.
			//그리고 실제 Data는 기본적으로 CPU메모리 상에 존재함으로 그 값을 넣을 수 있는 UploadBuffer를 만들고 해당 UploadBuffer에 pData값을 넣고 
			//CommandList의 CopyResource를 통해서 Defualt버퍼에 UploadBuffer에 값을 넣는다.
			if (ppd3dUploadBuffer)
			{
				d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
				hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)ppd3dUploadBuffer);
				/*			if (FAILED(hResult) || !(*ppd3dUploadBuffer)) {
								OutputDebugString(L"UploadBuffer 생성 실패!\n");
								return nullptr;
							}*/
							//업로드 버퍼에 pData에 값을 Copy해주는것 이건 GPU상에 있는 DEFAULT에 경우 사용 불가.
				D3D12_RANGE d3dReadRange = { 0,0 };
				UINT8* pBufferDataBegin = NULL;
				(*ppd3dUploadBuffer)->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
				memcpy(pBufferDataBegin, pData, nBytes);
				(*ppd3dUploadBuffer)->Unmap(0, NULL);

				//Copy명령어를 통해 업로드 버퍼에 값을 디폴트 버퍼에 넣어준다.
				pd3dCommandList->CopyResource(pd3dBuffer.Get(), *ppd3dUploadBuffer);

				D3D12_RESOURCE_BARRIER d3dResourceBarrier;
				::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
				d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				d3dResourceBarrier.Transition.pResource = pd3dBuffer.Get();
				d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				d3dResourceBarrier.Transition.StateAfter = d3dResoucreStates;
				d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
				break;
			}
		}
		case D3D12_HEAP_TYPE_UPLOAD:
		{
			D3D12_RANGE d3dReadRange = { 0, 0 };
			UINT8* pBufferDataBegin = NULL;
			pd3dBuffer->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin, pData, nBytes);
			pd3dBuffer->Unmap(0, NULL);
			break;
		}
		case D3D12_HEAP_TYPE_READBACK:
		{
			break;
		}
		}


	}
	return (pd3dBuffer);
}
