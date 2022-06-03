
/*
*    File psdcmean.c
*
*    Calculates mean powerspectrum of complex input data
*    using blocklength lblock and nblocks as number
*    of blocks to be processed.
*
*    Author M.Bos - PA0MBO
*    Date Feb 21st 2009
*
*    resulting power spectral density of complex input
*    signal in rsbuf is stored in cpsd[]
*    length of datablock to be processed is lblock
*    nblocks is the nuber of blocks of input data
*    to be processed.
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
void cfft(float *, int, int);
void psdcmean(float *rsbuf, float *cpsd, int lblock, int nblocks)
{
  float *pinput;
  int i, j;
  float tmpinbuf[2048];
  float result[512];


  /*  check space */
  if (lblock > 1024)

    {
      printf("not enough temp space\b");
      exit(EXIT_FAILURE);
    }

  /* clear array result before accumulating new data */
  for (i = 0; i < (lblock / 2); i++)

    {
      result[i] = 0.0;
    }

  /* loop over all blocks taking care to keep track of ptr in input */
  pinput = rsbuf;
  for (j = 0; j < nblocks; j++)

    {

      /* Now fill tmpinbuf with converted data from input */
      for (i = 0; i < lblock; i++)

        {
          tmpinbuf[i * 2] = pinput[2 * i];
          tmpinbuf[i * 2 + 1] = pinput[2 * i + 1];
        }
      cfft(tmpinbuf, lblock / 2, 1);
      for (i = 1; i < lblock / 2; ++i)

        {
          result[i] +=
              sqrt(tmpinbuf[i * 2] * tmpinbuf[i * 2] +
                   tmpinbuf[i * 2 + 1] * tmpinbuf[i * 2 + 1]);
        }
      pinput += 2 * lblock;	/* update pointer in input data */
    }
  for (i = 0; i < lblock / 2; i++)

    {
      result[i] = (float) (10.0 * log(result[i] + 1.0e-8) / 2.305 - 14.0);
    }
  /* interchange halfs of cpsd buffer as in matlab code from
     plot_input_spectrum in diorama */
  for (i = 1; i < lblock / 4; i++)

    {
      cpsd[i] = result[lblock / 4 + i];
      cpsd[i + lblock / 4 - 1] = result[i];
    }
}
