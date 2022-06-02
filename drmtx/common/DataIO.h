/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2005
 *
 * Author(s):
 *	Volker Fischer, Andrew Murphy
 *
 * Description:
 *	See Data.cpp
 *
 * 11/21/2005 Andrew Murphy, BBC Research & Development, 2005
 *	- Addition GetSDCReceive(), Added CSplit class
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

#if !defined(DATA_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
#define DATA_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_

# include "soundinterface.h"
#include "Parameter.h"
#include "util/Modul.h"
#include "FAC/FAC.h"
#include "SDC/SDC.h"
//#include "TextMessage.h"
//#include "util/AudioFile.h"
#include "util/Utilities.h"
//#include "AMDemodulation.h" // For CMixer

/* Definitions ****************************************************************/
/* In case of random-noise, define number of blocks */
#define DEFAULT_NUM_SIM_BLOCKS		50

/* Length of vector for audio spectrum. We use a power-of-two length to 
   make the FFT work more efficient */
#define NUM_SMPLS_4_AUDIO_SPECTRUM	256

/* Time span used for averaging the audio spectrum. Shall be higher than the
   400 ms DRM audio block */
#define TIME_AV_AUDIO_SPECT_MS		500 /* ms */		

/* Number of blocks for averaging the audio spectrum */
#define NUM_BLOCKS_AV_AUDIO_SPEC	Ceil(((_REAL) SOUNDCRD_SAMPLE_RATE * \
	TIME_AV_AUDIO_SPECT_MS / 1000 / NUM_SMPLS_4_AUDIO_SPECTRUM))

/* Normalization constant for two mixed signals. If this constant is 2, no
   overrun of the "short" variable can happen but signal has quite much lower
   power -> compromise */
#define MIX_OUT_CHAN_NORM_CONST		((_REAL) 1.0 / sqrt((_REAL) 2.0))



class CGenSimData : public CTransmitterModul<_BINARY, _BINARY>
{
public:
	CGenSimData() : eCntType(CT_TIME), iNumSimBlocks(DEFAULT_NUM_SIM_BLOCKS),
		iNumErrors(0), iCounter(0), strFileName("SimTime.dat"), tiStartTime(0) {}
	virtual ~CGenSimData() {}

	void SetSimTime(int iNewTi, string strNewFileName);
	void SetNumErrors(int iNewNE, string strNewFileName);

protected:
	enum ECntType {CT_TIME, CT_ERRORS};
	ECntType	eCntType;
	int			iNumSimBlocks;
	int			iNumErrors;
	int			iCounter;
	int			iMinNumBlocks;
	string		strFileName;
	time_t		tiStartTime;

	virtual void InitInternal(CParameter& TransmParam);
	virtual void ProcessDataInternal(CParameter& TransmParam);
};




/* FAC ---------------------------------------------------------------------- */
class CGenerateFACData : public CTransmitterModul<_BINARY, _BINARY>
{
public:
	CGenerateFACData() {}
	virtual ~CGenerateFACData() {}

protected:
	CFACTransmit FACTransmit;

	virtual void InitInternal(CParameter& TransmParam);
	virtual void ProcessDataInternal(CParameter& TransmParam);
};




/* SDC ---------------------------------------------------------------------- */
class CGenerateSDCData : public CTransmitterModul<_BINARY, _BINARY>
{
public:
	CGenerateSDCData() {}
	virtual ~CGenerateSDCData() {}

protected:
	CSDCTransmit SDCTransmit;
		
	virtual void InitInternal(CParameter& TransmParam);
	virtual void ProcessDataInternal(CParameter& TransmParam);
};




#endif // !defined(DATA_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
