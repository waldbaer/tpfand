#include "TpTempRegulator.h"

using namespace std;

/*************************************************************************************************/
/* Constructor / Destructor                                                                      */
/*************************************************************************************************/
TpTempRegulator::TpTempRegulator(TpTemperature *tempSensor, TpFan *fanDevice, int sampleTime, int regPrescaler,
                                 float nomTemp, float kp, float kd)
{
  this->tempSensor = tempSensor;
  this->fanDevice = fanDevice;

  if(regPrescaler > 0 && sampleTime > 0)
  {
    this->sampleTime = sampleTime;
    this->regPrescaler = regPrescaler;
  }
  else
  {
    throw invalid_argument("Regulation times must be greater than zero!");
  }
  this->regPrescaleCnt = 0;
  this->nomTemp = nomTemp; /* Nominal temperature */
  this->oldTemp = TpTemperature::ConvertToCelsius(tempSensor->getTemperature());
  this->kp = kp;
  this->kd = kd;
}

TpTempRegulator::~TpTempRegulator()
{

}

/*************************************************************************************************/
/* Public functions                                                                              */
/*************************************************************************************************/
void TpTempRegulator::process()
{
  /* Capture temperature with configured sample time */
  float currentTemp = TpTemperature::ConvertToCelsius(tempSensor->getTemperature());

  /* Regulator prescaler elapsed? */
  if(regPrescaleCnt == 0)
  {
    regPrescaleCnt = regPrescaler; /* Reload prescaler */

    float deltaTemp = (currentTemp - oldTemp) / (float) sampleTime;
    float regDiff = nomTemp - currentTemp;

    float regP = regDiff * kp;
    float regD = deltaTemp * kd;
    float correctionDiff = regP + regD;

    int newFanLevel = fanDevice->getLevel();
    newFanLevel += (int) round(correctionDiff);
    g_message(
        "Temp: %.2f  dT: %.2f  regP: %.2f  regD: %.2f  corr: %.2f  fanLevel: %d", currentTemp, deltaTemp, regP, regD, correctionDiff, newFanLevel);

    /* Set new fan level */
    fanDevice->setLevel(newFanLevel);

    oldTemp = currentTemp;
  }
  regPrescaleCnt--;
}
/*************************************************************************************************/
/* Static functions                                                                              */
/*************************************************************************************************/

/*************************************************************************************************/
/* Private functions                                                                             */
/*************************************************************************************************/

