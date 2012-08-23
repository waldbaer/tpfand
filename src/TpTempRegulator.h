#ifndef __TPTEMPREGULATOR_HEADER__
#define __TPTEMPREGULATOR_HEADER__

#include "stdincludes.h"
#include "TpTemperature.h"
#include "TpFan.h"

using namespace std;

class TpTempRegulator
{
  /* Constants */

  /* Attributes */
private:
  TpTemperature *tempSensor;
  TpFan *fanDevice;
  int sampleTime;
  int regPrescaler;
  int regPrescaleCnt;

  float nomTemp;
  float oldTemp;

  float kp; /* proportional gain factor */
  float kd; /* Differential gain factor */

  /* Constructor / Destructor */
public:
  TpTempRegulator(TpTemperature *tempSensor, TpFan *fanDevice, int sampleTime, int regTime, float nomTemp, float kp,
                  float kd);
  virtual ~TpTempRegulator();

  /* Public functions */
public:
  void process();

  /* Static functions */

  /* Private functions */
private:
  void init(TpTemperature *tempSensor, TpFan *fanDevice, int sampleTime, int regTime);
};

#endif /* __TPTEMPREGULATOR_HEADER__ */
