#include "drmparams.h"
#include "configparams.h"

int numTxFrames;
drmTxParams drmParams;
QList<short unsigned int> fixBlockList;

drmTxParams modeToParams(unsigned int mode)
{
  drmTxParams prm;
  prm.robMode=mode/10000;
  mode-=(mode/10000)*10000;
  prm.bandwith=mode/1000;
  mode-=(mode/1000)*1000;
  prm.protection=mode/100;
  mode-=(mode/100)*100;
  prm.qam=mode/10;
  prm.interleaver=0;
  prm.callsign=myCallsign;
  return prm;
}

unsigned int paramsToMode(drmTxParams prm)
{
  uint mode=1;
  mode+=prm.robMode*10000;
  mode+=prm.bandwith*1000;
  mode+=prm.protection*100;
  mode+=prm.qam*10;
  mode+=prm.interleaver;
  return mode;
}



