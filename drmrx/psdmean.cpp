
/*
*    File psdmean.c
*
*    Calculates mean powerspectrum of input data
*    using blocklength lblock and nblocks as number
*    of blocks to be processed.
*
*    Author M.Bos - PA0MBO
*    Date Feb 21st 2009
*
*    resulting power spectral density of real input data
*    vector input[]  is stored in psd[]
*    lblock is the length of the block of data to be
*    processed and nblock is the number of data blocks
*    to be processed
*   
*/

/*************************************************************************
*
*                           PA0MBO
*
*    COPYRIGHT (C)  2009  M.Bos 
*
*    This file is part of the distribution package RXAMADRM
*
*    This package is free software and you can redistribute is
*    and/or modify it under the terms of the GNU General Public License
*
*    More details can be found in the accompanying file COPYING
*************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <malloc.h>
void rfft(float *, int, int);
void psdmean(float *input, float *psd, int lblock, int nblocks)
{
  float *pinput;
  int i, j;
  float tmpinbuf[1024];


  /* check for space allocated */
  if (lblock > 1024)

    {
      printf("lblokc param too large in call psdmean\n");
      exit(EXIT_FAILURE);
    }

  /* clear array psd before accumulating new data */
  for (i = 0; i < lblock / 2; i++)

    {
      psd[i] = 0.0;
    }

  /* loop over all blocks taking care to keep track of ptr in input */
  pinput = input;
  for (j = 0; j < nblocks; j++)

    {

      /* Now fill tmpinbuf with converted data from input */
      for (i = 0; i < lblock; i++)

        {
          tmpinbuf[i] = pinput[i];
        }
      rfft(tmpinbuf, lblock / 2, 1);
      for (i = 1; i < lblock / 2; ++i)

        {
          psd[i] += sqrt(tmpinbuf[i * 2] * tmpinbuf[i * 2] + tmpinbuf[2 * i + 1] * tmpinbuf[2 * i + 1]);
        }
      pinput += lblock;		/* update pointer in input data */
    }
  for (i = 1; i < lblock / 2; ++i)
    psd[i] = (float) (10.0 * log(psd[i] + 1.0e-8) / 2.3025 - 14.0);
}
