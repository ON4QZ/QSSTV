#ifndef DRMTRANSMITTER_H
#define DRMTRANSMITTER_H

#include "common/GlobalDefinitions.h"
#include "common/DrmTransmitter.h"
#include "drmparams.h"


class drmTransmitter
{
public:
  drmTransmitter();
  ~drmTransmitter();
  void init(QByteArray *ba, QString name, QString format, drmTxParams params);
  void start(bool startTx);
  double getDuration() {return duration;} // expressed in seconds
  double transmissionTime;
private:
  CDRMTransmitter *DRMTransmitter;
  CParameter* TransmParam ;
  int dataLength;
  double duration;
};


#endif // DRMTRANSMITTER_H
