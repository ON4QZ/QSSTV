
/*
*    File filter1c.c
*
* implements filter routine a la MATLAB
*
* PA0MBO - M.Bos
* Date Feb 21st 2009
*
* input signal is complex and should be stored
* real 1 / imag  component 2 , real 2, imag2 etc..
* 
* result is stored in y[] in the same manner (y[] is alsdo complex)
* dataLen is number of elements in input (sigin) and coefLen is length of
* the filter vector h[]
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
void drmfilter1c(float *sigin, float *y, float *coef, int dataLen, int coefLen)
{
  int i, j;

  for (i = 0; i < dataLen; i++)

    {
      y[i * 2] = 0.0;
      y[i * 2 + 1] = 0.0;
      for (j = 0; ((j <= i) && (j < coefLen)); j++)

        {
          y[i * 2] += coef[j] * sigin[(i - j) * 2];
          y[i * 2 + 1] += coef[j] * sigin[(i - j) * 2 + 1];
        }
    }
}
