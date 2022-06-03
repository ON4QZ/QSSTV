/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * adapted for ham sstv use Ties Bos - PA0MBO
 * Description:

 NOTE:
 This code is NOT speed optimized! We should calculate the CRC byte-wise and
 precalculate the results in a table (TODO!)

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

#include "CRC.h"


/* Implementation *************************************************************/
void CCRC::Reset(const int iNewDegree)
{
	/* Build mask of bit, which was shifted out of the shift register */
	iBitOutPosMask = 1 << iNewDegree;

	/* Index of vector storing the polynominals for CRC calculation */
	iDegIndex = iNewDegree - 1;

	/* Init state shift-register with ones. Set all registers to "1" with
	   bit-wise not operation */
	iStateShiftReg = ~uint32_t(0);
}

void CCRC::AddByte(const _BYTE byNewInput)
{
	for (int i = 0; i < SIZEOF__BYTE; i++)
	{
		/* Shift bits in shift-register for transistion */
		iStateShiftReg <<= 1;

		/* Take bit, which was shifted out of the register-size and place it
		   at the beginning (LSB)
		   (If condition is not satisfied, implicitely a "0" is added) */
		if ((iStateShiftReg & iBitOutPosMask) > 0)
			iStateShiftReg |= 1;

		/* Add new data bit to the LSB */
		if ((byNewInput & (1 << (SIZEOF__BYTE - i - 1))) > 0)
			iStateShiftReg ^= 1;

		/* Add mask to shift-register if first bit is true */
		if (iStateShiftReg & 1)
			iStateShiftReg ^= iPolynMask[iDegIndex];
	}
}

void CCRC::AddBit(const _BINARY biNewInput)
{
	/* Shift bits in shift-register for transistion */
	iStateShiftReg <<= 1;

	/* Take bit, which was shifted out of the register-size and place it
	   at the beginning (LSB)
	   (If condition is not satisfied, implicitely a "0" is added) */
	if ((iStateShiftReg & iBitOutPosMask) > 0)
		iStateShiftReg |= 1;

	/* Add new data bit to the LSB */
	if (biNewInput > 0)
		iStateShiftReg ^= 1;

	/* Add mask to shift-register if first bit is true */
	if (iStateShiftReg & 1)
		iStateShiftReg ^= iPolynMask[iDegIndex];
}

uint32_t CCRC::GetCRC()
{
	/* Return inverted shift-register (1's complement) */
	iStateShiftReg = ~iStateShiftReg;

	/* Remove bit which where shifted out of the shift-register frame */
	return iStateShiftReg & (iBitOutPosMask - 1);
}

_BOOLEAN CCRC::CheckCRC(const uint32_t iCRC)
{
	if (iCRC == GetCRC())
		return true;
	else
		return false;
}

CCRC::CCRC()
{
	/* These polynominals are used in the DRM-standard */
	iPolynMask[0] = 0;
	iPolynMask[1] = 1 << 1;
	iPolynMask[2] = 1 << 1;
	iPolynMask[4] = (1 << 1) | (1 << 2) | (1 << 4);
	iPolynMask[5] = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 5);
	iPolynMask[7] = (1 << 2) | (1 << 3) | (1 << 4);
	iPolynMask[15] = (1 << 5) | (1 << 12);
}



/******************************************************************************/
/*                                                                            */
/*  University of Kaiserslautern, Institute of Communications Engineering     */
/*  Copyright (C) 2004 Andreas Dittrich, Torsten Schorr                       */
/*                                                                            */
/*  Author(s)    : Andreas Dittrich (dittrich@eit.uni-kl.de),                 */
/*                 Torsten Schorr (schorr@eit.uni-kl.de)                      */
/*  Project start: 27.07.2004                                                 */
/*  Last change  : 27.07.2004                                                 */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*  crc16_bytewise.c                                                          */
/*                                                                            */
/******************************************************************************/
/*  Description:                                                              */
/*  CRC-16 checksum calculation of a byte stream                              */
/*  Usage:                                                                    */
/*                                                                            */
/*  crc16_bytewise(double *checksum, unsigned char *in, long N);                                         */
/*                                                                            */
/*  calculates double checksum of uint8 bytes                                 */
/*                                                                            */
/******************************************************************************/


/*************
*
*   adjusted for use in own plain C programa
*   by M.Bos - PA0MBO
*
*   Date Dec 9th 2007
*/


/******************************************************************************/
/* function                                                                   */
/******************************************************************************/
void CCRC::crc16_bytewise(double  checksum[],
                          unsigned char in[], long N)
{
  long int i;
  int j;
  unsigned int b = 0xFFFF;
  unsigned int x = 0x1021;	/* (1) 0001000000100001 */
  unsigned int y=0;

  for (i = 0; i < N - 2; i++)

    {
      for (j = 7; j >= 0; j--)
        {
          y = (((b >> 15) + (unsigned int) (in[i] >> j)) & 0x01) & 0x01;	/* extra parenth pa0mbo */
          if (y == 1)
            b = ((b << 1) ^ x);

          else
            b = (b << 1);
        }
    }
  for (i = N - 2; i < N; i++)

    {
      for (j = 7; j >= 0; j--)
        {
          y = (((b >> 15) + (unsigned int) ((in[i] >> j) & 0x01)) ^ 0x01) & 0x01;	/* extra parent pa0mbo */
          if (y == 1)
            b = ((b << 1) ^ x);

          else
            b = (b << 1);
        }
    }
  *checksum = (double) (b & 0xFFFF);
}

