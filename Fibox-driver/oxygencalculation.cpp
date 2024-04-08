#define _USE_MATH_DEFINES
#include "oxygencalculation.h"
#include <cmath>
#include <stdio.h>


OxygenCalculation::OxygenCalculation(MeasureConfig* config)
{
    
    this->config = config;
    this->mPhaseAngle = 12;
    this->mPressure = 1013;
    this->mTemperature = 20;
}

void OxygenCalculation::setPressure(double valInPa)
{
    this->mPressure = valInPa / 100.0; // From Pa to hPa
}

void OxygenCalculation::setTemperature(double valInC)
{
    this->mTemperature = valInC;
}

void OxygenCalculation::setPhaseAngle(double valInDegres)
{
    this->mPhaseAngle = valInDegres;
}

double OxygenCalculation::convertOxygenValue(double val)
{
    if (!config->humidMode)
    {
        return val * 0.2095;
    }
    else
    {
        return (val * 0.2095 * ((mPressure - pwT(mTemperature + 273.15)) / mPressure));
    }
}

double OxygenCalculation::o2UnitCorrection()
{
    return config->o2Cal2nd;
}

double OxygenCalculation::tanPhi(double angle)
{
    return std::tan(angle * M_PI / 180.0);
}

double OxygenCalculation::tanPhi0(double temp)
{
    return std::tan((config->cal0 + config->DPHI1 * (temp - config->t0) + config->DPHI2 * (std::pow(temp, 2.0) - std::pow(config->t0, 2.0))) * (M_PI / 180.0));
}

double OxygenCalculation::B()
{
    double num1 = o2UnitCorrection();
    double num2 = tanPhi(config->cal2nd) / tanPhi0(config->t2nd);
    return num2 * num1 + num2 * (1.0 / config->M) * num1 - config->F1 * (1.0 / config->M) * num1 - num1 + config->F1 * num1;
}

double OxygenCalculation::ksv()
{
    double num1 = tanPhi(config->cal2nd) / tanPhi0(config->t2nd) * (1.0 / config->M) * std::pow(o2UnitCorrection(), 2.0);
    double x = B();
    double num2 = tanPhi(config->cal2nd) / tanPhi0(config->t2nd) - 1.0;
    return (-x + std::sqrt(std::pow(x, 2.0) - 4.0 * num1 * num2)) / (2.0 * num1);
}

double OxygenCalculation::ksvt()
{
    return ksv() - config->DKSV1 * (config->t2nd - mTemperature) + config->DKSV2 * (std::pow(config->t2nd, 2.0) - std::pow(mTemperature, 2.0));
}

double OxygenCalculation::a()
{
    double num1 = tanPhi(mPhaseAngle) / tanPhi0(mTemperature);
    double num2 = ksvt();
    double num3 = 1.0 / config->M;
    return num1 * num3 * std::pow(num2, 2.0);
}

double OxygenCalculation::b()
{
    double num = tanPhi(mPhaseAngle) / tanPhi0(mTemperature);
    return num * ksvt() + num * (1.0 / config->M) * ksvt() - config->F1 * (1.0 / config->M) * ksvt() - ksvt() + config->F1 * ksvt();
}

double OxygenCalculation::c()
{
    return tanPhi(mPhaseAngle) / tanPhi0(mTemperature) - 1.0;
}

double OxygenCalculation::pwT(double temperature)
{
    return std::exp(52.57 - 6690.9 / temperature - 4.681 * std::log(temperature));
}

double OxygenCalculation::basicOxyCalculation()
{
    double temperature = mTemperature;
    double pressure = config->pressure;
    double num1 = a();
    double num2 = b();
    double num3 = c();
    double num4 = (-num2 + std::sqrt(std::pow(num2, 2.0) - 4.0 * num1 * num3)) / (2.0 * num1);
    if (!config->calibIsHumid) {
        if (!config->humidMode)
        {
            return num4 * (pressure / mPressure);
        }
        else
        {
            return num4 * pressure / (pressure - pwT(temperature + 273.15)) * (pressure / mPressure);
        }
    } else {
        if (!config->humidMode) {
            return num4 * ((pressure - pwT(temperature + 273.15)) / pressure) * (pressure / mPressure);
        } else {
            return num4 * (pressure / mPressure);
        }
    }
}

double OxygenCalculation::getOxygenValue()
{
    return convertOxygenValue(basicOxyCalculation());
}
