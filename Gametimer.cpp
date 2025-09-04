#include "stdafx.h"
#include "GameTimer.h"

CGameTimer::CGameTimer()
{
	if (::QueryPerformanceFrequency((LARGE_INTEGER*)&m_nPerformanceFrequency))
	{
		m_bHardwareHasPerformanceCounter = TRUE;
		::QueryPerformanceCounter((LARGE_INTEGER*)&m_nLastTime);
		m_fTimeScale = 1.0f / m_nPerformanceFrequency;
	}
	else
	{
		m_bHardwareHasPerformanceCounter = FALSE;
		m_nLastTime = ::timeGetTime();
		m_fTimeScale = 0.001f;
	}

	m_nSampleCount = 0;
	m_nCurrentFrameRate = 0;
	m_nFramePerSecond = 0;
	m_fFPSTimeElapsed = 0.0f;
}

CGameTimer::~CGameTimer()
{

}

void CGameTimer::Tick(float fLockFPS)	//fLockFPS로 프레임을 조절
{
	if (m_bHardwareHasPerformanceCounter)
	{
		::QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentTime);
	}
	else
	{
		m_nCurrentTime = ::timeGetTime();
	}


	float fTimeElapsed = (m_nCurrentTime - m_nLastTime) * m_fTimeScale;	// 프레임 간격의 시간을 측정

	if (fLockFPS > 0.0f)	//0.0f보다 크다는건 프레임을 조정한다는 뜻.
	{
		while (fTimeElapsed < (1.0f / fLockFPS))	// 간격 시간이 1/fLockFPS 라는건 조정된 시간보다 엄청 빠르게 되었단것 
		{	//fTimeElapsed에 크기를 키워서 반복문을 빠져나가게 한다. 그렇게하면 지정한 프레임의 간격시간의 거의 가까워질 것이다.
			if (m_bHardwareHasPerformanceCounter)
			{
				::QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentTime);
			}
			else
			{
				m_nCurrentTime = ::timeGetTime();
			}
			fTimeElapsed = (m_nCurrentTime - m_nLastTime) * m_fTimeScale;
		}

	}
	m_nLastTime = m_nCurrentTime;

	/*
	마지막 프레임 처리 시간과 현재 프레임 처리 시간의 차이가 1초보다 작으면 현재 프레임 처리 시간
	을 m_fFrameTime[0]에 저장한다.
	*/
	if (fabsf(fTimeElapsed - m_fTimeElapsed) < 1.0f)
	{
		::memmove(&m_fFrameTime[1], m_fFrameTime, (MAX_SAMPLE_COUNT - 1) * sizeof(float));
		m_fFrameTime[0] = fTimeElapsed;
		if (m_nSampleCount < MAX_SAMPLE_COUNT) m_nSampleCount++;
	}

	m_nFramePerSecond++;
	m_fFPSTimeElapsed += fTimeElapsed;
	if (m_fFPSTimeElapsed > 1.0f)
	{
		m_nCurrentFrameRate = m_nFramePerSecond;	// 프레임 레이트는 1초에 몇번의 프레임 반복인지 계산하는거니까 1초를
		//넘는 순간에 횟수를 더한 결과가 현재 프레임 레이트로 결정될 것이다.
		m_nFramePerSecond = 0; //값을 집어넣었으니 다시 초기화
		m_fFPSTimeElapsed = 0.0f;	//다시 0초부터 시작
	}

	//프레임 간격시간을 평균으로 계산한다.
	m_fTimeElapsed = 0.0f;
	for (ULONG i = 0; i < m_nSampleCount; i++) m_fTimeElapsed += m_fFrameTime[i];
	if (m_nSampleCount > 0) m_fTimeElapsed /= m_nSampleCount;

}



//마지막 프레임 레이트를 문자열과 정수형으로 반환한다 ==> Window 창에 출력해주기 위해서
unsigned long CGameTimer::GetFrameRate(LPTSTR lpszSting, int nCharacters)
{
	//현재 프레임 레이트를 문자열로 변환하여 IpszString 버퍼에 쓰고 " FPS" 와 결합한다.
	if (lpszSting)
	{
		_itow_s(m_nCurrentFrameRate, lpszSting, nCharacters, 10);
		wcscat_s(lpszSting, nCharacters, _T(" FPS)"));
	}

	return(m_nCurrentFrameRate);
}

float CGameTimer::GetTimeElapsed()
{
	return(m_fTimeElapsed);
}

void CGameTimer::Reset()
{
	__int64 nPerformanceCounter;
	::QueryPerformanceCounter((LARGE_INTEGER*)&nPerformanceCounter);

	m_nLastTime = nPerformanceCounter;
	m_nCurrentTime = nPerformanceCounter;

	m_bStopped = false;
}
