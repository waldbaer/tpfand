#ifndef __TPTEMPERATURE_HEADER__
#define __TPTEMPERATURE_HEADER__

#include "stdincludes.h"

using namespace std;

class TpTemperature
{
  /* Constants */
private:
  static const char QUEUE_MAX_SIZE = 5;
public:
  static const int TEMP_PRESCALER = 1000;

  /* Attributes */
private:
  vector<string> *pSensors;
  deque<int> *tempQueue;
  int queueSize;

  /* Constructor / Destructor */
public:
  TpTemperature(vector<string> *pSensors);
  TpTemperature(vector<string> *pSensors, int queueSize);

  virtual ~TpTemperature();

  /* Public functions */
public:
  int getTemperature();
  /* Static functions */
public:
  static float ConvertToCelsius(int temp);

  /* Private functions */
private:
  void init(vector<string> *pSensors, int queueSize);
  void parseHwInfo();
  void queueTemp(int temp);
  int getMedian(vector<int> *pValues);
};

#endif /* __TPTEMPERATURE_HEADER__ */
