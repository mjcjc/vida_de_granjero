#include"pch.h"
#include "WorldEnviroment.h"
#include "Protocol.pb.h"      // S_WORLD_STATE
#include "GameSession.h"      // Send()
#include "GameSessionManager.h"
#include"ClientPacketHandler.h"



static std::unordered_map<ESeason, std::vector<WeatherWeight>> g_WeatherBySeason =
{
    { Spring, {
        { Clear, 40 },
        { Rain,  35 },
        { Dust,  20 },
        { Fog,    5 },
    }},
    { Summer, {
        { Clear, 35 },
        { Rain,  50 },
        { Fog,   15 },
    }},
    { Autumn, {
        { Clear, 45 },
        { Rain,  25 },
        { Fog,   20 },
        { Dust,  10 },
    }},
    { Winter, {
        { Clear, 30 },
        { Snow,  50 },
        { Fog,   20 },
    }},
};
static float SeasonMul(ESeason s)
{
    switch (s)
    {
    case Spring: return 1.20f;
    case Summer: return 1.00f;
    case Autumn: return 0.85f;
    case Winter: return 0.25f;
    default:     return 1.00f;
    }
}

static float WeatherMul(EWeather w)
{
    switch (w)
    {
    case Clear: return 1.00f;
    case Rain:  return 1.15f;
    case Snow:  return 0.70f;
    case Dust:  return 0.80f;
    case Fog:   return 0.90f;
    default:    return 1.00f;
    }
}

float WorldEnvironment::GetGrowthMultiplier() const
{
    return SeasonMul(GetSeason()) * WeatherMul(GetWeather());

}

bool WorldEnvironment::IsCropModel(uint32 modelType) const
{
    return (modelType >= 200 && modelType < 600); 
}

WorldEnvironment::WorldEnvironment()
    : _season(Spring)
    , _weather(Clear)
    , _missionScore(0)
{
}
void WorldEnvironment::Init()
{
    _season = static_cast<ESeason>(rand() % 4);
    _weather = PickWeatherBySeason(_season);
    _missionScore = 0;

    BroadcastWorldState();
}

// ===== Public API =====
void WorldEnvironment::ChangeSeason(ESeason season)
{
    _season = season;
    _weather = PickWeatherBySeason(season);
    BroadcastWorldState();
}

void WorldEnvironment::AddMissionScore(uint32_t score)
{
    _missionScore += (int32_t)score;

    while (_missionScore >= 100)
    {
        _missionScore -= 100;

        ESeason next = (ESeason)(((int)_season + 1) % 4);
        _season = next;

        // 시즌 바뀌면 그 시즌에 맞는 날씨 다시 뽑기
        _weather = PickWeatherBySeason(_season);
    }
    BroadcastWorldState();
}

ESeason WorldEnvironment::GetSeason() const
{
    return _season;
}

EWeather WorldEnvironment::GetWeather() const
{
    return _weather;
}

int32_t WorldEnvironment::GetMissionScore() const
{
    return _missionScore;
}

// ===== Internal =====
EWeather WorldEnvironment::PickWeatherBySeason(ESeason season)
{
    constexpr int kSpringDustPct = 30;
    constexpr int kSummerRainPct = 40;
    constexpr int kAutumnFogPct = 25;
    constexpr int kWinterSnowPct = 35;

    int roll = rand() % 50; 

    switch (season)
    {
    case ESeason::Spring:
        return EWeather::Dust ;

    case ESeason::Summer:
        return  EWeather::Rain ;

    case ESeason::Autumn:
        return  EWeather::Fog ;

    case ESeason::Winter:
        return EWeather::Snow ;

    default:
        return EWeather::Clear;
    }
}



void WorldEnvironment::BroadcastWorldState()
{
    Protocol::S_WORLD_STATE pkt;
    pkt.set_season((int32_t)_season);
    pkt.set_weather((int32_t)_weather);
    pkt.set_mission_score(_missionScore);
	cout << "월드 상태 브로드캐스트 시즌" << pkt.season() << "날씨" << pkt.weather() << "미션점수" << pkt.mission_score() << endl;

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);

    GSessionManager.Broadcast(sendBuffer);
}

void WorldEnvironment::SendCurrentStateToClient(GameSession* session)
{
    Protocol::S_WORLD_STATE pkt;
    pkt.set_season((int32_t)_season);
    pkt.set_weather((int32_t)_weather);
    pkt.set_mission_score(_missionScore);

    auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);

    session->Send(sendBuffer);
}
bool WorldEnvironment::IsMissionScoreReached(int32_t requiredScore) const
{
    return _missionScore >= requiredScore;
}

//void WorldEnvironment::weatherTick(uint64 nowMs)
//{
//    // 1) 이벤트 날씨가 끝났으면 Clear로
//    if (_weather != EWeather::Clear)
//    {
//        if (nowMs >= _weatherUntilMs)
//        {
//            _weather = EWeather::Clear;
//            _weatherUntilMs = 0;
//            BroadcastWorldState();
//        }
//        return;
//    }
//
//    // 2) Clear 상태일 때만 날씨 이벤트 시도
//    // (예: 10초에 한 번만 체크)
//    if (nowMs < _nextWeatherCheckMs)
//        return;
//
//    _nextWeatherCheckMs = nowMs + 10 * 1000; // 10초 주기
//
//    EWeather next = PickWeatherBySeason(_season);
//
//    if (next != EWeather::Clear)
//    {
//        _weather = next;
//
//        // 지속 시간은 PPT 기준으로 설정
//        // 예: 30~90초
//        int durationSec = 30 + (rand() % 60);
//        _weatherUntilMs = nowMs + durationSec * 1000ULL;
//
//        BroadcastWorldState();
//    }
//}