#ifndef REEDSOLOMONCODER_H
#define REEDSOLOMONCODER_H
#include "appglobal.h"
#include "appdefs.h"
#include <QFile>
#include <QByteArray>

enum eRSType {RSTNONE,RST1,RST2,RST3,RST4};

extern QString rsTypeStr[RST4+1];

union long_byte_union
{
        quint64 i;
        unsigned char b[8];
};


class reedSolomonCoder
{
public:
  reedSolomonCoder();
  ~reedSolomonCoder();
  void init();
  bool decode(QByteArray &ba, QString fn, QString &newFileName, QByteArray &baFile, QString extension, QList<int> &erasuresArray);
  bool encode(QByteArray &ba, QString extension, eRSType rsType);
private:
  void distribute(byte *src, byte *dst, int rows, int cols, int reverse);
  bool decode_and_write();
  QByteArray ec_buf;  /* pointer to encoding/decoding buffer */
  QByteArray tr_buf;  /* pointer to transmit-buffer (fread/fwrite) */
  QByteArray bk_buf; /* pointer to backup-buffer for resync */
  int rs_bsize;
  int rs_dsize;
  int bep_size;
  unsigned long sumOfFailures;
  unsigned long uncorrectableFailures;
//  int k;
  QFile fpin, fpout;
  long got,chunks;
  int coded_file_size ;
  char coded_file_ext[4] ;
  QString origFileName;
//  char *p ;
  eRSType fileType;
  int totalSegments;
  int segmentLength;
  int *zeroPositions;
  int *newZeroPositions;
  int numMissing;
};

#endif // REEDSOLOMONCODER_H

