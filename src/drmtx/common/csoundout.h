#ifndef CSOUNDOUT_H
#define CSOUNDOUT_H
#include "GlobalDefinitions.h"
#include "soundinterface.h"

class CSoundOut: public CSoundOutInterface
{
public:
  CSoundOut();
  ~CSoundOut();
//  void Init(int iNewBufferSize, bool bNewBlocking);
  bool Write(CVector< _SAMPLE >& psData);

};

#endif // CSOUNDOUT_H
