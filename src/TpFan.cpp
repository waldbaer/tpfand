#include "TpFan.h"

using namespace std;

#define TPFAN_FILE_SPEED            "speed"
#define TPFAN_FILE_LEVEL            "level"

#define TPFAN_FILE_LEVEL_AUTO       "auto"
#define TPFAN_FILE_LEVEL_FULL       "disengaged"
#define TPFAN_FILE_STATUS_ENABLED   "enabled"
#define TPFAN_FILE_STATUS_DISABLED  "disabled"

#define TPFAN_LEVEL_INVALID         -1
#define TPFAN_LEVEL_AUTO            10
#define TPFAN_LEVEL_LOWER_LIMIT     0
#define TPFAN_LEVEL_UPPER_LIMIT     8

/*************************************************************************************************/
/* Constructor / Destructor                                                                      */
/*************************************************************************************************/
TpFan::TpFan(string fanDevice)
{
  init(fanDevice);
}

TpFan::~TpFan()
{
}

/*************************************************************************************************/
/* Public functions                                                                              */
/*************************************************************************************************/
int TpFan::getLevel()
{
  parseHwInfo();
  return this->fanLevel;
}

bool TpFan::setLevel(int level)
{
  bool result = false;

  /* Limit level */
  if(level < TPFAN_LEVEL_LOWER_LIMIT)
    level = TPFAN_LEVEL_LOWER_LIMIT;
  else if(level > TPFAN_LEVEL_UPPER_LIMIT)
    level = TPFAN_LEVEL_UPPER_LIMIT;

  if(checkLevel(level))
  {
    ofstream fanDevice;
    fanDevice.open(fanDeviceInfo.c_str());
    if(fanDevice.good())
    {
      stringstream out;
      out << TPFAN_FILE_LEVEL << " ";
      if(level == TPFAN_LEVEL_UPPER_LIMIT)
      {
        out << TPFAN_FILE_LEVEL_FULL;
      }
      else
      {
        out << level;
      }
      string output = out.str().c_str();
      fanDevice.write(out.str().c_str(), out.str().size());
      fanDevice.close();

      int currentFanLevel = getLevel();
      if(currentFanLevel == level)
      {
        result = true;
      }
    }
    else
    {
      stringstream msg;
      msg << "Failed to set fan level for device '" << fanDeviceInfo << "'. Maybe root privileges required.";
      throw runtime_error(msg.str());
    }
  }
  return result;
}
int TpFan::getSpeed()
{
  parseHwInfo();
  return this->fanSpeed;
}

/*************************************************************************************************/
/* Private functions                                                                             */
/*************************************************************************************************/
void TpFan::init(string fanDevice)
{
  this->fanDeviceInfo = fanDevice;
  this->fanLevel = TPFAN_LEVEL_INVALID;
  this->fanSpeed = 0;
}

void TpFan::parseHwInfo()
{
  ifstream fanDevice;
  fanDevice.open(fanDeviceInfo.c_str());
  if(fanDevice.good())
  {
    while(fanDevice.good())
    {
      char lineBuff[256];
      fanDevice.getline(lineBuff, 256);

      string line = string(lineBuff);
      /* fan level */
      if(line.find(TPFAN_FILE_LEVEL, 0) == 0)
      {
        string strFanLevel = line.substr(string(TPFAN_FILE_LEVEL).size() + 3, 256);
        if(strFanLevel == TPFAN_FILE_LEVEL_AUTO)
        {
          this->fanLevel = TPFAN_LEVEL_AUTO;
        }
        else if(strFanLevel == TPFAN_FILE_LEVEL_FULL)
        {
          this->fanLevel = TPFAN_LEVEL_UPPER_LIMIT;
        }
        else
        {
          int level = atoi(strFanLevel.c_str());
          if(checkLevel(level))
          {
            this->fanLevel = level;
          }
        }
      }

      /* fan speed */
      if(line.find(TPFAN_FILE_SPEED, 0) == 0)
      {
        string strFanSpeed = line.substr(string(TPFAN_FILE_SPEED).size() + 3, 256);
        this->fanSpeed = atoi(strFanSpeed.c_str());
      }
    }
    fanDevice.close();
  }
  else
  {
    stringstream msg;
    msg << "Failed to open fan device '" << fanDeviceInfo << "'. Maybe root privileges required.";
    throw invalid_argument(msg.str());
  }

}

void TpFan::setHwFanLevel()
{
}

bool TpFan::checkLevel(int level)
{
  if(level >= TPFAN_LEVEL_LOWER_LIMIT && level <= TPFAN_LEVEL_UPPER_LIMIT)
  {
    return true;
  }
  else
  {
    return false;
  }
}
