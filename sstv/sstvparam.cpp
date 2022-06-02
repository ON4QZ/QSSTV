/**************************************************************************
*   Copyright (C) 2000-2019 by Johan Maes                                 *
*   on4qz@telenet.be                                                      *
*   http://users.telenet.be/on4qz                                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "sstvparam.h"
#include "appglobal.h"
#include "supportfunctions.h"
#include "configparams.h"
#include <math.h>


bool autoSlantAdjust;
bool autoSave;
int sensitivity;
int filterIndex;

DSPFLOAT *lineTimeTableRX=NULL;
DSPFLOAT *lineTimeTableTX=NULL;
sSSTVParam rxSSTVParam;
//sFAXParam  rxFAXParam;
sSSTVParam txSSTVParam;
//sFAXParam txFAXParam;
esstvMode sstvModeIndexRx;


esstvMode sstvModeIndexTx;


/**
 		\brief setup the lineTable 

	Setup a table containing the relative positions expressed in samples	
	\param modeIndex index of the different modes: Martin1=0 , ....
	\param clock the adjusted samplingrate
  \param transmit true if time table for transmit
*/
void setupSSTVLineTimeTable(esstvMode modeIndex,DSPFLOAT clock, bool transmit)
{
  unsigned int i;
  if(transmit)
    {
      if (lineTimeTableTX!=NULL) delete [] lineTimeTableTX;
      lineTimeTableTX=new DSPFLOAT [SSTVTable[modeIndex].numberOfDataLines+1];
      for (i=0;i<SSTVTable[modeIndex].numberOfDataLines+1;i++)
        {
          lineTimeTableTX[i]=((SSTVTable[modeIndex].imageTime*((DSPFLOAT)i))/(DSPFLOAT)SSTVTable[modeIndex].numberOfDataLines)*clock;
        }
    }
  else
  {
    if (lineTimeTableRX!=NULL) delete [] lineTimeTableRX;
    lineTimeTableRX=new DSPFLOAT [SSTVTable[modeIndex].numberOfDataLines+1];
    for (i=0;i<SSTVTable[modeIndex].numberOfDataLines+1;i++)
      {
        lineTimeTableRX[i]=((SSTVTable[modeIndex].imageTime*((DSPFLOAT)i))/(DSPFLOAT)SSTVTable[modeIndex].numberOfDataLines)*clock;
      }
  }



}

/*!
	return the linelength expressed in number of samples
*/

DSPFLOAT getLineLength(esstvMode modeIndex,DSPFLOAT clock)
{
	return (SSTVTable[modeIndex].imageTime/(DSPFLOAT)SSTVTable[modeIndex].numberOfDataLines)*clock;
}

void dumpSamplesPerLine()
{
  int i;
  for(i=0;i<ENDNARROW;i++)
    {
      QString msg=QString("%1  %2").arg(getSSTVModeNameShort((esstvMode) i)).arg(getLineLength((esstvMode) i,12000));
      logFilePtr->addToAux(msg);
      qDebug() << msg;
    }

}


/*!
	return the sync width expressed in number of samples
*/

DSPFLOAT getSyncWidth(esstvMode modeIndex,DSPFLOAT clock)
{
	return (SSTVTable[modeIndex].sync*clock);
}


/**
	\brief setup active parameters
  
	Setup active parameters given the VIS code 
  \param[in] viscode VIS code of the mode to select
  \param[in] tx if set to true then parameters for transmit
  \return Returns the modeIndex if successful else NOTVALID
*/
esstvMode initializeParametersVIS(unsigned int viscode,bool tx)
{
  esstvMode t;
  t=lookupVIS(viscode);
  initializeSSTVParametersIndex(t,tx);
	return t;
}



/**
 * \brief setup active parameters

		Setup active parameters given the modeIndex
 * \param[in] modeIndex selected mode
 * \param[in] tx if set to true then parameters for transmit
 * \return true if successful 
*/
bool  initializeSSTVParametersIndex(esstvMode modeIndex,bool tx)
{
  if (modeIndex < NUMSSTVMODES)
    {
      if(tx) txSSTVParam=SSTVTable[modeIndex];
      else rxSSTVParam=SSTVTable[modeIndex];
      printActiveSSTVParam(tx);
      return true;
    } 
	return false;
}

/**
 * \brief print active parameters

	Print the active parameters for debugging purposes (to logFilePtr and/or console) \sa logFile
*/


#ifndef QT_NO_DEBUG
void printActiveSSTVParam(bool tx)
{

  double clock;
  sSSTVParam * SSTVParam;
  if(tx)
  {
    SSTVParam=&txSSTVParam;
    clock=txClock;
  }
  else
  {
    SSTVParam=&rxSSTVParam;
    clock=rxClock;
  }
  addToLog(QString("name=%1, shortname=%2").arg(SSTVParam->name).arg(SSTVParam->shortName),LOGPARAM);
  addToLog(QString("imageTime=%1,numpix=%2,numDisplayLines=%3, numDataLines=%4,viscode=%5").arg(SSTVParam->imageTime)
              .arg(SSTVParam->numberOfPixels).arg(SSTVParam->numberOfDisplayLines).arg(SSTVParam->numberOfDataLines).arg(QString::number(SSTVParam->VISCode,16)),LOGPARAM);
  addToLog(QString("Samplecounters imageTime=%1 , lineTime=%2").arg(rint(SSTVParam->imageTime*clock)).arg(rint(SSTVParam->imageTime*11025./SSTVParam->numberOfDataLines)),LOGPARAM);
}
#else
void printActiveSSTVParam(bool) {}
#endif

/**
 * setup parameters given the VIS code 
 * @param vc VIS code of the mode to select
 * @return Returns the modeIndex if successful else NOTVALID
*/
esstvMode lookupVIS(unsigned int vc)
{
	if(vc==0) return NOTVALID;
  esstvMode t=M1;
  do
    {
      if (SSTVTable[(int)t].VISCode==vc)
    {
        break;
      }
	    t=(esstvMode)(t+1);
    }
  while (t<NUMSSTVMODES);
  if(t>=NUMSSTVMODES) return NOTVALID;
	return t;
}

/**
	\brief longname lookup
	
	Returns the long name of the mode
	\param[in] modeIndex selected mode 
	\return  short name of the mode or empty string if not a valid mode
*/

QString getSSTVModeNameLong(esstvMode modeIndex)
{
	if(modeIndex==NOTVALID) return QString("");
	return(SSTVTable[(int)modeIndex].name);
}

/**
	\brief shortname lookup
	
	Returns the short name of the mode
	\param[in] modeIndex selected mode 
	\return short name of the mode or empty string  if not a valid mode
*/

QString getSSTVModeNameShort(esstvMode modeIndex)
{
  if(modeIndex==NOTVALID) return QString("");
  return(SSTVTable[(int)modeIndex].shortName);
}


//QString getFAXModeNameLong(efaxMode modeIndex)
//{
//	if(modeIndex>=FAXNONE) return QString("");
//	return(FAXTable[(int)modeIndex].name);
//}

//QString getFAXModeShort(efaxMode modeIndex)
//{
//  if(modeIndex>=FAXNONE) return QString("");
//  return(FAXTable[(int)modeIndex].shortName);
//}

/**
	\brief lookup mode by line length
	
	Returns the closest match for a given line length
	\param[in] lineLength line length in samples
	\param clock	the rx or tx clock to be used
	\return returns the modeIndex or NOTVALID if none found
*/

esstvMode modeLookup(unsigned int lineLength,DSPFLOAT clock)
{
  int i;
  DSPFLOAT errLine, totalError;
  totalError=9999999;
  esstvMode lmode=NOTVALID;
  for (i=M1;i<NOTVALID;i++)
    {
      errLine=1.-(DSPFLOAT)lineLength/((SSTVTable[i].imageTime/((DSPFLOAT)SSTVTable[i].numberOfDataLines))*clock);
      errLine*=errLine;
      if(errLine<totalError)
				{
	  			lmode=(esstvMode)i;
	  			totalError=errLine;
				}
    }
	 if (totalError<0.001) return (esstvMode)lmode;
	 return NOTVALID;
}

bool lineIsValid(esstvMode mode,unsigned int lineLength,DSPFLOAT clock)
{
	DSPFLOAT errLine;
	errLine=1.-(DSPFLOAT)lineLength/((SSTVTable[mode].imageTime/((DSPFLOAT)SSTVTable[mode].numberOfDataLines))*clock);
  errLine*=errLine;
	return (errLine<0.001);
}

/**
	returns the value (in samples) of the longest line in the table
	*/
DSPFLOAT longestLine(DSPFLOAT clock)
{
	int i;
	DSPFLOAT highest=0;
	DSPFLOAT length;
	for(i=0;i<(int)AVT24;i++)
		{
			length=(SSTVTable[i].imageTime/(DSPFLOAT)SSTVTable[i].numberOfDataLines)*clock;
			if (length>highest) highest=length;
		}
	return highest;
}





// if slant \ raise image time

/**
	\brief parameter table for all SSTV, FAX and CALIBRATION modes
*/
sSSTVParam SSTVTable[NUMSSTVMODES+1]=
{
//  name      shortName  mode      imagetime Pix Dis Txl  VIS     sync    fp      bp     blank   synctx   fptx    bptx    blanktx
  {"Martin 1"  ,"M1",     M1,      114.29700,320,256,256,0xAC  ,0.00500,0.00080,0.00050,0.00050,0.00500,0.00080,0.00000,0.00050,0.,1900,400 },
  {"Martin 2"  ,"M2",     M2,       58.06400,320,256,256,0x28  ,0.00500,0.00080,0.00050,0.00050,0.00500,0.00080,0.00000,0.00050,0.,1900,400 },
  {"Scottie 1" ,"S1",     S1,      109.63250,320,256,256,0x3C  ,0.00900,0.00010,0.00125,0.00125,0.00900,0.00080,0.00080,0.00125,0.,1900,400 },
  {"Scottie 2" ,"S2",     S2,       71.09450,320,256,256,0xB8  ,0.00900,0.00010,0.00150,0.00150,0.00900,0.00000,0.00110,0.00125,0.,1900,400 },
  {"Scottie DX","SDX",    SDX,     268.89380,320,256,256,0xCC  ,0.00900,0.00000,0.00000,0.00100,0.00900,0.00000,0.00000,0.00100,0.,1900,400 },
  {"SC2 60",    "SC2-60", SC2_60,   61.54350,320,256,256,0xBB  ,0.00500,0.00100,0.00100,0.00100,0.00500,0.00000,0.00000,0.00000,0.,1900,400 },
  {"SC2 120",   "SC2-120",SC2_120, 121.74250,320,256,256,0x3F  ,0.00500,0.00100,0.00100,0.00100,0.00500,0.00000,0.00000,0.00000,0.,1900,400 },
  {"SC2 180",   "SC2-180",SC2_180, 182.03850,320,256,256,0xB7  ,0.00500,0.00100,0.00100,0.00100,0.00500,0.00000,0.00000,0.00000,0.,1900,400 },
  {"Robot 24",  "R24",    R24,      24.00150,160,120,120,0x84  ,0.00600,0.00010,0.00300,0.00450,0.00600,0.00000,0.00120,0.00380,0.,1900,400 },
  {"Robot 36"  ,"R36",    R36,      36.00200,320,240,240,0x88  ,0.00900,0.00040,0.00250,0.00700,0.00900,0.00000,0.00300,0.00540,0.,1900,400 },
  {"Robot 72"  ,"R72",    R72,      72.00500,320,240,240,0x0C  ,0.00900,0.00040,0.00350,0.00600,0.00900,0.00040,0.00250,0.00600,0.,1900,400 },
  {"P3"        ,"P3" ,    P3,      203.06420,640,496,496,0x71  ,0.00520,0.00210,0.00080,0.00250,0.00520,0.00104,0.00104,0.00104,0.,1900,400 },
  {"P5"        ,"P5" ,    P5,      304.59600,640,496,496,0x72  ,0.00780,0.00160,0.00160,0.00160,0.00780,0.00160,0.00160,0.00160,0.,1900,400 },
  {"P7"        ,"P7" ,    P7,      406.12800,640,496,496,0xF3  ,0.01040,0.00210,0.00210,0.00210,0.01040,0.00210,0.00210,0.00210,0.,1900,400 },
  {"B/W 8"     ,"BW8" ,   BW8,       8.02800,160,120,120,0x82  ,0.00600,0.00050,0.00050,0.00000,0.00600,0.00100,0.00050,0.00000,0.,1900,400 },
  {"B/W 12"    ,"BW12" ,  BW12,     12.00100,160,120,120,0x86  ,0.00600,0.00050,0.00150,0.00000,0.00600,0.00100,0.00100,0.00000,0.,1900,400 },
  {"PD50"      ,"PD50",   PD50,     49.68770,320,256,128,0xDD  ,0.02000,0.00000,0.00208,0.00000,0.02000,0.00000,0.00230,0.00000,0.,1900,400 },
  {"PD90"      ,"PD90",   PD90,     89.99500,320,256,128,0x63  ,0.02000,0.00000,0.00208,0.00000,0.02000,0.00000,0.00230,0.00000,0.,1900,400 },
  {"PD120"     ,"PD120",  PD120,   126.11150,640,496,248,0x5F  ,0.02000,0.00000,0.00208,0.00000,0.02000,0.00000,0.00230,0.00000,0.,1900,400 },
  {"PD160"     ,"PD160",  PD160,   160.89420,512,400,200,0xE2  ,0.02000,0.00000,0.00200,0.00000,0.02000,0.00000,0.00230,0.00000,0.,1900,400 },
  {"PD180"     ,"PD180",  PD180,   187.06450,640,496,248,0x60  ,0.02000,0.00000,0.00200,0.00000,0.02000,0.00000,0.00230,0.00000,0.,1900,400 },
  {"PD240"     ,"PD240",  PD240,   248.01700,640,496,248,0xE1  ,0.02000,0.00200,0.00200,0.00000,0.02000,0.00000,0.00230,0.00000,0.,1900,400 },
  {"PD290"     ,"PD290",  PD290,   288.70200,800,616,308,0xDE  ,0.02000,0.00000,0.00200,0.00000,0.02000,0.00000,0.00230,0.00000,0.,1900,400 },
  {"MP73"      ,"MP73",   MP73,     72.9642,320,256,128,0x2523,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"MP115"     ,"MP115",  MP115,   115.4640 ,320,256,128,0x2923,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"MP140"     ,"MP140",  MP140,   139.5280,320,256,128,0x2A23,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"MP175"     ,"MP175",  MP175,   175.3730,320,256,128,0x2C23,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"MR73"      ,"MR73",   MR73,     73.2980 ,320,256,256,0x4523,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"MR90"      ,"MR90",   MR90,     90.1950 ,320,256,256,0x4623,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"MR115"     ,"MR115",  MR115,   115.2850 ,320,256,256,0x4923,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"MR140"     ,"MR140",  MR140,   140.3740 ,320,256,256,0x4a23,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"MR175"     ,"MR175",  MR175,   175.1920 ,320,256,256,0x4a23,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"ML180"     ,"ML180",  ML180,   180.2085 ,640,496,496,0x8523,0.00900,0.00050,0.00050,0.00050,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"ML240"     ,"ML240",  ML240,   239.7328 ,640,496,496,0x8623,0.00900,0.00050,0.00050,0.00050,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"ML280"     ,"ML280",  ML280,   280.4060 ,640,496,496,0x8923,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"ML320"     ,"ML320",  ML320,   320.0910 ,640,496,496,0x8A23,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,1900,400 },
  {"FAX480"    ,"FAX480", FAX480,  133.63300,512,500,500,0x00  ,0.00512,0.00000,0.00000,0.00000,0.00512,0.00000,0.00000,0.00000,0.,1900,400 },
  {"AVT24"     ,"AVT24",  AVT24,    22.50160,128,120,120,0xc0  ,0.00500,0.00080,0.00050,0.00050,0.00500,0.00080,0.00000,0.00050,0.,1900,400 },
  {"AVT90"     ,"AVT90",  AVT90,    90.00450,320,240,240,0x44  ,0.00500,0.00080,0.00050,0.00050,0.00500,0.00080,0.00000,0.00050,0.,1900,400 },
  {"AVT94"     ,"AVT94",  AVT94,    93.75000,320,200,200,0x48  ,0.00500,0.00080,0.00050,0.00050,0.00500,0.00080,0.00000,0.00050,0.,1900,400 },
// narrowModes
  {"MP73-Narrow" ,"MP73-N",   MP73N,   72.9665,320,256,128, 0x5C256D,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,2172,128 },
  {"MP110-Narrow" ,"MP110-N", MP110N, 109.8320,320,256,128, 0x44456D,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,2172,128 },
  {"MP140-Narrow" ,"MP140-N", MP140N, 139.5300,320,256,128 ,0x40556D,0.00900,0.00000,0.00100,0.00000,0.00900,0.00000,0.00100,0.00000,0.,2172,128 },
  {"MC110-Narrow" ,"MC110-N",   MC110N,   109.703, 320,256,256,   0x05456D,0.00800,0.00000,0.00050,0.00000,0.00900,0.00000,0.00100,0.00000,0.,2172,128 },
  {"MC140-Narrow" ,"MC140-N",   MC140N,   140.426, 320,256,256,   0x01556D,0.00800,0.00000,0.00050,0.00000,0.00900,0.00000,0.00100,0.00000,0.,2172,128 },
  {"MC180-Narrow" ,"MC180-N",   MC180N,   180.363, 320,256,256,   0x0D656D,0.00800,0.00000,0.00050,0.00000,0.00900,0.00000,0.00100,0.00000,0.,2172,128 },
  {"No Mode"   ,"NOTVALID",NOTVALID ,0.00000,0,0,0,0x0000,0.00000,0.00000,0.000000,0.0000,0.00000,0.00000,0.000000,0.0000,0.,0,0}
};


//sFAXParam FAXTable[NUMFAXMODES+1]=
//{
////  name      shortName     mode      modulation     lpm  ioc  lines  pixels carr dev  start sFreq stop sFreq  phL invert  colorM
	
//	{"NOAA"    	 ,"NOAA",		 NOAA       ,DEMODAM,      120.,  576,  800, 1810, 2400, 400,  0   , 300,  0,  450 ,  0,  false,  1},
//	{"HFFAX"   	 ,"HFFXAX",	 HFFAX      ,DEMODFM,      120.,  288,  800,  905, 1900, 400,  5   , 300,  5,  450 , 20,  false,  1},
//	{"Custom"    ,"Custom",	 FAXCUSTOM  ,DEMODAM,      121.,  288,  180,  905, 1900, 400,  5   , 300,  5,  450 , 20,  false,  1},
//	{"No Mode"   ,"NOTVALID",FAXNONE    ,0,         0,    0,    0,     0,  0,   0,  0   ,   0,  0,    0 ,  0,  false,  0},
//};

//#ifndef QT_NO_DEBUG

//void printActiveFAXParam(bool tx)
//{
//  sFAXParam * FAXParam;
//  if(tx)
//  {
//    FAXParam=&txFAXParam;
//  }
//  else
//  {
//    FAXParam=&rxFAXParam;
//  }


//  addToLog(QString("name=%1, shortname=%2\n").arg(FAXParam->name).arg(FAXParam->shortName),LOGPARAM);
//  addToLog(QString("modulation=%1,lpm=%2,ioc=%3,numDisplayLines=%4, subcarrier=%5,deviation=%6\n").arg(FAXParam->modulation)
//              .arg(FAXParam->lpm).arg(FAXParam->ioc).arg(FAXParam->numberOfDisplayLines).arg(FAXParam->subcarrier)
//              .arg(FAXParam->deviation),LOGPARAM);
//}
//#else
//void printActiveFAXParam(bool ) {}
//#endif




// * \brief setup active parameters

//		Setup active parameters given the modeIndex
// * \param[in] modeIndex selected mode
// * \return true if successful
//*/
//bool  initializeFAXParametersIndex(efaxMode modeIndex,bool tx)
//{

//  if (modeIndex < NUMFAXMODES)
//    {
//      if(tx)
//      {
//        txFAXParam=FAXTable[modeIndex];
//        txFAXParam.numberOfPixels=(txFAXParam.ioc*31419+5000)/10000;
//      }
//      else
//      {
//        rxFAXParam=FAXTable[modeIndex];
//        rxFAXParam.numberOfPixels=(rxFAXParam.ioc*31419+5000)/10000;
//      }

//      printActiveFAXParam(tx);
//      return true;
//    }
//	return false;
//}

//void copyCustomParam(bool tx)
//{
////	memmove((char *)&FAXTable[FAXCUSTOM],(char *)&activeFAXParam,sizeof(sFAXParam));
//  if(tx) FAXTable[FAXCUSTOM]=txFAXParam;
//  else FAXTable[FAXCUSTOM]=rxFAXParam;;
//}

/***
 		\brief setup the FAX lineTable 

	Setup a table containing the relative positions expressed in samples	
	\param clock the adjusted samplingrate
*/
//void setupFAXLineTimeTable(DSPFLOAT clock,bool tx)
//{
//  unsigned int i;
//  if(tx)
//    {
//      for (i=1;i<=txFAXParam.numberOfDisplayLines;i++)
//        {
//          // one lineTime= lpm/60
//          lineTimeTableTX[i-1]=((60.*(DSPFLOAT)i)/txFAXParam.lpm)*clock;
//        }
//    }
//  else
//  {
//    for (i=1;i<=rxFAXParam.numberOfDisplayLines;i++)
//      {
//        // one lineTime= lpm/60
//        lineTimeTableRX[i-1]=((60.*(DSPFLOAT)i)/rxFAXParam.lpm)*clock;
//      }
//  }

//}

quint32 getMaxLineSamples()
{
  int i;
  DSPFLOAT maxTime=0;
  DSPFLOAT time;
  for(i=M1;i<=FAX480;i++)
  {
    time=SSTVTable[i].imageTime/(DSPFLOAT)SSTVTable[i].numberOfDataLines;
    if(time>maxTime) maxTime=time;
  }
  return rint(maxTime*SAMPLERATE);
}

