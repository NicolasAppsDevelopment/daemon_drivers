#ifndef OXYGENCALCULATION_H
#define OXYGENCALCULATION_H

#include "../MeasureConfig.h"

/**
 * OxygenCalculation - Oxygen calculation class
 * It implements the oxygen calculation algorithm
 */
class OxygenCalculation
{
private:
    double o2UnitCorrection();
    double convertOxygenValue(double val);
    double tanPhi(double angle);
    double tanPhi0(double temp);
    double B();
    double ksv();
    double ksvt();
    double a();
    double b();
    double c();
    double pwT(double temperature);
    double basicOxyCalculation();

    MeasureConfig* config;

    // measurement data
    double mPressure;
    double mTemperature;
    double mPhaseAngle;

public:
    /**
     * @brief Construct a new Oxygen Calculation object with a specific measure configuration
     *
     * @param config The measure configuration pointer
     */
    OxygenCalculation(MeasureConfig* config);

    /**
     * @brief Set the pressure value in Pascal
     *
     * @param valInPa The pressure value in Pascal
     */
    void setPressure(double valInPa);

    /**
     * @brief Set the temperature value in Celsius
     *
     * @param valInC The temperature value in Celsius
     */
    void setTemperature(double valInC);

    /**
     * @brief Set the phase angle value in degrees
     *
     * @param valInDegres The phase angle value in degrees
     */
    void setPhaseAngle(double valInDegres);

    /**
     * @brief Get the oxygen value
     *
     * @return The oxygen value
     */
    double getOxygenValue();
};

#endif // OXYGENCALCULATION_H
