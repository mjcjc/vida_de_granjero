#pragma once
#include "EnvironmentTypes.h"


class WeatherWeight
{
public:
    WeatherWeight(EWeather type, int32_t weight) : _type(type), _weight(weight) {}

    EWeather GetType() const { return _type; }
    int32_t  GetWeight() const { return _weight; }

private:
    EWeather _type;
    int32_t  _weight;
};
