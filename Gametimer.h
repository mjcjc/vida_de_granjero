#pragma once
#include "stdafx.h"

const ULONG MAX_SAMPLE_COUNT = 50; // 50회의 프레임 처리시간을 누적하여 평균한다.
//dletaTime 등의 프레임 간격 시간을 측정 혹은 이런 평균없이 실시간 계속 계산하도록 한다면
// 프레임이 엄청나게 튀는걸 볼 수 있다.

class CGameTimer
{
public:
	CGameTimer();
	virtual ~CGameTimer();

	void Start() {}
	void Stop() {}
	void Reset();
	void Tick(float fLockFPS = 0.0f); // 타이머의 시간을 갱신한다.
	unsigned long GetFrameRate(LPTSTR lpszString = NULL, int nCharacters = 0); //프레임 레이트가 얼마인지 출력
	float GetTimeElapsed(); // 프레임 평균 경과시간을 반환한다.

private:
	bool m_bHardwareHasPerformanceCounter;	//컴푸터가  Performance Counter를 가지고 있는가 여부
	float m_fTimeScale; // Scale Counter의 양
	float m_fTimeElapsed; // 마지막 프레임 이후 지난 시간 그러니까 프레임간 시간 간격
	__int64 m_nCurrentTime; // 현재의 시간
	__int64 m_nLastTime;	//마지막 프레임의 시간
	__int64 m_nPerformanceFrequency; //컴퓨터의 Performance Frequency

	float m_fFrameTime[MAX_SAMPLE_COUNT];	//프레임 시간을 누적하기 위한 배열
	ULONG m_nSampleCount;	//누적된 프레임 횟수

	unsigned long m_nCurrentFrameRate;	//현재의 프레임 레이트
	unsigned long m_nFramePerSecond;	//초당 프레임 수
	float m_fFPSTimeElapsed;	//프레임 레이트 계산 소요 시간

	bool m_bStopped;


};

