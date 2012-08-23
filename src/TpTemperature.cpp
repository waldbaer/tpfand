#include "TpTemperature.h"

using namespace std;

/*************************************************************************************************/
/* Constructor / Destructor                                                                      */
/*************************************************************************************************/
TpTemperature::TpTemperature(vector<string> *pSensors)
{
  init(pSensors, -1);
}

TpTemperature::TpTemperature(vector<string> *pSensors, int queueSize)
{
  init(pSensors, queueSize);
}

TpTemperature::~TpTemperature()
{
  delete this->tempQueue;
}

/*************************************************************************************************/
/* Public functions                                                                              */
/*************************************************************************************************/
int TpTemperature::getTemperature()
{
  parseHwInfo();

  /* Calculate average value */
  vector<int> queueValues;
  for(deque<int>::iterator itQueue = tempQueue->begin(); itQueue != tempQueue->end(); itQueue++)
  {
    queueValues.push_back(*itQueue);
  }
  int medianTemp = getMedian(&queueValues);
  return medianTemp;
}

/*************************************************************************************************/
/* Static functions                                                                              */
/*************************************************************************************************/
float TpTemperature::ConvertToCelsius(int temp)
{
  return (float) temp / TpTemperature::TEMP_PRESCALER;
}

/*************************************************************************************************/
/* Private functions                                                                             */
/*************************************************************************************************/

void TpTemperature::init(vector<string> *pSensors, int queueSize)
{
  /* Init Sensor Map */
  this->pSensors = pSensors;
  if(pSensors->size() < 1)
  {
    throw invalid_argument("Temperature sensor needs at least one sensor device...");
  }

  /* Init Queue / size */
  this->tempQueue = new deque<int>();
  if(queueSize < 0)
  {
    this->queueSize = TpTemperature::QUEUE_MAX_SIZE;
  }
  else
  {
    this->queueSize = queueSize;
  }
}

void TpTemperature::parseHwInfo()
{
  for(vector<string>::iterator itSensors = pSensors->begin(); itSensors != pSensors->end(); ++itSensors)
  {
    ifstream sensorDevice;
    sensorDevice.open(itSensors->c_str());
    if(sensorDevice.good())
    {
      char lineBuff[256];
      sensorDevice.getline(lineBuff, 256);
      int rawTemp = atoi(lineBuff);
      queueTemp(rawTemp);
      sensorDevice.close();
    }
    else
    {
      stringstream msg;
      msg << "Failed to open sensor device '" << *itSensors << "'. Maybe root privileges required.";
      throw invalid_argument(msg.str());
    }
  }
}

void TpTemperature::queueTemp(int temp)
{
  tempQueue->push_front(temp);
  unsigned int maxQueueSize = this->pSensors->size() * queueSize;
  if(tempQueue->size() >= maxQueueSize)
  {
    tempQueue->resize(maxQueueSize);
  }
}

int TpTemperature::getMedian(vector<int> *pValues)
{
  sort(pValues->begin(), pValues->end());
  int count = pValues->size();
  if(count % 2 == 1)
  {
    return (*pValues)[(count + 1) / 2 - 1];
  }
  else
  {
    int lower = (*pValues)[count / 2 - 1];
    int upper = (*pValues)[count / 2];
    return (lower + upper) / 2;
  }
}
