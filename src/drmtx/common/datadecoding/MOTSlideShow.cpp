 /******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Adapted for ham sstv use Ties Bos - PA0MBO
 *
 * Description:
 *	MOT applications (MOT Slideshow and Broadcast Web Site)
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/

#include "MOTSlideShow.h"
#include <stdio.h>
#include <stdlib.h>
#include "drm.h"
#include "drmtransmitter.h"
//#include"supportfunctions.h"


/* Implementation *************************************************************/
/******************************************************************************\
* Encoder                                                                      *
\******************************************************************************/
void CMOTSlideShowEncoder::GetDataUnit (CVector < _BINARY > &vecbiNewData)
{
    if (allDataSend)
      {
        if(extraBlocks-- <=0) stopDRM=true;
      }

    /* Get new data group from MOT encoder. If the last MOT object was
       completely transmitted, this functions returns true. In this case, put
       a new picture to the MOT encoder object */
    if (MOTDAB.GetDataGroup (vecbiNewData) == true)
      {
        allDataSend=true;
        AddNextPicture (); //basically the same
//        addToLog("loading raw data",LOGDRMMOT);

      }
//    arrayBinDump(QString("slice %1").arg(sliceCounter++),vecbiNewData,32,true);
}

_BOOLEAN CMOTSlideShowEncoder::GetTransStat (string & strCurPict, _REAL & rCurPerc) const
{
/*
	Name and current percentage of transmitted data of current picture.
*/
    strCurPict = strCurObjName;
    rCurPerc = MOTDAB.GetProgPerc ();

    if (vecPicFileNames.Size () != 0)
	return true;
    else
	return false;
}

void CMOTSlideShowEncoder::Init (CParameter &TParam)
{
    bytesToBeUsed=(TParam.iNumDecodedBitsMSC/SIZEOF__BYTE);
    iPictureCnt = 0;
    strCurObjName = "";
    MOTDAB.Reset ();
    AddNextPicture ();
    allDataSend=false;
    extraBlocks=5;
    sliceCounter=0;
    MOTDAB.prepareSegmentList(1);  // one repitition
    addToLog("Init and loading raw data",LOGDRMMOT);
}

void CMOTSlideShowEncoder::AddNextPicture ()
{
  int i;
  unsigned char byteIn;

  /* Here at least one picture is in container */
  if (vecPicFileNames.Size () > 0)
    {
      /* Get current file name */
      QString tmp=vecPicFileNames[iPictureCnt].name+"."+vecPicFileNames[iPictureCnt].format;
      strCurObjName = tmp.toLatin1().data();
      CMOTObject MOTPicture;
          /* Set file name and format string */
      MOTPicture.strName = strCurObjName;
      MOTPicture.strFormat = vecPicFileNames[iPictureCnt].format.toLatin1().data();
      MOTPicture.vecbRawData.Init (0);
      for(i=0;i<vecPicFileNames[iPictureCnt].arrayPtr->count();i++)
         {
           byteIn=vecPicFileNames[iPictureCnt].arrayPtr->at(i);
//            byteIn=0;
           /* Add one byte = SIZEOF__BYTE bits */
           MOTPicture.vecbRawData.Enlarge (SIZEOF__BYTE);
           MOTPicture.vecbRawData.Enqueue ((uint32_t)byteIn,SIZEOF__BYTE);
         }
       MOTDAB.SetMOTObject (MOTPicture,bytesToBeUsed);
      /* Set file counter to next picture, test for wrap around */
      iPictureCnt++;
      if (iPictureCnt == vecPicFileNames.Size ()) iPictureCnt = 0;
    }
}

void CMOTSlideShowEncoder::AddArray (QByteArray *ba,const QString name,const QString format)
{
    /* Only ContentSubType "JFIF" (JPEG) and ContentSubType "PNG" are allowed
       for SlideShow application (not tested here!) */
    /* Add file name to the list */
    int iOldNumObj = vecPicFileNames.Size ();
    vecPicFileNames.Enlarge (1);
    vecPicFileNames[iOldNumObj].arrayPtr = ba;
    vecPicFileNames[iOldNumObj].name=name.toLatin1();
    vecPicFileNames[iOldNumObj].format=format;
}
