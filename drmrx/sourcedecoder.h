#ifndef SOURCEDECODER_H
#define SOURCEDECODER_H
//#include <QFTP>
#include <QObject>
#include <QList>
#define PACKETBUFFERLEN 512
#define NUMSERVICES 4
#define MAXTRANSPORTLISTS 5

#include <QList>
#include <QDateTime>
#include <QByteArray>
#include <QVector>
#include "appdefs.h"
#include "hybridcrypt.h"

class ftpFunctions;

enum edataGroupType {GENDATA,CAMESS,GENCA,MOTHEAD,MOTDATA,MOTDATACA};


struct dataSegment
{
  dataSegment(int newSize)
  {
    crcOK=false;
    recovered=false;
    segmentNumber=-1;
    data.resize(newSize);
    data.fill(0XAA,newSize);
  }

  void setData(QByteArray ba,short int segNumber,bool crcok)
  {
    crcOK=crcok;
    data=ba;
    segmentNumber=segNumber;
  }

  void clearData()
  {
    data.clear();
  }
  bool crcOK;
  bool recovered;
  short int segmentNumber;
  bool hasData() {return (crcOK||recovered);}
  QByteArray data;
};

struct dataPacket
{
  void log();
  QByteArray ba;
  bool extFlag;
  bool crcFlag;
  bool sessionFlag;
  bool userFlag;
  bool crcOK;
  edataGroupType dataGroupType;
  unsigned char continuityIndex;
  unsigned char repetitionIndex;
  unsigned short segmentNumber;
  bool lastSegment;
  unsigned short transportID;
  unsigned char userAccessField;
  unsigned short segmentSize;
  int offset;
  int lenght;
  unsigned int advance(int numBytes)
  {
    ba.remove(0,numBytes);
    return ba.count();
  }
  unsigned int chop(int numBytes)
  {
    ba.chop(numBytes);
    return ba.count();
  }

};


struct dataBlock
{
  void log();
  bool firstFlag;
  bool lastFlag;
  short packetID;
  bool  PPI;
  unsigned char continuityIndex;
  bool crcOK;
  unsigned int length;
  QByteArray ba;
};



struct transportBlock
{
  transportBlock(unsigned short tId)
  {
    clear();
    transportID=tId;
  }

  void clear()
  {
    totalSegments=0;
    headerReceived=false;
    segmentsReceived=0;
    bodySize=0;
    headerSize=0;
    alreadyReceived=false;
    retrieveTries=0;
    lastSegmentReceived=false;
    defaultSegmentSize=0;
  }
  bool isComplete()
  {
    if(!headerReceived) return false;
    if(segmentsReceived<totalSegments) return false;
    return true;
  }

  int isAlmostComplete()
  {
    if(!headerReceived) return 0;
    if(totalSegments==0) return 0;
    return (segmentsReceived*100)/totalSegments;
  }

  void setAlreadyReceived(bool aRx)
  {
    int i;
    if(aRx)
      {
        alreadyReceived=true;
        for(i=0;i<dataSegmentPtrList.count();i++)
          {
            dataSegmentPtrList.at(i)->clearData();
          }
      }
  }

  unsigned short transportID;
  unsigned int bodySize;
  unsigned int headerSize;
  unsigned short contentType;
  unsigned short contentSubtype;
  unsigned int defaultSegmentSize;
  QString fileName;
  QString newFileName;
  bool headerReceived;
  bool alreadyReceived;
  int  retrieveTries;
  unsigned short segmentsReceived;
  bool lastSegmentReceived;
  int totalSegments;
  QString callsign;
  //  QList <short int> blockList;
  QVector<dataSegment *> dataSegmentPtrList;
  int robMode;
  int interLeaver;
  int mscMode; // qam
  int mpx;
  int spectrum;
  QByteArray baBSR;
  uint modeCode; //mode(A=0,B=1,E=2) BW(0=2.3,1=2.5) prot(High=0,LOW=1) QAM(4=0,16=1,64=2) ineterleaver
};


struct bsrBlock
{
  bsrBlock(transportBlock *tb)
  {
    tbPtr=tb;
  }
  transportBlock *tbPtr;
};



class sourceDecoder : public QObject
{
  Q_OBJECT
public:
  explicit sourceDecoder(QObject *parent = 0);
  void init();
  bool decode();
  //    bool hasStarted(){return started;}
  QList <bsrBlock> *getBSR();
  bool checkSaveImage(QByteArray ba, transportBlock *tbPtr);
  void saveImage(transportBlock *tbPtr);
  bool storeBSR(transportBlock *tb,bool compat);
//  bool rxNotifySetup();
//  bool rxNotifyCheck(QString fn);
private slots:
  void slotDownloadDone(bool err, QString filename);

private:
  bool setupDataBlock(unsigned char *buffer,bool crcIsOK,int len);
  bool setupDataPacket(QByteArray ba);
  void addDataSegment();
  bool addHeaderSegment();
  void loadParams(transportBlock *tbPtr, unsigned char paramID, int len);
  void writeData(transportBlock *tbPtr);
  transportBlock *getTransporPtr(unsigned short tId,bool create);
  void removeTransporPtr(transportBlock * ptr);
  unsigned char packetBuffer[PACKETBUFFERLEN];
  dataBlock currentDataBlock;
  dataPacket currentDataPacket;
  QList<transportBlock *> transportBlockPtrList;
  transportBlock *lastTransportBlockPtr;
  QList <int> erasureList;
  QByteArray holdingBuffer;
  short int lastContinuityIndex;
  bool checkIt;
  QList <bsrBlock> bsrList;
  bool isHybrid;
  hybridCrypt hc;
  bool alreadyDisplayed;
  ftpFunctions *ff;
  void displayReceivedImage(bool isHybrid, QString filename);
  uint modeCodeTmp;
  QString callsignTmp;
};
#endif // SOURCEDECODER_H
