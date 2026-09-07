#pragma once
#include "EnvironmentTypes.h"
#include "WeatherWeight.h"



class WorldEnvironment
{
public:
    int score = 0;
    int stage = 1;
    uint64 _weatherUntilMs = 0;   
    uint64 _nextWeatherCheckMs = 0; 
public:
    WorldEnvironment();

    void Init();

    void ChangeSeason(ESeason season);
    bool ChangeWorldState(ESeason season, EWeather weather);


    void AddMissionScore(int32_t scoreDelta);

    // Getter
    ESeason  GetSeason() const;
    EWeather GetWeather() const;
    int32_t  GetMissionScore() const;
    ESeason SetSeason(ESeason _season) { return _season; };
    float GetGrowthMultiplier() const;
    bool IsCropModel(uint32 modelType) const;
    bool IsMissionScoreReached(int32_t requiredScore) const;

    // 새 클라 접속 시 상태 전송
    void SendCurrentStateToClient(class GameSession* session);

private:
    // 내부 처리
    EWeather PickWeatherBySeason(ESeason season);
    void BroadcastWorldState();

private:
    ESeason  _season;
    EWeather _weather;
    int32_t  _missionScore;
};
extern WorldEnvironment GWorldEnv;
