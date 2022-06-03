/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 * 
 * Adapted for ham sstv use Ties Bos - PA0MBO
 *
 *
 * Description:
 *	Multi-level-channel (de)coder (MLC)
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

#include "MLC.h"

#define NUM_FAC_CELLS 45


/* Implementation *************************************************************/
/******************************************************************************\
* MLC-encoder                                                                  *
\******************************************************************************/
void CMLCEncoder::ProcessDataInternal(CParameter&)
{
	int	i, j;
	int iElementCounter;

	/* Energy dispersal ----------------------------------------------------- */
	/* VSPP is treated as a separate part for energy dispersal */
	EnergyDisp.ProcessData(pvecInputData);


	/* Partitioning of input-stream ----------------------------------------- */
	iElementCounter = 0;

	if (iL[2] == 0)
	{
		/* Standard departitioning */
		/* Protection level A */
		for (j = 0; j < iLevels; j++)
		{
			/* Bits */
			for (i = 0; i < iM[j][0]; i++)
			{
				vecEncInBuffer[j][i] =
					// BitToSoft((*pvecInputData)[iElementCounter]);
					(*pvecInputData)[iElementCounter];

				iElementCounter++;
			}
		}

		/* Protection level B */
		for (j = 0; j < iLevels; j++)
		{
			/* Bits */
			for (i = 0; i < iM[j][1]; i++)
			{
				vecEncInBuffer[j][iM[j][0] + i] =
					// BitToSoft((*pvecInputData)[iElementCounter]);
					(*pvecInputData)[iElementCounter];

				iElementCounter++;
			}
		}
	}
	else
	{
		/* Special partitioning with hierarchical modulation. First set
		   hierarchical bits at the beginning, then append the rest */
		/* Hierarchical frame (always "iM[0][1]"). "iM[0][0]" is always "0" in
		   this case */
		for (i = 0; i < iM[0][1]; i++)
		{
			vecEncInBuffer[0][i] =
				// BitToSoft((*pvecInputData)[iElementCounter]);
				(*pvecInputData)[iElementCounter];

			iElementCounter++;
		}


		/* Protection level A (higher protected part) */
		for (j = 1; j < iLevels; j++)
		{
			/* Bits */
			for (i = 0; i < iM[j][0]; i++)
			{
				vecEncInBuffer[j][i] =
				//	BitToSoft((*pvecInputData)[iElementCounter]);
					(*pvecInputData)[iElementCounter];

				iElementCounter++;
			}
		}

		/* Protection level B  (lower protected part) */
		for (j = 1; j < iLevels; j++)
		{
			/* Bits */
			for (i = 0; i < iM[j][1]; i++)
			{
				vecEncInBuffer[j][iM[j][0] + i] =
					// BitToSoft((*pvecInputData)[iElementCounter]);
					(*pvecInputData)[iElementCounter];

				iElementCounter++;
			}
		}
	}


	/* Convolutional encoder ------------------------------------------------ */
	for (j = 0; j < iLevels; j++)
		ConvEncoder[j].Encode(vecEncInBuffer[j], vecEncOutBuffer[j]);


	/* Bit interleaver ------------------------------------------------------ */
	for (j = 0; j < iLevels; j++)
		if (piInterlSequ[j] != -1)
			BitInterleaver[piInterlSequ[j]].Interleave(vecEncOutBuffer[j]);


	/* QAM mapping ---------------------------------------------------------- */
	QAMMapping.Map(vecEncOutBuffer[0],
				   vecEncOutBuffer[1],
				   vecEncOutBuffer[2],
				   vecEncOutBuffer[3],
				   vecEncOutBuffer[4],
				   vecEncOutBuffer[5], pvecOutputData);
}

void CMLCEncoder::InitInternal(CParameter& TransmParam)
{
	int i;
	int	iNumInBits;

	TransmParam.Lock(); 
	CalculateParam(TransmParam, eChannelType);
	TransmParam.Unlock(); 
	
	iNumInBits = iL[0] + iL[1] + iL[2];


	/* Init modules --------------------------------------------------------- */
	/* Energy dispersal */
	EnergyDisp.Init(iNumInBits, iL[2]);

	/* Encoder */
	for (i = 0; i < iLevels; i++)
		ConvEncoder[i].Init(eCodingScheme, eChannelType, iN[0], iN[1],
			iM[i][0], iM[i][1], iCodeRate[i][0], iCodeRate[i][1], i);

	/* Bit interleaver */
	/* First init all possible interleaver (According table "TableMLC.h" ->
	   "Interleaver sequence") */
	if (eCodingScheme == CS_3_HMMIX)
	{
		BitInterleaver[0].Init(iN[0], iN[1], 13);
		BitInterleaver[1].Init(iN[0], iN[1], 21);
	}
	else
	{
		BitInterleaver[0].Init(2 * iN[0], 2 * iN[1], 13);
		BitInterleaver[1].Init(2 * iN[0], 2 * iN[1], 21);
	}

	/* QAM-mapping */
	QAMMapping.Init(iN_mux, eCodingScheme);


	/* Allocate memory for internal bit-buffers ----------------------------- */
	for (i = 0; i < iLevels; i++)
	{
		/* Buffers for each encoder on all different levels */
		/* Add bits from higher protected and lower protected part */
		vecEncInBuffer[i].Init(iM[i][0] + iM[i][1]);
	
		/* Encoder output buffers for all levels. Must have the same length */
		vecEncOutBuffer[i].Init(iNumEncBits);
	}

	/* Define block-size for input and output */
	iInputBlockSize = iNumInBits;
	iOutputBlockSize = iN_mux;
	// printf("In init MSCMLCEnc inputblk = %d outpublk = %d\n",
			 //  iInputBlockSize, iOutputBlockSize);

}




/******************************************************************************\
* MLC base class                                                               *
\******************************************************************************/
void CMLC::CalculateParam(CParameter& Parameter, int iNewChannelType)
{
	int i;
//	int iMSCDataLenPartA;

	switch (iNewChannelType)
	{
	/* FAC ********************************************************************/
	case CT_FAC:
		eCodingScheme = CS_1_SM;
		iN_mux = NUM_FAC_CELLS;

		iNumEncBits = NUM_FAC_CELLS * 2;

		iLevels = 1;

		/* Code rates for prot.-Level A and B for each level */
		/* Protection Level A */
		iCodeRate[0][0] = 0;

		/* Protection Level B */
		iCodeRate[0][1] = iCodRateCombFDC4SM;

		/* Define interleaver sequence for all levels */
		piInterlSequ = iInterlSequ4SM;


		/* iN: Number of OFDM-cells of each protection level ---------------- */
		iN[0] = 0;
		iN[1] = iN_mux;


		/* iM: Number of bits each level ------------------------------------ */
		iM[0][0] = 0;
		iM[0][1] = NUM_FAC_BITS_PER_BLOCK;


		/* iL: Number of bits each protection level ------------------------- */
		/* Higher protected part */
		iL[0] = 0;

		/* Lower protected part */
		iL[1] = iM[0][1];

		/* Very strong protected part (VSPP) */
		iL[2] = 0;
		break;



	/* MSC ********************************************************************/
	case CT_MSC:
		eCodingScheme = Parameter.eMSCCodingScheme;
		iN_mux = Parameter.CellMappingTable.iNumUsefMSCCellsPerFrame;
                // printf("iN_mux %d \n", iN_mux);

		/* Data length for part A is the sum of all lengths of the streams */
		
//		iMSCDataLenPartA = 0; // for hamversion

/*		iMSCDataLenPartA = Parameter.Stream[0].iLenPartA +
						   Parameter.Stream[1].iLenPartA +
						   Parameter.Stream[2].iLenPartA +
						   Parameter.Stream[3].iLenPartA;  */

		switch (eCodingScheme)
		{
		case CS_1_SM:
			iLevels = 1;

			/* Code rates for prot.-Level A and B for each level */
				/* Protection Level A */
			iCodeRate[0][0] = 0;

			/* Protection Level B */
			iCodeRate[0][1] = iCodRateCombMSC4SM;
			
			/* Define interleaver sequence for all levels */
			piInterlSequ = iInterlSequ4SM;
			iNumEncBits = iN_mux * 2;

			/* iN: Number of OFDM-cells of each protection level ---------------- */
			iN[0] = 0;
			iN[1] = iN_mux;

			/* iM: Number of bits each level ------------------------------------ */
			iM[0][0] = 0;

			/* M_p,2 = RX_p * floor((2 * N_2 - 12) / RY_p) */
			iM[0][1] = iPuncturingPatterns[iCodRateCombMSC4SM][0] *
				(int) ((_REAL) (2 * iN_mux - 12) /
				iPuncturingPatterns[iCodRateCombMSC4SM][1]);

			/* iL: Number of bits each protection level ------------------------- */
			/* Higher protected part */
			iL[0] = 0;

			/* Lower protected part */
			iL[1] = iM[0][1];

			/* Very strong protected part (VSPP) */
			iL[2] = 0;
                      /*   printf("in CalcuPar CS_1_SM iN[0]= %d iN[1]= %d iM[0][0] = %d iM[0][1] = %d\n",
					 iN[0], iN[1], iM[0][0], iM[0][1] );
			 printf("in CalcuPar iL[o]= %d iL[1]= %d iL[2] = %d\n",
					 iL[0], iL[1], iL[1]);  */
			break;

		case CS_2_SM:
			iLevels = 2;

			/* Code rates for prot.-Level A and B for each level */
			for (i = 0; i < 2; i++)
			{
				/* Protection Level A */
				iCodeRate[i][0] = 0 ;  // hamversion

               /*				iCodeRate[i][0] =
					iCodRateCombMSC16SM[Parameter.MSCPrLe.iPartA][i]; */

				/* Protection Level B */
				iCodeRate[i][1] =
					iCodRateCombMSC16SM[Parameter.MSCPrLe.iPartB][i];
			}

			/* Define interleaver sequence for all levels */
			piInterlSequ = iInterlSequ16SM;

			iNumEncBits = iN_mux * 2;


			/* iN: Number of OFDM-cells of each protection level ------------ */
			/* N_1 = ceil(8 * X / (2 * RY_Icm * sum(R_p)) * RY_Icm */
			iN[0] = 0;  // hamversion

/*			iN[0] = (int) ceil(8 * (_REAL) iMSCDataLenPartA / (2 *
				(_REAL) iCodRateCombMSC16SM[Parameter.MSCPrLe.iPartA][2] *
				(
				(_REAL) iPuncturingPatterns[iCodRateCombMSC16SM[
					Parameter.MSCPrLe.iPartA][0]][0] /
					iPuncturingPatterns[iCodRateCombMSC16SM[
					Parameter.MSCPrLe.iPartA][0]][1] +
				(_REAL) iPuncturingPatterns[iCodRateCombMSC16SM[
					Parameter.MSCPrLe.iPartA][1]][0] /
					iPuncturingPatterns[iCodRateCombMSC16SM[
					Parameter.MSCPrLe.iPartA][1]][1]))) *
				iCodRateCombMSC16SM[Parameter.MSCPrLe.iPartA][2];
*/
			/* Check if result can be possible, if not -> correct. This can
			   happen, if a wrong number is in "Param.Stream[x].iLenPartA" */
			if (iN[0] > iN_mux)
				iN[0] = 0;

			iN[1] = iN_mux - iN[0];


			/* iM: Number of bits each level -------------------------------- */
			for (i = 0; i < 2; i++)
			{
				/* M_p,1 = 2 * N_1 * R_p */
				iM[i][0] = 0;
	/*			iM[i][0] = (int) (2 * iN[0] *
					(_REAL) iPuncturingPatterns[iCodRateCombMSC16SM[
					Parameter.MSCPrLe.iPartA][i]][0] /
					iPuncturingPatterns[iCodRateCombMSC16SM[
					Parameter.MSCPrLe.iPartA][i]][1]);   */

				/* M_p,2 = RX_p * floor((2 * N_2 - 12) / RY_p) */
				iM[i][1] = 
					iPuncturingPatterns[iCodRateCombMSC16SM[
					Parameter.MSCPrLe.iPartB][i]][0] *
					(int) ((_REAL) (2 * iN[1] - 12) /
					iPuncturingPatterns[iCodRateCombMSC16SM[
					Parameter.MSCPrLe.iPartB][i]][1]);
				// printf("In calcparam iM[%d][0] = %d  iM[%d][1] = %d \n", i, iM[i][0], i, iM[i][1]);
			}


			/* iL: Number of bits each protection level --------------------- */
			/* Higher protected part */
			iL[0] = iM[0][0] + iM[1][0];

			/* Lower protected part */
			iL[1] = iM[0][1] + iM[1][1];

			/* Very strong protected part (VSPP) */
			iL[2] = 0;
                         /* printf("in CalcuPar CS_2_SM iN[0]= %d iN[1]= %d iM[0][0] = %d iM[0][1] = %d\n",
					 iN[0], iN[1], iM[0][0], iM[0][1] );
			 printf("in CalcuPar iL[o]= %d iL[1]= %d iL[2] = %d\n",
					 iL[0], iL[1], iL[1]);  */
			break;

		case CS_3_SM:
			iLevels = 3;

			/* Code rates for prot.-Level A and B for each level */
			for (i = 0; i < 3; i++)
			{
				/* Protection Level A */
				iCodeRate[i][0] = 0;
/*				iCodeRate[i][0] =
					iCodRateCombMSC64SM[Parameter.MSCPrLe.iPartA][i]; */

				/* Protection Level B */
				iCodeRate[i][1] =
					iCodRateCombMSC64SM[Parameter.MSCPrLe.iPartB][i];
			}

			/* Define interleaver sequence for all levels */
			piInterlSequ = iInterlSequ64SM;

			iNumEncBits = iN_mux * 2;


			/* iN: Number of OFDM-cells of each protection level ------------ */
			/* N_1 = ceil(8 * X / (2 * RY_Icm * sum(R_p)) * RY_Icm */
			iN[0] = 0;

			/* Check if result can be possible, if not -> correct. This can
			   happen, if a wrong number is in "Param.Stream[x].iLenPartA" */
			if (iN[0] > iN_mux)
				iN[0] = 0;

			iN[1] = iN_mux - iN[0];


			/* iM: Number of bits each level -------------------------------- */
			for (i = 0; i < 3; i++)
			{
				/* M_p,1 = 2 * N_1 * R_p */
				iM[i][0] = 0;
/*				iM[i][0] = (int) (2 * iN[0] *
					(_REAL) iPuncturingPatterns[iCodRateCombMSC64SM[
					Parameter.MSCPrLe.iPartA][i]][0] /
					iPuncturingPatterns[iCodRateCombMSC64SM[
					Parameter.MSCPrLe.iPartA][i]][1]); */

				/* M_p,2 = RX_p * floor((2 * N_2 - 12) / RY_p) */
				iM[i][1] = 
					iPuncturingPatterns[iCodRateCombMSC64SM[
					Parameter.MSCPrLe.iPartB][i]][0] *
					(int) ((_REAL) (2 * iN[1] - 12) /
					iPuncturingPatterns[iCodRateCombMSC64SM[
					Parameter.MSCPrLe.iPartB][i]][1]); 
			}


			/* iL: Number of bits each protection level --------------------- */
			/* Higher protected part */
			iL[0] = iM[0][0] + iM[1][0] + iM[2][0];

			/* Lower protected part */
			iL[1] = iM[0][1] + iM[1][1] + iM[2][1];

			/* Very strong protected part (VSPP) */
			iL[2] = 0;
                         /* printf("in CalcuPar CS_3_SM iN[0]= %d iN[1]= %d iM[0][0] = %d iM[0][1] = %d\n",
					 iN[0], iN[1], iM[0][0], iM[0][1] );
			 printf("in CalcuPar iL[o]= %d iL[1]= %d iL[2] = %d\n",
					 iL[0], iL[1], iL[2]); */
			break;


		default:
			break;
		}

		/* Set number of output bits for next module */
		Parameter.SetNumDecodedBitsMSC(iL[0] + iL[1] + iL[2]);

		/* Set total number of bits for hiearchical frame (needed for MSC
		   demultiplexer module) */
		Parameter.SetNumBitsHieraFrTot(iL[2]);
              //  Parameter.DataParam.iPacketLen = 1234;
		break;
	}
}
