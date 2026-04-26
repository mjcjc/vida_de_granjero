#pragma once
#include "Scene.h"
class CScene;


class CSceneManager
{
private:
	std::vector<CScene*> m_sceneStack; //Scene을 관리해줄 Stack으로 활용할 컨테이너
	CSceneManager() {};
	virtual ~CSceneManager() {};
	int m_nChangeNum = 0;

public:
	// 복사 생성, 대입 방지
	CSceneManager(const CSceneManager&) = delete;
	CSceneManager& operator=(const CSceneManager&) = delete;

	int m_nSceneNumber = 0; // 0: 기본, 1: main, 2 : test

	// 싱글톤 접근 함수
	static CSceneManager& GetInstance()
	{
		static CSceneManager instance;
		return instance;
	};

	void SetSceneNumber(int num);
	int GetSceneNumber();
	void PushScene(CScene* scene); //Stack의 Scene을 추가
	CScene* GetTopScene(); //현재 최상위 Scene(현재 동작중인 Scene)
	CScene* PopScene(); //현재 최상단의 Scene을 빼고 이전 Scene으로 전환
	void PopAllScene();
	void NotifyEmptyStack();
	void ChangeScene(CScene* scene);
	void NotifySceneStuck(int num = 0); // Scene에서 다른 Scene으로 스택을 넣거나 뺄때 GameFramework에 
	//알려줄 함수.
	int GetChangeNum();
};

