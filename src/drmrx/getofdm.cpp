
/*
*     File getofdm.c
*    
*     Author PA0MBO - M.BOS
*     DATE Feb 21st 2009
*
*     implements more or less the get_ofdm_symbol 
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

#define PI (atan(1.0)*4.0)
int getofdm( /*@null@ */ float *rs, float time_offset_fractional_init,
             float freq_offset_init, float delta_time_offset_I_init, int Ts,
             int Tu, /*@null@ */ float Zi[], /*@null@ */ float *out,
             int init, int TIME_SYNC_ENABLE, int FREQ_SYNC_ENABLE)
{
  float EPSILON = 1.0E-10;
  float max_theta = 10.0;
  //  float max_time_offset_ctrl = 10.0;
  //  float max_delta_time_offset = 2.0;
  float kP_small_timing_controller = 0.01;
  float kP_large_timing_controller = 0.01;
  float threshold_timing_small_large = 2.0;
  float kI_timing_controller = 0.00020;
  float kP_small_freq_controller = 0.750;
  float threshold_freq_small_large = 0.75;
  float kI_freq_controller = 0.0008;
  int Tg, Tgh;
  float phi_freq_correction_last;
  float delta_time_offset_I;
  float delta_time_offset_P;
  float dfreq_offset_I;
  float freq_offset, time_offset_fractional;
  int i;
  float temp1[] = { 0.0, 0.0 }, temp2, temp3, temp4;
  float temp[] = { 0.0, 0.0 };
  float temp5[] = { 0.0, 0.0 };
  float temp6, temp7;
  float temp8[] = { 0.0, 0.0 };
  float temp9, temp10;
  float theta_plus, theta_minus, delta_theta;
  float time_offset_ctrl;
  float delta_time_offset;
  int delta_time_offset_integer;
  float epsilon_ML, freq_offset_ctrl, dfreq_offset_P;
  float kP_large_freq_controller, dfreq_offset;
  float tmptheta, term1;
  static float *exp_temp;
  static float *out1;

  static drmComplex s[288], S[288];	/* 288 = max Tu */
  static fftwf_plan p = NULL;

  if (init == 1)

    {

      /* malloc space for arrays */
      if ((exp_temp = (float *)malloc(Tu * 2 * sizeof(float))) == NULL)

        {
          printf("cannot malloc space for exp_temp in get_ofdm_symbol\n");
          exit(EXIT_FAILURE);
        }
      if ((out1 = (float *)malloc(Tu * 2 * sizeof(float))) == NULL)

        {
          printf("cannot malloc space for out1 in get_ofdm_symbol\n");
          exit(EXIT_FAILURE);
        }
      if (p != NULL)

        {
          fftwf_destroy_plan(p);
        }
      addToLog("fftwf_plan_dft_1d getofdm start",LOGFFT);
      p = fftwf_plan_dft_1d(Tu,(fftwf_complex *)s,(fftwf_complex *)S,FFTW_FORWARD, FFTW_ESTIMATE);
      addToLog("fftwf_plan_dft_1d getofdm stop",LOGFFT);
      return (0);
    }

  else

    {

      /* fixed parameters */
      EPSILON = 1.0E-10;
      max_theta = 5.0;
      //      max_time_offset_ctrl = 10.0;
      //      max_delta_time_offset = 2.0;
      kP_small_timing_controller = 0.01;
      kP_large_timing_controller = 0.01;
      threshold_timing_small_large = 2.0;
      kI_timing_controller = 0.00005;
      kP_small_freq_controller = 0.05;
      kP_large_freq_controller = 0.75;
      threshold_freq_small_large = 0.5;
      kI_freq_controller = 0.0008;
      Tg = Ts - Tu;
      Tgh = (int) floor(Tg / 2 + 0.5);
      if (Zi[0] < 0.0)

        {
          phi_freq_correction_last = 0.0;
          delta_time_offset_I = delta_time_offset_I_init;
          dfreq_offset_I = 0.0;
          freq_offset = freq_offset_init;
          time_offset_fractional = time_offset_fractional_init;
        }

      else

        {
          phi_freq_correction_last = Zi[1];
          delta_time_offset_I = Zi[2];
          dfreq_offset_I = Zi[3];
          freq_offset = Zi[4];
          time_offset_fractional = Zi[5];
        }
      if (TIME_SYNC_ENABLE == 1)

        {
          temp1[0] = 0.0;
          temp1[1] = 0.0;
          temp2 = 0.0;
          temp3 = 0.0;
          temp5[0] = 0.0;
          temp5[1] = 0.0;
          temp6 = 0.0;
          temp7 = 0.0;
          temp8[0] = 0.0;
          temp8[1] = 0.0;
          temp9 = 0.0;
          temp10 = 0.0;
          for (i = 0; i < Tg + 2; i++)

            {
//              logfile->addToAux(QString("tm1 %1 %2 %3 %4 %5").arg(rs[i * 2] ).arg(rs[(Tu + i) * 2]).arg(rs[i * 2 + 1]).arg(rs[(Tu + i) * 2 + 1]).arg(temp1[0]));
              temp1[0] += rs[i * 2] * rs[(Tu + i) * 2] + rs[i * 2 + 1] * rs[(Tu + i) * 2 + 1];	/* real part */
              temp1[1] += -rs[i * 2] * rs[(Tu + i) * 2 + 1] + rs[i * 2 + 1] * rs[(Tu + i) * 2];	/* imag part */
              temp2 += rs[i * 2] * rs[i * 2] + rs[i * 2 + 1] * rs[i * 2 + 1];
              temp3 += rs[(i + Tu) * 2] * rs[(i + Tu) * 2] + rs[(i + Tu) * 2 + 1] * rs[(i + Tu) * 2 + 1];
            }
          temp4 = (float) (EPSILON + 0.5 * (temp2 + temp3));
//          logfile->addToAux(QString("tmp %1 %2 %3 %4 %5").arg(temp1[0]).arg(temp1[1]).arg(temp2).arg(temp3).arg(temp4));

          /* time offset measurement : theta */
          for (i = 0; i < 5; i++)

            {
              temp5[0] += rs[i * 2] * rs[(Tu + i) * 2] + rs[i * 2 + 1] * rs[(Tu + i) * 2 + 1];	/* 1-5 * Tu_1_5 */
              temp5[1] += -rs[i * 2] * rs[(Tu + i) * 2 + 1] + rs[i * 2 +1] * rs[(Tu + i) * 2];
              temp6 += rs[i * 2] * rs[i * 2] + rs[i * 2 + 1] * rs[i * 2 + 1];	/* 1-5 * 1-5' */
              temp7 += rs[(i + Tu) * 2] * rs[(i + Tu) * 2] + rs[(i + Tu) * 2 + 1] * rs[(i + Tu) * 2 + 1];	/* Tu_1_5 * Tu_1_5' */ /* pa0mbo check for OK */
              temp8[0] += rs[(i + Tg - 3) * 2] * rs[(i + Tu + Tg - 3) * 2] + rs[(i + Tg - 3) * 2 + 1] * rs[(i + Tu + Tg - 3) * 2 + 1];	/* Tg -2 +2 * Tg + Tu -2 +2 */
              temp8[1] += -rs[(i + Tg - 3) * 2] * rs[(i + Tu + Tg - 3) * 2 + 1] + rs[(i + Tg - 3) * 2 + 1] + rs[(i + Tu + Tg - 3) * 2];
              temp9 += rs[(i + Tg - 3) * 2] * rs[(i + Tg - 3) * 2] + rs[(i + Tg - 3) * 2 + 1] * rs[(i + Tg - 3) * 2 + 1];	/* Tg-2+2 * Tg-2+2 */
              temp10 += rs[(i + Tg + Tu - 3) * 2] * rs[(i + Tg + Tu - 3) * 2] + rs[(i + Tg + Tu - 3) * 2 + 1] * rs[(i + Tg + Tu - 3) * 2 + 1];	/* TG+Tu-2+2 * Tg+Tu-2+2 */
            }
          theta_plus = (float) sqrt((temp1[0] - temp5[0]) * (temp1[0] - temp5[0]) + (temp1[1] - temp5[1]) * (temp1[1] - temp5[1]));
          theta_plus -= (0.5 * (-temp6 - temp7));
          theta_minus = (float) sqrt((temp1[0] - temp8[0]) * (temp1[0] - temp8[0]) + (temp1[1] - temp8[1]) * (temp1[1] - temp8[1]));
          theta_minus -= (0.5 * (temp9 - temp10));
          delta_theta = (theta_plus - theta_minus) * Tgh / temp4;

          /* now limit the delta_theta value */
          if (delta_theta < -max_theta)  delta_theta = -max_theta;
          if (delta_theta > max_theta)   delta_theta = max_theta;

          /* P-I controller for theta */
          time_offset_ctrl = delta_theta - time_offset_fractional;
          delta_time_offset_I += kI_timing_controller * time_offset_ctrl;
          delta_time_offset_P = kP_large_timing_controller * time_offset_ctrl +
              threshold_timing_small_large * (kP_small_timing_controller -kP_large_timing_controller) *
              tanh(time_offset_ctrl / threshold_timing_small_large);
          delta_time_offset = delta_time_offset_P + delta_time_offset_I + time_offset_fractional;
          delta_time_offset_integer = (int) floor(delta_time_offset + 0.5);
//          logfile->addToAux(QString("dt %1 %2 %3 %4 %5").arg(time_offset_ctrl).arg(delta_time_offset_I).arg(delta_time_offset_P).arg(delta_time_offset).arg(delta_time_offset_integer));

          /* now limit delta_time_offset_integer */
          if (delta_time_offset_integer < -1) delta_time_offset_integer = -1;
          if (delta_time_offset_integer > 1)  delta_time_offset_integer = 1;
          time_offset_fractional =  delta_time_offset - delta_time_offset_integer;

          /* printf("delta_time_offset_integer as used in phi_freq_corr %d phifcl %g \n",
       delta_time_offset_integer, phi_freq_correction_last); */
          phi_freq_correction_last += (delta_time_offset_integer / Tu) * freq_offset;

        }

      else

        {
          delta_time_offset_integer = 0;
          time_offset_fractional = 0.0;
        }

      /*  printf("in getofdm delta_time_offset_integer is %d time_offs_fract %g\n",
         delta_time_offset_integer, time_offset_fractional);   */
      if (FREQ_SYNC_ENABLE == 1)

        {
          temp[0] = 0.0;
          temp[1] = 0.0;
          for (i = 0; i < Tg; i++)

            {
              temp[0] +=
                  rs[(i + 1 + delta_time_offset_integer) * 2] * rs[(i + 1 +
                                                                    delta_time_offset_integer
                                                                    + Tu) * 2] +
                  rs[(i + 1 + delta_time_offset_integer) * 2 +
                  1] * rs[(i + 1 + delta_time_offset_integer + Tu) * 2 + 1];
              temp[1] +=
                  rs[(i + 1 + delta_time_offset_integer) * 2 +
                  1] * rs[(i + 1 + delta_time_offset_integer + Tu) * 2] -
                  rs[(i + 1 + delta_time_offset_integer) * 2] * rs[(i + 1 +
                                                                    delta_time_offset_integer
                                                                    + Tu) * 2 +
                  1];
            }
          epsilon_ML = (float) (atan2(temp[1], temp[0]));

          /* filter epsilon_ML */
          freq_offset_ctrl =
              fmodf(epsilon_ML - freq_offset + PI + 100.0 * PI, 2 * PI) - PI;
          dfreq_offset_I += kI_freq_controller * freq_offset_ctrl;
          dfreq_offset_P =
              kP_large_freq_controller * freq_offset_ctrl +
              threshold_freq_small_large * (kP_small_freq_controller -
                                            kP_large_freq_controller) *
              tanh(freq_offset_ctrl / threshold_freq_small_large);
          dfreq_offset = dfreq_offset_P + dfreq_offset_I;
          freq_offset += dfreq_offset;
        }

      else

        {

          /* freq_offset =0.0  */
        }

      /* printf("in getofdm freq_offset = %g\n", freq_offset); */

      /* get symbol and correct frequency */
      for (i = 0; i < Tu; i++)

        {
          tmptheta = (freq_offset / Tu) * i + phi_freq_correction_last;
          exp_temp[i * 2] = (float) cos(tmptheta);
          exp_temp[i * 2 + 1] = (float) sin(tmptheta);
        }

      for (i = 0; i < Tu; i++)
        {
          (s[i]).re =
              rs[(1 + delta_time_offset_integer + Tgh +
                  i) * 2] * exp_temp[i * 2] - rs[(1 +
                                                  delta_time_offset_integer +
                                                  Tgh + i) * 2 +
              1] * exp_temp[i * 2 + 1];
          (s[i]).im =
              rs[(1 + delta_time_offset_integer + Tgh +
                  i) * 2] * exp_temp[i * 2 + 1] + rs[(1 +
                                                      delta_time_offset_integer
                                                      + Tgh + i) * 2 +
              1] * exp_temp[i * 2];
        }
      phi_freq_correction_last = (float) fmod(phi_freq_correction_last + (float) Ts / (float) Tu * freq_offset, (float)(2.0 * PI));

      /* Now do fft and output symbol */
      fftwf_execute(p);
      for (i = 0; i <= Tu / 2; i++)

        {
          term1 = (float) (i * 2.0 * PI * (Tgh + time_offset_fractional) / Tu);	/* Euler */
          exp_temp[i * 2] = (float) cos(term1);
          exp_temp[i * 2 + 1] = (float) sin(term1);
        }
      /* now calc out */
      for (i = 0; i < Tu / 2; i++)

        {
          out1[i * 2] = (float) (((S[(Tu - 1 - i)]).re) * exp_temp[(i + 1) * 2] + ((S[(Tu - 1 - i)]).im) * exp_temp[(i + 1) * 2 + 1]);	/* real = ac+bd */
          out1[i * 2 + 1] = (float) (((S[(Tu - 1 - i)]).im) * exp_temp[(i + 1) * 2] - ((S[(Tu - 1 - i)]).re) * exp_temp[(i + 1) * 2 + 1]);	/* imag bc -ad */
        }
      /* Now flip out1 to out */
      for (i = 0; i < Tu / 2; i++)

        {
          out[i * 2] = out1[(Tu / 2 - 1 - i) * 2];
          out[i * 2 + 1] = out1[(Tu / 2 - i - 1) * 2 + 1];
          out[(Tu / 2 + i) * 2] =
              (float) (((S[i]).re) * exp_temp[i * 2] -
                       ((S[i]).im) * exp_temp[i * 2 + 1]);
          out[(Tu / 2 + i) * 2 + 1] =
              (float) (((S[i]).im) * exp_temp[i * 2] +
                       ((S[i]).re) * exp_temp[i * 2 + 1]);
        } Zi[0] = 1.0;
      Zi[1] = phi_freq_correction_last;
      Zi[2] = delta_time_offset_I;
      Zi[3] = dfreq_offset_I;
      Zi[4] = freq_offset;
      Zi[5] = time_offset_fractional;
      return (delta_time_offset_integer);
    }
}
