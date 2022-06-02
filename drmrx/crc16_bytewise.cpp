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
/*  crc16_bytewise.c                                                          */
/*                                                                            */
/******************************************************************************/
/*  Description:                                                              */
/*  CRC-16 checksum calculation of a byte stream                              */
/*  Usage:                                                                    */
/*                                                                            */
/*  crc16_bytewise(double *checksum, unsigned char *in, long N);              */
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

#include <math.h>
#include <stdlib.h>


/******************************************************************************/

/* function                                                                   */

/******************************************************************************/
void crc16_bytewise(double /*@out@ */ checksum[],unsigned char in[], long N)
{
  long int i;
  int j;
  unsigned int b = 0xFFFF;
  unsigned int x = 0x1021;	/* (1) 0001000000100001 */
  unsigned int y;

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
