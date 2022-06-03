/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2005
 *
 * Author(s):
 *	Volker Fischer, Andrew Murphy
 *
 * Description:
 *	See SDC.cpp
 *
 * 11/21/2005 Andrew Murphy, BBC Research & Development, 2005
 *	- AMSS data entity groups (no AFS index), added eSDCType, data type 11
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

#if !defined(SDC_H__3B0BA660_CA63SDBOJKEWROBNER89NE877A0D312__INCLUDED_)
#define SDC_H__3B0BA660_CA63SDBOJKEWROBNER89NE877A0D312__INCLUDED_

#include "../GlobalDefinitions.h"
#include "../Parameter.h"
#include "../util/CRC.h"
#include "vector.h"
#include "../util/Utilities.h"

/* Definitions ****************************************************************/
/* Number of bits of header of SDC block */
#define NUM_BITS_HEADER_SDC			12


/* Classes ********************************************************************/
class CSDCTransmit
{
public:
	CSDCTransmit() {}
	virtual ~CSDCTransmit() {}

	void SDCParam(CVector<_BINARY>* pbiData, CParameter& Parameter);

protected:
	void DataEntityType0(CVector<_BINARY>& vecbiData, CParameter& Parameter);
	void DataEntityType1(CVector<_BINARY>& vecbiData, int ServiceID,
						 CParameter& Parameter);
// ...
	void DataEntityType5(CVector<_BINARY>& vecbiData, int ServiceID,
						 CParameter& Parameter);
// ...
	void DataEntityType9(CVector<_BINARY>& vecbiData, int ServiceID,
						 CParameter& Parameter);

	CCRC CRCObject;
};


#endif // !defined(SDC_H__3B0BA660_CA63SDBOJKEWROBNER89NE877A0D312__INCLUDED_)
