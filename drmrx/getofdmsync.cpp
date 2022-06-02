
/*
*     File getofdmsync.c
*    
*     Author PA0MBO - M.BOS
*     DATE Feb 21st 2009
*
*     implements more or less the get_ofdm_symbol_sync 
*     MATLAB routine from diorama-1.1.1
*
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
#include <fftw3.h>
#include "drmproto.h"
#include "appglobal.h"
#include "supportfunctions.h"

extern int FREQ_SYNC_ENABLE;


#define PI (atan(1.0)*4.0)
void filter1(float *, float *, float *, int, int);

static int Tg, Tgh;
static int Tc;
static int Tgs, Tgsh;
static float kP_small_timing_controller = 0.0005;
static float kP_large_timing_controller = 0.003;	/* pa0mbo was 0.01 */
static float threshold_timing_small_large = 2.0;	/* was 2.0 */
static float kI_timing_controller = 0.000020;	/* was 0.00005 */
static drmComplex s[512], S[512];
static drmComplex s1[512], S1[512];
static fftwf_plan p1=NULL;
static fftwf_plan p2=NULL;
static float /*@only@ */ *exp_temp=NULL;
static float /*@only@ */ *out1=NULL;


int getofdmsync( /*@null@ */ float *rs, int Ts, int Tu, /*@null@ */ float *H,
             int lH,
             float delta_freq_offset, /*@null@ */ float *Zi, /*@null@ */
             float *out, int init,
             int TIME_SYNC_ENABLE, int FREQ_SYNC_ENABLE)
{

  int max_delta_theta;
  int max_delta_time_offset_integer = 3;
  int max_symbol_position_offset;


  float h_absq[512];		/* 2 x max el */
  static float sinfilter[256];	/* too long */
  float h_absq_filtered[512];
  float dummy;
  float delta_theta, delta_theta_tmp;
  int dftmp, symbol_position_offset, spotmp;
  float freq_offset_ctrl;

  int i;
  float kP_freq_controller;
//  float *pinput;
  float time_offset_ctrl;
  float delta_time_offset_P, delta_time_offset;
  int delta_time_offset_integer;
  float tmptheta, term1;
  int indexin;
  float time_offset_fractional, freq_offset;
  float delta_time_offset_I, phi_freq_correction_last;


  /* pa0mbo if-part checken 16 april 2007 */
  if (init == 1)

    {
      Tg = Ts - Tu;
      Tgh = (int) floor(Tg / 2 + 0.5);
      Tc = (int) pow(2, ceil(log((double) lH) / log(2.0)));
      Tgs = (int) floor((float) Tg / (float) Tu * (float) Tc);
      Tgsh = (int) floor((float) Tg / (float) Tu / 2.0 * (float) Tc);

      /* printf("Tg %d, Tgh %d, Tc %d,Tu %d,  Tgs %d, Tgsh %d\n",
         Tg, Tgh, Tc, Tu, Tgs, Tgsh);   */
      /* malloc space for arrays */
      if(exp_temp!=NULL) free(exp_temp);
      if(out1!=NULL) free(out1);

      if ((exp_temp = (float *)malloc((Tu * 2 + 2) * sizeof(float))) == NULL)

        {
          printf("cannot malloc space for exp_temp in get_ofdm_symbol\n");
          exit(EXIT_FAILURE);
        }
      if ((out1 = (float *)malloc(Tu * 2 * sizeof(float))) == NULL)

        {
          printf("cannot malloc space for out1 in get_ofdm_symbol\n");
          exit(EXIT_FAILURE);
        }
      addToLog("syn 1",LOGPERFORM);
      if (p1 != NULL)
        {
          fftwf_destroy_plan(p1);
        }
      if (p2 != NULL)
        {
          fftwf_destroy_plan(p2);
        }
      addToLog("fftwf_plan_dft_1d getofdmsync p1 start",LOGFFT);
      p1 = fftwf_plan_dft_1d(Tc,(fftwf_complex *)s,(fftwf_complex *)S,FFTW_FORWARD, FFTW_ESTIMATE);
      p2 = fftwf_plan_dft_1d(Tu,(fftwf_complex *)s1,(fftwf_complex *)S1, FFTW_FORWARD, FFTW_ESTIMATE);
      addToLog("fftwf_plan_dft_1d getofdmsync p1 stop",LOGFFT);

      /*      printf("xxxx sinfilter\n");   */
      for (i = 0; i < Tgs; i++)

        {
          sinfilter[i] = (float) pow(sin((float) (i + 1.0) / (float) (Tgs + 1.0) * PI), 0.001);
        } return (0);
    }

  else

    {

      /* fixed parameters */
      Tg = Ts - Tu;
      Tgh = (int) floor(Tg / 2 + 0.5);
      Tc = (int) pow(2, ceil(log((double) lH) / log(2.0)));
      Tgs = (int) floor((float) Tg / (float) Tu * (float) Tc);
      Tgsh = (int) floor((float) Tg / (float) Tu / 2.0 * (float) Tc);
      kP_small_timing_controller = 0.001;	/* pa0mbo was 0.005 */
      kP_large_timing_controller = 0.001;	/* pa0mbo was 0.01 */
      threshold_timing_small_large = (float) Tgh;
      kI_timing_controller = 2E-5;	/* pa0mbo was 0.000005 */
      max_delta_theta = Tgh;
      max_delta_time_offset_integer = 3;
      max_symbol_position_offset = Tgh - max_delta_time_offset_integer;
      kP_freq_controller = 0.01;	/* pa0mbo was 0.01 */
      phi_freq_correction_last = Zi[1];
      delta_time_offset_I = Zi[2];
      freq_offset = Zi[4];
      time_offset_fractional = Zi[5];




      /* debugging  */

      addToLog(QString("ofdmsync: dfo= %1 tof=%2 fofs=  %3 dtoI = %4 phicl = %5")
               .arg(delta_freq_offset).arg(time_offset_fractional).arg(freq_offset)
               .arg(delta_time_offset_I).arg(phi_freq_correction_last),LOGDRMDEMOD);
      if (TIME_SYNC_ENABLE == 1)

        {

          /* estimate time offset */
          /* first copy H data to  s buffer  that is destroyed by fft */
//          pinput = H;
          for (i = 0; i < lH; i++)
            {
              s[i].re = H[i * 2];
              s[i].im = H[i * 2 + 1];
            }

          /* zero fill to power of 2 elements */
          for (i = lH; i < Tc; i++)
            {
              s[i].re = 0.0;
              s[i].im = 0.0;
            }
          fftwf_execute(p1);	/* do complex fft */


          /*  printf("xxx h_absq\n"); */
          for (i = 0; i < Tc; i++)

            {
              h_absq[i] = (float) (S[i].re * S[i].re + S[i].im * S[i].im);
              h_absq[i + Tc] = (float) (S[i].re * S[i].re + S[i].im * S[i].im);	/* needed 2 times */

              /*   printf(" %g\n", h_absq[i]); */
            }
          drmfilter1(h_absq, h_absq_filtered, sinfilter, 2 * Tc, Tgs);

          /* debugging
       printf("xxx filter h_abs \n");
       for (i=0; i < 2*Tc ; i++)
       printf("%g \n", h_absq_filtered[i]);
       printf("ooooo\n");    */

          /* now determine max and position */
          dummy = -1.0E30;
          delta_theta = 0.0;
          for (i = 0; i < 2 * Tc; i++)

            {
              if (h_absq_filtered[i] > dummy)

                {
                  dummy = h_absq_filtered[i];
                  delta_theta = (float) i;
                }
            }
          /* debugging
       printf("ofdmsync: dummy= %g delta_theta=%g\n", dummy, delta_theta);  */
          delta_theta =
              (float) (((((Tc + Tgsh - (int) delta_theta +
                           (int) (Tc * 1.5)) % Tc) -
                         Tc / 2) * Tu) / (float) Tc);

          /* printf("delta_theta rescaled %g\n", delta_theta);   */
          if (delta_theta >= (float) max_delta_theta)
            delta_theta_tmp = (float) max_delta_theta;

          else
            delta_theta_tmp = delta_theta;
          if (delta_theta_tmp > (float) -max_delta_theta)
            delta_theta = delta_theta_tmp;

          else
            delta_theta = (float) -max_delta_theta;

          /* filter theta: P-I controller */
          time_offset_ctrl = delta_theta - time_offset_fractional;
          delta_time_offset_I += kI_timing_controller * time_offset_ctrl;
          delta_time_offset_P =
              kP_large_timing_controller * time_offset_ctrl +
              threshold_timing_small_large * (kP_small_timing_controller -
                                              kP_large_timing_controller) *
              tanhf(time_offset_ctrl / threshold_timing_small_large);
          delta_time_offset =
              delta_time_offset_P + delta_time_offset_I +
              time_offset_fractional;
          delta_time_offset_integer = (int) floor(delta_time_offset + 0.5);
          if (delta_time_offset_integer > -max_delta_time_offset_integer)
            dftmp = delta_time_offset_integer;

          else
            dftmp = -max_delta_time_offset_integer;
          if (dftmp > max_delta_time_offset_integer)
            delta_time_offset_integer = max_delta_time_offset_integer;

          else
            delta_time_offset_integer = dftmp;	/* only +/- one symbol */
          time_offset_fractional =
              delta_time_offset - delta_time_offset_integer;

          /* debugging
       printf("delta_t_offs  %g delta_t_int %d time_offs_fract %g \n",
       delta_time_offset, delta_time_offset_integer, time_offset_fractional);  */

          /* get best time window */
          symbol_position_offset = (int) floor(delta_theta - delta_time_offset_integer + 0.5);

          if (symbol_position_offset > -max_symbol_position_offset) spotmp = symbol_position_offset;
          else           spotmp = -max_symbol_position_offset;
          if (spotmp < max_symbol_position_offset) symbol_position_offset = spotmp;
          else symbol_position_offset = max_symbol_position_offset;

          /* do integer time offset correction and comp phase shift */
          phi_freq_correction_last +=
              ((float) delta_time_offset_integer / Tu) * freq_offset;
        }

      else

        {
          delta_time_offset_integer = 0;
          time_offset_fractional = 0;
          symbol_position_offset = 0;
        }
      if (FREQ_SYNC_ENABLE == 1)

        {

          /* frequency offset estimation */
          freq_offset_ctrl = delta_freq_offset;
          freq_offset += kP_freq_controller * freq_offset_ctrl;
        }

      /* get symbol and correct frequency */

      for (i = 0; i < Tu; i++)

        {
          indexin = i + symbol_position_offset;
          tmptheta = (freq_offset / Tu) * indexin + phi_freq_correction_last;
          exp_temp[i * 2] = (float) cos(tmptheta);
          exp_temp[i * 2 + 1] = (float) sin(tmptheta);
        }

      for (i = 0; i < Tu; i++)
        {
          indexin =i + 1 + delta_time_offset_integer + Tgh + symbol_position_offset;
          s1[i].re = rs[indexin * 2] * exp_temp[i * 2] - rs[indexin * 2 + 1] * exp_temp[i * 2 + 1];
          s1[i].im = rs[indexin * 2] * exp_temp[i * 2 + 1] + rs[indexin * 2 + 1] * exp_temp[i * 2];
        }
      phi_freq_correction_last =
          fmodf(phi_freq_correction_last +
                (float) Ts / (float) Tu * freq_offset, 2.0 * PI);

      /* Now do fft and output symbol */
      fftwf_execute(p2);

      /*  printf("xxx exp_temp in getofdmsync \n"); */
      for (i = 0; i <= Tu / 2; i++)

        {
          term1 = (float) ((i * 2.0 * PI / (float) Tu) * (Tgh + time_offset_fractional - symbol_position_offset));	/* Euler */
          exp_temp[i * 2] = (float) cos(term1);
          exp_temp[i * 2 + 1] = (float) sin(term1);

          /*    printf(" %g %g \n", exp_temp[i*2], exp_temp[i*2+1]); */
        }
      /* now calc out */
      for (i = 0; i < Tu / 2; i++)

        {
          out1[i * 2] = (float) (((S1[(Tu - 1 - i)]).re) * exp_temp[(i + 1) * 2] + ((S1[(Tu - 1 - i)]).im) * exp_temp[(i + 1) * 2 + 1]);	/* real = ac+bd */
          out1[i * 2 + 1] = (float) (((S1[(Tu - 1 - i)]).im) * exp_temp[(i + 1) * 2] - ((S1[(Tu - 1 - i)]).re) * exp_temp[(i + 1) * 2 + 1]);	/* imag bc -ad */
        }
      for (i = 0; i < (Tu / 2 - 1); i++)
        {
          out1[(Tu / 2 + i) * 2] =
              (float) (((S1[i]).re) * exp_temp[i * 2] -
                       ((S1[i]).im) * exp_temp[i * 2 + 1]);
          out1[(Tu / 2 + i) * 2 + 1] =
              (float) (((S1[i]).im) * exp_temp[i * 2] +
                       ((S1[i]).re) * exp_temp[i * 2 + 1]);
        }
      /* Now flip out1 to out */
      for (i = 0; i < Tu / 2; i++)
         {
          out[i * 2] = out1[(Tu / 2 - 1 - i) * 2];
          out[i * 2 + 1] = out1[(Tu / 2 - 1 - i) * 2 + 1];
        }

      /* now put in the rest */
      for (i = Tu / 2; i < Tu; i++)
         {
          out[i * 2] = out1[i * 2];
          out[i * 2 + 1] = out1[i * 2 + 1];
        }
      Zi[1] = phi_freq_correction_last;
      Zi[2] = delta_time_offset_I;
      Zi[4] = freq_offset;
      Zi[5] = time_offset_fractional;
      return (delta_time_offset_integer);
    }
}
