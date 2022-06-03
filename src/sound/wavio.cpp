/***************************************************************************
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

#include "wavio.h"
#include "appglobal.h"
#include "configparams.h"
#include "dirdialog.h"
#include "unistd.h"
#include <qfiledialog.h>
#include "dispatcher.h"

#include <errno.h>

/**
  constructor: creates a waveIO instance
  \param samplingRate wave file samplingrate (e.g. 8000, 11025 ...)
*/

wavIO::wavIO(unsigned int samplingRate)
{
  samplingrate=samplingRate;
  reading=false;
  writing=false;
}


wavIO::~wavIO()
{
}

void wavIO::closeFile()
{
  if(inopf.isOpen() && writing)
    {
      inopf.flush();
      writeHeader();
    }
  inopf.close();
  reading=false;
  writing=false;
}

/**
   opens a wave file for reading
  \param fname the name of the file to open
  \param ask if ask==true, a filedialog will be opened
  \return true if the file is succcesfully opened. The file is also checked if it is a supported format.
  \sa read
*/ 

bool  wavIO::openFileForRead(QString fname,bool ask)
{
  displayMBoxEvent *stmb;
  QString tmp;

  if (ask)
    {
      dirDialog d((QWidget *)mainWindowPtr,"Wave file");
      QString s=d.openFileName(audioPath,"*");
      if (s.isNull()) return false;
      if (s.isEmpty()) return false;
      inopf.setFileName(s);
    }
  else
    {
      inopf.setFileName(fname);
    }
  if(!inopf.open(QIODevice::ReadOnly))
    {
      stmb= new displayMBoxEvent("WAVIO Error", QString("Unable to open file %1\nError: %2").arg(fname).arg(strerror(errno)));
      QApplication::postEvent( dispatcherPtr, stmb );
      return false;
    }
  reading=true;
  if(inopf.read(&waveHeader.chunkID[0],sizeof(sWave))!=sizeof(sWave))
    {
      stmb= new displayMBoxEvent("WAVIO Error", QString("Invalid format"));
      QApplication::postEvent( dispatcherPtr, stmb );
      closeFile();
      return false;
    }


  // check the header
  if(  (!checkString(waveHeader.chunkID,"RIFF"))
       ||(!checkString(waveHeader.format,"WAVE"))
       ||(!checkString(waveHeader.subChunk1ID,"fmt "))
       ||(!checkString(waveHeader.subChunk2ID,"data")))
    {
      addToLog("wavio read header error",LOGALL);
      stmb= new displayMBoxEvent("WAVIO Error", QString("Invalid Header Format"));
      QApplication::postEvent( dispatcherPtr, stmb );
      closeFile();
      return false;
    }

  if( (waveHeader.subChunk1Size!=16)
      ||(waveHeader.audioFormat!=1)
      ||(waveHeader.numChannels>STEREOCHANNEL)
      ||(waveHeader.sampleRate!=samplingrate)
      //      ||(waveHeader.byteRate!=sizeof(SOUNDFRAME)*samplingrate)
      ||(waveHeader.blockAlign!=waveHeader.numChannels*2)
      ||(waveHeader.bitsPerSample!=16))
    {
      addToLog("wavio read header error, not supported",LOGALL);
      stmb= new displayMBoxEvent("WAVIO Error", QString("Format not supported"));
      QApplication::postEvent( dispatcherPtr, stmb );
      closeFile();
      return false;
    }
  numberOfChannels=waveHeader.numChannels;
  numberOfSamples=waveHeader.subChunk2Size/(2*numberOfChannels);  // number of mono or stereo samples
  if(numberOfSamples==0)
    {
      addToLog("wavio read header : number of samples = 0",LOGALL);
      stmb= new displayMBoxEvent("WAVIO Error", QString("Number of samples  is zero"));
      QApplication::postEvent( dispatcherPtr, stmb );
      closeFile();
      return false;
    }
  samplesRead=0;
  return true;
}

/**
  read data from wave file

  \param dPtr pointer to buffer for SOUNDFRAME type samples.
  \param numSamples  number of samples to read
  \return returns the number of samples read. -1 is returned if there is an error, 0 is returned on end of the file else the numeber of samples read.

The file will be closed on reaching the end of file;

Output is always in mono.
*/

int  wavIO::read(short int *dPtr ,uint numSamples)
{
  int i,llen,result;
  qint16 *tempBuf;

  if(!inopf.isOpen())
    {
      addToLog("wavio not open during read",LOGALL);
      return -1;
    }

  llen=numSamples*sizeof(quint16)*numberOfChannels; // lenght in bytes
  if(waveHeader.numChannels==1)
    {
      result=inopf.read((char*)dPtr,llen); //we do not need conversion
    }
  else
    {
      tempBuf=new qint16[llen/2];
      result=inopf.read((char*)tempBuf,llen);
      for(i=0;i<(result/4);i++)
        {
          dPtr[i]=tempBuf[2*i];
        }
      delete [] tempBuf;

    }

  if((result==0) || (numberOfSamples==samplesRead))
    {
      result=0;
      inopf.close();
    }
  samplesRead+=result/(sizeof(quint16)*numberOfChannels);

  return result/(sizeof(quint16)*numberOfChannels);
}


/**
   opens a wave file for writing
  \param fname the name of the file to open
  \param ask if ask==true, a filedialog will be opened
  \param isStereo false if mono,true if stereo
  \return true if the file is succcesfully opened, and the header written, false otherwise?
  \sa write
*/

bool  wavIO::openFileForWrite(QString fname,bool ask,bool isStereo)
{
  QFileInfo fin;
  if (ask)
    {
      dirDialog d((QWidget *)mainWindowPtr,"wave IO");
      QString fn=d.saveFileName(audioPath,"*.wav","wav");
      if(fn.isEmpty()) return false;
      inopf.setFileName(fn);

    }
  else
    {
      inopf.setFileName(fname);
    }
  if(!inopf.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
      return false;
    }
  numberOfSamples=0;
  if(isStereo) numberOfChannels=2;
  else numberOfChannels=1;

  initHeader();

  if(!writeHeader()) return false;
  writing=true;
  numberOfSamples=0;
  return true;
}

/**
  \brief write data to wave file

  To signal the end, call this function with numSamples=0. The file will automatically be closed.
  \param dPtr pointer to buffer for 16 bit samples SOUNDFRAME indicates if samples are mono or stereo
  \param numSamples  number of samples to read
  \param isStereo false if mono,true if stereo
  \return returns true the correct number of samples are written. false otherwise.
*/

bool  wavIO::write(quint16 *dPtr, uint numSamples,bool isStereo)
{
  uint i;
  int len;
  quint16 *tempBufPtr;
  len=numSamples*sizeof(quint16)*2; // we're outputting stereo
  tempBufPtr=0;
  quint16 *tmpPtr;

  tmpPtr=dPtr;


  if((!writing)&&(numSamples!=0))
    {
      addToLog("wavio not open during write",LOGALL);
      return true;
    }
  if((!writing)&&(numSamples==0)) return true;
  if(numSamples==0)
    {
      addToLog(QString("wavio write close samples=%1").arg(numberOfSamples),LOGWAVIO);
      inopf.flush();
      writeHeader();
      closeFile();
      return true;
    }

  // we need stereo output and input is mono


  if(!isStereo)
    {
      tempBufPtr=new quint16 [numSamples*2];
      tmpPtr=tempBufPtr;
      for(i=0;i<numSamples;i++)
        {
          tempBufPtr[i*2]=dPtr[i];
          tempBufPtr[i*2+1]=0;
        }
    }
  else
    {
      tempBufPtr=new quint16 [numSamples*2];
      tmpPtr=tempBufPtr;
      for(i=0;i<numSamples*2;i++)
        {
          tempBufPtr[i]=dPtr[i];
        }
    }

  if(inopf.write((char *)tmpPtr,len)!=len)
    {
      addToLog("wavio write error",LOGALL);
      closeFile();
      if(tempBufPtr) delete []tempBufPtr;
      return false;
    }
  numberOfSamples+=numSamples;
  addToLog(QString("wavio write:%1 total samples=%2").arg(numSamples).arg(numberOfSamples),LOGWAVIO);
  if(tempBufPtr) delete []tempBufPtr;
  return true;
}


/** setup the defaults in the wave header */

void wavIO::initHeader()
{
  waveHeader.chunkID[0]='R';
  waveHeader.chunkID[1]='I';
  waveHeader.chunkID[2]='F';
  waveHeader.chunkID[3]='F';

  waveHeader.format[0]='W';
  waveHeader.format[1]='A';
  waveHeader.format[2]='V';
  waveHeader.format[3]='E';


  waveHeader.subChunk1ID[0]='f';
  waveHeader.subChunk1ID[1]='m';
  waveHeader.subChunk1ID[2]='t';
  waveHeader.subChunk1ID[3]=' ';

  waveHeader.subChunk2ID[0]='d';
  waveHeader.subChunk2ID[1]='a';
  waveHeader.subChunk2ID[2]='t';
  waveHeader.subChunk2ID[3]='a';

  waveHeader.subChunk1Size=16;      // always 16 for PCM
  waveHeader.audioFormat=1;         // PCM
  waveHeader.numChannels=sizeof(SOUNDFRAME)/sizeof(quint16);         // Stereo
  waveHeader.sampleRate=samplingrate;
  waveHeader.byteRate=sizeof(SOUNDFRAME)*samplingrate;    // 16 bit samples
  waveHeader.blockAlign=4;
  waveHeader.bitsPerSample=16;
  waveHeader.subChunk2Size=numberOfSamples*sizeof(short int);
  waveHeader.chunkSize=36+numberOfSamples*sizeof(short int);
}

bool  wavIO::checkString(char *str,const char *cstr)
{
  for (int i=0;i<4;i++)
    {
      if (str[i]!=cstr[i]) return false;
    }
  return true;
}

bool  wavIO::writeHeader()
{
  int err;
  waveHeader.subChunk2Size=numberOfSamples*sizeof(quint16)*numberOfChannels;
  lseek(inopf.handle(),0,SEEK_SET); //position at beginning
  if((err=inopf.write(&waveHeader.chunkID[0],sizeof(sWave)))!=sizeof(sWave))
    {

      addToLog(QString("wavio write header error %1").arg(err),LOGWAVIO);
      closeFile();
      return false;
    }
  inopf.flush();
  lseek(inopf.handle(),0,SEEK_END); //position at beginning
  addToLog(QString("wavio write header %1 %2 %3 %4").arg(waveHeader.chunkID[0]).arg(waveHeader.chunkID[1]).arg(waveHeader.chunkID[2]).arg(waveHeader.chunkID[3]),LOGWAVIO);
  addToLog(QString("wavio write header samples=%1").arg(numberOfSamples),LOGWAVIO);
  addToLog(QString("wavio write header total bytes=%1").arg(numberOfSamples*2+sizeof(sWave)),LOGWAVIO);
  return true;
}
