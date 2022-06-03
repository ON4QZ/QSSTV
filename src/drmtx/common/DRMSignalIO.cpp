/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer, Cesco (HB9TLK)
 *
 * Adapted for ham sstv use Ties Bos - PA0MBO
 *
 * Description:
 *	Transmit and receive data
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

#include "DRMSignalIO.h"
#include <iostream>
#include "appglobal.h"

#define pi  4.0*atan(1.0) 

/******************************************************************************\
* Transmitter                                                                  *
\******************************************************************************/
void CTransmitData::ProcessDataInternal(CParameter&)
{
  int i;
  const int iNs2 = iInputBlockSize * 2;
  rNormFactor =1000.0 ; // pa0mbo (was 16000)
  for (i = 0; i < iNs2; i += 2)
    {
      const int iCurIndex = iBlockCnt * iNs2 + i;

      /* Imaginary, real */
      const short sCurOutReal = (short) ((*pvecInputData)[i / 2].real() * rNormFactor);
      const short  sCurOutImag = (short) ((*pvecInputData)[i / 2].imag() * rNormFactor);

      /* Envelope, phase */
      const short sCurOutEnv = (short) (Abs((*pvecInputData)[i / 2]) * (_REAL) 256.0);
      const short sCurOutPhase = 	(short) (Angle((*pvecInputData)[i / 2]) * (_REAL) 5000.0);  /* 2^15 / pi / 2 -> approx. 5000 */

      switch (eOutputFormat)
        {
        case OF_REAL_VAL:
//          vecsDataOut[iCurIndex]  = sCurOutReal;
          vecsDataOut[iCurIndex]  = sCurOutReal;
          vecsDataOut[iCurIndex + 1] = 0;


          //                  (short) 15000.0*sin(pi*1500.0*i/48000.0) ; //  pa0mbo 1500 Hz test signaal
          //                printf("%d %d \n", i/2 , vecsDataOut[iCurIndex]);
        break;

        case OF_IQ_POS:
          /* Send inphase and quadrature (I / Q) signal to stereo sound card
         output. I: left channel, Q: right channel */
          vecsDataOut[iCurIndex] = sCurOutReal;
          vecsDataOut[iCurIndex + 1] = sCurOutImag;
        break;

        case OF_IQ_NEG:
          /* Send inphase and quadrature (I / Q) signal to stereo sound card output. I: right channel, Q: left channel */
          vecsDataOut[iCurIndex] = sCurOutImag;
          vecsDataOut[iCurIndex + 1] = sCurOutReal;
        break;

        case OF_EP:
          /* Send envelope and phase signal to stereo sound card output. Envelope: left channel, Phase: right channel */
          vecsDataOut[iCurIndex] = sCurOutEnv;
          vecsDataOut[iCurIndex + 1] = sCurOutPhase;
        break;
        }
    }
  iBlockCnt++;
  if (iBlockCnt == iNumBlocks)
    {
      iBlockCnt = 0;
      pSound->Write(vecsDataOut); //  printf("DRMSignalIO na pSound-> write\n");
      addToLog(QString("writing vecsDataOut:%1").arg(vecsDataOut.size()),LOGDRMTX);
     }
}

void CTransmitData::InitInternal(CParameter& TransmParam)
{
/*
	float*	pCurFilt;
	int		iNumTapsTransmFilt;
	CReal	rNormCurFreqOffset;
*/
	const int iSymbolBlockSize = TransmParam.CellMappingTable.iSymbolBlockSize;

	/* Init vector for storing a complete DRM frame number of OFDM symbols */
	iBlockCnt = 0;
	TransmParam.Lock(); 
	iNumBlocks = TransmParam.CellMappingTable.iNumSymPerFrame;
	ESpecOcc eSpecOcc = TransmParam.GetSpectrumOccup();
	TransmParam.Unlock(); 
	iBigBlockSize = iSymbolBlockSize * 2 * iNumBlocks;  /* stereo */
        // printf("iBigBlockSize in init Ctransmitdata = %d\n", iBigBlockSize);

	vecsDataOut.Init(iBigBlockSize);

	if (pFileTransmitter != NULL)
	{
		fclose(pFileTransmitter);
	}

  /* Init sound interface */
//		pSound->Init(iBigBlockSize, true);

	/* Init bandpass filter object */
  BPFilter.Init(iSymbolBlockSize, rDefCarOffset, eSpecOcc,CDRMBandpassFilt::FT_TRANSMITTER);
     /*   printf("DRMSignalIO BPFilter init Symbolsize %d rDefCaroffset %g  eSpecooc %d \n",
			iSymbolBlockSize, rDefCarOffset, eSpecOcc); */


	/* All robustness modes and spectrum occupancies should have the same output
	   power. Calculate the normaization factor based on the average power of
	   symbol (the number 3000 was obtained through output tests) */
	rNormFactor = (CReal) 6000.0 / Sqrt(TransmParam.CellMappingTable.rAvPowPerSymbol);  // pa0mbo was 3000.0 nu as in ham
	/* Define block-size for input */
	iInputBlockSize = iSymbolBlockSize;
}

CTransmitData::~CTransmitData()
{
	/* Close file */
	if (pFileTransmitter != NULL)
		fclose(pFileTransmitter);
}

