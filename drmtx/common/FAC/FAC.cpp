/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Adapted for ham sstv use Ties Bos PA0MBO
 *
 * Description:
 *	FAC
 *
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

#include "FAC.h"


/* Implementation *************************************************************/
/******************************************************************************\
* CFACTransmit																   *
\******************************************************************************/
void CFACTransmit::FACParam(CVector<_BINARY>* pbiFACData, CParameter& Parameter)
{


	/* Reset enqueue function */
	(*pbiFACData).ResetBitAccess();

	/* Put FAC parameters on stream */
	/* Channel parameters --------------------------------------------------- */

	/* Identity m 2 bits  */
	/* Manage index of FAC block in super-frame */
	switch (Parameter.iFrameIDTransm)
	{
	case 0:
		/* Assuming AFS is valid (AFS not used here), if AFS is not valid, the
		   parameter must be 3 (11) */
		(*pbiFACData).Enqueue(3 /* 11 */, 2);
		break;

	case 1:
		(*pbiFACData).Enqueue(1 /* 01 */, 2);
		break;

	case 2:
		(*pbiFACData).Enqueue(2 /* 10 */, 2);
		break;
	}

	/* Spectrum occupancy for ham use only 1 bit  */
	switch (Parameter.GetSpectrumOccup())
	{
	case SO_0:
		(*pbiFACData).Enqueue(0 /* 0 */, 1);
		break;

	case SO_1:
		(*pbiFACData).Enqueue(1 /* 1 */, 1);
		break;
    default:
      (*pbiFACData).Enqueue(0 /* 0 */, 1);
      break;


	}

	/* Interleaver depth flag ham situation 1 bit */
	switch (Parameter.eSymbolInterlMode)
	{
	case CParameter::SI_LONG:
		(*pbiFACData).Enqueue(0 /* 0 */, 1);
		break;

	case CParameter::SI_SHORT:
		(*pbiFACData).Enqueue(1 /* 1 */, 1);
		break;
	}

	/* MSC mode */
	switch (Parameter.eMSCCodingScheme)
	{
	case CS_3_SM:
		(*pbiFACData).Enqueue(0 /* 0 */, 1);
		break;

	case CS_1_SM:
		(*pbiFACData).Enqueue(1 /* 1 */, 1);
		break;

	case CS_2_SM:
		(*pbiFACData).Enqueue(1 /* 1 */, 1);
		break;
	default:
		break;
	}

	/* Prot Level  1 bit */
	switch (Parameter.MSCPrLe.iPartB)
	{
	case 0:
		(*pbiFACData).Enqueue(0 /* 0 */, 1);
		break;

	case 1:
		(*pbiFACData).Enqueue(1 /* 1 */, 1);
		break;
	}

	/* Audio/Data flag */
	switch (Parameter.Service[0].eAudDataFlag)
	{
	case CService::SF_AUDIO:
	
		(*pbiFACData).Enqueue(0 /* 0 */, 1);
		(*pbiFACData).Enqueue(3 /* 11 */, 2);  // pa0mbo dummy bits SSTV
		(*pbiFACData).Enqueue(0 /* 0 */, 1);  // pa0mbo  no text
		break;

	case CService::SF_DATA:
		(*pbiFACData).Enqueue(1 /* 1 */, 1);

		/* Packet Id */
		(*pbiFACData).Enqueue( 
			(uint32_t) Parameter.Service[0].DataParam.iPacketID, 2);
		/* Extended MSC mode 1 bit */
		if (Parameter.eMSCCodingScheme == CS_1_SM)
			(*pbiFACData).Enqueue(1 /* 1 */, 1);  // QAM 4
		else
			(*pbiFACData).Enqueue(0 /* 0 */, 1);  // others
		break;

	}

	{
		int	iLenLabel;
		int iframet = Parameter.iFrameIDTransm;
		const int iLenLabelTmp = Parameter.Service[0].strLabel.length();
		if (iLenLabelTmp > 9)
			iLenLabel = 9;
		else
			iLenLabel = iLenLabelTmp;
		/* Set all characters of label string */
		for (int i = 3*iframet; i < 3*iframet+3; i++)
		{
			char cNewChar;
			if (i >= iLenLabel)
				cNewChar = 0;
			else
				cNewChar = Parameter.Service[0].strLabel[i];
			cNewChar &= 127;
			/* Set character */
			(*pbiFACData).Enqueue((uint32_t) cNewChar, 7);
		}
	}


	/* CRC ------------------------------------------------------------------ */
	/* Calculate the CRC and put at the end of the stream */
	CRCObject.Reset(8);

	(*pbiFACData).ResetBitAccess();

	for (int i = 0; i < NUM_FAC_BITS_PER_BLOCK / SIZEOF__BYTE - 1; i++)
		CRCObject.AddByte((_BYTE) (*pbiFACData).Separate(SIZEOF__BYTE));

	/* Now, pointer in "enqueue"-function is back at the same place,
	   add CRC */
	(*pbiFACData).Enqueue(CRCObject.GetCRC(), 8);
}

void CFACTransmit::Init(CParameter& Parameter)
{
	set<int>	actServ;

	/* Get active services */
	Parameter.GetActiveServices(actServ);
	const size_t iTotNumServices = actServ.size();

	/* Check how many audio and data services present */
	vector<int>	veciAudioServ;
	vector<int>	veciDataServ;
	size_t		iNumAudio = 0;
	size_t		iNumData = 0;

	for (set<int>::iterator i = actServ.begin(); i!=actServ.end(); i++)
	{
		if (Parameter.Service[*i].eAudDataFlag == CService::SF_AUDIO)
		{
			veciAudioServ.push_back(*i);
			iNumAudio++;
		}
		else
		{
			veciDataServ.push_back(*i);
			iNumData++;
		}
	}

	/* Now check special cases which are defined in 6.3.6-------------------- */
	/* If we have only data or only audio services. When all services are of
	   the same type (e.g. all audio or all data) then the services shall be
	   signalled sequentially */
	if ((iNumAudio == iTotNumServices) || (iNumData == iTotNumServices))
	{
		/* Init repetition vector */
		FACNumRep = iTotNumServices;
		FACRepetition.resize(0);

		for (set<int>::iterator i = actServ.begin(); i!=actServ.end(); i++)
			FACRepetition.push_back(*i);
	}
	else
	{
		/* Special cases according to Table 60 (Service parameter repetition
		   patterns for mixtures of audio and data services) */
		if (iNumAudio == 1)
		{
			if (iNumData == 1)
			{
				/* Init repetion vector */
				FACNumRep = 5;
				FACRepetition.resize(FACNumRep);

				/* A1A1A1A1D1 */
				FACRepetition[0] = veciAudioServ[0];
				FACRepetition[1] = veciAudioServ[0];
				FACRepetition[2] = veciAudioServ[0];
				FACRepetition[3] = veciAudioServ[0];
				FACRepetition[4] = veciDataServ[0];
			}
			else if (iNumData == 2)
			{
				/* Init repetion vector */
				FACNumRep = 10;
				FACRepetition.resize(FACNumRep);

				/* A1A1A1A1D1A1A1A1A1D2 */
				FACRepetition[0] = veciAudioServ[0];
				FACRepetition[1] = veciAudioServ[0];
				FACRepetition[2] = veciAudioServ[0];
				FACRepetition[3] = veciAudioServ[0];
				FACRepetition[4] = veciDataServ[0];
				FACRepetition[5] = veciAudioServ[0];
				FACRepetition[6] = veciAudioServ[0];
				FACRepetition[7] = veciAudioServ[0];
				FACRepetition[8] = veciAudioServ[0];
				FACRepetition[9] = veciDataServ[1];
			}
			else /* iNumData == 3 */
			{
				/* Init repetion vector */
				FACNumRep = 15;
				FACRepetition.resize(FACNumRep);

				/* A1A1A1A1D1A1A1A1A1D2A1A1A1A1D3 */
				FACRepetition[0] = veciAudioServ[0];
				FACRepetition[1] = veciAudioServ[0];
				FACRepetition[2] = veciAudioServ[0];
				FACRepetition[3] = veciAudioServ[0];
				FACRepetition[4] = veciDataServ[0];
				FACRepetition[5] = veciAudioServ[0];
				FACRepetition[6] = veciAudioServ[0];
				FACRepetition[7] = veciAudioServ[0];
				FACRepetition[8] = veciAudioServ[0];
				FACRepetition[9] = veciDataServ[1];
				FACRepetition[10] = veciAudioServ[0];
				FACRepetition[11] = veciAudioServ[0];
				FACRepetition[12] = veciAudioServ[0];
				FACRepetition[13] = veciAudioServ[0];
				FACRepetition[14] = veciDataServ[2];
			}
		}
		else if (iNumAudio == 2)
		{
			if (iNumData == 1)
			{
				/* Init repetion vector */
				FACNumRep = 5;
				FACRepetition.resize(FACNumRep);

				/* A1A2A1A2D1 */
				FACRepetition[0] = veciAudioServ[0];
				FACRepetition[1] = veciAudioServ[1];
				FACRepetition[2] = veciAudioServ[0];
				FACRepetition[3] = veciAudioServ[1];
				FACRepetition[4] = veciDataServ[0];
			}
			else /* iNumData == 2 */
			{
				/* Init repetion vector */
				FACNumRep = 10;
				FACRepetition.resize(FACNumRep);

				/* A1A2A1A2D1A1A2A1A2D2 */
				FACRepetition[0] = veciAudioServ[0];
				FACRepetition[1] = veciAudioServ[1];
				FACRepetition[2] = veciAudioServ[0];
				FACRepetition[3] = veciAudioServ[1];
				FACRepetition[4] = veciDataServ[0];
				FACRepetition[5] = veciAudioServ[0];
				FACRepetition[6] = veciAudioServ[1];
				FACRepetition[7] = veciAudioServ[0];
				FACRepetition[8] = veciAudioServ[1];
				FACRepetition[9] = veciDataServ[1];
			}
		}
		else /* iNumAudio == 3 */
		{
			/* Init repetion vector */
			FACNumRep = 7;
			FACRepetition.resize(FACNumRep);

			/* A1A2A3A1A2A3D1 */
			FACRepetition[0] = veciAudioServ[0];
			FACRepetition[1] = veciAudioServ[1];
			FACRepetition[2] = veciAudioServ[2];
			FACRepetition[3] = veciAudioServ[0];
			FACRepetition[4] = veciAudioServ[1];
			FACRepetition[5] = veciAudioServ[2];
			FACRepetition[6] = veciDataServ[0];
		}
	}
}


/******************************************************************************\
* CFACReceive																   *
\******************************************************************************/
_BOOLEAN CFACReceive::FACParam(CVector<_BINARY>* pbiFACData,
							   CParameter& Parameter)
{
/*
	First get new data from incoming data stream, then check if the new
	parameter differs from the old data stored in the receiver. If yes, init
	the modules to the new parameter
*/
	uint32_t	iTempServiceID;
	int			iTempShortID;

	/* CRC ------------------------------------------------------------------ */
	/* Check the CRC of this data block */
	CRCObject.Reset(8);

	(*pbiFACData).ResetBitAccess();

	for (int i = 0; i < NUM_FAC_BITS_PER_BLOCK / SIZEOF__BYTE - 1; i++)
		CRCObject.AddByte((_BYTE) (*pbiFACData).Separate(SIZEOF__BYTE));

	if (CRCObject.CheckCRC((*pbiFACData).Separate(8)) == true)
	{
		/* CRC-check successful, extract data from FAC-stream */
		/* Reset separation function */
		(*pbiFACData).ResetBitAccess();

		Parameter.Lock();

		/* Channel parameters ----------------------------------------------- */
		/* Base/Enhancement flag (not used) */
		(*pbiFACData).Separate(1);

		/* Identity */
		switch ((*pbiFACData).Separate(2))
		{
		case 0: /* 00 */
			Parameter.iFrameIDReceiv = 0;
			break;

		case 1: /* 01 */
			Parameter.iFrameIDReceiv = 1;
			break;

		case 2: /* 10 */
			Parameter.iFrameIDReceiv = 2;
			break;

		case 3: /* 11 */
			Parameter.iFrameIDReceiv = 0;
			break;
		}

		/* Spectrum occupancy */
		switch ((*pbiFACData).Separate(4))
		{
		case 0: /* 0000 */
			Parameter.SetSpectrumOccup(SO_0);
			break;

		case 1: /* 0001 */
			Parameter.SetSpectrumOccup(SO_1);
			break;

		case 2: /* 0010 */
			Parameter.SetSpectrumOccup(SO_2);
			break;

		case 3: /* 0011 */
			Parameter.SetSpectrumOccup(SO_3);
			break;

		case 4: /* 0100 */
			Parameter.SetSpectrumOccup(SO_4);
			break;

		case 5: /* 0101 */
			Parameter.SetSpectrumOccup(SO_5);
			break;
		}

		/* Interleaver depth flag */
		switch ((*pbiFACData).Separate(1))
		{
		case 0: /* 0 */
			Parameter.SetInterleaverDepth(CParameter::SI_LONG);
			break;

		case 1: /* 1 */
			Parameter.SetInterleaverDepth(CParameter::SI_SHORT);
			break;
		}

		/* MSC mode */
		switch ((*pbiFACData).Separate(2))
		{
		case 0: /* 00 */
			Parameter.SetMSCCodingScheme(CS_3_SM);
			break;

		case 1: /* 01 */
			Parameter.SetMSCCodingScheme(CS_3_HMMIX);
			break;

		case 2: /* 10 */
			Parameter.SetMSCCodingScheme(CS_3_HMSYM);
			break;

		case 3: /* 11 */
			Parameter.SetMSCCodingScheme(CS_2_SM);
			break;
		}

		/* SDC mode */
		switch ((*pbiFACData).Separate(1))
		{
		case 0: /* 0 */
			Parameter.SetSDCCodingScheme(CS_2_SM);
			break;

		case 1: /* 1 */
			Parameter.SetSDCCodingScheme(CS_1_SM);
			break;
		}

		/* Number of services */
		/* Search table for entry */
		int iNumServTabEntry = (*pbiFACData).Separate(4);
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 5; j++)
				if (iNumServTabEntry == iTableNumOfServices[i][j])
					Parameter.SetNumOfServices(i, j);

		/* Reconfiguration index (not used, yet) */
		(*pbiFACData).Separate(3);

		/* rfu */
		/* Do not use rfu */
		(*pbiFACData).Separate(2);


		/* Service parameters ----------------------------------------------- */
		/* Service identifier */
		iTempServiceID = (*pbiFACData).Separate(24);

		/* Short ID (the short ID is the index of the service-array) */
		iTempShortID = (*pbiFACData).Separate(2);

		/* Set service identifier */
		Parameter.SetServiceID(iTempShortID, iTempServiceID);

		/* CA indication */
		switch ((*pbiFACData).Separate(1))
		{
		case 0: /* 0 */
			Parameter.Service[iTempShortID].eCAIndication =
				CService::CA_NOT_USED;
			break;

		case 1: /* 1 */
			Parameter.Service[iTempShortID].eCAIndication =
				CService::CA_USED;
			break;
		}

		/* Language */
		Parameter.Service[iTempShortID].iLanguage = (*pbiFACData).Separate(4);

		/* Audio/Data flag */
		switch ((*pbiFACData).Separate(1))
		{
		case 0: /* 0 */
			Parameter.SetAudDataFlag(iTempShortID, CService::SF_AUDIO);
			break;

		case 1: /* 1 */
			Parameter.SetAudDataFlag(iTempShortID, CService::SF_DATA);
			break;
		}

		/* Service descriptor */
		Parameter.Service[iTempShortID].iServiceDescr =
			(*pbiFACData).Separate(5);

		Parameter.Unlock();

		/* Rfa */
		/* Do not use Rfa */
		(*pbiFACData).Separate(7);

		/* CRC is ok, return true */
		return true;
	}
	else
	{
		/* Data is corrupted, do not use it. Return failure as false */
		return false;
	}
}

