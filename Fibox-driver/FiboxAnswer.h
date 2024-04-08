#pragma once

#include "../types.h"
#include <map>

/**
* FiboxAnswer - Fibox driver answer class
* It represents the answer of the Fibox driver after a measurement request
*/
class FiboxAnswer
{
public:
    double temperature;
    double pressure;
    double phase;
    std::map<UInt, String> errors;
    bool isTemperatureEnabled;

    FiboxAnswer(double temperature, double pressure, double phase, std::map<UInt, String> errors);
};

