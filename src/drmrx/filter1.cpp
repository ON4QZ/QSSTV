
/*
*    File filter1.c
*
* implements filter routine a la MATLAB
* for real data in vector sigin[]
*
* PA0MBO - M.Bos
* Date Feb 21st 2009
*
* result is stored in y[]
* dataLen is number of elements in input (sigin) and mih is length of
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

void drmfilter1(float *sigin, float *out, float *coef, int dataLen, int coefLen)
{
  int i, j;


  for (i = 0; i < dataLen; i++)

    {
      out[i] = 0.0;
      for (j = 0; ((j <= i) && (j < coefLen)); j++)

        {
          out[i] += coef[j] * sigin[i - j];
        }
    }
}
