#pragma once

/**
 * @brief The MeasureConfig class
 * This class is used to store the configuration used to recalculate and correct measurements.
 * The configuration is set by the user by the SET_CONFIG command.
 */
class MeasureConfig
{
public:
	int altitude;

    // constants
    double F1;
    double M;
    double DPHI1;
    double DPHI2;
    double DKSV1;
    double DKSV2;

    // calibration data
    double pressure;
    double cal0;
    double cal2nd;
    double t0;
    double t2nd;
    double o2Cal2nd;
    bool calibIsHumid;

    bool enableTempFibox;
    bool humidMode;

    MeasureConfig();

    /**
    * @brief Sets the configuration of the measurement.
    * 
    * @param altitude The altitude of the measurement.
    * @param F1 The F1 constant.
    * @param M The M constant.
    * @param DPHI1 The DPHI1 constant.
    * @param DPHI2 The DPHI2 constant.
    * @param DKSV1 The DKSV1 constant.
    * @param DKSV2 The DKSV2 constant.
    * @param pressure The pressure of the measurement.
    * @param cal0 The cal0 constant.
    * @param cal2nd The cal2nd constant.
    * @param t0 The t0 constant.
    * @param t2nd The t2nd constant.
    * @param o2Cal2nd The o2Cal2nd constant.
    * @param calibIsHumid The calibIsHumid constant (true if the calibration data has been calculated in a humid environment).
    * @param enableTempFibox The enableTempFibox constant (true to use the temperature sensor of the Fibox device).
    * @param humidMode The humidMode constant (true if measurements are realised in humid environment).
    */
    void set(int altitude, double F1, double M, double DPHI1, double DPHI2, double DKSV1, double DKSV2, double pressure, double cal0, double cal2nd, double t0, double t2nd, double o2Cal2nd, bool calibIsHumid, bool enableTempFibox, bool humidMode);
};

