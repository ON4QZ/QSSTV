
/*
*
*    file getfoffsint.c
*
*    implements get_frequency_offset_integer function
*    from diorama MATLAB code
*
*    Author M.Bos - PA0MBO
*
*    Date Feb 21st 2009
*
*    output:  freq_offset_integer (float)
*    inputs:  symbol_buffer (filled bu getofdm), N_symbols, K_dc,
*             K_modulo, Tu
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

#define PI (4.0*atan(1.0))
struct cmplxnmbr
{
  float re;
  float im;
};
float
getfoffsint(float *symbolbuf, int N_symbols, int K_dc, int K_modulo, int Tu)
{
  int i, j;
  struct cmplxnmbr S[288][30];	/* Tu x 30 ? pa0mbo? */
  float dS_sum[288 * 2];
  float tmp1, tmp2;
  int k_pilot1, k_pilot2, k_pilot3;
  float abs_dS_sum[288], pilot_indicator[288];
  float dummy, freq_offset_integer;
  int K_dc_offset;


  /* reshape symbolbuf to matrix S */
  for (i = 0; i < N_symbols; i++)

    {
      for (j = 0; j < K_modulo; j++)

	{
	  (S[j][i]).re = symbolbuf[(j + i * K_modulo) * 2];
	  (S[j][i]).im = symbolbuf[(j + i * K_modulo) * 2 + 1];
	}
    }

  /* now accumulate phase diffs all carriers */
  for (i = 0; i < Tu; i++)

    {
      dS_sum[i * 2] = 0.0;
      dS_sum[i * 2 + 1] = 0.0;
    }
  for (i = 1; i < N_symbols; i++)

    {
      for (j = 0; j < Tu; j++)

	{
	  tmp1 = (S[j][i - 1]).re * (S[j][i]).re +	/* real ac + bd */
	    (S[j][i - 1]).im * (S[j][i]).im;
	  tmp2 = (S[j][i - 1]).im * (S[j][i]).re -	/* imag. bc - ad */
	    (S[j][i - 1]).re * (S[j][i]).im;
	  dS_sum[j * 2] += cos(atan2(tmp2, tmp1));
	  dS_sum[j * 2 + 1] += sin(atan2(tmp2, tmp1));
	}
    }

  /* detect pilots */


  k_pilot1 = (int) ceil((float) (9 * Tu / 288));
  k_pilot2 = (int) ceil((float) (27 * Tu / 288));
  k_pilot3 = (int) ceil((float) (36 * Tu / 288));



  for (i = 0; i < Tu; i++)

    {
      abs_dS_sum[i] =
	(float) sqrt(dS_sum[i * 2] * dS_sum[i * 2] +
		     dS_sum[i * 2 + 1] * dS_sum[i * 2 + 1]);


    }

  for (i = 0; i < Tu - k_pilot1; i++)

    {
      pilot_indicator[i] = abs_dS_sum[k_pilot1 + i];
    }
  for (i = 0; i < k_pilot1; i++)

    {
      pilot_indicator[i + Tu - k_pilot1] = abs_dS_sum[i];
    }
  for (i = 0; i < Tu - k_pilot2; i++)

    {
      pilot_indicator[i] += abs_dS_sum[k_pilot2 + i];
    }
  for (i = 0; i < k_pilot2; i++)

    {
      pilot_indicator[i + Tu - k_pilot2] += abs_dS_sum[i];
    }
  for (i = 0; i < Tu - k_pilot3; i++)

    {
      pilot_indicator[i] += abs_dS_sum[k_pilot3 + i];
    }
  for (i = 0; i < k_pilot3; i++)

    {
      pilot_indicator[i + Tu - k_pilot3] += abs_dS_sum[i];
    }

  /* Now find max pilot_indicator and index */
  dummy = -1.0E20;
  K_dc_offset = 0;


  for (i = 0; i < Tu; i++)

    {


      if (pilot_indicator[i] > dummy)

	{
	  dummy = pilot_indicator[i];
	  K_dc_offset = i;
	}
    }

  K_dc_offset = ((K_dc_offset - K_dc + Tu / 2 + Tu) % Tu) - Tu / 2;
  freq_offset_integer = (float) (2 * PI * K_dc_offset);
  return (freq_offset_integer);
}
