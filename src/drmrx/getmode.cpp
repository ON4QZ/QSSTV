
/*
*    File getmode.c
*
*    Author M.Bos - PA0MBO
*    Date Feb 21st 2009
*
*    routine to determine robustness mode
*    of baseband drm signal (complex)
*    in "in"  real and imag components
*    stored alternatively in sequence
*
*    returns number of the mode A=0, B=1, etc...
*    input param. n is number of samples in rsbuf
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
#include "structtemplates.h"
#include "drmproto.h"
#include "drmdefs.h"
#include "appglobal.h"
#include "supportfunctions.h"

static /*@only@ */ float *in_, *abs_in_, *abs_in_in_;
static /*@only@ */ float *conv_in_, *conv_abs_in_in_;
static bool initDone=false;

void initGetmode(int n)
{
  if(initDone) return;
  initDone=true;
  /* create and initialize  arrays */
  in_ =(float *) malloc((n - Tu_D) * 2 * sizeof(float));	/* complex data */
  if (in_ == NULL)

    {
      printf("mem alloc problem in getmode \n");
      exit(EXIT_FAILURE);
    }
  conv_in_ = (float *)malloc((n - Tu_D) * 2 * sizeof(float));	/* complex */
  if (conv_in_ == NULL)

    {
      printf("mem alloc problem in getmode \n");
      exit(EXIT_FAILURE);
    }
  abs_in_ = (float *)malloc(n * sizeof(float));
  if (abs_in_ == NULL)

    {
      printf("mem alloc problem in getmode \n");
      exit(EXIT_FAILURE);
    }
  abs_in_in_ = (float *)malloc((n - Tu_D) * sizeof(float));
  if (abs_in_in_ == NULL)

    {
      printf("mem alloc problem in getmode \n");
      exit(EXIT_FAILURE);
    }
  conv_abs_in_in_ = (float *)malloc((n - Tu_D) * sizeof(float));
  if (conv_abs_in_in_ == NULL)

    {
      printf("mem alloc problem in getmode \n");
      exit(EXIT_FAILURE);
    }
  return;
}				/* end initializations */


void getmode(float *input, int n, smode_info * result)
{
  float EPSILON = 1.0E-10;
  float SNR_mode_det = (float) (exp(15.0 * log(10) / 10.0));
  float rho;
  int Ts_list[4] = { Ts_A, Ts_B, Ts_C, Ts_D };
  int Tu_list[4] = { Tu_A, Tu_B, Tu_C, Tu_D };
  float max_abs_gamma_rel_list[] = { 0.0, 0.0, 0.0, 0.0 };
  int theta_list[] = { 0, 0, 0, 0 };
  float epsilon_ML_list[] = { 0.0, 0.0, 0.0, 0.0 };
  int N_symbols_mode_det, Ts, Tu, Tg, t_smp;

  int mode, i, j, theta, maxOK;
  float max_abs_gamma_rel, tmpmax, epsilon_ML;
  //  float frequency_offset_fract;
  int b[20], time_offset_mean;
  float sumx, sumy, sumxx, sumxy, slope, boffs;
  //  float a[20];
  float gamma[2 * Ts_A], Phi[Ts_A];
  float my_rect[Ts_D - Tu_D];

  rho = SNR_mode_det / (SNR_mode_det + 1);
  N_symbols_mode_det = ((n + 1) / Ts_A) - 1;
  for (mode = 0; mode < 3; mode++) // ON4QZ 3 was 4
    {
      Ts = Ts_list[mode];
      Tu = Tu_list[mode];
      Tg = Ts - Tu;
      t_smp = 0;

      /* initialize arrays with zero's */
      for (i = 0; i < Ts; i++)
        {
          gamma[i * 2] = 0.0;
          gamma[i * 2 + 1] = 0.0;
          Phi[i] = 0.0;
        }
      for (i = 0; i < n - Tu; i++)	/* complex mult */
        {
          in_[2 * i] = input[2 * i] * input[(i + Tu) * 2] + input[2 * i + 1] * input[(i + Tu) * 2 +1];
          in_[2 * i + 1] = -input[2 * i] * input[(i + Tu) * 2 + 1] + input[2 * i + 1] * input[(i + Tu) * 2];
        }
//      arrayDump("gM1",input,16,true);
      my_rect[0] = 0.5; // ON4QZ
      for (i = 1; i < Tg; i++)
        {
          my_rect[i] = 1.0;
        }
//       my_rect[Tg-1] = 0.5; // ON4QZ

      drmfilter1c(in_, conv_in_, my_rect, n - Tu, Tg);
      for (i = 0; i < n; i++)
        {
          abs_in_[i] = input[i * 2] * input[i * 2] + input[i * 2 + 1] * input[i * 2 + 1];
        }
      for (i = 0; i < n - Tu; i++)
        {
          abs_in_in_[i] = abs_in_[i] + abs_in_[i + Tu];
        }
      drmfilter1(abs_in_in_, conv_abs_in_in_, my_rect, n - Tu, Tg);
      for (j = 0; j < N_symbols_mode_det; j++)
        {
          for (i = 0; i < Ts; i++)
            {
              gamma[i * 2] = gamma[i * 2] + conv_in_[(t_smp + Tg + i - 1) * 2];	/* pa0mbo -1 ios nieuw */
              gamma[i * 2 + 1] = gamma[i * 2 + 1] + conv_in_[(t_smp + Tg + i - 1) * 2 + 1];
              Phi[i] =  Phi[i] +  (float) (0.5 * (EPSILON + conv_abs_in_in_[t_smp + Tg + i - 1]));
            }
          t_smp += Ts;
        }
      /* detmn max and index in abs(gamma .. rho*Phi) */
      theta = 0;
      max_abs_gamma_rel = -1.0E20;

      /* debugging
         printf("==== mode %d === gamma\n",mode);   */
      for (i = 0; i < Ts; i++)
        {
          tmpmax = (float) sqrt(gamma[2 * i] * gamma[2 * i] + gamma[2 * i + 1] * gamma[2 * i + 1]);

          /* printf("%g\n", tmpmax);  */
          tmpmax -= rho * Phi[i];
          if (tmpmax > max_abs_gamma_rel)
            {
              max_abs_gamma_rel = tmpmax;
              theta = i;
            }
        }
   //   arrayDump("gM2",gamma,Ts*2,true);

      /*  printf("===============\n");    */
      max_abs_gamma_rel = (float) sqrt(gamma[theta * 2] * gamma[theta * 2] + gamma[theta * 2 + 1] * gamma[theta * 2 + 1]) / (rho * Phi[theta]);
      epsilon_ML = (float) atan2(gamma[2 * theta], gamma[2 * theta + 1]);
      max_abs_gamma_rel_list[mode] = max_abs_gamma_rel;
      theta_list[mode] = theta;
      epsilon_ML_list[mode] = epsilon_ML;
    }
  /* debugging
  printf("max gamma list %g %g %g %g \n", max_abs_gamma_rel_list[0],
   max_abs_gamma_rel_list[1], max_abs_gamma_rel_list[2],
   max_abs_gamma_rel_list[3]);
  printf("theta list %d %d %d %d \n", theta_list[0], theta_list[1],
   theta_list[2], theta_list[3]);
  printf("epsilon ML  list %g %g %g %g \n", epsilon_ML_list[0], epsilon_ML_list[1], epsilon_ML_list[2], epsilon_ML_list[3]);	 end debug info */

  /* now decide for particular mode */
  max_abs_gamma_rel = -1.0E20;
  for (i = 0; i < 4; i++)
    {
      if (max_abs_gamma_rel_list[i] > max_abs_gamma_rel)
        {
          max_abs_gamma_rel = max_abs_gamma_rel_list[i];
          mode = i;
        }
    }
//              <<max_abs_gamma_rel_list[1] << max_abs_gamma_rel_list[2] << max_abs_gamma_rel_list[3];
  /* check if result is reliable */
  maxOK = 1;			/* start with reliable */
  if (max_abs_gamma_rel > 0.6) // was 0.6
    {
      for (i = 0; i < 3; i++)  //ON4QZ 3 was 4
        {
          if ((i != mode) && (max_abs_gamma_rel_list[i] > 0.6))
            {
              maxOK = 0;
            }
        }
    }
  else
    {
      maxOK = 0;
    }
  if (maxOK == 0)
    {
      result->mode_indx = 99;
      result->time_offset = 0.0;
      result->sample_rate_offset = 0.0;
      result->freq_offset_fract = 0.0;
      return;
    }
  else
    {
      addToLog("max mode ok",LOGDRMDEMOD);
      Ts = Ts_list[mode];
      Tu = Tu_list[mode];
      Tg = Ts - Tu;
      time_offset_mean = theta_list[mode];	/* pa0mbo checked  +1 removed on Dec 22nd 2006  */
      //      frequency_offset_fract = epsilon_ML_list[mode];
      /* now recalculate several vars with the established mode */
      for (i = 0; i < n - Tu; i++)
        {
          in_[2 * i] = input[2 * i] * input[(i + Tu) * 2] + input[2 * i + 1] * input[(i + Tu) * 2 + 1];
          in_[2 * i + 1] = -input[2 * i] * input[(i + Tu) * 2 + 1] + input[2 * i + 1] * input[(i + Tu) * 2];
        }
      my_rect[0] = 0.5;
      for (i = 1; i < Tg; i++)
        {
          my_rect[i] = 1.0;
        }
      my_rect[Tg - 1] = 0.5;
      drmfilter1c(in_, conv_in_, my_rect, n - Tu, Tg);
      for (i = 0; i < n; i++)
        {
          abs_in_[i] = input[i * 2] * input[i * 2] + input[i * 2 + 1] * input[i * 2 + 1];
        }
      for (i = 0; i < n - Tu; i++)
       {
          abs_in_in_[i] = abs_in_[i] + abs_in_[i + Tu];
        }
      drmfilter1(abs_in_in_, conv_abs_in_in_, my_rect, n - Tu, Tg);
      t_smp = Tg + time_offset_mean + Ts / 2;
      for (j = 0; j < (N_symbols_mode_det - 2); j++)
        {
          max_abs_gamma_rel = -1.0E20;
          for (i = 0; i < Ts; i++)
            {
              gamma[i * 2] = conv_in_[(t_smp + i) * 2];
              gamma[i * 2 + 1] = conv_in_[(t_smp + i) * 2 + 1];
              Phi[i] = (float) (0.5 * (EPSILON + conv_abs_in_in_[t_smp + i]));

              /* detmn max and its indx */
              tmpmax =
                  (float) sqrt(gamma[2 * i] * gamma[2 * i] +
                               gamma[2 * i + 1] * gamma[2 * i + 1]) -
                  rho * Phi[i];
              if (tmpmax > max_abs_gamma_rel)

                {
                  max_abs_gamma_rel = tmpmax;
                  //                  a[j] = tmpmax;
                  b[j] = i;
                }
            }
          t_smp += Ts;
        }

      /* Now least squares to 0...N_symbols_mode_det-3 and b[0] .. */
      sumx = 0.0;
      sumy = 0.0;
      sumxx = 0.0;
      sumxy = 0.0;
      for (i = 0; i < N_symbols_mode_det - 2; i++)

        {
          sumx += (float) i;
          sumy += (float) b[i];
          sumxx += (float) i *(float) i;
          sumxy += (float) i *(float) b[i];
        }
      slope = (float) (((N_symbols_mode_det - 2) * sumxy - sumx * sumy) / ((N_symbols_mode_det - 2) * sumxx - sumx * sumx));
      boffs = (float) ((sumy * sumxx - sumx * sumxy) / ((N_symbols_mode_det - 2) * sumxx - sumx * sumx));
    }
  /* printf("in getmode N_symbols_mode_det %d \n", N_symbols_mode_det);
  printf("mode is %d toffs %g samplroffs %g f_offs_fract %g\n", mode, fmod((boffs + Ts / 2 + time_offset_mean - 1), (float) Ts), slope / ((float) Ts), epsilon_ML_list[mode]);	 */

  /* pa0mbo todo reliability check */
  result->mode_indx = mode;
  result->time_offset = fmodf((boffs + Ts / 2 + time_offset_mean - 1), (float) Ts);	/* fp rest pa0mbo was -2  */
  result->sample_rate_offset = slope / ((float) Ts);
  result->freq_offset_fract = epsilon_ML_list[mode];
//logfile->addToAux(QString("%1 %2 %3 %4").arg( mode).arg(result->time_offset).arg(result->sample_rate_offset).arg(result->freq_offset_fract));

}
