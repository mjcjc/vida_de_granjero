#include "stdafx.h"
#include "SceneManager.h"


void CSceneManager::SetSceneNumber(int num)
{
	m_nSceneNumber = num;
}

int CSceneManager::GetSceneNumber()
{
	return m_nSceneNumber;
}

void CSceneManager::PushScene(CScene* scene)
{
	int ntopIndex = m_sceneStack.size() - 1;

	if (ntopIndex >= 0) //Scene 스택이 활성화 되 어 있다는 의미
	{
		//m_sceneStack[nLength].
	}
	m_sceneStack.push_back(scene);
	scene->OnEnter(); // 진입했다는 의미
}

CScene* CSceneManager::PopScene()
{
	int ntopIndex = m_sceneStack.size() - 1;
	if (ntopIndex < 0) //scene의 스택이 비어있어다는 것을 의미
	{
		NotifyEmptyStack();
		return NULL;
	}
	CScene* scene = m_sceneStack.back(); //가장 최상위를 꺼낸다
	m_sceneStack.pop_back(); // 가장 최상위를 삭제한다
	scene->OnExit(); //해당 Scene이 삭제될거란걸 전달
	if (ntopIndex >= 1)
	{
		m_sceneStack[ntopIndex - 1]->OnResume();
	}
	else
	{
		NotifyEmptyStack();
	}
	return scene;
}

void CSceneManager::PopAllScene()
{
	int nCount = m_sceneStack.size();
	std::cout << "현재 Scene의 쌓인 Stack 갯수는 " << nCount << "개이다." << endl;
	for (int i = nCount - 1; i >= 0; i--)
	{
		CScene* scene = m_sceneStack[i];
		scene->OnExit();
	}
	m_sceneStack.clear();
	if (nCount > 0)
	{
		NotifyEmptyStack();
	}
}

CScene* CSceneManager::GetTopScene()
{
	int ntopIndex = m_sceneStack.size() - 1;
	if (ntopIndex < 0)
	{
		return NULL;
	}
	else
	{
		return m_sceneStack[ntopIndex];
	}
}

void CSceneManager::NotifyEmptyStack()
{
	std::cout << "Stack이 비어있습니다"<<endl;
}

void CSceneManager::ChangeScene(CScene* scene) //Scene을 바꾸는 경우
{
	int ntopIndex = m_sceneStack.size() - 1;
	if (ntopIndex < 0)
	{
		return;
	}
	m_sceneStack[ntopIndex]->OnExit();
	m_sceneStack.push_back(scene);
	scene->OnEnter();
}

void CSceneManager::NotifySceneStuck(int num)
{
	if (num == 0)
	{
		m_nChangeNum = 0;
	}
	else
	{
		m_nChangeNum = 1;
	}
}

int CSceneManager::GetChangeNum()
{
	return m_nChangeNum;
}
