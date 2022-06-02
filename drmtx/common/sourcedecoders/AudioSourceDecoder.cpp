/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	Audio source encoder/decoder
 *
 * adapted to ham sstv use PA0MBO - Ties Bos
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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 1111
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/

#include "AudioSourceDecoder.h"
#include <iostream>
#include <time.h>
#define AUD_DEC_TRANSFROM_LENGTH 960

/******************************************************************************\
* Encoder                                                                      *
\******************************************************************************/

void CAudioSourceEncoderImplementation::ProcessDataInternal( CVectorEx < _BINARY >  *pvecOutputData, int &iOutputBlockSize)
{
	int i;

	/* Reset data to zero. This is important since usually not all data is used
	   and this data has to be set to zero as defined in the DRM standard */
  for (i = 0; i < iOutputBlockSize; i++) (*pvecOutputData)[i] = 0;
	/* Data service and text message application ---------------------------- */
	if (bIsDataService == true)
	{
    // TODO: make a separate modul for data encoding
		/* Write data packets in stream */
		CVector < _BINARY > vecbiData;
		const int iNumPack = iOutputBlockSize / iTotPacketSize;

	//	printf("In process data AudioSourceEnc iOutputBlcksi %d iTotPacketSize = %d \n", iOutputBlockSize, iTotPacketSize);
		int iPos = 0;

		for (int j = 0; j < iNumPack; j++)
		{
			/* Get new packet */
			DataEncoder.GeneratePacket(vecbiData);

			/* Put it on stream */
			for (i = 0; i < iTotPacketSize; i++)
			{
				(*pvecOutputData)[iPos] = vecbiData[i];
				iPos++;
			}
		}
	}
}



void CAudioSourceEncoderImplementation::InitInternalTx(CParameter & TransmParam, int &iOutputBlockSize)
{
	int iCurStreamID;
	int iCurSelServ = 0;		// TEST
	TransmParam.Lock(); 

  // Calculate number of input samples in mono. Audio block are always 400 ms long
//  const int iNumInSamplesMono = (int) ((_REAL) SOUNDCRD_SAMPLE_RATE * (_REAL) 0.4 /* 400 ms */ );

	/* Set the total available number of bits, byte aligned */
  iTotNumBitsForUsage = (TransmParam.iNumDecodedBitsMSC / SIZEOF__BYTE) * SIZEOF__BYTE;

	/* Total number of bytes which can be used for data and audio */
	const int iTotNumBytesForUsage = iTotNumBitsForUsage / SIZEOF__BYTE;

        /* printf("in audiosourceEncoder init smplmono = %d  iTotNumBitsForUsage = %d bytefor usage = %d \n", 
			iNumInSamplesMono,  iTotNumBitsForUsage, iTotNumBytesForUsage ); */

	if (TransmParam.iNumDataService == 1)
	{
		/* Data service ----------------------------------------------------- */
		bIsDataService = true;
		// printf("Data servic is true \n");
		iTotPacketSize = DataEncoder.Init(TransmParam);
                // printf("na DataEncoder Init in AudioSourceEncoder init iTotPacketSize = %d\n", iTotPacketSize);

		/* Get stream ID for data service */
		iCurStreamID = TransmParam.Service[iCurSelServ].DataParam.iStreamID;
                /* printf(" in AudioSourceEncoder dataservice is true iTotPacketSize = %d, custreamId = %d \n",
				iTotPacketSize, iCurStreamID ); */

	}
	else
	{
          printf("Not implemented \n");
          exit(1);
	}

	/* Adjust part B length for SDC stream. Notice, that the
	   "TransmParam.iNumDecodedBitsMSC" parameter depends on these settings.
	   Thus, length part A and B have to be set before, preferably in the
	   DRMTransmitter initialization */
  if ((TransmParam.Stream[iCurStreamID].iLenPartA == 0) || (iTotNumBytesForUsage < TransmParam.Stream[iCurStreamID].iLenPartA))
	{
    /* Equal error protection was chosen or protection part A was chosen too high, set to equal error protection! */
		TransmParam.Stream[iCurStreamID].iLenPartA = 0;
		TransmParam.Stream[iCurStreamID].iLenPartB = iTotNumBytesForUsage;
	}
	else
    TransmParam.Stream[iCurStreamID].iLenPartB = iTotNumBytesForUsage - TransmParam.Stream[iCurStreamID].iLenPartA;

	/* Define input and output block size */
	iOutputBlockSize = TransmParam.iNumDecodedBitsMSC;
	TransmParam.Unlock(); 
}


CAudioSourceEncoderImplementation::~CAudioSourceEncoderImplementation()
{

}



