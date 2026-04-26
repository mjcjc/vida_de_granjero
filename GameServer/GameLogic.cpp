#include"pch.h"
#include "GameLogic.h"
#include "WorldEnviroment.h"    

GameLogic::GameLogic(WorldEnvironment* env)
    : _env(env)
{
}

void GameLogic::OnMissionCompleted(int32_t scoreDelta)
{
    std::cout << "[GameLogic] MissionCompleted +" << scoreDelta << "\n";

    _env->AddMissionScore(scoreDelta);


    std::cout << "[GameLogic] TotalScore="
        << _env->GetMissionScore() << "\n";
    if (_env->GetMissionScore() / 100)
    {
        _env->SetMissionScore(0);
        ESeason next = (ESeason)((_env->GetSeason() + 1) % 4);
        std::cout << "[GameLogic] ChangeSeason\n";
        _env->ChangeSeason(next);
        
    }
}