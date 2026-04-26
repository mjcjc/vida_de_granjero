#pragma once


class CGameObject;
class CSkinnedMesh;
class CLoadedModelInfo;
////////////////////////////////////////////////////////////////////////////////////////////////////
//  CAnimationClip
//  하나의 애니메이션 클립(예: Walk, Run, Attack)을 표현하는 클래스
//  각 본(Bone)에 대한 키프레임 데이터를 시간에 따라 저장하고, 보간(Interpolation)을 통해 현재 프레임의 변환행렬을 계산함
/////////////////////////////////////////////////////////////////////////////////////////////////////

class CAnimationClip
{
	// 각 본의 특정 시간(fPosition)에서의 변환행렬(SRT 또는 Transform)을 계산하여 반환
	// 키프레임 사이의 보간(Lerp 또는 Slerp)을 통해 자연스러운 애니메이션 전환을 수행
	// 멤버 구성 :
	// m_pfKeyFrameTimes            : 각 키프레임의 시간 정보
	// m_ppxmf4x4KeyFrameTransforms : [키프레임][본] 형태의 2차원 배열로 변환행렬 저장
	// m_fLength, m_nFramesPerSecond: 애니메이션의 길이 및 초당 프레임 수
public:
	CAnimationClip(float fLength, int nFramesPerSecond, int nKeyFrameTransforms, int nSkinningBones, char* pstrName);
	~CAnimationClip();

public:
	char							m_pstrAnimationSetName[64];

	float							m_fLength = 0.0f;
	int								m_nFramesPerSecond = 0; //m_fTicksPerSecond

	int								m_nKeyFrames = 0;
	float* m_pfKeyFrameTimes = NULL;
	XMFLOAT4X4** m_ppxmf4x4KeyFrameTransforms = NULL;

#ifdef _WITH_ANIMATION_SRT
	int								m_nKeyFrameScales = 0;
	float* m_pfKeyFrameScaleTimes = NULL;
	XMFLOAT3** m_ppxmf3KeyFrameScales = NULL;
	int								m_nKeyFrameRotations = 0;
	float* m_pfKeyFrameRotationTimes = NULL;
	XMFLOAT4** m_ppxmf4KeyFrameRotations = NULL;
	int								m_nKeyFrameTranslations = 0;
	float* m_pfKeyFrameTranslationTimes = NULL;
	XMFLOAT3** m_ppxmf3KeyFrameTranslations = NULL;
#endif

public:
	XMFLOAT4X4 GetSRT(int nBone, float fPosition);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// CAnimationClips
// 여러 개의 애니메이션 클립을 묶어서 관리하는 컨테이너 클래스
// 모델이 가질 수 있는 모든 애니메이션 (예: Idle, Walk, Run 등)을 배열 형태로 관리
/////////////////////////////////////////////////////////////////////////////////////////////////////

class CAnimationClips
{
	// 멤버 구성:
	// m_pAnimationClips : CAnimationClip 포인터 배열 (각 애니메이션 클립 객체)
	// m_ppBoneFrameCaches : 각 본에 대응되는 GameObject 캐시 (현재 프레임의 본 변환 결과를 저장)

public:
	CAnimationClips(int nAnimationClips);
	~CAnimationClips();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }


public:
	int								m_nAnimationClips = 0;
	CAnimationClip** m_pAnimationClips = NULL;

	int								m_nBoneFrames = 0;
	CGameObject** m_ppBoneFrameCaches = NULL; //[m_nBoneFrames]
};

struct CALLBACKKEY
{
	float  							m_fTime = 0.0f;
	void* m_pCallbackData = NULL;
};

class CAnimationCallbackHandler
{
public:
	CAnimationCallbackHandler() { }
	~CAnimationCallbackHandler() { }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition) { }
};

#define _WITH_ANIMATION_INTERPOLATION

/////////////////////////////////////////////////////////////////////////////////////////////////////
// CAnimationTrack
// 실제 재생 중인 애니메이션 트랙의 상태를 관리
// 트랙은 하나의 애니메이션 클립을 재생하며, 속도, 가중치, 루프 여부 등을 제어함
// 동시에 여러 트랙을 재생하여 블렌딩(예: 상체는 Attack, 하체는 Run) 가능
/////////////////////////////////////////////////////////////////////////////////////////////////////

class CAnimationTrack
{
public:
	CAnimationTrack() { }
	~CAnimationTrack();
	// 멤버 구성:
	// m_bEnable   : 트랙 활성화 여부
	// m_fSpeed    : 재생 속도 배율
	// m_fWeight   : 애니메이션 블렌딩 시 가중치
	// m_nType     : 루프/한 번/왕복 재생 타입
	// m_fPosition : 현재 프레임 위치 (시간)
	// m_pCallbackKeys : 애니메이션 이벤트 시점 배열
	// m_pAnimationCallbackHandler : 이벤트 콜백 핸들러
public:
	BOOL 							m_bEnable = true;
	float 							m_fSpeed = 1.0f;
	float 							m_fPosition = -ANIMATION_CALLBACK_EPSILON;
	float 							m_fWeight = 1.0f;
	float							m_fPrevPosition; //
	int 							m_nAnimationClip = 0; //AnimationSet Index

	int 							m_nType = ANIMATION_TYPE_LOOP; //Once, Loop, PingPong

	int 							m_nCallbackKeys = 0;
	CALLBACKKEY* m_pCallbackKeys = NULL;

	CAnimationCallbackHandler* m_pAnimationCallbackHandler = NULL;

public:
	void SetAnimationClip(int nAnimationClip) { m_nAnimationClip = nAnimationClip; }

	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	void SetWeight(float fWeight) { m_fWeight = fWeight; }

	void SetPosition(float fPosition) { m_fPosition = fPosition; }

	void SetType(int type) { m_nType = type; }

	// 현재 트랙의 위치를 업데이트 (Loop, Once, PingPong 타입 지원)
	// fTrackPosition: 현재 위치, fElapsedTime: 경과 시간
	float UpdatePosition(float fTrackPosition, float fTrackElapsedTime, float fAnimationLength);

	// 콜백 관련 키프레임 등록 (특정 시간에 이벤트를 발생시키는 용도)
	void SetCallbackKeys(int nCallbackKeys);
	void SetCallbackKey(int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(CAnimationCallbackHandler* pCallbackHandler);

	// 콜백 핸들러 호출: 특정 시간대(ε 오차 내)에서 애니메이션 이벤트 실행
	void HandleCallback();
};





/////////////////////////////////////////////////////////////////////////////////////////////////////
// CAnimationController
// 애니메이션의 전체 실행을 관리하는 핵심 클래스
// 여러 트랙을 동시에 제어하고, 본별 변환 행렬을 GPU 버퍼에 업로드하여 쉐이더로 전달함
// 스키닝 메시(가중치가 적용된 본 기반 메시)에 대한 본 행렬 업데이트 수행
/////////////////////////////////////////////////////////////////////////////////////////////////////
class CAnimationController
{
public:
	CAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel);
	~CAnimationController();
	// 멤버 구성:
	// m_pAnimationTracks : 재생 중인 트랙 배열
	// m_pAnimationClips  : 모델의 모든 애니메이션 클립
	// m_ppSkinnedMeshes  : 스키닝 메시 배열
	// m_pcpd3dcbSkinningBoneTransforms : 본 행렬용 GPU Constant Buffer
	// m_ppcbxmf4x4MappedSkinningBoneTransforms : 실제 매핑된 CPU 버퍼 포인터
public:
	float 							m_fTime = 0.0f;

	int 							m_nAnimationTracks = 0;
	CAnimationTrack* m_pAnimationTracks = NULL;

	CAnimationClips* m_pAnimationClips = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh** m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

	std::vector<ComPtr<ID3D12Resource>> m_pcpd3dcbSkinningBoneTransforms ; //[SkinnedMeshes]
	XMFLOAT4X4** m_ppcbxmf4x4MappedSkinningBoneTransforms = NULL; //[SkinnedMeshes]

public:
	// GPU에 본 행렬 버퍼를 업데이트 (스키닝 셰이더에서 본 변형 적용용)
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	// 해당 트랙에 Animation Clip을 저장
	void SetTrackAnimationClip(int nAnimationTrack, int nAnimationClip);
	// 트랙 관련 설정 함수 (활성화, 속도, 가중치, 위치 등)
	void SetTrackEnable(int nAnimationTrack, bool bEnable);
	void SetTrackPosition(int nAnimationTrack, float fPosition);
	void SetTrackSpeed(int nAnimationTrack, float fSpeed);
	void SetTrackWeight(int nAnimationTrack, float fWeight);

	void SetTrackType(int nAnimationTrack, int type);

	// 콜백 관련 설정
	void SetCallbackKeys(int nAnimationTrack, int nCallbackKeys);
	void SetCallbackKey(int nAnimationTrack, int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(int nAnimationTrack, CAnimationCallbackHandler* pCallbackHandler);

	//애니메이션 동작
	void AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject);


public:
	bool							m_bRootMotion = false;
	CGameObject* m_pModelRootObject = NULL;

	CGameObject* m_pRootMotionObject = NULL;
	XMFLOAT3						m_xmf3FirstRootMotionPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	void SetRootMotion(bool bRootMotion) { m_bRootMotion = bRootMotion; }

	virtual void OnRootMotion(CGameObject* pRootGameObject) { }
	virtual void OnAnimationIK(CGameObject* pRootGameObject) { }
};

class CAnimation
{
public:


};