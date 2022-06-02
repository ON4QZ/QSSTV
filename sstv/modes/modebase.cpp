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
#include "modebase.h"
#include "appglobal.h"
#include "configparams.h"
#include "dispatcher.h"
#include "synthes.h"
#include "rxwidget.h"
#include "txwidget.h"
#include <QApplication>
#include "logging.h"

const QString stateStr[modeBase::MBTXGAPROBOT+1]=
{
  "ERROR",
  "SETUPLINE",
  "PIXELS",
  "SYNC",
  "ENDOFLINE",
  "EOIMAGE",
  "RXWAIT",
  "MB1500",
  "MB2300",
  "TXGAP",
  "TXGAPROBOT"
};



modeBase::modeBase(esstvMode m, unsigned int len, bool tx, bool narrowMode)
{
  mode=m;
  narrow=narrowMode;
  if(narrow)
    {
     lowerFreq=2044;
     syncFreq=1900;
    }
  else
    {
      lowerFreq=1500;
      syncFreq=1200;
    }


  transmit=tx;
  length=len;
  greenArrayPtr=NULL;
  blueArrayPtr=NULL;
  redArrayPtr=NULL;
  yArrayPtr=NULL;
  pixelArrayPtr=NULL;
  pixelPositionTable=NULL;
  debugStatePtr=NULL;
  addToLog(QString("mb constructor mode=%1").arg((int) m),LOGMODES);
  if(transmit)
    {
      localClock=txClock;
      activeSSTVParam=&txSSTVParam;
    }
  else
    {
      localClock=rxClock;
      activeSSTVParam=&rxSSTVParam;
    }
}

modeBase::~modeBase()
{
  deleteBuffers();
}


/**
    \brief delete existing buffers

    Deletes all buffers, and sets the pointers to NULL;
 */
void modeBase::deleteBuffers()
{
  if(pixelPositionTable) delete [] pixelPositionTable;
  if(greenArrayPtr) delete [] greenArrayPtr;
  if(blueArrayPtr) delete [] blueArrayPtr;
  if(redArrayPtr) delete [] redArrayPtr;
  if(yArrayPtr) delete [] yArrayPtr;
  if(debugStatePtr) delete [] debugStatePtr;
  greenArrayPtr=blueArrayPtr=redArrayPtr=yArrayPtr=NULL;
  debugStatePtr=NULL;
  pixelPositionTable=NULL;
}

/**
  \brief initialize the selected mode

  This function initializes all buffers and mode parameters. The localClock will be set to rxClk if rxClk is not zero in receive mode, else the rxClock of the configuration will be used. The localClock is always equal to the txClock from the configuration while in transmit mode;
  \param[in] clk adjusted receive clock
*/

void modeBase::init(DSPFLOAT clk)
{
  localClock=clk;
  lineCounter=0;
  displayLineCounter=0;
  subLine=0;
  sampleCounter=0;
  setupSSTVLineTimeTable(mode,localClock,transmit);
  state=MBSETUPLINE;
  debugState=stHUNT;
  deleteBuffers();
  pixelPositionTable=new unsigned int[activeSSTVParam->numberOfPixels];
  greenArrayPtr=new unsigned char[activeSSTVParam->numberOfPixels];
  blueArrayPtr=new unsigned char[activeSSTVParam->numberOfPixels];
  redArrayPtr=new unsigned char[activeSSTVParam->numberOfPixels];
  yArrayPtr=new unsigned char[activeSSTVParam->numberOfPixels];
  debugStatePtr=new unsigned int [length];
  for(unsigned int i=0;i<length;i++)
    {
      debugStatePtr[i]=stHUNT;
    }
  if(transmit)
    {
      fp=activeSSTVParam->fpt*localClock;
      bp=activeSSTVParam->bpt*localClock;
      blank=activeSSTVParam->blankt*localClock;
    }
  else
    {
      fp=activeSSTVParam->fp*localClock;
      bp=activeSSTVParam->bp*localClock;
      blank=activeSSTVParam->blank*localClock;
    }
  syncDuration=activeSSTVParam->sync*localClock;
  setupParams(localClock);
  activeSSTVParam->pixelDuration=visibleLineLength/(double)activeSSTVParam->numberOfPixels;
  avgSample=0;
  avgSampleCounter=0;
  isRunning=false;
  //  if(transmit)
  //    {
  //      txWidgetPtr->getImageViewerPtr()->createImage(QSize(activeSSTVParam->numberOfPixels,activeSSTVParam->numberOfDisplayLines),QColor(128,128,128));
  //    }
  //  else
  //    {
  //     rxWidgetPtr->getImageViewerPtr()->createImage(QSize(activeSSTVParam->numberOfPixels,activeSSTVParam->numberOfDisplayLines),QColor(128,128,128));
  //    }
}


void modeBase::redrawFast(bool r)
{
  fastRedraw=r;
  if (!fastRedraw)
    {
      lineDisplayEvent *ce= new lineDisplayEvent(displayLineCounter);
      QApplication::postEvent( dispatcherPtr, ce );  // Qt will delete it when done
    }
}

modeBase::eModeBase modeBase::process(quint16 *demod,unsigned int syncPos,bool goToSync,unsigned int rxPos)
{
  Q_UNUSED (rxPos);
  unsigned int i=0;
  if(goToSync)
    {
      if(syncPos >=length)
        {
          addToLog(QString("modebase:process: syncPos: %1 > length %2").arg(syncPos).arg(length),LOGMODES);
          return MBENDOFIMAGE;
        }
      else
        {
          for(i=0;i<syncPos;i++)  ;//debugStatePtr[i]=debugState;
        }
      rxSampleCounter+=syncPos;
    }
  for(;i<length;i++)
    {
      if(state==MBSETUPLINE)
        {
          switchState(rxSetupLine());
          if(state==MBENDOFLINE)
            {
              showLine();
              if (!fastRedraw)
                {
                  lineDisplayEvent *ce= new lineDisplayEvent(displayLineCounter);
                  QApplication::postEvent( dispatcherPtr, ce );  // Qt will delete it when done
                }
              lineCounter++;
              if (displayLineCounter>=activeSSTVParam->numberOfDisplayLines)
                {
                  switchState(MBEOIMAGE);
                }
              else
                {
                  subLine=0;
                  switchState(rxSetupLine());
                }
            }
          pixelCounter=0;
          subLine++;
        }
      sample=demod[i];
      debugStatePtr[i]=debugState;

      switch(state)
        {
        case MBPIXELS:
          if(getPixels())
            {
              switchState(MBSETUPLINE);
            }
          break;
        case MBEOIMAGE:
          return  MBENDOFIMAGE;
          break;
        case MBRXWAIT:
          if(sampleCounter>=marker)
            {
              switchState(MBSETUPLINE);
            }
          break;
        case MBSYNC:
          if(sampleCounter>=syncPosition)
            {
              //    addToLog(QString("modebase:mbsync =%1").arg(sampleCounter+rxSampleCounter),LOGMODES);
              switchState(MBSETUPLINE);
            }
          break;
        case MB1500:
          {
            // check ODD/EVEN Line for Robot 12/36
            if(sampleCounter>=marker)
              {
                logFilePtr->addToAux(QString("m15avg\t%1\t%2\t%3").arg(avgFreqGap).arg(avgFreqGapCounter-AVGFRQOFFSET).arg(avgFreqGap/(avgFreqGapCounter-AVGFRQOFFSET)));
                avgFreqGap/=(avgFreqGapCounter-AVGFRQOFFSET);
                addToLog(QString("GapCounter1 %1 at %2").arg(avgFreqGap).arg(sampleCounter),LOGMODES);
                if(avgFreqGap > 2100)
                  {
//                    subLine=10;
                    avgOddEvenFreq=avgFreqGap;
                    logFilePtr->addToAux(QString("m15 eval: %1").arg(avgOddEvenFreq));
                  }
                switchState(MBSETUPLINE);
              }
            else
              {
                if(avgFreqGapCounter>=AVGFRQOFFSET) avgFreqGap+=demod[i];
                avgFreqGapCounter++;
                logFilePtr->addToAux(QString("m15\t%1\t%2\t%3").arg(demod[i]).arg(avgFreqGapCounter).arg(avgFreqGap));
              }
          }
          break;
        case MB2300:
          {
            // check ODD/EVEN Line for Robot 12/36
            if(sampleCounter>=marker)
              {
                logFilePtr->addToAux(QString("m23avg\t%1\t%2\t%3").arg(avgFreqGap).arg(avgFreqGapCounter-AVGFRQOFFSET).arg(avgFreqGap/(avgFreqGapCounter-AVGFRQOFFSET)));
                avgFreqGap/=(avgFreqGapCounter-AVGFRQOFFSET);
                addToLog(QString("GapCounter2 %1 at %2").arg(avgFreqGap).arg(sampleCounter),LOGMODES);
                if(avgFreqGap < 1700)
                  {
                    addToLog(QString("Switching to 1500 GapCounter2 %1 at %2").arg(avgFreqGap).arg(i+ rxPos),LOGMODES);
                    if(avgOddEvenFreq>avgFreqGap)
                      {
                        subLine=3;
                        logFilePtr->addToAux("m23 switch subline 3");
                      }
                  }
                switchState(MBSETUPLINE);
              }
            else
              {
                if(avgFreqGapCounter>=AVGFRQOFFSET) avgFreqGap+=demod[i];
                avgFreqGapCounter++;
                logFilePtr->addToAux(QString("m23\t%1\t%2\t%3").arg(demod[i]).arg(avgFreqGapCounter).arg(avgFreqGap));
              }
          }
          break;
        default:
          addToLog(QString("unknown state in modeBase: %1 receive").arg((int)state),LOGMODES);
          break;
        }
      sampleCounter++;
    }
  return MBRUNNING;
}

/**
  \brief transfer pixels to pixelArrayPtr

  This function checks the sampleCounter and stores the pixel in the pixelArray
  \return true if end of line (all pixels stored)
*/

bool modeBase::getPixels()
{
  int color;
  double dev=activeSSTVParam->deviation*2;
  double fc=activeSSTVParam->subcarrier;
  avgSample+=sample;
  avgSampleCounter++;
  if(sampleCounter>=pixelPositionTable[pixelCounter]+(activeSSTVParam->pixelDuration/2))
    {
      //      addToLog(QString("modebase:getPixels[0] =%1").arg(sampleCounter+rxSampleCounter),LOGMODES);
      //      color=128+lround(((double)avgSample/(double)avgSampleCounter-fc)*255./dev);
      color=128+lround(((double)sample-fc)*255./dev);
      if(color<0) color=0;
      if (color>255) color=255;
      pixelArrayPtr[pixelCounter]=(unsigned char)color;
      pixelCounter++;
      avgSample=0;
      avgSampleCounter=0;
      if(pixelCounter>=activeSSTVParam->numberOfPixels) return true;
    }
  return false; // indicate, it's not the end of the line
}	

/**
  \brief transfer data to rxImage (mode depended)

  This function is the default behaviour. it calls combineColors(). This function must be reimplemented in the derived classes for all other colour modes.
*/

void modeBase::showLine()
{
  combineColors();
}
/**
  \brief tranfer data to rxImage in RGB mode

  Combine  R, G and B arrays (like in Martin mode) into the rxImage and advances the displayCounter
*/

void modeBase::combineColors()
{
  unsigned int i;
  QRgb *pixelArray=rxWidgetPtr->getImageViewerPtr()->getScanLineAddress(displayLineCounter);
  for(i=0;i<activeSSTVParam->numberOfPixels;i++)
    {
      pixelArray[i]=qRgb(redArrayPtr[i],greenArrayPtr[i],blueArrayPtr[i]);
      //      pixelArray[i]=qRgb(greenArrayPtr[i],greenArrayPtr[i],greenArrayPtr[i]);

      //        pixelArray[i]=qRgb(255,0,0);
    }
  displayLineCounter++;
}



/**
  \brief tranfer data to rxImage in grayscale

  Black and White image transfer. greenArray contains the luminance info.
*/


void modeBase::grayConversion()
{
  unsigned int i;
  QRgb *pixelArray=rxWidgetPtr->getImageViewerPtr()->getScanLineAddress(displayLineCounter);
  for(i=0;i<activeSSTVParam->numberOfPixels;i++)
    {
      pixelArray[i]=qRgb(greenArrayPtr[i],greenArrayPtr[i],greenArrayPtr[i]);
    }
  displayLineCounter++;
}

/**
  \brief tranfer data to rxImage in YUV mode

  Combine  Y, U  and V arrays (like in PD modes) into the rxImage and advances the displayCounter
*/

void modeBase::yuvConversion(unsigned char *array)
{
  unsigned int i;
  int r,g,b;
  QRgb *pixelArray=rxWidgetPtr->getImageViewerPtr()->getScanLineAddress(displayLineCounter);
  for (i=0;i<activeSSTVParam->numberOfPixels;i++)
    {
      r=(100*array[i]+140*redArrayPtr[i]-17850)/100;
      b=(100*array[i]+178*blueArrayPtr[i]-22695)/100;
      g=(100*array[i]- 71*redArrayPtr[i]-33*blueArrayPtr[i]+13260)/100;
//      r=b=g=array[i]; //test
      r=(r>255 ? 255 : r); r=(r<0 ? 0 : r);
      b=(b>255 ? 255 : b); b=(b<0 ? 0 : b);
      g=(g>255 ? 255 : g); g=(g<0 ? 0 : g);
      pixelArray[i]=qRgb(r,g,b);
    }
  displayLineCounter++;
}

modeBase::eModeBase modeBase::transmitImage(imageViewer *iv)
{
  txImPtr=iv;
  if(!iv->hasValidImage()) return MBENDOFIMAGE;
  addToLog(QString("Starting Transmit Image"),LOGMODES);
  displayLineCounter=0;
  lineCounter=0;
  sampleCounter=0;
  getLine();
  state=MBSETUPLINE;
  start=0;
  subLine=0;
  abortRun=false;
  while(!abortRun)
    {
      isRunning=true;
      if(state==MBSETUPLINE)
        {
          switchState(txSetupLine());
          subLine++;
          pixelCounter=0;
        }
      switch (state)
        {
        case MBPIXELS:
          {
            addToLog(QString("MBPIXELS: samplcntr=%1").arg(sampleCounter),LOGMODES);
            sendPixelBuffer();
            switchState(MBSETUPLINE); // check for end of subline
          }
          break;
        case MBTXGAP:
          {
            //       addToLog(QString("MBTXGAP: samplcntr=%1").arg(sampleCounter),LOGMODES);
            synthesPtr->sendSamples(txDur,txFreq); //expressed in samples;
            sampleCounter+=txDur;

            switchState(MBSETUPLINE);
          }
          break;
        case MBENDOFLINE:
          {
            //     addToLog(QString("MBENDOFLINE samplcntr=%1 line: %2").arg(sampleCounter).arg(lineCounter),LOGMODES);
            if(++lineCounter>=activeSSTVParam->numberOfDataLines) state=MBEOIMAGE;
            else
              {
                getLine();
                switchState(MBSETUPLINE);
                subLine=0;
              }

          }
          break;
        default:
          //   addToLog(QString("default: samplcntr=%1").arg(sampleCounter),LOGMODES);
          sampleCounter=0;
          return MBENDOFIMAGE;
        }
    }
  isRunning=false;
  abortRun=false;
  addToLog("abortrun detected",LOGMODES);
  return MBABORTED;
}

/**
  \brief abort tx

  Only used for aborting a transmission
*/

void  modeBase::abort()
{
  abortRun=true;
  addToLog("modebase: abort received",LOGMODES);
}

/**
  \brief send pixels in the pixelArray

  This function sends one subLine of colour information contained in the pixelArray.

*/

void modeBase::sendPixelBuffer()
{
  double f;
  // addToLog (QString(" sendPixelBuffer: pixelBuffer: %1").arg(QString::number((ulong)pixelArrayPtr,16)),LOGMODES);
  do
    {
      f=lowerFreq+((double)pixelArrayPtr[pixelCounter]*(2300-lowerFreq)/255.);

      while(sampleCounter<pixelPositionTable[pixelCounter])
        {
          if(f>2300) f=2300;
          if (f<lowerFreq) f=lowerFreq;
          synthesPtr->sendSample(f);
          sampleCounter++;
        }
      pixelCounter++;
    }
  while(pixelCounter<activeSSTVParam->numberOfPixels);
  //  addToLog(QString("modebase: lpw=%1").arg(sampleCounter),LOGMODES);
}

/**
  \brief get YUV information

  Get the YUV information from the image and setup the colour arrays. The displayCounter is incremented by 1 or 2, depending on the mode (e.g. 1 if Robot24 and 2 if Robot36).

*/

void modeBase::getLineY(bool evenodd)
{
  // we will process 2 lines at a time
  //	QColor c;
  int tO,tE;
  int r,yo,ye,b;
  if ((displayLineCounter&1) && (evenodd)) return; // only even lines accepted
  //  txImPtr->createImage(QSize(activeSSTVParam->numberOfPixels,activeSSTVParam->numberOfDisplayLines),QColor(128,128,128),imageStretch);
  unsigned int *pixelArrayE=txImPtr->getScanLineAddress(displayLineCounter);
  if (evenodd)
    {
      unsigned int *pixelArrayO=txImPtr->getScanLineAddress(displayLineCounter+1);
      for (unsigned int i=0;i<activeSSTVParam->numberOfPixels;i++)
        {
          tE=pixelArrayE[i];
          ye=(59*qGreen(tE)+30*qRed(tE)+11*qBlue(tE))/100;
          tO=pixelArrayO[i];
          yo=(59*qGreen(tO)+30*qRed(tO)+11*qBlue(tO))/100;
          r=(qRed(tO)+qRed(tE))/2;
          b=(qBlue(tO)+qBlue(tE))/2;
          r=(10*r-5*(yo+ye)+7*255)/14;
          b=(100*b-50*(yo+ye)+89*255)/178;
          redArrayPtr[i]=(r>255 ? 255 : r); redArrayPtr[i]=(r<0 ? 0 : r);
          blueArrayPtr[i]=(b>255 ? 255 : b); blueArrayPtr[i]=(b<0 ? 0 : b);
          yArrayPtr[i]=(ye>255 ? 255 : ye); yArrayPtr[i]=(ye<0 ? 0 : ye);
          greenArrayPtr[i]=(yo>255 ? 255 : yo); greenArrayPtr[i]=(yo<0 ? 0 : yo);
        }
      displayLineCounter++;
    }
  else
    {
      //			addToLog(QString("getline=%1").arg(lineCounter),LOGMODES);
      for (unsigned int i=0;i<activeSSTVParam->numberOfPixels;i++)
        {
          tE=pixelArrayE[i];
          ye=(59*qGreen(tE)+30*qRed(tE)+11*qBlue(tE))/100;
          r=qRed(tE);
          b=qBlue(tE);
          r=(10*r-10*(ye)+7*255)/14;
          b=(100*b-100*(ye)+89*255)/178;
          redArrayPtr[i]=(r>255 ? 255 : r<0 ? 0 : r);
          blueArrayPtr[i]=(b>255 ? 255 : b<0 ? 0 : b);
          yArrayPtr[i]=(ye>255 ? 255 : ye<0 ? 0 : ye);
        }
    }
  displayLineCounter++;
}

/**
  \brief get luminance (B&W) information

  Get the luminance information from the image and setup the colour array. The displayCounter is incremented

*/

void modeBase::getLineBW()
{

  unsigned int t;
  //  txImPtr->createImage(QSize(activeSSTVParam->numberOfPixels,activeSSTVParam->numberOfDisplayLines),QColor(128,128,128),imageStretch);
  unsigned int *pixelArray=txImPtr->getScanLineAddress(displayLineCounter);
  for (unsigned int i=0;i<activeSSTVParam->numberOfPixels;i++)
    {
      t=pixelArray[i];
      greenArrayPtr[i]=qGray(t);
    }
  displayLineCounter++;
}


/**
  \brief get RGB information

  Get the RGB information from the image and setup the colour arrays. The displayCounter is incremented

*/

void modeBase::getLine()
{
  unsigned int t;

  //  txImPtr->createImage(QSize(activeSSTVParam->numberOfPixels,activeSSTVParam->numberOfDisplayLines),QColor(128,128,128));
  //  addToLog (QString(" Bufferpointers: green: %1 red %2, blue %3")
  //      .arg(QString::number((ulong)greenArrayPtr,16))
  //      .arg(QString::number((ulong)redArrayPtr,16))
  //      .arg(QString::number((ulong)blueArrayPtr,16)),LOGMODES);
  unsigned int *pixelArray=txImPtr->getScanLineAddress(displayLineCounter);


  for (unsigned int i=0;i<activeSSTVParam->numberOfPixels;i++)
    {
      t=pixelArray[i];
      greenArrayPtr[i]=qGreen(t);
      redArrayPtr[i]=qRed(t);
      blueArrayPtr[i]=qBlue(t);
      //            greenArrayPtr[i]=255;
      //            redArrayPtr[i]=0;
      //            blueArrayPtr[i]=0;
    }
  displayLineCounter++;
}

void modeBase::switchState(embState  newState)
{
//  addToLog(QString("%1 to %2").arg(stateStr[state]).arg(stateStr[newState]),LOGMODES);
  state=newState;
}
