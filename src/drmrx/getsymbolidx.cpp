
/*
*
*    file  getsymbolidx.c
*
*    corresponds with matlab file get_symbol_index.m
*    from diorama-1.1.1
*    by Andreas Dittrich
* 
*    translated to C
*    by M.Bos  - PA0MBO
*
*    Date feb 21st 2009
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
struct cmplxnmbr
{
  float re;
  float im;
};


#define PI  (4.0*atan(1.00))
int
getsymbolidx(float *symbol_buffer, int symbols_per_frame,
	     int *time_ref_cells_k, int *time_ref_cells_theta_1024, int K_dc,
	     int K_modulo, int n_time_ref_cells)
{
  struct cmplxnmbr S[288][25];	/* max 24 symbols per frame */
  int i, j, n, symbol0;
  float sum_real_xx[25];
//  float sum_imag_xx[25];
  float tmp1real, tmp1imag, tmp2real, tmp2imag;
  int k1_index, k2_index;
  float phi1, phi2;
  float max_sum_xx;


  /* reshape symbol buffer to matrix S */
  for (i = 0; i < symbols_per_frame; i++)

    {
      for (j = 0; j < K_modulo; j++)

	{
	  (S[j][i]).re = symbol_buffer[(j + i * K_modulo) * 2];
	  (S[j][i]).im = symbol_buffer[(j + i * K_modulo) * 2 + 1];
	}
    }

  /* debugging 
     printf("ooo symbols buffer in get symbolidx\n");
     for (i=0; i < K_modulo; i++)
     printf("%g\n", sqrt( (S[i][0]).re* (S[i][0]).re+
     (S[i][0]).im*(S[i][0]).im));

     printf("oooo\n");  */
  for (i = 0; i < symbols_per_frame; i++)

    {
      sum_real_xx[i] = 0.0;
//      sum_imag_xx[i] = 0.0;
    }
  for (n = 0; n < n_time_ref_cells - 1; n++)

    {
      if (time_ref_cells_k[n + 1] - time_ref_cells_k[n] == 1)

	{
	  k1_index = K_dc + time_ref_cells_k[n];
	  k2_index = K_dc + time_ref_cells_k[n + 1];
	  phi1 = (2.0 * PI * time_ref_cells_theta_1024[n]) / 1024.0;
	  phi2 = (2.0 * PI * time_ref_cells_theta_1024[n + 1]) / 1024.0;

	  /*    printf("timeref[n] is %d timeref[n+1] is %d n is %d\n", 
	     time_ref_cells_k[n], time_ref_cells_k[n+1], n); 
	     printf("in getsymbolidx K_dc is %d k1_index %d k2_index %d phi1 %g phi2 %g\n",
	     K_dc, k1_index, k2_index, phi1, phi2);  */
	  for (j = 0; j < symbols_per_frame; j++)

	    {
	      tmp1real =
		(S[k1_index][j]).re * cos(phi1) +
		(S[k1_index][j]).im * sin(phi1);
	      tmp1imag =
		(S[k1_index][j]).im * cos(phi1) -
		(S[k1_index][j]).re * sin(phi1);
	      tmp2real =
		(S[k2_index][j]).re * cos(phi2) +
		(S[k2_index][j]).im * sin(phi2);
	      tmp2imag =
		(S[k2_index][j]).im * cos(phi2) -
		(S[k2_index][j]).re * sin(phi2);
	      sum_real_xx[j] += tmp1real * tmp2real + tmp1imag * tmp2imag;


/*           sum_imag_xx[j]  += tmp1imag*tmp2real - tmp1real*tmp2imag; */
	    }
	}
    }

  /* detmn of index of max in abs(sum..xx) */
  max_sum_xx = -1.0E20;
  symbol0 = 0;
  for (j = 0; j < symbols_per_frame; j++)

    {
      tmp1real = fabs(sum_real_xx[j]);

      /* debugging 
         printf("abs_sum_real_xx[%d] %g\n", j, tmp1real);   */
      if (tmp1real > max_sum_xx)

	{
	  max_sum_xx = tmp1real;
	  symbol0 = j;
	}
    }
  return (symbol0);
}
