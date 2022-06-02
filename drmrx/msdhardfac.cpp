
/******************************************************************************/

/*                                                                            */

/*  University of Kaiserslautern, Institute of Communications Engineering     */

/*  Copyright (C) 2004 Torsten Schorr                                         */

/*                                                                            */

/*  Author(s)    : Torsten Schorr (schorr@eit.uni-kl.de)                      */

/*  Project start: 15.06.2004                                                 */

/*  Last change  : 22.07.2004                                                 */

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

/*  msdhardfac.c                                                                */

/*                                                                            */

/******************************************************************************/

/*  Description:                                                              */

/*  Multi-Stage-Decoder for DRM QAM signals (iterations with hard decisions)  */

/*  Usage:                                                                    */

/*                                                                            */

/*  [LPhardout, HPhardout, VSPPhardout] =                                     */

/*                      msd_hard (received, H, N1, L, Lvspp,                  */

/*                      Deinterleaver,PL, maxiter, SDCorMSC);                 */

/*                                                                            */

/*  received: samples of an FAC, SDC or MSC frame                             */

/*  H: estimated channel transfer function                                    */

/*  N1: number of OFDM cells in the higher protected part (part A)            */

/*  L: number of information bits for Part A/B for each level in (2xl)-matrix */

/*  Lvspp: number of information bits in the very strongly protected part     */

/*  Deinterleaver: deinterleavers for each levels in (Nxl)-int32-matrix       */

/*  PL: Protection Levels for Part A/B for each level in (2xl)-matrix         */

/*  maxiter: maximum number of decoding iterations                            */

/*  SDCorMSC: 1 for SDC and MSC frames, 0 for FAC frames                      */

/*                                                                            */

/******************************************************************************/


/* 
*  changed filename to msdhardfac.c
*  and added new interface to accomodate
*  own C-language interface instead of 
*  Matlab interface
*
*  Author of changes M.Bos - PA0MBO
*  Date July 10th 2007
*  changed memory alignment of lastiter and malloc to prevent IRIX problem
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include "viterbi_decode.h"
#include "msd_hard.h"

#define ITER_BREAK
#define CONSIDERING_SNR

#ifdef CONSIDERING_SNR

#define ARG_INDEX_OFFSET 1
#define NARGS_RHS_STR "9"
#define NARGS_RHS 9

#else /*  */

#define ARG_INDEX_OFFSET 0
#define NARGS_RHS_STR "8"
#define NARGS_RHS 8

#endif /*  */

#define PROGNAME "msd_hard"

#define PRBS_INIT(reg) reg = 511;
#define PRBS_BIT(reg) ((reg ^ (reg >> 4)) & 0x1)
#define PRBS_SHIFT(reg) reg = (((reg ^ (reg >> 4)) & 0x1) << 8) | (reg >> 1)
int
msdhardfac(double /*@out@ */ *received_real, double /*@out@ */ *received_imag,
	   int Lrxdata, /*@out@ */ double *snr, int N1, double *L,
	   int dimL, int Lvspp, int *Deinterleaver, int *PL, int maxiter,
	   int SDCorMSC, double *facdata)
{
  double *received, *first_received, *L1, *L2, L1_real[10], *L2_real,
    *L1_imag, *L2_imag;
  double *PL1, *PL2, PL1_real[10], *PL2_real, *PL1_imag, *PL2_imag,
    *output_ptr, L_dummy[3] = { 0, 0, 0 };
  float *metric_real, *metric_imag, *metric, *first_metric, closest_one,
    closest_zero, sample, *llr, dist;
  char *memory_ptr, *viterbi_mem, *msd_mem, *hardpoints, *hardpoints_ptr,
    *lastiter, *infoout[3];
  int m, n, N, no_of_levels, iteration, diff;
  int sample_index, rp_real[3], rp_imag[3], *rp, level, subset_point,
    no_of_bits, error, msd_mem_size, viterbi_mem_size;
  int PRBS_reg;
  int HMmix = 0, HMsym = 0;
  int i;


#ifdef CONSIDERING_SNR
  double *signal_to_noise_ratio;
  float SNR;


#endif /*  */


/*  new interface to C-language */
  signal_to_noise_ratio = snr;
  no_of_levels = 1;
  for (i = 0; i < dimL; i++)

    {
      L1_real[i] = L[i];

      /* debugging 
         printf("L[%d] = %g , L1_real[%d] = %g\n", i, L[i], i,  L1_real[i]);  */
    }
  L2_real = L1_real + no_of_levels;
  L1_imag = L_dummy;
  L2_imag = L_dummy;
  for (i = 0; i < 2; i++)

    {
      PL1_real[i] = (double) PL[i];
    } PL2_real = PL1_real + no_of_levels;
  PL1_imag = PL2_real + no_of_levels;
  PL2_imag = PL1_imag + no_of_levels;	/* pa0mbo will not be OK has to be checked !! will do for the moment */

  /* debugging 
     printf("PL1_real[0]= %g, PL1_real[1]= %g, PL2_real[0]= %g, PL2_real[1]= %g, PL1_imag[0]= %g, PL2_imag[0]= %g\n",
     PL1_real[0], PL1_real[1], PL2_real[0], PL2_real[1],
     PL1_imag[0], PL2_imag[0]);   */
  SDCorMSC = ((0 - SDCorMSC) != 0);

  /* debugging 
     printf("SDCorMSC = %d\n", SDCorMSC);   */
  if (Lrxdata < 20)

    {
      printf("msdhardfac: length rxdata should be >= 20\n");
      exit(1);
    }
  N = Lrxdata;
  if (N < 20)
    {
      printf("msdhardfac: N  has to be >= 20!\n");
      exit(1);
    }
  if ((N1 < 0) || (N1 > N - 20))
    {
      printf("msdhardfac: N1 has to be >= 0!\n");
      exit(1);
    }
  if (Lvspp < 0)
    {
      printf("msdhardfac: Lvspp has to be >= 0!\n");
      exit(1);
    }
  if (maxiter < 0)
    {
      printf("msdhardfac: maxiter must not be negativ.");
      exit(1);
    }
  if (HMmix && (Lvspp == 0))
    {
      printf("msdhardfac:  HMmix requires Lvspp > 0.");
      exit(1);
    }

  /* printf("start mem alloc \n ");
     printf("N= %d, no_of_levels= %d\n", N, no_of_levels);  */

  /* memory allocation and initialization: */
  no_of_bits = 0;
  for (level = 0; level < no_of_levels; level++)
    {
      no_of_bits +=
	(int) L1_real[level] + (int) L2_real[level] + 6 +
	(int) L1_imag[level] + (int) L2_imag[level] + 6;

      /* printf(" --- level %d L1real %d L2real %d L1imga %d L2imag %d\n",
         level, (int)L1_real[level], (int)L2_real[level], (int)L1_imag[level], (int)L2_imag[level]);    */
    } msd_mem_size =
    2 * N * sizeof(float) + 2 * N * sizeof(char) + 2 * N * sizeof(char) +
    no_of_bits * sizeof(char);
  viterbi_mem_size =
    STATES * sizeof(float) + STATES * sizeof(float) +
    2 * N * STATES * sizeof(char);

  /* printf("msdhardfac: viterbi_mem_size is %d STATES is %d\n", viterbi_mem_size, STATES);   */
  if (received_imag == NULL)
    {
      memory_ptr =
	(char *) malloc(viterbi_mem_size + msd_mem_size + N * sizeof(double) +
			2);
      received_imag =
	(double *) (memory_ptr + viterbi_mem_size + msd_mem_size);
      memset(received_imag, 0, N * sizeof(double));
    }
  else
    {
      memory_ptr = (char *) malloc(viterbi_mem_size + msd_mem_size + 2);

      /* printf("msdhardfac: debugging memory_ptr alloc succeeded viterbi_size = %d mds_size=%d  end addr is %x\n",
         viterbi_mem_size, msd_mem_size, memory_ptr+ viterbi_mem_size+msd_mem_size); */
      if (memory_ptr == NULL)

	{
	  printf("msdhardfac: cannot malloc for memory_ptr\n");
	  exit(1);
	}
    }
  if (!memory_ptr)
    {
      printf("Failed memory request!\n");
      exit(1);
    }
  viterbi_mem = memory_ptr;
  msd_mem = memory_ptr + viterbi_mem_size;
  llr = (float *) msd_mem;
  hardpoints = (char *) (msd_mem + 2 * N * sizeof(float));
  lastiter =
    (char *) (msd_mem + 2 * N * sizeof(float) + 2 * N * sizeof(char) + 2);
  infoout[0] =
    (char *) (msd_mem + 2 * N * sizeof(float) + 2 * N * sizeof(char) + 2 +
	      2 * N * sizeof(char));
  infoout[1] = 0;
  for (m = 1; m < no_of_levels; m++)
    {
      infoout[m] =
	infoout[m - 1] + (int) L1_real[m - 1] + (int) L2_real[m - 1] + 6 +
	(int) L1_imag[m - 1] + (int) L2_imag[m - 1] + 6;

      /* debugging pa0mbo 
         printf("infoout[%d] = %p \n", m, infoout[m]);  */
    } memset(hardpoints, 0, 2 * N * sizeof(char));

  /* choosing partitioning type: */
  if (no_of_levels == 3)
    {
      if ((Lvspp != 0) && HMmix)
	{			/* HMmix 64-QAM */
	  metric_real = partitioning[1];
	  metric_imag = partitioning[0];
	  rp_real[0] =
	    (N - 12) -
	    RY[(int) PL2_real[0]] * ((N - 12) / RY[(int) PL2_real[0]]);
	  rp_real[1] =
	    ((N - N1) - 12) -
	    RY[(int) PL2_real[1]] * (((N - N1) - 12) / RY[(int) PL2_real[1]]);
	  rp_real[2] =
	    ((N - N1) - 12) -
	    RY[(int) PL2_real[2]] * (((N - N1) - 12) / RY[(int) PL2_real[2]]);
	  rp_imag[0] =
	    ((N - N1) - 12) -
	    RY[(int) PL2_imag[0]] * (((N - N1) - 12) / RY[(int) PL2_imag[0]]);
	  rp_imag[1] =
	    ((N - N1) - 12) -
	    RY[(int) PL2_imag[1]] * (((N - N1) - 12) / RY[(int) PL2_imag[1]]);
	  rp_imag[2] =
	    ((N - N1) - 12) -
	    RY[(int) PL2_imag[2]] * (((N - N1) - 12) / RY[(int) PL2_imag[2]]);
	}
      else if (Lvspp != 0)
	{			/* HMsym 64-QAM */
	  HMsym = 1;
	  metric_real = partitioning[1];
	  metric_imag = partitioning[1];
	  rp_real[0] =
	    (2 * N - 12) -
	    RY[(int) PL2_real[0]] * ((2 * N - 12) / RY[(int) PL2_real[0]]);
	  rp_real[1] =
	    (2 * (N - N1) - 12) -
	    RY[(int) PL2_real[1]] * ((2 * (N - N1) - 12) /
				     RY[(int) PL2_real[1]]);
	  rp_real[2] =
	    (2 * (N - N1) - 12) -
	    RY[(int) PL2_real[2]] * ((2 * (N - N1) - 12) /
				     RY[(int) PL2_real[2]]);
	}
      else
	{			/* SM 64-QAM */
	  metric_real = partitioning[0];
	  metric_imag = partitioning[0];
	  rp_real[0] =
	    (2 * (N - N1) - 12) -
	    RY[(int) PL2_real[0]] * ((2 * (N - N1) - 12) /
				     RY[(int) PL2_real[0]]);
	  rp_real[1] =
	    (2 * (N - N1) - 12) -
	    RY[(int) PL2_real[1]] * ((2 * (N - N1) - 12) /
				     RY[(int) PL2_real[1]]);
	  rp_real[2] =
	    (2 * (N - N1) - 12) -
	    RY[(int) PL2_real[2]] * ((2 * (N - N1) - 12) /
				     RY[(int) PL2_real[2]]);
    }}
  else if (no_of_levels == 2)
    {				/* SM 16-QAM */
      rp_real[0] =
	(2 * (N - N1) - 12) -
	RY[(int) PL2_real[0]] * ((2 * (N - N1) - 12) / RY[(int) PL2_real[0]]);
      rp_real[1] =
	(2 * (N - N1) - 12) -
	RY[(int) PL2_real[1]] * ((2 * (N - N1) - 12) / RY[(int) PL2_real[1]]);
      metric_real = partitioning[2];
      metric_imag = partitioning[2];
    }
  else
    {				/* SM 4-QAM */
      rp_real[0] =
	(2 * (N - N1) - 12) -
	RY[(int) PL2_real[0]] * ((2 * (N - N1) - 12) / RY[(int) PL2_real[0]]);
      metric_real = partitioning[3];
      metric_imag = partitioning[3];
  } if (!SDCorMSC)
    {
      rp_real[0] = -12;
      rp_real[1] = -12;
      rp_real[2] = -12;
    }
  if (Lvspp != 0)
    {
      L1_real[0] = 0;
      L2_real[0] = (double) Lvspp;
    }

  /* debugging pa0mbo 
     printf("=== voor  viterbi \n");
     for (i=0; i < 2*N ; i++)
     {
     printf("hadpoints[%d] = %d \n",i,  hardpoints[i]);
     }  */

  /* Multi-Stage Decoding: */

  /* first decoding: */
  PL1 = PL1_real;
  PL2 = PL2_real;
  L1 = L1_real;
  L2 = L2_real;
  rp = rp_real;
  first_metric = metric_real;
  first_received = received_real;
  hardpoints_ptr = hardpoints;

  /* debugging 
     printf("msdhardfac: at start first decoding\n");
     printf("PL1[0] = %g, PL2[0]= %g, L1_real[0]= %g, L2_real[0]=%g, L1[0]=%g, L2[0]= %g, rp[0]= %d\n",
     PL1[0], PL2[0], L1_real[0], L2_real[0], L1[0], L2[0], rp[0]);   */
  for (n = 0; n <= HMmix; n++)

    {
      for (level = 0; level < no_of_levels; level++)

	{
	  metric = first_metric;
	  received = first_received;
	  for (m = 0; m < 2 - HMmix; m++)

	    {			/* for real and imaginary part */
	      for (sample_index = m; sample_index < (2 - HMmix) * N;
		   sample_index += 2 - HMmix)

		{
		  sample = (float) received[sample_index >> (1 - HMmix)];	/* extract real or imaginary part respectively */
		  closest_zero =
		    fabs(sample - metric[(int) hardpoints_ptr[sample_index]]);

		  /* printf("msdhardfac: index= %d  sample = %g metric = %g closest_zero = %g \n",
		     sample_index, sample,  metric[hardpoints_ptr[sample_index]], closest_zero);   pa0mbo */
		  for (subset_point = (0x1 << (level + 1));
		       subset_point < (0x1 << no_of_levels);
		       subset_point += (0x1 << (level + 1)))

		    {
		      dist =
			fabs(sample -
			     metric[hardpoints_ptr[sample_index] +
				    subset_point]);
		      if (dist < closest_zero)

			{
			  closest_zero = dist;
			}
		    }
		  closest_one =
		    fabs(sample -
			 metric[hardpoints_ptr[sample_index] +
				(0x1 << level)]);

		  /* printf("closest_one %g\n", closest_one);  pa0mbo */
		  for (subset_point = (0x3 << level);
		       subset_point < (0x1 << no_of_levels);
		       subset_point += (0x1 << (level + 1)))

		    {
		      dist =
			fabs(sample -
			     metric[hardpoints_ptr[sample_index] +
				    subset_point]);
		      if (dist < closest_one)

			{
			  closest_one = dist;
			}
		    }

		  /* printf("final closest_zero=%g closest_one=%g\n", closest_zero, closest_one);  pa0mbo */

#ifdef CONSIDERING_SNR
		  SNR =
		    (float) signal_to_noise_ratio[sample_index >>
						  (1 - HMmix)];
		  llr[sample_index] = (closest_zero - closest_one) * SNR;

		  /* printf("llr[%d] = %g\n", sample_index, llr[sample_index]);   */

		  /* llr[sample_index] = (closest_zero*closest_zero - closest_one*closest_one) * SNR * SNR; */
#else /*  */
		  llr[sample_index] = (closest_zero - closest_one);

		  /* llr[sample_index] = (closest_zero*closest_zero - closest_one*closest_one); */
#endif /*  */
		}		/* end loop sample_index */
	      metric = metric_imag;
	      received = received_imag;
	    }			/* end loop m */

	  /* printf(" level %d HMsym %d HMmix %d N1 %d n %d\n", level, HMsym, HMmix, N1, n);  
	     printf("msdhardfac: (level || (!HMsym ..) = %d\n", (level || (!HMsym && (n || !HMmix)))* (2 - HMmix)*N1 ); 
	     printf("eerste viter PL1[0] %g PL2[0] %g L1[0] %g L2[0] %g L1_real[0] %g L2_real[0] %g rp[0] %d\n",
	     PL1[0], PL2[0], L1[0], L2[0], L1_real[0], L2_real[0], rp[0]);  
	     for (i=0; i < 17 ; i++)
	     printf("puncturing[6][%d] = %d\n", i, puncturing[6][i]);   */
	  error =
	    viterbi_decode(llr, (2 - HMmix) * N,
			   (level
			    || (!HMsym
				&& (n
				    || !HMmix))) * (2 - HMmix) * N1,
			   puncturing[(int) PL1[level]],
			   puncturing[(int) PL2[level]],
			   tailpuncturing[rp[level] + 12],
			   infoout[level] + n * ((int) L1_real[level] +
						 (int) L2_real[level] + 6),
			   hardpoints_ptr, level,
			   Deinterleaver + (2 - HMmix) * N * level,
			   (int) L1[level] + (int) L2[level] + 6,
			   rp[level] + 12, viterbi_mem);

	  /* debugging pa0mbo 
	     printf("=== na eerste viterbi \n");
	     for (i=0; i < 2*N ; i++)
	     {
	     printf("infoout[0][%d] = %d \n",i,  infoout[0][i]);
	     }   */
	  if (error)

	    {
	      free(memory_ptr);
	      printf("msdhardfac: Error in Viterbi decoder");
	      return 1;
	    }
	}			/* end loop level */
      PL1 = PL1_imag;
      PL2 = PL2_imag;
      L1 = L1_imag;
      L2 = L2_imag;
      rp = rp_imag;
      first_metric = metric_imag;
      first_received = received_imag;
      hardpoints_ptr = hardpoints + N;
    }				/* end loop over n */
  diff = 1;
  iteration = 0;

  /* iterations: */
  while (iteration < maxiter)

    {
      PL1 = PL1_real;
      PL2 = PL2_real;
      L1 = L1_real;
      L2 = L2_real;
      rp = rp_real;
      first_metric = metric_real;
      first_received = received_real;
      hardpoints_ptr = hardpoints;

#ifdef ITER_BREAK
      memcpy(lastiter, hardpoints, 2 * N);

#endif /*  */
      for (n = 0; n <= HMmix; n++)

	{
	  for (level = 0; level < no_of_levels; level++)

	    {
	      metric = first_metric;
	      received = first_received;
	      for (m = 0; m < 2 - HMmix; m++)

		{		/* for real and imaginary part */
		  for (sample_index = m; sample_index < (2 - HMmix) * N;
		       sample_index += 2 - HMmix)

		    {
		      sample = (float) received[sample_index >> (1 - HMmix)];	/* extract real or imaginary part respectively */
		      closest_zero =
			fabs(sample -
			     metric[hardpoints_ptr[sample_index] &
				    ~(0x1 << level)]);
		      closest_one =
			fabs(sample -
			     metric[hardpoints_ptr[sample_index] |
				    (0x1 << level)]);

#ifdef CONSIDERING_SNR
		      SNR =
			(float) signal_to_noise_ratio[sample_index >>
						      (1 - HMmix)];
		      llr[sample_index] = (closest_zero - closest_one) * SNR;

		      /* llr[sample_index] = (closest_zero*closest_zero - closest_one*closest_one) * SNR * SNR; */
#else /*  */
		      llr[sample_index] = (closest_zero - closest_one);

		      /* llr[sample_index] = (closest_zero*closest_zero - closest_one*closest_one); */
#endif /*  */
		    }		/* end loop over sample_index */
		  metric = metric_imag;
		  received = received_imag;
		}		/* end loop over m */

	      /* printf("Tweede viterbi PL1[0] %g PL2[0] %g L1[0] %g L2[0] %g L1_real[0] %g L2_real[0] %g rp[0] %d\n",
	         PL1[0], PL2[0], L1[0], L2[0], L1_real[0], L2_real[0], rp[0]);   */
	      error =
		viterbi_decode(llr, (2 - HMmix) * N,
			       (level
				|| (!HMsym
				    && (n
					|| !HMmix))) * (2 - HMmix) * N1,
			       puncturing[(int) PL1[level]],
			       puncturing[(int) PL2[level]],
			       tailpuncturing[rp[level]],
			       infoout[level] + n * ((int) L1_real[level] +
						     (int) L2_real[level] +
						     6), hardpoints_ptr,
			       level, Deinterleaver + (2 - HMmix) * N * level,
			       (int) L1[level] + (int) L2[level] + 6,
			       rp[level] + 12, viterbi_mem);
	      if (error)

		{
		  free(memory_ptr);
		  printf("msdhardfac: Error in Viterbi decoder");
		  return 1;
		}

#ifdef ITER_BREAK
	      if (level == 0)

		{
		  diff = 0;
      for (sample_index = 0;sample_index <((int)(((2 - HMmix) * N * sizeof(char)) / sizeof(int))); sample_index++)

		    {
		      diff +=
			(((int *) hardpoints)[sample_index] ^
			 ((int *) lastiter)[sample_index]) != 0;
		    }
		  /*diff = memcmp (lastiter,hardpoints,2 * N); */
		  if (!diff)

		    {
		      break;
		    }
		}

#endif /*  */
	    }			/* for (level = 0; level < no_of_levels; level++) */
	  PL1 = PL1_imag;
	  PL2 = PL2_imag;
	  L1 = L1_imag;
	  L2 = L2_imag;
	  rp = rp_imag;
	  first_metric = metric_imag;
	  first_received = received_imag;
	  hardpoints_ptr = hardpoints + N;
	}			/* for (n = 0; n <= HMmix; n++) */

#ifdef ITER_BREAK
      if (!diff)

	{
	  break;
	}

#endif /*  */
      iteration++;
    }				/* while (iteration < maxiter) */

  /* Energy Dispersal */
  no_of_bits = 0;
  for (level = (Lvspp != 0); level < no_of_levels; level++)
    {
      no_of_bits += (int) L1_real[level] + (int) L2_real[level];
  } for (level = 0; level < no_of_levels; level++)
    {
      no_of_bits += (int) L1_imag[level] + (int) L2_imag[level];
    } output_ptr = facdata;
  PRBS_INIT(PRBS_reg);
  n = 0;
  if (HMmix)
    {
      for (m = Lvspp + 6; m < Lvspp + 6 + (int) L1_imag[0]; m++)
	{
	  output_ptr[n++] = (double) (infoout[0][m] ^ PRBS_BIT(PRBS_reg));
	  PRBS_SHIFT(PRBS_reg);
    }}
  for (level = (Lvspp != 0); level < no_of_levels; level++)
    {
      for (m = 0; m < (int) L1_real[level]; m++)
	{
	  output_ptr[n++] = (double) (infoout[level][m] ^ PRBS_BIT(PRBS_reg));
	  PRBS_SHIFT(PRBS_reg);
      } for (m = (int) L1_real[level] + (int) L2_real[level] + 6;
	       m <
	       (int) L1_real[level] + (int) L2_real[level] + 6 +
	       (int) L1_imag[level]; m++)
	{
	  output_ptr[n++] = (double) (infoout[level][m] ^ PRBS_BIT(PRBS_reg));
	  PRBS_SHIFT(PRBS_reg);
  }} if (HMmix)
    {
      for (m = Lvspp + 6 + (int) L1_imag[0];
	   m < Lvspp + 6 + (int) L1_imag[0] + (int) L2_imag[0]; m++)
	{
	  output_ptr[n++] = (double) (infoout[0][m] ^ PRBS_BIT(PRBS_reg));
	  PRBS_SHIFT(PRBS_reg);
    }}
  for (level = (Lvspp != 0); level < no_of_levels; level++)
    {
      for (m = (int) L1_real[level];
	   m < (int) L1_real[level] + (int) L2_real[level]; m++)
	{
	  output_ptr[n++] = (double) (infoout[level][m] ^ PRBS_BIT(PRBS_reg));
	  PRBS_SHIFT(PRBS_reg);
      } for (m =
	       (int) L1_real[level] + (int) L2_real[level] + 6 +
	       (int) L1_imag[level];
	       m <
	       (int) L1_real[level] + (int) L2_real[level] + 6 +
	       (int) L1_imag[level] + (int) L2_imag[level]; m++)
	{
	  output_ptr[n++] = (double) (infoout[level][m] ^ PRBS_BIT(PRBS_reg));
	  PRBS_SHIFT(PRBS_reg);
    }} PRBS_INIT(PRBS_reg);
  if (Lvspp != 0)
    {
      printf
	("msdhardfac: There is a very strongly protected part, but no variable to put it into!");
    }
  free(memory_ptr);
  return 0;
}
