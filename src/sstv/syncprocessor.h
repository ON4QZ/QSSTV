#ifndef SYNCPROCESSOR_H
#define SYNCPROCESSOR_H

#include <sstvparam.h>
#include "visfskid.h"
#include <QObject>
#include <QList>
#include "filters.h"

#define MAXSYNCENTRIES 2048
#define STATESCALER 100



class modeBase;


struct ssenitivity
{
  unsigned int minMatchedLines;
  unsigned int maxLineDistanceModeDetect;
  unsigned int maxLineDistanceInSync;
  DSPFLOAT onRatio;
  DSPFLOAT offRatio;
  int minVolume;
  int maxTempOutOfSyncLines;
  int maxOutOfSyncLines;
};

struct ssyncEntry
{
  ssyncEntry()
  {
    init();
  }

  void init()
  {
    start=0;
    end=0;
    startVolume=0;
    maxVolume=0;
    width=0;
    inUse=false;
    retrace=false;
    lineNumber=0;
    length=0;
  }
  int diffStartEnd()
  {
    width=end-start;
    return width;
  }
  uint start;
  uint startVolume;
  uint end;
  uint maxVolume;
  uint width;
  bool inUse;
  bool retrace;
  uint lineNumber;
  uint length;
};

struct smatchEntry
{
  smatchEntry()
  {
    init();
  }

  smatchEntry(uint fromIdx,uint toIdx,uint lineSpace,double fract,uint endFromSample,uint endToSample)
  {
    from=fromIdx;
    to=toIdx;
    lineSpacing=lineSpace;
    fraction=fract;
    endFrom=endFromSample;
    endTo=endToSample;
  }
  void init()
  {
    from=0;
    to=0;
    lineSpacing=0;
    fraction=0;
    endFrom=0;
    endTo=0;
  }
  uint from; /**< the from index pointing to the syncArray */
  uint to;   /**< the to index pointing to  the syncArray */
  uint lineSpacing;
  double fraction;
  uint endFrom; /**< sampleCounter From */
  uint endTo; /**< sampleCounter To */
};

struct sslantXY
{
  DSPFLOAT x;
  DSPFLOAT y;
};

typedef QList<smatchEntry *> modeMatchList;
typedef QList<modeMatchList *>  modeMatchChain;

class syncProcessor : public QObject
{
  Q_OBJECT
public:
  //  enum esyncState {SYNCOFF,SYNCUP,SYNCSTART,SYNCON,SYNCDOWN,SYNCEND,SYNCVALID};
  enum esyncState {SYNCOFF,SYNCACTIVE,SYNCVALID};
  enum esyncProcessState {MODEDETECT,INSYNC,SYNCLOSTNEWMODE,SYNCLOSTFALSESYNC,SYNCLOSTMISSINGLINES,SYNCLOST,RETRACEWAIT};
  explicit syncProcessor(bool narrow,QObject *parent = 0);
  ~syncProcessor();
  void init();
  void reset();
  void process();
  esyncProcessState getSyncState(quint32  &syncPos) {syncPos=syncPosition; return syncProcesState; }
  esstvMode getMode() {return currentMode;}
  void resetRetraceFlag();
  bool hasNewClock()
  {
    bool nc=newClock;
    newClock=false;
    return nc;
  }
  void clear();
  void recalculateMatchArray();
  DSPFLOAT getNewClock() {return modifiedClock;}
  void setSyncDetectionEnabled(bool enable) {enableSyncDetection=enable;}


  quint32 sampleCounter;
  quint32 syncPosition;
  quint32 lastValidSyncCounter;
  //  DSPFLOAT trackMax;
  int      syncQuality;
  modeBase *currentModePtr;

  quint16      *freqPtr;
  DSPFLOAT     *syncVolumePtr;
  DSPFLOAT     *inputVolumePtr;
  videoFilter  *videoFilterPtr;
  bool         retraceFlag;
  bool tempOutOfSync;

#ifndef QT_NO_DEBUG
  void setOffset(unsigned int dataScopeOffset);
  unsigned int xOffset;
  unsigned int syncStateBuffer[RXSTRIPE];
#endif

public slots:
  void slotNewCall(QString call);
  void slotVisCodeDetected(int, uint visSampleCounter);

signals:
  void callReceived(QString);

private:
  quint32 maxLineSamples;
  quint16 syncArrayIndex;
  ssyncEntry syncArray[MAXSYNCENTRIES];
  modeMatchChain matchArray[NOTVALID];
  quint16 slantAdjustLine;
  esstvMode currentMode;
  esyncState syncState;
  esyncProcessState syncProcesState;
  bool newClock;
  esstvMode idxStart;
  esstvMode idxEnd;
  DSPFLOAT modifiedClock;
  DSPFLOAT samplesPerLine;
  streamDecoder streamDecode;
  DSPFLOAT lineTolerance;
  modeMatchList *activeChainPtr;

  bool currentModeMatchChanged;
  uint lastSyncTest;
  uint lastUpdatedSync;
  sslantXY slantXYArray[MAXSYNCENTRIES];
  esstvMode visMode;

  void extractSync();
  bool validateSync();
  bool findMatch();
  bool addToMatch(esstvMode mode);
  bool addToChain(esstvMode mode, uint fromIdx);
  bool lineCompare(DSPFLOAT samPerLine, int srcIdx, int dstIdx, quint16 &lineNumber, double &fraction);
  void switchSyncState(esyncState newState, quint32 sampleCntr);
  void switchProcessState(esyncProcessState  newState);
  uint calcTotalLines(modeMatchList *mlPtr);
  double calcTotalFract(modeMatchList *mlPtr);
  void clearMatchArray();
  void removeMatchArrayChain(esstvMode mode,int chainIdx);
  void cleanupMatchArray();
  void dropTop();
  void deleteSyncArrayEntry(uint entry);
  bool createModeBase();
  void checkSyncArray();
  void trackSyncs();
  void calcSyncQuality();
  void calculateLineNumber(uint fromIdx, uint toIdx);
  void regression(DSPFLOAT &a, DSPFLOAT &b, bool initial);
  bool slantAdjust(bool initial);
  DSPFLOAT syncWidth;


  // signal quality
  quint16 falseSlantSync;
  quint16 unmatchedSyncs;
  quint16 falseSyncs;
  quint16 avgLineSpacing;
  quint16 missingLines;

  bool detectNarrow;
  bool enableSyncDetection;
  uint  minMatchedLines;
  uint  visEndCounter;
//  DSPFLOAT syncAvgPtr[RXSTRIPE];
//  DSPFLOAT syncAvg;

};

#endif // SYNCPROCESSOR_H




