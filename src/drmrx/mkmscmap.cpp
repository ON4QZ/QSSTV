
/*
*   file mkmscmap.c
*
*   makes MSC_Demapper
*
*   cf create_MSC_demapper.m matlab code by
*   Torsten Schorr 2004
*
*   recoded in C by PA0MBO - M.Bos
*
*   date Feb 21st 2009
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
#include <sys/types.h>
#include <math.h>
#include <malloc.h>
#include "drmdefs.h"
#include "structtemplates.h"
#include "drmproto.h"
extern int symbols_per_frame_list[4];
extern int time_ref_cells_k_list[4][21];
extern int y_list[4];
extern int K_min_K_max_list[2][24];
extern int freq_ref_cells_k_list[4][3];
extern int x_list[4];
extern int k0_list[4];
extern int mode_and_occupancy_code_table[24];
extern int time_ref_cells_cnt_list[4];
extern int lFAC;
extern int MSC_Demapper[6][2959];
int
mkmscmap(int robustness_mode, int spectrum_occupancy, int interleaver_depth,
	 int K_dc, int K_modulo)
{
  int frames_per_superframe = 3;
  int K_min;
  int K_max;
  int mode_and_occupancy_code;
  int x, y, k0;
  int s;
//  int Tu;
  int gain_ref_cells_k[712];
  int unused_carriers_k[3];
  int i, m, j;
  int first_symbol;
  int control_cells_k[65];
  int pilot_cells_k[600];
  int cnt_msc_cells = 0;
  int rndcnt, cnt_time_ref_cells;
  int n, k, p, p_min, p_max;
  int contains;
  int term;
  int N_SFA, N_L, N_MUX, D;
  int cnt_msc_cells_3_superframes;
  static int *PICMSC_inv = NULL;
  int step,  ncGIMSC;
//  int nrGIMSC;
  int columnv[5];
  int rowv[1000];
  int vproduct[5][1000];
  int GIMSC_inv[5000];
  int cnt_GIMSC;
  int Cell_Deinterleaver[5000];
  int FAC_cells_k[65];		/*   reeds global */
//  int Tu_list[] = { Tu_A, Tu_B, Tu_C, Tu_D };
  int symbols_per_frame;
  int freq_ref_cells_k[3];
  int time_ref_cells_k[21];
//  int carrier_per_symbol;
  int MSC_cells_k[8877];
  int MSC_cells_3_superframes[3 * 8877];

//  Tu = Tu_list[robustness_mode];
  K_min = K_min_K_max_list[0][spectrum_occupancy + robustness_mode * 6];
  K_max = K_min_K_max_list[1][spectrum_occupancy + robustness_mode * 6];
  mode_and_occupancy_code =
    mode_and_occupancy_code_table[robustness_mode * 6 + spectrum_occupancy];
  if (mode_and_occupancy_code < 0)

    {
      printf("BAD MODE AND OCCUPANCY CODE \n");
    }
  symbols_per_frame = symbols_per_frame_list[robustness_mode];
//  carrier_per_symbol = K_max - K_min + 1;
  x = x_list[robustness_mode];
  y = y_list[robustness_mode];
  k0 = k0_list[robustness_mode];
  rndcnt = 0;
  for (s = 0; s < symbols_per_frame; s++)

    {
      n = s % y;
      m = s / y;
      p_min = (int) ceil((double) ((K_min - k0 - x * n) / (x * y)));
      p_max = (K_max - k0 - x * n) / (x * y);
      for (p = p_min; p <= p_max; p++)

	{
	  k = k0 + x * n + x * y * p;
	  gain_ref_cells_k[rndcnt++] = k + s * K_modulo;

	  /* printf("gain_ref_cells_k[%d] = %d\n", rndcnt-1, gain_ref_cells_k[rndcnt-1]);  */
	}
    }
  unused_carriers_k[0] = 0;
  if (robustness_mode == 0)

    {
      unused_carriers_k[1] = 1;
    }
  for (i = 0; i < 3; i++)

    {
      freq_ref_cells_k[i] = freq_ref_cells_k_list[robustness_mode][i];
    }
  cnt_time_ref_cells = time_ref_cells_cnt_list[robustness_mode];
  for (i = 0; i < cnt_time_ref_cells; i++)

    {
      time_ref_cells_k[i] = time_ref_cells_k_list[robustness_mode][i];
    }
  lFAC = mkfacmap(robustness_mode, 0, K_modulo, FAC_cells_k);

  /* MSC cells per superframe */
  first_symbol = 0;
  cnt_msc_cells = 0;

  for (m = 0; m < frames_per_superframe; m++)

    {
      for (i = 0; i < lFAC; i++)

	{
	  control_cells_k[i] =
	    FAC_cells_k[i] + K_modulo * symbols_per_frame * m;

	  /* printf("control_cells_k[%d] = %d  FAC_cells_k %d K_modulo %d symbols_per_frame %d m %d \n",
	     i, control_cells_k[i], FAC_cells_k[i], K_modulo, symbols_per_frame, m);   */
	}
      for (j = 3; j < cnt_time_ref_cells + 3; j++)

	{
	  pilot_cells_k[j] =
	    K_modulo * symbols_per_frame * m + time_ref_cells_k[j - 3];
	}
      for (j = 3 + cnt_time_ref_cells; j < 3 + cnt_time_ref_cells + rndcnt;
	   j++)

	{
	  pilot_cells_k[j] =
	    K_modulo * symbols_per_frame * m + gain_ref_cells_k[j - 3 -
								cnt_time_ref_cells];
	}
      for (s = first_symbol; s < symbols_per_frame; s++)

	{
	  for (j = 0; j < 3; j++)

	    {
	      pilot_cells_k[j] =
		K_modulo * symbols_per_frame * m + K_modulo * s +
		freq_ref_cells_k[j];
	    }
	  for (k = K_min; k <= K_max; k++)

	    {
	      contains = 0;
	      term = k + K_modulo * symbols_per_frame * m + K_modulo * s;
	      for (j = 0; j < 3 + cnt_time_ref_cells + rndcnt; j++)

		{
		  if (term == pilot_cells_k[j])

		    {
		      contains = 1;
		      goto uit;
		    }
		}
	      for (j = 0; j < lFAC; j++)

		{
		  if (term == control_cells_k[j])

		    {
		      contains = 1;
		      goto uit;
		    }
		}
	      if (robustness_mode == 0)

		{
		  for (j = 0; j < 1; j++)

		    {
		      if (term ==
			  K_modulo * symbols_per_frame * m + K_modulo * s +
			  unused_carriers_k[j])

			{
			  contains = 1;
			  goto uit;
			}
		    }
		}

	      else

		{
		  if (term == K_modulo * symbols_per_frame * m + K_modulo * s)	/* pa0mbo 29-11-2007 was + unused_carriers_k[0] */

		    {
		      contains = 1;
		      goto uit;
		    }
		}
	    uit:if (contains == 0)

		{
		  MSC_cells_k[cnt_msc_cells++] = term;

		  /* debugging   
		     printf("%d  cell=%d \n", cnt_msc_cells, MSC_cells_k[cnt_msc_cells -1]);   */
		}
	    }			/* end k-loop */
	}			/* end s-loop */
      first_symbol = 0;
    }				/* end m -loop */
  N_SFA = cnt_msc_cells;
  N_L = N_SFA % frames_per_superframe;
  N_MUX = (N_SFA - N_L) / frames_per_superframe;


  /*  cell interleaver for MSC_cells */
  cnt_msc_cells_3_superframes = 0;
  for (i = 0; i < N_SFA - N_L; i++)

    {
      MSC_cells_3_superframes[cnt_msc_cells_3_superframes++] = K_dc + MSC_cells_k[i];	
    }
  for (i = 0; i < N_SFA - N_L; i++)

    {
      MSC_cells_3_superframes[cnt_msc_cells_3_superframes++] =
	K_dc + symbols_per_frame * frames_per_superframe * K_modulo +
	MSC_cells_k[i];
    }
  for (i = 0; i < N_SFA - N_L; i++)

    {
      MSC_cells_3_superframes[cnt_msc_cells_3_superframes++] =
	K_dc + 2 * symbols_per_frame * frames_per_superframe * K_modulo +
	MSC_cells_k[i];
    }

  /*  for (i=0; i < cnt_msc_cells_3_superframes; i++)
     printf("MSC_cells3..[%d] = %d\n", i, MSC_cells_3_superframes[i]);
     printf("====\n");    */
  if (PICMSC_inv != NULL) free(PICMSC_inv);
  PICMSC_inv = deinterleaver(0, 1, N_MUX, 5);


  if (interleaver_depth == 0)

    {

      /* convolutional deinterleaver 
         ETSI ES 201980 / 7.6 */
      D = 5;

      /* calc of GIMSC_inv in steps */
      /* first step [1:N_MUX+1:D*(N_MUX+1)] */
      step = N_MUX + 1;
//      nrGIMSC = D;
      ncGIMSC = (int) ceil((float) N_MUX / D);

      /* printf("ncGIMSC = %d\n", ncGIMSC); */
      columnv[0] = 1;
      rowv[0] = 0;
      for (i = 1; i < D; i++)
	columnv[i] = columnv[i - 1] + step;
      for (i = 1; i < ncGIMSC; i++)
	rowv[i] = rowv[i - 1] + D;
      for (i = 0; i < D; i++)

	{
	  for (j = 0; j < ncGIMSC; j++)

	    {
	      vproduct[i][j] = columnv[i] + rowv[j];
	    }
	}
      cnt_GIMSC = 0;
      for (j = 0; j < ncGIMSC; j++)

	{
	  for (i = 0; i < D; i++)

	    {
	      GIMSC_inv[cnt_GIMSC++] = vproduct[i][j];

	      /* printf("GIMSC_inv[%d] = %d \n", cnt_GIMSC-1,  vproduct[i][j]);  */
	    }
	}
      cnt_GIMSC--;

      /*  printf("xxx Cell_Deinterleaver\n");  */
      for (i = 0; i < N_MUX; i++)

	{
	  Cell_Deinterleaver[i] = GIMSC_inv[PICMSC_inv[i]];

	  /* printf("%d \n", Cell_Deinterleaver[i]);  */
	}

      /*  printf("xxx\n");   */
      for (i = 0; i < N_MUX; i++)

	{
	  MSC_Demapper[5][i] =
	    MSC_cells_3_superframes[Cell_Deinterleaver[i] - 1];
	  MSC_Demapper[0][i] =
	    ((MSC_cells_3_superframes[N_MUX + Cell_Deinterleaver[i] - 1] +
	      1) % (2 * symbols_per_frame * frames_per_superframe *
		    K_modulo)) - 1;
	  MSC_Demapper[1][i] =
	    ((MSC_cells_3_superframes[2 * N_MUX + Cell_Deinterleaver[i] - 1] +
	      1) % (2 * symbols_per_frame * frames_per_superframe *
		    K_modulo)) - 1;
	  MSC_Demapper[2][i] =
	    ((symbols_per_frame * frames_per_superframe * K_modulo +
	      MSC_cells_3_superframes[Cell_Deinterleaver[i] - 1] +
	      1) % (2 * symbols_per_frame * frames_per_superframe *
		    K_modulo)) - 1;
	  MSC_Demapper[3][i] =
	    ((symbols_per_frame * frames_per_superframe * K_modulo +
	      MSC_cells_3_superframes[N_MUX + Cell_Deinterleaver[i] - 1] +
	      1) % (2 * symbols_per_frame * frames_per_superframe *
		    K_modulo)) - 1;
	  MSC_Demapper[4][i] =
	    ((symbols_per_frame * frames_per_superframe * K_modulo +
	      MSC_cells_3_superframes[2 * N_MUX + Cell_Deinterleaver[i] - 1] +
	      1) % (2 * symbols_per_frame * frames_per_superframe *
		    K_modulo)) - 1;
	}
    }

  else

    {

      /* printf("xxx Cell_Deinterleaver\n");  */
      for (i = 0; i < N_MUX; i++)

	{
	  Cell_Deinterleaver[i] = PICMSC_inv[i];

	  /*    printf("%d \n", Cell_Deinterleaver[i]);  */
	}

      /*   printf("xxx\n");  */
      /*  printf("xxxx MSC_Demap[1]\n");  */
      for (i = 0; i < N_MUX; i++)

	{
	  MSC_Demapper[1][i] = MSC_cells_3_superframes[Cell_Deinterleaver[i]];	/* pa0mbo -1 binnen [] weggehaald */

	  /* printf("%d \n", MSC_Demapper[1][i]);  */
	  MSC_Demapper[2][i] =
	    (MSC_cells_3_superframes[N_MUX + Cell_Deinterleaver[i]]) % (2 *
									symbols_per_frame
									*
									frames_per_superframe
									*
									K_modulo);
	  MSC_Demapper[3][i] =
	    (MSC_cells_3_superframes[2 * N_MUX + Cell_Deinterleaver[i]]) %
	    (2 * symbols_per_frame * frames_per_superframe * K_modulo);
	  MSC_Demapper[4][i] =
	    (symbols_per_frame * frames_per_superframe * K_modulo +
	     MSC_cells_3_superframes[Cell_Deinterleaver[i]]) % (2 *
								symbols_per_frame
								*
								frames_per_superframe
								* K_modulo);
	  MSC_Demapper[5][i] =
	    (symbols_per_frame * frames_per_superframe * K_modulo +
	     MSC_cells_3_superframes[N_MUX +
				     Cell_Deinterleaver[i]]) % (2 *
								symbols_per_frame
								*
								frames_per_superframe
								* K_modulo);
	  MSC_Demapper[0][i] =
	    (symbols_per_frame * frames_per_superframe * K_modulo +
	     MSC_cells_3_superframes[2 * N_MUX +
				     Cell_Deinterleaver[i]]) % (2 *
								symbols_per_frame
								*
								frames_per_superframe
								* K_modulo);
	}
    }
  return N_MUX;
}
