#ifndef SSTVPARAM_H
#define SSTVPARAM_H
#include <QString>
#include "appglobal.h"

/** \file */

/**
SSTV Parameter functions
@author Johan Maes - ON4QZ
*/


//#define NUMFAXMODES 3 //!< Number of FAX Modes for RX and TX


#define GREENLINE 0 //!< index for green line buffer
#define BLUELINE 1  //!< index for blue line buffer
#define REDLINE 2		//!< index for red line buffer
#define YLINEODD 3 //!< index for intensity line buffer 0 
#define YLINEEVEN 4 //!< index for intensity line buffer 1
#define VIDEOBW 800 
#define MAXLINES 800

#define DEMODAM 0
#define DEMODFM 1
#define MINSYNCWIDTH 0.004
#define MAXSYNCWIDTH 0.020
#define RETRACEWIDTH 0.29



/**
  \brief SSTV Modes

  M1 to FAX480 are using sync pulses, AVT modes do not use syncs.
*/
enum esstvMode
{
  M1,
  M2,
  S1,
  S2,
  SDX,
  SC2_60,
  SC2_120,
  SC2_180,
  R24,
  R36,
  R72,
  P3,
  P5,
  P7,
  BW8,
  BW12,
  PD50,
  PD90,
  PD120,
  PD160,
  PD180,
  PD240,
  PD290,
  MP73,
  MP115,
  MP140,
  MP175,
  MR73,
  MR90,
  MR115,
  MR140,
  MR175,
  ML180,
  ML240,
  ML280,
  ML320,
  FAX480,
  AVT24,
  AVT90,
  AVT94,
  MP73N,
  MP110N,
  MP140N,
  MC110N,
  MC140N,
  MC180N,
  NOTVALID
};

#define NUMSSTVMODES NOTVALID //!< Number of SSTV Modes for RX and TX
#define STARTNARROW MP73N
#define ENDNARROW MC180N

#define STARTWIDE M1
#define ENDWIDE FAX480

//enum efaxMode
//{
//	NOAA,
//	HFFAX,
//	FAXCUSTOM,
//	FAXNONE
//};

/*

// struc used in frequency detection 
struct sTimeFreq
{
  DSPFLOAT t;            // time in sec
  DSPFLOAT dt;           // minimum duration in sec
  unsigned int f;     // frequency to detect (freq=0 if any freq)
};
*/

/** \brief sstv parameter structure

  Structure containing all the mode depended parameters  Some of the parameters are updated at run time

*/

struct sSSTVParam
{
  QString name;
  QString shortName;
  enum esstvMode mode;
  DSPFLOAT imageTime;
  unsigned int numberOfPixels; // NumberOfPixels per Line
  unsigned int numberOfDisplayLines;
  unsigned int numberOfDataLines;  // data lines
  quint32 VISCode;
  float sync;			//used for rx
  float fp;
  float bp;
  float blank;
  float synct;		// used for tx
  float fpt;
  float bpt;
  float blankt;
  DSPFLOAT pixelDuration;
  int subcarrier;
  int deviation;
};

//struct sFAXParam
//{
//	~sFAXParam(){}
//  QString name;
//  QString shortName;
////	enum efaxMode mode;
//	uint modulation;
//	double lpm;
//	unsigned int ioc;
//	unsigned int numberOfDisplayLines;
//	unsigned int numberOfPixels;
//	int subcarrier;
//	int deviation;
//	int aptStartDuration;
//	int aptStartFreq;
//	int aptStopDuration;
//	int aptStopFreq;
//	unsigned int numberOfPhasingLines;
//	bool inverted;
//	unsigned int colorMode;
//};


extern sSSTVParam SSTVTable[NUMSSTVMODES+1];
//extern sFAXParam FAXTable[NUMFAXMODES+1];

extern sSSTVParam rxSSTVParam;
//extern sFAXParam  rxFAXParam;
extern sSSTVParam txSSTVParam;
//extern sFAXParam txFAXParam;
extern DSPFLOAT *lineTimeTableRX;
extern DSPFLOAT *lineTimeTableTX;


void setupSSTVLineTimeTable(esstvMode modeIndex,DSPFLOAT clock,bool transmit);
DSPFLOAT getLineLength(esstvMode modeIndex,DSPFLOAT clock);
DSPFLOAT getSyncWidth(esstvMode modeIndex,DSPFLOAT clock);
//void setupFAXLineTimeTable(DSPFLOAT clock);
esstvMode initializeParametersVIS(unsigned int viscode,bool tx);
bool  initializeSSTVParametersIndex(esstvMode modeIndex,bool tx);
//bool  initializeFAXParametersIndex(efaxMode modeIndex,bool tx);
esstvMode lookupVIS(unsigned int vc);
QString getSSTVModeNameLong(esstvMode m);
QString getSSTVModeNameShort(esstvMode m);

void dumpSamplesPerLine();
//QString getFAXModeNameLong(efaxMode m);
//QString getFAXModeShort(efaxMode m);

esstvMode modeLookup(unsigned int lineLength,DSPFLOAT clock);
DSPFLOAT longestLine(DSPFLOAT clock);
bool lineIsValid(esstvMode mode,unsigned int lineLength,DSPFLOAT clock);
void printActiveSSTVParam(bool tx);
//void copyCustomParam(bool tx);
quint32 getMaxLineSamples();

extern bool autoSlantAdjust;
extern bool autoSave;
extern int sensitivity;
extern esstvMode sstvModeIndexRx;
extern esstvMode sstvModeIndexTx;
#endif





