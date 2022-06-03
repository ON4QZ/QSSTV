/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Adapted for ham sstv use Ties Bos - PA0MBO
 *
 * Description:
 *	High level class for all modules. The common functionality for reading
 *	and writing the transfer-buffers are implemented here.
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

#if !defined(AFX_MODUL_H__41E39CD3_2AEC_400E_907B_148C0EC17A43__INCLUDED_)
#define AFX_MODUL_H__41E39CD3_2AEC_400E_907B_148C0EC17A43__INCLUDED_

#include "Buffer.h"
#include "vector.h"
#include "../Parameter.h"
#include <iostream>


/* Classes ********************************************************************/
/* CModul ------------------------------------------------------------------- */
template<class TInput, class TOutput>
class CModul  
{
public:
	CModul();
	virtual ~CModul() {}

	virtual void Init(CParameter& Parameter);
	virtual void Init(CParameter& Parameter, CBuffer<TOutput>& OutputBuffer);

protected:
	CVectorEx<TInput>*	pvecInputData;
	CVectorEx<TOutput>*	pvecOutputData;

	/* Max block-size are used to determine the size of the requiered buffer */
	int					iMaxOutputBlockSize;
	/* Actual read (or written) size of the data */
	int					iInputBlockSize;
	int					iOutputBlockSize;

	void				Lock() {Mutex.Lock();}
	void				Unlock() {Mutex.Unlock();}

	void				InitThreadSave(CParameter& Parameter);
	virtual void		InitInternal(CParameter& Parameter) = 0;
	void				ProcessDataThreadSave(CParameter& Parameter);
	virtual void		ProcessDataInternal(CParameter& Parameter) = 0;

private:
	CMutex				Mutex;
};


/* CTransmitterModul -------------------------------------------------------- */
template<class TInput, class TOutput>
class CTransmitterModul : public CModul<TInput, TOutput>
{
public:
	CTransmitterModul();
	virtual ~CTransmitterModul() {}

	virtual void		Init(CParameter& Parameter);
	virtual void		Init(CParameter& Parameter, 
							 CBuffer<TOutput>& OutputBuffer);
	virtual void		ReadData(CParameter& Parameter, 
								 CBuffer<TOutput>& OutputBuffer);
	virtual void		ProcessData(CParameter& Parameter, 
								 CBuffer<TOutput>& OutputBuffer);
	virtual _BOOLEAN	ProcessData(CParameter& Parameter, 
									CBuffer<TInput>& InputBuffer, 
									CBuffer<TOutput>& OutputBuffer);
	virtual void		ProcessData(CParameter& Parameter, 
									CBuffer<TInput>& InputBuffer,
									CBuffer<TInput>& InputBuffer2, 
				//					CBuffer<TInput>& InputBuffer3,   pa0mbo 
									CBuffer<TOutput>& OutputBuffer);
	virtual _BOOLEAN	WriteData(CParameter& Parameter, 
								  CBuffer<TInput>& InputBuffer);

protected:
	/* Additional buffers if the derived class has multiple input streams */
	CVectorEx<TInput>*	pvecInputData2;
	CVectorEx<TInput>*	pvecInputData3;

	/* Actual read (or written) size of the data */
	int					iInputBlockSize2;
	int					iInputBlockSize3;
};





/* Implementation *************************************************************/
/******************************************************************************\
* CModul                                                                       *
\******************************************************************************/
template<class TInput, class TOutput>
CModul<TInput, TOutput>::CModul()
{
	/* Initialize everything with zeros */
	iMaxOutputBlockSize = 0;
	iInputBlockSize = 0;
	iOutputBlockSize = 0;
	pvecInputData = NULL;
	pvecOutputData = NULL;
}

template<class TInput, class TOutput>
void CModul<TInput, TOutput>::ProcessDataThreadSave(CParameter& Parameter)
{
	/* Get a lock for the resources */
	Lock();

	/* Call processing routine of derived modul */
	ProcessDataInternal(Parameter);

	/* Unlock resources */
	Unlock();
}

template<class TInput, class TOutput>
void CModul<TInput, TOutput>::InitThreadSave(CParameter& Parameter)
{
	/* Get a lock for the resources */
	Lock();

	try
	{
		/* Call init of derived modul */
		InitInternal(Parameter);

		/* Unlock resources */
		Unlock();
	}

	catch (CGenErr)
	{
		/* Unlock resources */
		Unlock();

		/* Throws the same error again which was send by the function */
		throw;
	}
}

template<class TInput, class TOutput>
void CModul<TInput, TOutput>::Init(CParameter& Parameter)
{
	/* Init some internal variables */
	iInputBlockSize = 0;

	/* Call init of derived modul */
	InitThreadSave(Parameter);
}

template<class TInput, class TOutput>
void CModul<TInput, TOutput>::Init(CParameter& Parameter, 
								   CBuffer<TOutput>& OutputBuffer)
{
	/* Init some internal variables */
	iMaxOutputBlockSize = 0;
	iInputBlockSize = 0;
	iOutputBlockSize = 0;

	/* Call init of derived modul */
	InitThreadSave(Parameter);

	/* Init output transfer buffer */
	if (iMaxOutputBlockSize != 0)
		OutputBuffer.Init(iMaxOutputBlockSize);
	else
	{
		if (iOutputBlockSize != 0)
			OutputBuffer.Init(iOutputBlockSize);
	}
}


/******************************************************************************\
* Transmitter modul (CTransmitterModul)                                        *
\******************************************************************************/
template<class TInput, class TOutput>
CTransmitterModul<TInput, TOutput>::CTransmitterModul()
{
	/* Initialize all member variables with zeros */
	iInputBlockSize2 = 0;
	iInputBlockSize3 = 0;
	pvecInputData2 = NULL;
	pvecInputData3 = NULL;
}

template<class TInput, class TOutput>
void CTransmitterModul<TInput, TOutput>::Init(CParameter& Parameter)
{
	/* Init some internal variables */
	iInputBlockSize2 = 0;
	iInputBlockSize3 = 0;

	/* Init base-class */
	CModul<TInput, TOutput>::Init(Parameter);
}

template<class TInput, class TOutput>
void CTransmitterModul<TInput, TOutput>::Init(CParameter& Parameter, 
											  CBuffer<TOutput>& OutputBuffer)
{
	/* Init some internal variables */
	iInputBlockSize2 = 0;
	iInputBlockSize3 = 0;

	/* Init base-class */
	CModul<TInput, TOutput>::Init(Parameter, OutputBuffer);
}

template<class TInput, class TOutput>
_BOOLEAN CTransmitterModul<TInput, TOutput>::
	ProcessData(CParameter& Parameter, CBuffer<TInput>& InputBuffer,
				CBuffer<TOutput>& OutputBuffer)
{
        // printf("Tx Par Inp Outp request is %d\n", OutputBuffer.GetRequestFlag());
	/* OUTPUT-DRIVEN modul implementation in the transmitter ---------------- */
	/* Look in output buffer if data is requested */
	if (OutputBuffer.GetRequestFlag() == true)
	{
                // printf("CTransmitterModul Getrequest is true\n");
		/* Check, if enough input data is available */
		if (InputBuffer.GetFillLevel() < this->iInputBlockSize)
		{
			/* Set request flag */
			InputBuffer.SetRequestFlag(true);

			return false;
		}

		/* Get vector from transfer-buffer */
		this->pvecInputData = InputBuffer.Get(this->iInputBlockSize);

		/* Query vector from output transfer-buffer for writing */
		this->pvecOutputData = OutputBuffer.QueryWriteBuffer();

		/* Copy extended data from vectors */
    (*(this->pvecOutputData)).SetExData((*(this->pvecInputData)).GetExData());

    /* Call the underlying processing-routine */
		this->ProcessDataInternal(Parameter);
	
		/* Write processed data from internal memory in transfer-buffer */
		OutputBuffer.Put(this->iOutputBlockSize);

		/* Data was provided, clear data request */
		OutputBuffer.SetRequestFlag(false);
	}

	return true;
}

template<class TInput, class TOutput>
void CTransmitterModul<TInput, TOutput>::
	ProcessData(CParameter& Parameter, CBuffer<TInput>& InputBuffer,
				CBuffer<TInput>& InputBuffer2,
	//			CBuffer<TInput>& InputBuffer3,    pa0mbo
				CBuffer<TOutput>& OutputBuffer)
{
	/* OUTPUT-DRIVEN modul implementation in the transmitter ---------------- */
	/* Look in output buffer if data is requested */
        // printf("Tx Par Inp1 Inp2 Outp request is %d  inp1size %d inp2size %d \n",
           //   OutputBuffer.GetRequestFlag(), this->iInputBlockSize, iInputBlockSize2);
	if (OutputBuffer.GetRequestFlag() == true)
	{
		/* Check, if enough input data is available from all sources */
    if (InputBuffer.GetFillLevel() < this->iInputBlockSize)
		{
			/* Set request flag */
                        // printf("tx modul 2 inputs setting request on buf1 \n");
			InputBuffer.SetRequestFlag(true);

			return;
		}
		if (InputBuffer2.GetFillLevel() < iInputBlockSize2)
		{
			/* Set request flag */
                        // printf("tx modul 2 inputs setting request on buf2 \n");
			InputBuffer2.SetRequestFlag(true);

			return;
		}
/*		if (InputBuffer3.GetFillLevel() < iInputBlockSize3)
		{
			InputBuffer3.SetRequestFlag(true);

			return;
		}
		*/
	
		/* Get vectors from transfer-buffers */
    addToLog(QString("ProcesData get %1").arg(this->iInputBlockSize),LOGDRMTX);
		this->pvecInputData = InputBuffer.Get(this->iInputBlockSize);
		pvecInputData2 = InputBuffer2.Get(iInputBlockSize2);
//		pvecInputData3 = InputBuffer3.Get(iInputBlockSize3);   pa0mbo

		/* Query vector from output transfer-buffer for writing */
		this->pvecOutputData = OutputBuffer.QueryWriteBuffer();

		/* Call the underlying processing-routine */
		this->ProcessDataInternal(Parameter);
	
		/* Write processed data from internal memory in transfer-buffer */
		OutputBuffer.Put(this->iOutputBlockSize);

		/* Data was provided, clear data request */
		OutputBuffer.SetRequestFlag(false);
	}
}

template<class TInput, class TOutput> void CTransmitterModul<TInput, TOutput>::ProcessData(CParameter& Parameter, CBuffer<TOutput>& OutputBuffer)
{
  // printf("Tx par Outp request  flag is %d\n", OutputBuffer.GetRequestFlag());
	/* OUTPUT-DRIVEN modul implementation in the transmitter ---------------- */
	/* Look in output buffer if data is requested */
	if (OutputBuffer.GetRequestFlag() == true)
	{
		/* Read data and write it in the transfer-buffer.
		   Query vector from output transfer-buffer for writing */

		this->pvecOutputData = OutputBuffer.QueryWriteBuffer();

		/* Call the underlying processing-routine */
		this->ProcessDataInternal(Parameter);
		
		/* Write processed data from internal memory in transfer-buffer */
		OutputBuffer.Put(this->iOutputBlockSize);

		/* Data was provided, clear data request */
		OutputBuffer.SetRequestFlag(false);
	}
}

template<class TInput, class TOutput>
void CTransmitterModul<TInput, TOutput>::
	ReadData(CParameter& Parameter, CBuffer<TOutput>& OutputBuffer)
{
        // printf("CTransmitterModul entry flag is %d\n", OutputBuffer.GetRequestFlag());
	/* OUTPUT-DRIVEN modul implementation in the transmitter ---------------- */
	/* Look in output buffer if data is requested */
        // printf("Tx ReadData request flag is %d \n", OutputBuffer.GetRequestFlag());
	if (OutputBuffer.GetRequestFlag() == true)
	{
		/* Read data and write it in the transfer-buffer.
		   Query vector from output transfer-buffer for writing */
		this->pvecOutputData = OutputBuffer.QueryWriteBuffer();

		/* Call the underlying processing-routine */
		this->ProcessDataInternal(Parameter);
		
		/* Write processed data from internal memory in transfer-buffer */
		OutputBuffer.Put(this->iOutputBlockSize);

		/* Data was provided, clear data request */
		OutputBuffer.SetRequestFlag(false);
	}
}

template<class TInput, class TOutput> _BOOLEAN CTransmitterModul<TInput, TOutput>:: 	WriteData(CParameter& Parameter, CBuffer<TInput>& InputBuffer)
{
	 // printf("WriteData fill %d Inpblk size %d \n",
	//		InputBuffer.GetFillLevel(), this->iInputBlockSize);  

	/* OUTPUT-DRIVEN modul implementation in the transmitter */
	/* Check, if enough input data is available */
	if (InputBuffer.GetFillLevel() < this->iInputBlockSize)
	{
		 // printf("set request flag transmitter module WriteData \n"); 
		/* Set request flag */
		InputBuffer.SetRequestFlag(true);

		return false;
	}
        // printf("Getting the data in TXModul from input buffer\n");
	/* Get vector from transfer-buffer */
	this->pvecInputData = InputBuffer.Get(this->iInputBlockSize);

	/* Call the underlying processing-routine */
	this->ProcessDataInternal(Parameter);

	return true;
}

#endif // !defined(AFX_MODUL_H__41E39CD3_2AEC_400E_907B_148C0EC17A43__INCLUDED_)
