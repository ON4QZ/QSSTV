/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer, Ollie Haffenden
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

#if !defined(AUIDOSOURCEDECODER_H__3B0BA660_CABB2B_23E7A0D31912__INCLUDED_)
#define AUIDOSOURCEDECODER_H__3B0BA660_CABB2B_23E7A0D31912__INCLUDED_

#include "../GlobalDefinitions.h"
#include "../Parameter.h"
#include "../util/Modul.h"
#include "../util/CRC.h"
//#include "../TextMessage.h"
#include "../datadecoding/DataDecoder.h"
#include "../util/Utilities.h"




/* Classes ********************************************************************/
class CAudioSourceEncoderImplementation
{
public:
  CAudioSourceEncoderImplementation() : bUsingTextMessage(false)
  {}
  virtual ~CAudioSourceEncoderImplementation();

//  void SetTextMessage(const string& strText);
//  void ClearTextMessage();

  void SetPicFileName(QByteArray *ba, const QString name,const QString format)
  {
    DataEncoder.GetSliShowEnc()->AddArray(ba,name,format);
  }
  void ClearPicFileNames()
  {DataEncoder.GetSliShowEnc()->ClearAllFileNames();}
  _BOOLEAN GetTransStat(string& strCPi, _REAL& rCPe)
  {return DataEncoder.GetSliShowEnc()->GetTransStat(strCPi, rCPe);}

protected:
//  CTextMessageEncoder		TextMessage;
  _BOOLEAN				bUsingTextMessage;
  CDataEncoder			DataEncoder;
  int						iTotPacketSize;
  _BOOLEAN				bIsDataService;
  int						iTotNumBitsForUsage;


public:
  virtual void InitInternalTx(CParameter &TransmParam, int &iOutputBlockSize);
  //		virtual void InitInternalRx(CParameter& Param, int &iInputBlockSize, int &iOutputBlockSize);
  virtual void ProcessDataInternal( CVectorEx<_BINARY>* pvecOutputData, int &iOutputBlockSize);
  //		virtual void ProcessDataInternalRx( CVectorEx<_SAMPLE>* pvecInputData,
  //                                                 CVectorEx<_BINARY>* pvecOutputData, int &InputBlockSize, int &iOutputBlockSize);
//  virtual void ProcessDataInternal(CVectorEx<_SAMPLE>*,CVectorEx<_BINARY>* pvecOutputData, int &, int &iOutputBlockSize);
};


class CAudioSourceEncoder : public CTransmitterModul<_SAMPLE, _BINARY>
{
public:
	CAudioSourceEncoder() {}
	virtual ~CAudioSourceEncoder() {}

//	void SetTextMessage(const string& strText) {AudioSourceEncoderImpl.SetTextMessage(strText);}
//	void ClearTextMessage() {AudioSourceEncoderImpl.ClearTextMessage();}
	
  void SetPicFileName(QByteArray *ba, const QString name,const QString format)
      {
         AudioSourceEncoderImpl.SetPicFileName(ba,name,format);
     }

	void ClearPicFileNames() {AudioSourceEncoderImpl.ClearPicFileNames();}

	_BOOLEAN GetTransStat(string& strCPi, _REAL& rCPe)
			{return AudioSourceEncoderImpl.GetTransStat(strCPi, rCPe);}

protected:
	CAudioSourceEncoderImplementation AudioSourceEncoderImpl;

	virtual void InitInternal(CParameter& TransmParam)
	{
		AudioSourceEncoderImpl.InitInternalTx(TransmParam, iOutputBlockSize);
	}
	
	virtual void ProcessDataInternal(CParameter& )
	{
		AudioSourceEncoderImpl.ProcessDataInternal(pvecOutputData,  iOutputBlockSize);
	}

//  virtual void ProcessDataInternalRx(CParameter& )
//  {
//    AudioSourceEncoderImpl.ProcessDataInternalRx(pvecInputData, pvecOutputData, iInputBlockSize,   iOutputBlockSize);
//  }

};



#endif // !defined(AUIDOSOURCEDECODER_H__3B0BA660_CABB2B_23E7A0D31912__INCLUDED_)
