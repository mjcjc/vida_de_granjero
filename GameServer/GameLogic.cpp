#include"pch.h"
#include "GameLogic.h"
#include "WorldEnviroment.h"    

GameLogic::GameLogic(WorldEnvironment* env)
    : _env(env)
{
}

void GameLogic::OnMissionCompleted(int32_t scoreDelta)
{
    if (_env == nullptr || scoreDelta <= 0)
        return;

    std::cout << "[GameLogic] MissionCompleted +" << scoreDelta << "\n";

    _env->AddMissionScore(scoreDelta);

    std::cout << "[GameLogic] TotalScore="
        << _env->GetMissionScore() << "\n";
}
