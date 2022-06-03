/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	Mapping of the symbols on the carriers
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
#include "OFDMCellMapping.h"
#include "../tables/TableCarrier.h"

/* Implementation *************************************************************/
/******************************************************************************\
* OFDM cells mapping														   *
\******************************************************************************/

/* Dummy cells for the MSC ****************************************************/
/* Already normalized */
const _COMPLEX cDummyCells64QAM[2] = {
  _COMPLEX(0.1543033499f,  0.1543033499f),
  _COMPLEX(0.1543033499f, -0.1543033499f)
};

const _COMPLEX cDummyCells16QAM[2] = {
  _COMPLEX(0.3162277660f,  0.3162277660f),
  _COMPLEX(0.3162277660f, -0.3162277660f)
};


void COFDMCellMapping::ProcessDataInternal(CParameter& TransmParam)
{
	
        const CCellMappingTable& Param = TransmParam.CellMappingTable;  /*  pa0mbo */
	int iCar;
	int iMSCCounter;
	int iFACCounter;
	int iDummyCellCounter;
	int iSymbolCounterAbs;



	/* Mapping of the data and pilot cells on the OFDM symbol --------------- */
	/* Set absolute symbol position */
	iSymbolCounterAbs =
		TransmParam.iFrameIDTransm * iNumSymPerFrame + iSymbolCounter;

	/* Init temporary counter */
	iDummyCellCounter = 0;
	iMSCCounter = 0;
	iFACCounter = 0;

	for (iCar = 0; iCar < iNumCarrier; iCar++)
	{

		/* MSC */

		if (_IsMSC(Param.matiMapTab[iSymbolCounterAbs][iCar]))   /* pa0mbo was TransmParam. */
		{
			if (iMSCCounter >= Param.veciNumMSCSym[iSymbolCounterAbs])
			{
				/* Insert dummy cells */
				(*pvecOutputData)[iCar] = pcDummyCells[iDummyCellCounter];

				iDummyCellCounter++;

			}
			else
				(*pvecOutputData)[iCar] = (*pvecInputData)[iMSCCounter];
				
			iMSCCounter++;
		}

		/* FAC */
		if (_IsFAC(Param.matiMapTab[iSymbolCounterAbs][iCar]))
		{
			(*pvecOutputData)[iCar] = (*pvecInputData2)[iFACCounter];
				
			iFACCounter++;
		}

		/* DC carrier */
		if (_IsDC(Param.matiMapTab[iSymbolCounterAbs][iCar]))
			(*pvecOutputData)[iCar] = _COMPLEX((_REAL) 0.0, (_REAL) 0.0);

		/* Pilots */
		if (_IsPilot(Param.matiMapTab[iSymbolCounterAbs][iCar]))
			(*pvecOutputData)[iCar] = 
				Param.matcPilotCells[iSymbolCounterAbs][iCar];
	}

	/* Increase symbol-counter and wrap if needed */
	iSymbolCounter++;
	if (iSymbolCounter == iNumSymPerFrame)
	{
		iSymbolCounter = 0;

		/* Increase frame-counter (ID) (Used also in FAC.cpp) */
		TransmParam.iFrameIDTransm++;
		if (TransmParam.iFrameIDTransm == NUM_FRAMES_IN_SUPERFRAME)
			TransmParam.iFrameIDTransm = 0;
	}

	/* Set absolute symbol position (for updated relative positions) */
	iSymbolCounterAbs =
		TransmParam.iFrameIDTransm * iNumSymPerFrame + iSymbolCounter;

	/* Set input block-sizes for next symbol */
	iInputBlockSize = Param.veciNumMSCSym[iSymbolCounterAbs];
	iInputBlockSize2 = Param.veciNumFACSym[iSymbolCounterAbs];
	/* printf("OFDMCellmapping proces updated iInpblk %d iInp2 %d iSymbcntrabs %d \n",
			iInputBlockSize, iInputBlockSize2, iSymbolCounterAbs); */
}

void COFDMCellMapping::InitInternal(CParameter& TransmParam)
{
        const CCellMappingTable& Param = TransmParam.CellMappingTable;  /*  pa0mbo */
	iNumSymPerFrame = Param.iNumSymPerFrame;
	iNumCarrier = Param.iNumCarrier;

	/* Init symbol-counter */
	iSymbolCounter = 0;

	/* Init frame ID */
	TransmParam.iFrameIDTransm = 0;

	/* Choose right dummy cells for MSC QAM scheme */
	switch (TransmParam.eMSCCodingScheme)
	{
    case CS_3_HMSYM: // not use so default to case CS_2_SM
    case CS_3_HMMIX:
    case CS_1_SM:
		case CS_2_SM:                // pa0mbo was CParameter::CS_2_SM
		pcDummyCells = (_COMPLEX*) &cDummyCells16QAM[0];
		break;

	case CS_3_SM:
		pcDummyCells = (_COMPLEX*) &cDummyCells64QAM[0];  // pa0mbo was CParameter::CS_#_SM
		break;
	}

	/* Define block-sizes for input and output of the module ---------------- */
	iInputBlockSize = Param.veciNumMSCSym[0]; /* MSC */
	iInputBlockSize2 = Param.veciNumFACSym[0]; /* FAC */
	iOutputBlockSize = Param.iNumCarrier; /* Output */
	// printf("OFDM CELLMAP : Inputblksiz %d Inpublksiz2 %d OutputBlkSiz %d\n",
			// iInputBlockSize, iInputBlockSize2, iOutputBlockSize);
}
