#include "MeasureConfig.h"

MeasureConfig::MeasureConfig()
{
	set(237, 0.808, 30, -0.068, -0.00035, 0.000371, 0, 967, 60.22, 26.82, 20, 20, 100, true, false, false);
}

void MeasureConfig::set(int altitude, double F1, double M, double DPHI1, double DPHI2, double DKSV1, double DKSV2, double pressure, double cal0, double cal2nd, double t0, double t2nd, double o2Cal2nd, bool calibIsHumid, bool enableTempFibox, bool humidMode)
{
	this->altitude = altitude;
	this->F1 = F1;
	this->M = M;
	this->DPHI1 = DPHI1;
	this->DPHI2 = DPHI2;
	this->DKSV1 = DKSV1;
	this->DKSV2 = DKSV2;
	this->pressure = pressure;
	this->cal0 = cal0;
	this->cal2nd = cal2nd;
	this->t0 = t0;
	this->t2nd = t2nd;
	this->o2Cal2nd = o2Cal2nd;
	this->calibIsHumid = calibIsHumid;
	this->enableTempFibox = enableTempFibox;
	this->humidMode = humidMode;
}
