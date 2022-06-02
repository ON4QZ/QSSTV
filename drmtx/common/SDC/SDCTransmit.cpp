/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	SDC
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

#include "SDC.h"


/* Implementation *************************************************************/
void CSDCTransmit::SDCParam(CVector<_BINARY>* pbiData, CParameter& Parameter)
{
/* 
	Put SDC parameters on a stream 
*/
	int					i;
	int					iSize;
	CVector<_BINARY>	vecbiData;

	/* Calculate length of data field in bytes 
	   (consistant to table 61 in (6.4.1)) */
	const int iLengthDataFieldBytes = 
		(int) ((_REAL) (Parameter.iNumSDCBitsPerSFrame - 20) / 8);

	/* 20 bits from AFS index and CRC */
	const int iUsefulBitsSDC = 20 + iLengthDataFieldBytes * 8;

	/* "- 20" for the AFS-index and CRC! */
	const int iMaxNumBitsDataBlocks = iUsefulBitsSDC - 20;

	/* Reset enqueue function */
	(*pbiData).ResetBitAccess();


	/* SDC Header ----------------------------------------------------------- */
	/* AFS index (not used by this application, insert a "1" */
	(*pbiData).Enqueue((uint32_t) 1, 4);


	/* Data Entities -------------------------------------------------------- */
	/* Init bit-count */
	int iNumUsedBits = 0;

// Choose types, TEST. Send only important types for this test!
// TODO: test, if SDC block is long enough for all types!
	/* Type 0 */
	DataEntityType0(vecbiData, Parameter);

	// TODO: nicer solution
	iSize = vecbiData.Size();
	if (iNumUsedBits + iSize < iMaxNumBitsDataBlocks)
	{
		iNumUsedBits += iSize;

		vecbiData.ResetBitAccess();
		for (i = 0; i < iSize; i++)
			(*pbiData).Enqueue(vecbiData.Separate(1), 1);
	}


// Only working for either one audio or data service!
if (Parameter.iNumAudioService == 1)
{
	/* Type 9 */
	DataEntityType9(vecbiData, 0, Parameter);
}
else
{
	/* Type 5 */
	DataEntityType5(vecbiData, 0, Parameter);
}

// TODO: nicer solution
iSize = vecbiData.Size();
if (iNumUsedBits + iSize < iMaxNumBitsDataBlocks)
{
	iNumUsedBits += iSize;

	vecbiData.ResetBitAccess();
	for (i = 0; i < iSize; i++)
		(*pbiData).Enqueue(vecbiData.Separate(1), 1);
}

	/* Type 1 */
	DataEntityType1(vecbiData, 0, Parameter);

	// TODO: nicer solution
	iSize = vecbiData.Size();
	if (iNumUsedBits + iSize < iMaxNumBitsDataBlocks)
	{
		iNumUsedBits += iSize;

		vecbiData.ResetBitAccess();
		for (i = 0; i < iSize; i++)
			(*pbiData).Enqueue(vecbiData.Separate(1), 1);
	}


	/* Zero-pad the unused bits in this SDC-block */
	for (i = 0; i < iMaxNumBitsDataBlocks - iNumUsedBits; i++)
		(*pbiData).Enqueue((uint32_t) 0, 1);


	/* CRC ------------------------------------------------------------------ */
	/* Calculate the CRC and put it at the end of the stream */
	CRCObject.Reset(16);

	(*pbiData).ResetBitAccess();

	/* Special treatment of SDC data stream: The CRC (Cyclic Redundancy 
	Check) field shall contain a 16-bit CRC calculated over the AFS 
	index coded in an 8-bit field (4 msbs are 0) and the data field.
	4 MSBs from AFS-index. Insert four "0" in the data-stream */
	const _BYTE byFirstByte = (_BYTE) (*pbiData).Separate(4);
	CRCObject.AddByte(byFirstByte);

	for (i = 0; i < (iUsefulBitsSDC - 4) / SIZEOF__BYTE - 2; i++)
		CRCObject.AddByte((_BYTE) (*pbiData).Separate(SIZEOF__BYTE));

	/* Now, pointer in "enqueue"-function is back at the same place, 
	   add CRC */
	(*pbiData).Enqueue(CRCObject.GetCRC(), 16);
}


/******************************************************************************\
* Data entity Type 0 (Multiplex description data entity)					   *
\******************************************************************************/
void CSDCTransmit::DataEntityType0(CVector<_BINARY>& vecbiData,
								   CParameter& Parameter)
{
	/* 24 bits for each stream description + 4 bits for protection levels */
	const int iNumBitsTotal = 4 + Parameter.GetTotNumServices() * 24;

	/* Init return vector (storing this data block) */
	vecbiData.Init(iNumBitsTotal + NUM_BITS_HEADER_SDC);
	vecbiData.ResetBitAccess();

	/* Length of the body, excluding the initial 4 bits ("- 4"), 
	   measured in bytes ("/ 8") */
	uint32_t iLengthInBytes = (iNumBitsTotal - 4) / 8;
	vecbiData.Enqueue(iLengthInBytes, 7);

	/* Version flag (not used in this implementation) */
	vecbiData.Enqueue((uint32_t) 0, 1);

	/* Data entity type */
	vecbiData.Enqueue((uint32_t) 00, 4); /* Type 00 */


	/* Actual body ---------------------------------------------------------- */
	/* Protection level for part A */
	vecbiData.Enqueue((uint32_t) Parameter.MSCPrLe.iPartA, 2);

	/* Protection level for part B */
	vecbiData.Enqueue((uint32_t) Parameter.MSCPrLe.iPartB, 2);

	for (size_t i = 0; i < Parameter.GetTotNumServices(); i++)
	{
		/* In case of hirachical modulation stream 0 describes the protection
		   level and length of hirarchical data */
		if ((i == 0) && 
			((Parameter.eMSCCodingScheme == CS_3_HMSYM) ||
			(Parameter.eMSCCodingScheme == CS_3_HMMIX)))
		{
			/* Protection level for hierarchical */
			vecbiData.Enqueue((uint32_t) Parameter.MSCPrLe.iHierarch, 2);
		
			/* rfu */
			vecbiData.Enqueue((uint32_t) 0, 10);
	
			/* Data length for hierarchical */
			vecbiData.Enqueue((uint32_t) Parameter.Stream[i].iLenPartB, 12);
		}
		else
		{
			/* Data length for part A */
			vecbiData.Enqueue((uint32_t) Parameter.Stream[i].iLenPartA, 12);
		
			/* Data length for part B */
			vecbiData.Enqueue((uint32_t) Parameter.Stream[i].iLenPartB, 12);
		}
	}
}


/******************************************************************************\
* Data entity Type 1 (Label data entity)									   *
\******************************************************************************/
void CSDCTransmit::DataEntityType1(CVector<_BINARY>& vecbiData, int ServiceID,
								   CParameter& Parameter)
{
	int	iLenLabel;

	/* Length of label. Label is a variable length field of up to 16 bytes
	   defining the label using UTF-8 coding */
	const int iLenLabelTmp = Parameter.Service[ServiceID].strLabel.length();
	if (iLenLabelTmp > 16)
		iLenLabel = 16;
	else
		iLenLabel = iLenLabelTmp;

	/* Number in bits (* 8) plus initial 4 bits (+ 4) */
	const int iNumBitsTotal = iLenLabel * 8 + 4;

	/* Init return vector (storing this data block) */
	vecbiData.Init(iNumBitsTotal + NUM_BITS_HEADER_SDC);
	vecbiData.ResetBitAccess();


	/**** Multiplex description data entity - type 1 ****/
	/* Length of the body, excluding the initial 4 bits, 
	   measured in bytes -> only number bytes of label */
	vecbiData.Enqueue((uint32_t) iLenLabel, 7);

	/* Version flag (not used in this implementation) */
	vecbiData.Enqueue((uint32_t) 0, 1);

	/* Data entity type */
	vecbiData.Enqueue((uint32_t) 01, 4); /* Type 01 */


	/* Actual body ---------------------------------------------------------- */
	/* Short Id */
	vecbiData.Enqueue((uint32_t) ServiceID, 2);

	/* rfu */
	vecbiData.Enqueue((uint32_t) 0, 2);

	/* Set all characters of label string */
	for (int i = 0; i < iLenLabel; i++)
	{
		const char cNewChar = Parameter.Service[ServiceID].strLabel[i];

		/* Set character */
		vecbiData.Enqueue((uint32_t) cNewChar, 8);
	}
}


/******************************************************************************\
* Data entity Type 5 (Application information data entity)					   *
\******************************************************************************/
void CSDCTransmit::DataEntityType5(CVector<_BINARY>& vecbiData, int ServiceID,
								   CParameter& Parameter)
{
	int	iNumBitsTotal = 0;

	/* Set total number of bits */
	switch (Parameter.Service[ServiceID].DataParam.ePacketModInd)
	{
	case CDataParam::PM_SYNCHRON_STR_MODE:
		iNumBitsTotal = 12 + 16 /* TEST */ /* + application data TODO! */;
		break;

	case CDataParam::PM_PACKET_MODE:
		iNumBitsTotal = 20 + 16 /* TEST */ /* + application data TODO! */;
		break;
	}

	/* Init return vector (storing this data block) */
	vecbiData.Init(iNumBitsTotal + NUM_BITS_HEADER_SDC);
	vecbiData.ResetBitAccess();

	/* Length of the body, excluding the initial 4 bits ("- 4"), 
	   measured in bytes ("/ 8") */
	vecbiData.Enqueue((uint32_t) (iNumBitsTotal - 4) / 8, 7);

	/* Version flag (not used in this implementation) */
	vecbiData.Enqueue((uint32_t) 0, 1);

	/* Data entity type */
	vecbiData.Enqueue((uint32_t) 05, 4); /* Type 05 */


	/* Actual body ---------------------------------------------------------- */
	/* Short Id */
	vecbiData.Enqueue((uint32_t) ServiceID, 2);

	/* Stream Id */
	vecbiData.Enqueue((uint32_t) Parameter.Service[ServiceID].DataParam.
		iStreamID, 2);

	/* Packet mode indicator */
	switch (Parameter.Service[ServiceID].DataParam.ePacketModInd)
	{
	case CDataParam::PM_SYNCHRON_STR_MODE:
		vecbiData.Enqueue(0 /* 0 */, 1);

		/* Descriptor */
		vecbiData.Enqueue((uint32_t) 0, 7);
		break;

	case CDataParam::PM_PACKET_MODE:
		vecbiData.Enqueue(1 /* 1 */, 1);

		/* Descriptor */
		/* Data unit indicator */
		switch (Parameter.Service[ServiceID].DataParam.eDataUnitInd)
		{
		case CDataParam::DU_SINGLE_PACKETS:
			vecbiData.Enqueue(0 /* 0 */, 1);
			break;

		case CDataParam::DU_DATA_UNITS:
			vecbiData.Enqueue(1 /* 1 */, 1);
			break;
		}

		/* Packet Id */
		vecbiData.Enqueue( 
			(uint32_t) Parameter.Service[ServiceID].DataParam.iPacketID, 2);

		/* Application domain */
		switch (Parameter.Service[ServiceID].DataParam.eAppDomain)
		{
		case CDataParam::AD_DRM_SPEC_APP:
			vecbiData.Enqueue(0 /* 0000 */, 4);
			break;

		case CDataParam::AD_DAB_SPEC_APP:
			vecbiData.Enqueue(1 /* 0001 */, 4);
			break;
		default:
			throw CGenErr("bad application domain in SDC preparation");
		}

		/* Packet length */
		vecbiData.Enqueue( 
			(uint32_t) Parameter.Service[ServiceID].DataParam.iPacketLen, 8);

		break;
	}

	/* Application data */
// Not used

// TEST
/* Fixed implementation for MOTSlideshow application which is the one and
   only supported application right now. TODO */
/* rfu */
vecbiData.Enqueue((uint32_t) 0, 5);

/* User application identifier. SlideShow = 2 */
vecbiData.Enqueue((uint32_t) 2, 11);
}


/******************************************************************************\
* Data entity Type 9 (Audio information data entity)						   *
\******************************************************************************/
void CSDCTransmit::DataEntityType9(CVector<_BINARY>& vecbiData, int ServiceID,
								   CParameter& Parameter)
{
	/* Set total number of bits */
	const int iNumBitsTotal = 20;

	/* Init return vector (storing this data block) */
	vecbiData.Init(iNumBitsTotal + NUM_BITS_HEADER_SDC);
	vecbiData.ResetBitAccess();

	/* Length of the body, excluding the initial 4 bits ("- 4"), 
	   measured in bytes ("/ 8") */
	vecbiData.Enqueue((uint32_t) (iNumBitsTotal - 4) / 8, 7);

	/* Version flag (not used in this implementation) */
	vecbiData.Enqueue((uint32_t) 0, 1);

	/* Data entity type */
	vecbiData.Enqueue((uint32_t) 9, 4); /* Type 09 */


	/* Actual body ---------------------------------------------------------- */
	/* Short Id */
	vecbiData.Enqueue((uint32_t) ServiceID, 2);

	/* Stream Id */
	vecbiData.Enqueue((uint32_t) Parameter.Service[ServiceID].AudioParam.
		iStreamID, 2);

	/* Audio coding */
	switch (Parameter.Service[ServiceID].AudioParam.eAudioCoding)
	{
	case CAudioParam::AC_AAC:
		vecbiData.Enqueue(0 /* 00 */, 2);
		break;

	case CAudioParam::AC_CELP:
		vecbiData.Enqueue(1 /* 01 */, 2);
		break;

	case CAudioParam::AC_HVXC:
		vecbiData.Enqueue(2 /* 10 */, 2);
		break;
	}

	/* SBR flag */
	switch (Parameter.Service[ServiceID].AudioParam.eSBRFlag)
	{
	case CAudioParam::SB_NOT_USED:
		vecbiData.Enqueue(0 /* 0 */, 1);
		break;

	case CAudioParam::SB_USED:
		vecbiData.Enqueue(1 /* 1 */, 1);
		break;
	}

	/* Audio mode */
	switch (Parameter.Service[ServiceID].AudioParam.eAudioCoding)
	{
	case CAudioParam::AC_AAC:
		/* Channel type */
		switch (Parameter.Service[ServiceID].AudioParam.eAudioMode)
		{
		case CAudioParam::AM_MONO:
			vecbiData.Enqueue(0 /* 00 */, 2);
			break;

		case CAudioParam::AM_P_STEREO:
			vecbiData.Enqueue(1 /* 01 */, 2);
			break;

		case CAudioParam::AM_STEREO:
			vecbiData.Enqueue(2 /* 10 */, 2);
			break;
		}
		break;

	case CAudioParam::AC_CELP:
		/* rfa */
		vecbiData.Enqueue((uint32_t) 0, 1);

		/* CELP_CRC */
    if (!Parameter.Service[ServiceID].AudioParam.bCELPCRC) vecbiData.Enqueue(0 /* 0 */, 1);
    else vecbiData.Enqueue(1 /* 1 */, 1);


		break;

	case CAudioParam::AC_HVXC:
		/* HVXC_rate */
		switch (Parameter.Service[ServiceID].AudioParam.eHVXCRate)
		{
		case CAudioParam::HR_2_KBIT:
			vecbiData.Enqueue(0 /* 0 */, 1);
			break;

		case CAudioParam::HR_4_KBIT:
			vecbiData.Enqueue(1 /* 1 */, 1);
			break;
		}

		/* HVXC CRC */
    if (!Parameter.Service[ServiceID].AudioParam.bHVXCCRC) vecbiData.Enqueue(0 /* 0 */, 1);
    else vecbiData.Enqueue(1 /* 1 */, 1);

		break;
	}

	/* Audio sampling rate */
	switch (Parameter.Service[ServiceID].AudioParam.eAudioSamplRate)
	{
	case CAudioParam::AS_8_KHZ:
		vecbiData.Enqueue(0 /* 000 */, 3);
		break;

	case CAudioParam::AS_12KHZ:
		vecbiData.Enqueue(1 /* 001 */, 3);
		break;

	case CAudioParam::AS_16KHZ:
		vecbiData.Enqueue(2 /* 010 */, 3);
		break;

	case CAudioParam::AS_24KHZ:
		vecbiData.Enqueue(3 /* 011 */, 3);
		break;
	}

	/* Text flag */
  if (!Parameter.Service[ServiceID].AudioParam.bTextflag) vecbiData.Enqueue(0 /* 0 */, 1);
  else vecbiData.Enqueue(1 /* 1 */, 1);

	/* Enhancement flag */
  if (!Parameter.Service[ServiceID].AudioParam.bEnhanceFlag)
    {
      vecbiData.Enqueue(0 /* 0 */, 1);
    }
  else
    {
    vecbiData.Enqueue(1 /* 1 */, 1);
    }


	/* Coder field */
	if (Parameter.Service[ServiceID].AudioParam.
		eAudioCoding == CAudioParam::AC_CELP)
	{
		/* CELP index */
		vecbiData.Enqueue( 
			(uint32_t) Parameter.Service[ServiceID].AudioParam.iCELPIndex, 5);
	}
	else
	{
		/* rfa 5 bit */
		vecbiData.Enqueue((uint32_t) 0, 5);
	}
	
	/* rfa 1 bit */
	vecbiData.Enqueue((uint32_t) 0, 1);
}
