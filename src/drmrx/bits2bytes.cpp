/******************************************************************************/
/*                                                                            */
/*  University of Kaiserslautern, Institute of Communications Engineering     */
/*  Copyright (C) 2004 Torsten Schorr                                         */
/*                                                                            */
/*  Author(s)    : Torsten Schorr (schorr@eit.uni-kl.de)                      */
/*  Project start: 23.07.2004                                                 */
/*  Last change  : 23.07.2004                                                 */
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
/*  bits2bytes.c                                                              */
/*                                                                            */
/******************************************************************************/
/*  Description:                                                              */
/*  Deinterleaver/Interleaver generation for DRM frames                       */
/*  Usage:                                                                    */
/*                                                                            */
/*  bytes = bits2bytes(bits);                                                 */
/*                                                                            */
/*  converts a serial double bit-stream into a uint8 byte-stream              */
/******************************************************************************/


/*
*   modified for use directly in  C-language without Matlab interface
*   M.Bos - PA0MBO
*   Date Nov 15th 2007
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

void bits2bytes(double *inbits, int N, unsigned char /*@out@ */ *outbytes)
{
  unsigned char single_byte;
  int m, n;

  if (N % 8 != 0)
    {
      return;
    }
  for (m = 0; m < N / 8; m++)
    {
      single_byte = '\0';
      for (n = 7; n >= 0; n--)
	{
	  single_byte |=
	    ((fabs(inbits[8 * m + 7 - n]) > DBL_EPSILON) & 0x01) << n;
	}
      outbytes[m] = single_byte;
    }
  return;
}
