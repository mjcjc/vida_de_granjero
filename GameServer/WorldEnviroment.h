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


    void AddMissionScore(uint32_t score);

    // Getter
    ESeason  GetSeason() const;
    EWeather GetWeather() const;
    int32_t  GetMissionScore() const;
	int32_t  SetMissionScore(int32_t _score) { return _missionScore = _score; };
    ESeason SetSeason(ESeason _season) { return _season; };
    float GetGrowthMultiplier() const;
    bool IsCropModel(uint32 modelType) const;
    bool IsMissionScoreReached(int32_t requiredScore) const;

    // 货 努扼 立加 矫 惑怕 傈价
    void SendCurrentStateToClient(class GameSession* session);

private:
    // 郴何 贸府
    EWeather PickWeatherBySeason(ESeason season);
    void BroadcastWorldState();

private:
    ESeason  _season;
    EWeather _weather;
    int32_t  _missionScore;
};
extern WorldEnvironment GWorldEnv;