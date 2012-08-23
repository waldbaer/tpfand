#ifndef __TPFAN_HEADER__
#define __TPFAN_HEADER__

#include "stdincludes.h"

using namespace std;

class TpFan
{
  /* Constants */
private:

  /* Attributes */
private:
  string fanDeviceInfo;
  int fanLevel;
  int fanSpeed;

  /* Constructor / Destructor */
public:
  TpFan(string fanDevice);
  virtual ~TpFan();

  /* Public functions */
public:
  bool setLevel(int level);
  int getLevel();
  int getSpeed();

  /* Static functions */
  /* Private functions */
private:
  void init(string fanDevice);
  void parseHwInfo();
  void setHwFanLevel();
  bool checkLevel(int level);
};

#endif /* __TPFAN_HEADER__ */
