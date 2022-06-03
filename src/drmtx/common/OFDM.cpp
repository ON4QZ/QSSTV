/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Adapted for ham sstv use  Ties Bos - PA0MBO
 *
 * Description:
 *	OFDM modulation;
 *	OFDM demodulation, SNR estimation, PSD estimation
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

#include "OFDM.h"


/* Implementation *************************************************************/
/******************************************************************************\
* OFDM-modulation                                                              *
\******************************************************************************/
void COFDMModulation::ProcessDataInternal(CParameter&)
{
	int	i;
         // printf("In COFDMModul iShiftKmin %d iEndIndex %d , iDFTSize %d iGuardSize %d\n",
//			iShiftedKmin, iEndIndex, iDFTSize, iGuardSize); 
	/* Copy input vector in matlib vector and place bins at the correct
	   position */
	for (i = iShiftedKmin; i < iEndIndex; i++) 
	{
//		veccFFTInput[150] = _COMPLEX((_REAL) 1.0, (_REAL) -1.0);   // test pa0mbo
		veccFFTInput[i] = (*pvecInputData)[i - iShiftedKmin]; 
//		veccFFTInput[iDFTSize -1  - i] = Conj((*pvecInputData)[i - iShiftedKmin]); 
		// printf(" veccFFTInput %d  %g  %g\n", i,(veccFFTInput[i]).real(), (veccFFTInput[i]).imag());
	}

	/* Calculate inverse fast Fourier transformation */
	veccFFTOutput = Ifft(veccFFTInput, FftPlan);

	/* Copy complex FFT output in output buffer and scale */
	for (i = 0; i < iDFTSize; i++)
		(*pvecOutputData)[i + iGuardSize] = veccFFTOutput[i] * (CReal) iDFTSize;

	/* Copy data from the end to the guard-interval (Add guard-interval) */
	for (i = 0; i < iGuardSize; i++)
		(*pvecOutputData)[i] = (*pvecOutputData)[iDFTSize + i];

	/* tbv test printout pa0mbo  
        printf("===========\n");
	 for (i=0; i < iDFTSize + iGuardSize ; i++)
		 printf("%d  %g \n",i,  ((*pvecOutputData)[i]).real()); 
	 printf("=============\n");  */ 

	/* Shift spectrum to desired IF ----------------------------------------- */
	/* Only apply shifting if phase is not zero */
	if (cExpStep != _COMPLEX((_REAL) 1.0, (_REAL) 0.0))
	{
		for (i = 0; i < iOutputBlockSize; i++)
		{
			(*pvecOutputData)[i] = (*pvecOutputData)[i] * Conj(cCurExp);   // pa0mbo shift off
			
			/* Rotate exp-pointer on step further by complex multiplication
			   with precalculated rotation vector cExpStep. This saves us from
			   calling sin() and cos() functions all the time (iterative
			   calculation of these functions) */
			cCurExp *= cExpStep;
		}
	}
}

void COFDMModulation::InitInternal(CParameter& TransmParam)
{
	TransmParam.Lock(); 
	/* Get global parameters */
	iDFTSize = TransmParam.CellMappingTable.iFFTSizeN;
	iGuardSize = TransmParam.CellMappingTable.iGuardSize;
	iShiftedKmin = TransmParam.CellMappingTable.iShiftedKmin;

	/* Last index */
	iEndIndex = TransmParam.CellMappingTable.iShiftedKmax + 1;

	/* Normalized offset correction factor for IF shift. Subtract the
	   default IF frequency ("VIRTUAL_INTERMED_FREQ") */
	const _REAL rNormCurFreqOffset = (_REAL) -2.0 * crPi *
//		((_REAL ) -6751.0)/ SOUNDCRD_SAMPLE_RATE;  
		(rDefCarOffset - VIRTUAL_INTERMED_FREQ) / SOUNDCRD_SAMPLE_RATE;  
         // printf("COFDMMod rDefCarOffset %g Virt IF %g rNormCurFreqoffset %g \n", rDefCarOffset, (_REAL) VIRTUAL_INTERMED_FREQ,  rNormCurFreqOffset);

	/* Rotation vector for exp() calculation */
	cExpStep = _COMPLEX(cos(rNormCurFreqOffset), sin(rNormCurFreqOffset));

         //  printf("COFDMMod cExpStep real %g imag  %g \n", cos(rNormCurFreqOffset), sin(rNormCurFreqOffset));
	/* Start with phase null (can be arbitrarily chosen) */
	cCurExp = (_REAL) 1.0;

	/* Init plans for FFT (faster processing of Fft and Ifft commands) */
	FftPlan.Init(iDFTSize);
   //     printf("In FftPlan.Init  iDFTSize is %d \n", iDFTSize);

	/* Allocate memory for intermediate result of fft. Zero out input vector
	   (because only a few bins are used, the rest has to be zero) */
	veccFFTInput.Init(iDFTSize, (CReal) 0.0);
	veccFFTOutput.Init(iDFTSize);

	/* Define block-sizes for input and output */
	iInputBlockSize = TransmParam.CellMappingTable.iNumCarrier;
	iOutputBlockSize = TransmParam.CellMappingTable.iSymbolBlockSize;

	// printf("In COFDMMOd init DFTsize %d Guardsize %d ShiftedKmin %d \n", iDFTSize, iGuardSize, iShiftedKmin);
	 // printf("In COFDMMOd init Inputblcksize %d Outputblocksize %d \n", iInputBlockSize, iOutputBlockSize);


	TransmParam.Unlock(); 
}


