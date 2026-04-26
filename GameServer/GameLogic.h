#pragma once
#include <cstdint>

class WorldEnvironment;

class GameLogic
{
public:
    explicit GameLogic(WorldEnvironment* env);

    void OnMissionCompleted(int32_t scoreDelta);

private:
    WorldEnvironment* _env = nullptr;
    int32_t _requiredScore = 0; 
};