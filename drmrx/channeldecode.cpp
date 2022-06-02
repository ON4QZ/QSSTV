/*
*   file channeldecode.c
*
*   Author M.Bos - PA0MBO
*   Date Feb 21st 2009
*
*   superframe sync of drm demapping
*   deinterleaving channel decoding of FAC  and MSC frame
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

#include "appglobal.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <malloc.h>
#include <float.h>
#include "drmproto.h"
#include "drmdefs.h"
#include "drm.h"
#include <fftw3.h>
#include "configparams.h"

#define PI (4.0*atan(1.0))

#define CHANNELDECODING 1
char localDrmCallsign[9]= {0, 0, 0, 0, 0, 0, 0, 0, 0}; //changed ON4QZ

#define MSD_ITER 4
extern int transmission_frame_buffer_data_valid;
extern int fac_valid;
extern int robustness_mode;
extern int spectrum_occupancy;
extern int symbols_per_frame;
extern int K_modulo;
extern int K_dc;
extern float mean_energy_of_used_cells;
extern int FAC_cells_k[65];
extern float transmission_frame_buffer[82980];
extern float channel_transfer_function_buffer[82980];
extern int transmission_frame_buffer_wptr;
extern int lFAC;
extern int runstate;
extern struct mplex_desc multiplex_description;
extern struct audio_info audio_information;
extern struct appl_info application_information;
extern struct stream_info stream_information;
extern struct time_info time_and_date;
extern int channel_decoded_data_buffer_data_valid;
extern double channel_decoded_data_buffer[110000];
extern int audio_data_flag;
extern int length_decoded_data;
extern int MSC_Demapper[6][2959];

int lMSC;  //changed ON4QZ
float MSC_cells_sequence[2 * 2959]; //changed ON4QZ
bool MSCAvailable;
int spectrum_occupancy_new;
int msc_mode_new;
int interleaver_depth_new;
bool callsignValid;

char getfacchar(double *);

void channel_decoding(void)
{
  static int /*@only@ */ *FAC_Deinterleaver;
  static double RX[13] = { 1, 3, 1, 4, 1, 4, 3, 2, 8, 3, 4, 7, 8 };
  static double RY[13] = { 4, 10, 3, 11, 2, 7, 5, 3, 11, 4, 5, 8, 9 };
  //  static int RYlcmSM16[2] = { 3, 4 };
  //  static int RYlcmSM64[4] = { 4, 15, 8, 45 };
  static int RatesSM16[2][2] = { {3, 8}, {5, 10} };
  static int RatesSM64[4][3] =
  { {1, 5, 10}, {3, 8, 11}, {5, 10, 12}, {8, 11, 13}
  };
  static int frame_index;
  static int enough_frames;
  static int frame_count;
  static int msc_parameters_valid;
  static int robustness_mode_old;
  static int msc_mode;
  static int interleaver_depth;
  static int MSC_Demapper_symbolwise[6][2959];
  static double SNR_estimation[2959];
  static double squared_noise_signal_buffer[121490];
  static double noise_power_density[461];
  static int SNR_estimation_valid;
  int i, j, k, n;
  static int msc_parameters_changed;
  int symbol_period;
  int trxfrmbufptr;
  double received_real[3000], received_imag[3000], snr[3000];
  float transfer_function_FAC[3000];	/* complex */
  static double L[6];
  static double L_FAC[4];
  static int PL[6];
  static int PL_FAC[4];
  static double fac_data[72];
  double checksum;
  static double channel_parameters[20];
  double facblock[72];
  double temp;
  static int identity;
  static int identityCount;
  static int old_ptr;
  static int MSC_carrier_usage[288];
  static int cnt_MSC_used_carriers;
  static int MSC_used_carriers[288];
  static int min_index_equal_samples;
  static int max_index_equal_samples;
  static int /*@only@ */ *Part_Deinterleaver = NULL;
  static int Deinterleaver[18000];
  int no_of_streams = 1;
  static int N_MUX;
  static int N1, N2;
  static int ratesA[3], ratesB[3], Lvspp, xin1, xin2;
  float sum1;
  float transfer_function_MSC[2 * 2959];	/* complex nrs */

  static int rowdimL, coldimL;
  double SPPhard[10628];
  int n_SPPhard;
  double VSPPhard[100];
  static double res_iters, calc_variance;
  static double noise_signal[2 * 2959];
  static double squared_noise_signal[2959];
  double sum2;
  double weighted_noise_power_density[288];
  double samples_resorted[288][15];
  int posrow, poscolumn, totindex;
  int VSPPlength, HPPlength;
  int Tu_list[] = { Tu_A, Tu_B, Tu_C, Tu_D };
  double part1, part2;

  if (runstate == RUN_STATE_POWER_ON)

    {
      if (FAC_Deinterleaver != NULL)
        free(FAC_Deinterleaver);
      FAC_Deinterleaver = deinterleaver(0, 1, 90, 21);

      return;
    }
  if (runstate == RUN_STATE_INIT)

    {
      transmission_frame_buffer_wptr = 0;
      frame_index = 1;
      enough_frames = 0;
      frame_count = 0;
      msc_parameters_valid = 0;
      robustness_mode_old = -1;
      msc_mode = -1;
      interleaver_depth = -1;
      fac_valid = -1;
      multiplex_description.HM_length = 0;
      callsignValid=false;
      return;
    }
  channel_decoded_data_buffer_data_valid = 0;

  if (transmission_frame_buffer_data_valid == 0)

    {
      MSCAvailable=false;
      /* clear various datastructures */
      frame_index = 1;
      enough_frames = 0;
      frame_count = 0;
      msc_parameters_valid = 0;
      transmission_frame_buffer_wptr = 0;
      multiplex_description.PL_PartA = -1;
      multiplex_description.PL_PartB = -1;
      multiplex_description.HM_length = 0;
      multiplex_description.PL_HM = -1;
      audio_information.ID[0] = 0;
      audio_information.ID[1] = 0;
      audio_information.ID[2] = 0;
      audio_information.ID[3] = 0;
      audio_information.stream_ID[0] = -1;
      audio_information.stream_ID[1] = -1;
      audio_information.stream_ID[2] = -1;
      audio_information.stream_ID[3] = -1;
      audio_information.audio_coding[0] = 0;
      audio_information.audio_coding[1] = 0;
      audio_information.audio_coding[2] = 0;
      audio_information.audio_coding[3] = 0;
      audio_information.SBR_flag[0] = 0;
      audio_information.SBR_flag[1] = 0;
      audio_information.SBR_flag[2] = 0;
      audio_information.SBR_flag[3] = 0;
      audio_information.audio_mode[0] = 0;
      audio_information.audio_mode[1] = 0;
      audio_information.audio_mode[2] = 0;
      audio_information.audio_mode[3] = 0;
      audio_information.sampling_rate[0] = 0;
      audio_information.sampling_rate[1] = 0;
      audio_information.sampling_rate[2] = 0;
      audio_information.sampling_rate[3] = 0;
      audio_information.text_flag[0] = 0;
      audio_information.text_flag[1] = 0;
      audio_information.text_flag[2] = 0;
      audio_information.text_flag[3] = 0;
      audio_information.enhancement_flag[0] = 0;
      audio_information.enhancement_flag[1] = 0;
      audio_information.enhancement_flag[2] = 0;
      audio_information.enhancement_flag[3] = 0;
      audio_information.coder_field[0] = 0;
      audio_information.coder_field[1] = 0;
      audio_information.coder_field[2] = 0;
      audio_information.coder_field[3] = 0;
      audio_information.bytes_per_frame[0] = 0;
      audio_information.bytes_per_frame[1] = 0;
      audio_information.bytes_per_frame[2] = 0;
      audio_information.bytes_per_frame[3] = 0;
      application_information.ID[0] = 0;
      application_information.ID[1] = 0;
      application_information.ID[2] = 0;
      application_information.ID[3] = 0;
      application_information.stream_ID[0] = -1;
      application_information.stream_ID[1] = -1;
      application_information.stream_ID[2] = -1;
      application_information.stream_ID[3] = -1;
      application_information.packet_mode[0] = 0;
      application_information.packet_mode[1] = 0;
      application_information.packet_mode[2] = 0;
      application_information.packet_mode[3] = 0;
      application_information.data_unit_indicator[0] = 0;
      application_information.data_unit_indicator[1] = 0;
      application_information.data_unit_indicator[2] = 0;
      application_information.data_unit_indicator[3] = 0;
      application_information.packet_ID[0] = 0;
      application_information.packet_ID[1] = 0;
      application_information.packet_ID[2] = 0;
      application_information.packet_ID[3] = 0;
      application_information.enhancement_flag[0] = 0;
      application_information.enhancement_flag[1] = 0;
      application_information.enhancement_flag[2] = 0;
      application_information.enhancement_flag[3] = 0;
      application_information.application_domain[0] = 0;
      application_information.application_domain[1] = 0;
      application_information.application_domain[2] = 0;
      application_information.application_domain[3] = 0;
      application_information.packet_length[0] = 0;
      application_information.packet_length[1] = 0;
      application_information.packet_length[2] = 0;
      application_information.packet_length[3] = 0;
      application_information.user_application_type[0] = 0;
      application_information.user_application_type[1] = 0;
      application_information.user_application_type[2] = 0;
      application_information.user_application_type[3] = 0;
      application_information.user_application_identifier[0] = 0;
      application_information.user_application_identifier[1] = 0;
      application_information.user_application_identifier[2] = 0;
      application_information.user_application_identifier[3] = 0;
      application_information.label[0][0] = '\0';
      application_information.label[1][0] = '\0';
      application_information.label[2][0] = '\0';
      application_information.label[3][0] = '\0';
      application_information.country[0][0] = '\0';
      application_information.country[1][0] = '\0';
      application_information.country[2][0] = '\0';
      application_information.country[3][0] = '\0';
      application_information.language_code[0] = 0;
      application_information.language_code[1] = 0;
      application_information.language_code[2] = 0;
      application_information.language_code[3] = 0;
      application_information.programme_type_code[0] = 0;
      application_information.programme_type_code[1] = 0;
      application_information.programme_type_code[2] = 0;
      application_information.programme_type_code[3] = 0;
      application_information.bytes_per_frame[0] = 0;;
      application_information.bytes_per_frame[1] = 0;;
      application_information.bytes_per_frame[2] = 0;;
      application_information.bytes_per_frame[3] = 0;;
      stream_information.number_of_audio_services = 0;
      stream_information.number_of_data_services = 0;
      stream_information.number_of_streams = 0;
      stream_information.number_of_audio_streams = 0;
      stream_information.number_of_data_streams = 0;
      time_and_date.day = -1;
      time_and_date.month = -1;
      time_and_date.year = -1;
      time_and_date.hours = -1;
      time_and_date.minutes = -1;
      //      sdc_mode = -1;
      msc_mode = -1;
      robustness_mode_old = -1;
      interleaver_depth = -1;
      for (i = 0; i < 41490; i++)
        squared_noise_signal_buffer[i] = 0.0;
      for (i = 0; i < 461; i++)
        noise_power_density[i] = 0.0;
      SNR_estimation_valid = 0;
      fac_valid = -1;
      //      audio_service_index = 1;
      return;
    }
  symbol_period = Tu_list[robustness_mode];
  if (CHANNELDECODING == 0)

    {
      frame_index = (frame_index % 6) + 1;

      /* in matlab check for existence of symbol_period & symbols_per_frame */
      if ((symbol_period != -1) & (symbols_per_frame != -1))

        {
          transmission_frame_buffer_wptr = (transmission_frame_buffer_wptr + symbol_period * symbols_per_frame) % (symbol_period * symbols_per_frame * 6);

        }
      SNR_estimation_valid = 0;
      return;
    }
  //  iterations = 0;
  calc_variance = -0.05;
  msc_parameters_changed = 0;

  if (robustness_mode != robustness_mode_old)
    {
      if (robustness_mode < 0)
        return;
      symbol_period = Tu_list[robustness_mode];
      lFAC = mkfacmap(robustness_mode, K_dc, K_modulo, FAC_cells_k);
    }

  /* FAC decoding */
  fac_valid = 1;

  for (i = 0; i < lFAC; i++)

    {
      trxfrmbufptr = (frame_index - 1) * symbol_period * symbols_per_frame + FAC_cells_k[i];
      received_real[i] = (double) transmission_frame_buffer[2 * trxfrmbufptr];
      received_imag[i] = (double) transmission_frame_buffer[2 * trxfrmbufptr + 1];
      transfer_function_FAC[i * 2] = channel_transfer_function_buffer[2 * trxfrmbufptr];
      transfer_function_FAC[i * 2 + 1] = channel_transfer_function_buffer[2 * trxfrmbufptr + 1];
      snr[i] = sqrt(transfer_function_FAC[i * 2] * transfer_function_FAC[i * 2] +
          transfer_function_FAC[i * 2 + 1] * transfer_function_FAC[i * 2 +1]);

    }
  received_real[9] = 0.0;
  received_imag[9] = 0.0;
  L_FAC[0] = 0.0;
  L_FAC[1] = 48.0;
  PL_FAC[0] = 0;
  PL_FAC[1] = 6;
  (void) msdhardfac(received_real, received_imag, lFAC, snr, 0, L_FAC, 2, 0, FAC_Deinterleaver, PL_FAC, 4, 0, fac_data);

  for (i = 0; i < 40; i++)
    {
      facblock[i] = fac_data[i];
    }
  for (i = 40; i < 48; i++)
      {
      facblock[i] = 1.0 - fac_data[i];
    }
  for (i = 0; i < 10; i++)
      {
      channel_parameters[i] = fac_data[i];
    }

  crc8_c(&checksum, facblock, 48);

  if (fabs(checksum) > DBL_EPSILON)
     {
      spectrum_occupancy = -1;
      fac_valid = 0;
      identityCount=0;
      msc_parameters_valid = 0 ;  /* added pa0mbo 23 nov 2011 */
      return;
    }
  msc_parameters_valid = 1;
   /* frame alignment */
  temp = 2.0 * channel_parameters[0] + channel_parameters[1];
  identity = (int) temp % 3;
  if (identity != ((frame_index - 1) % 3))
      {
      old_ptr = transmission_frame_buffer_wptr;
      transmission_frame_buffer_wptr = identity * symbol_period * symbols_per_frame;
      for (i = 0; i < symbol_period * symbols_per_frame; i++)
        {
          trxfrmbufptr = transmission_frame_buffer_wptr + i;
          transmission_frame_buffer[2 * trxfrmbufptr] = transmission_frame_buffer[2 * (old_ptr + i)];
          transmission_frame_buffer[2 * trxfrmbufptr + 1] =transmission_frame_buffer[2 * (old_ptr + i) + 1];
          channel_transfer_function_buffer[2 * trxfrmbufptr] = channel_transfer_function_buffer[2 * (old_ptr + i)];
          channel_transfer_function_buffer[2 * trxfrmbufptr + 1] =channel_transfer_function_buffer[2 * (old_ptr + i) + 1];
        }
      frame_index = identity + 1;
    }
  interleaver_depth_new = (int) channel_parameters[3];
  msc_mode_new = (int) channel_parameters[4];
  if (fabs(channel_parameters[6] - 1.0) < DBL_EPSILON)
    {
      msc_mode_new = msc_mode_new + 2 * (int) channel_parameters[9];
    }
  spectrum_occupancy_new = (int) channel_parameters[2];
  if (spectrum_occupancy_new > 1)
   {
      spectrum_occupancy = -1;
      fac_valid = 0;
      identityCount=0;
      return;
    }
  identityCount++;
  audio_data_flag = (int) channel_parameters[6];
  // we need 3 consequetive valid fac's to have a complete call

  /* decoding of text in fac data */
  localDrmCallsign[3*identity]   = getfacchar(&facblock[10]);
  localDrmCallsign[3*identity+1] = getfacchar(&facblock[17]);
  localDrmCallsign[3*identity+2] = getfacchar(&facblock[24]);
  localDrmCallsign[3*identity+3] = '\0';
  if ((identity == 2) && (identityCount>=3))
    {
      identityCount=0;
      drmCallsign=localDrmCallsign;
      callsignValid=true;
    }

  if ((spectrum_occupancy != spectrum_occupancy_new) || (robustness_mode_old != robustness_mode))
    {
      spectrum_occupancy = spectrum_occupancy_new;
      interleaver_depth = interleaver_depth_new;
      lMSC = mkmscmap(robustness_mode, spectrum_occupancy, interleaver_depth, K_dc, K_modulo);
      for (j = 0; j < 6; j++)
        {
          for (i = 0; i < lMSC; i++)
            {
              MSC_Demapper_symbolwise[j][i] = MSC_Demapper[j][i] % symbol_period;
            }
        }

      /* do the binning and calc carrier usage */
      for (i = 0; i < symbol_period; i++)
        {
          MSC_carrier_usage[i] = 0;
          for (j = 0; j < 6; j++)
            {
              for (k = 0; k < lMSC; k++)
               {
                  if (MSC_Demapper_symbolwise[j][k] == i)
                    {
                      (MSC_carrier_usage[i])++;
                    }
                }
            }
        }
      cnt_MSC_used_carriers = 0;
      for (i = 0; i < symbol_period; i++)
        {
          if (MSC_carrier_usage[i] != 0)
           {
              MSC_used_carriers[cnt_MSC_used_carriers++] = i;
            }
        }
      msc_parameters_changed = 1;
      //      sdc_parameters_changed = 1;
    }

  else

    {
      if (interleaver_depth != interleaver_depth_new)
        {
          interleaver_depth = interleaver_depth_new;
          lMSC = mkmscmap(robustness_mode, spectrum_occupancy, interleaver_depth,K_dc, K_modulo);
          /* do the binning and calc carrier usage */
          for (i = 0; i < symbol_period; i++)
            {
              MSC_carrier_usage[i] = 0;
              for (j = 0; j < 6; j++)
                {
                  for (k = 0; k < lMSC; k++)
                    {
                      if (MSC_Demapper_symbolwise[j][k] == i)
                        {
                          (MSC_carrier_usage[i])++;
                        }
                    }
                }
            }
          cnt_MSC_used_carriers = 0;
          for (i = 0; i < symbol_period; i++)
            {
              if (MSC_carrier_usage[i] != 0)

                {
                  MSC_used_carriers[cnt_MSC_used_carriers++] = i;
                }
            }
          msc_parameters_changed = 1;
        }
    }


  robustness_mode_old = robustness_mode;
  interleaver_depth = interleaver_depth_new;

  /* frame count : deinterleaving possible after 2 received frames
     for short and after 6 received frames for long interleaving */
  frame_count++;
  if (frame_count >= 6 - 4 * interleaver_depth)
    {
      enough_frames = 1;
    }
  else
    {
      if (frame_count == 1)
        {
          min_index_equal_samples = transmission_frame_buffer_wptr;
          max_index_equal_samples = transmission_frame_buffer_wptr + symbol_period * symbols_per_frame;
        }
      else
        {
          if (transmission_frame_buffer_wptr < min_index_equal_samples)
            {
              min_index_equal_samples = transmission_frame_buffer_wptr;
            }
          if (transmission_frame_buffer_wptr + symbol_period * symbols_per_frame > max_index_equal_samples)
            {
              max_index_equal_samples = transmission_frame_buffer_wptr + symbol_period * symbols_per_frame;
            }
        }
    }
  if (msc_mode != msc_mode_new)

    {
      msc_mode = msc_mode_new;
      msc_parameters_changed = 1;
      msc_parameters_valid = 1;
    }
  multiplex_description.HM_length = 0;
  multiplex_description.PL_HM = 0;
  if (fabs(fac_data[5] - 1.0) < DBL_EPSILON)
    {
      multiplex_description.PL_PartA = 1;
      multiplex_description.PL_PartB = 1;
    }
  else
    {
      multiplex_description.PL_PartA = 0;
      multiplex_description.PL_PartB = 0;
    }
  application_information.stream_ID[0] = 0;
  application_information.stream_ID[1] = -1;
  application_information.stream_ID[2] = -1;
  application_information.stream_ID[3] = -1;
  application_information.packet_mode[0] = 1;
  application_information.packet_mode[1] = 0;
  application_information.packet_mode[2] = 0;
  application_information.packet_mode[3] = 0;
  application_information.data_unit_indicator[0] = 1;
  application_information.data_unit_indicator[1] = 0;
  application_information.data_unit_indicator[2] = 0;
  application_information.data_unit_indicator[3] = 0;
  application_information.application_domain[0] = 1;
  application_information.application_domain[1] = 0;
  application_information.application_domain[2] = 0;
  application_information.application_domain[3] = 0;
  for (i = 0; i < 16; i++)
    application_information.application_data[0][i] = 0;

  /* ***   MSC DECODING *** */
  N_MUX = lMSC;

  /* MSC parameters settings */
  if ((msc_parameters_changed == 1) && (msc_parameters_valid == 1))
    {
      if (msc_mode == 0)	/* 64-QAM SM */
        {
          //	  rylcm = RYlcmSM64[multiplex_description.PL_PartA];
          for (i = 0; i < 3; i++)
            {
              ratesA[i] = RatesSM64[multiplex_description.PL_PartA][i] - 1;
            }
          N1 = 0;
          N2 = N_MUX - N1;
          for (i = 0; i < 3; i++)
            {
              ratesB[i] = RatesSM64[multiplex_description.PL_PartB][i] - 1;
            }
          for (i = 0; i < 3; i++)
            {
              L[i] = 2 * N1 * (RX[ratesA[i]] / RY[ratesA[i]]);
              L[i + 3] = (RX[ratesB[i]] * floor((2 * N2 - 12) / RY[ratesB[i]]));
            }
          Lvspp = 0;
          rowdimL = 3;
          coldimL = 2;
          xin1 = 2 * N1;
          xin2 = 2 * N2;
          for (i = 0; i < xin1 + xin2; i++)  Deinterleaver[i] = i;
          if (Part_Deinterleaver != NULL) free(Part_Deinterleaver);
          Part_Deinterleaver = deinterleaver(xin1, 13, xin2, 13);
          for (i = 0; i < xin1 + xin2; i++) Deinterleaver[i + xin1 + xin2] = Part_Deinterleaver[i];
          free(Part_Deinterleaver);
          Part_Deinterleaver = deinterleaver(xin1, 21, xin2, 21);
          for (i = 0; i < xin1 + xin2; i++) Deinterleaver[i + 2 * (xin1 + xin2)] = Part_Deinterleaver[i];

          for (i = 0; i < 3; i++)
            {
              PL[i] = ratesA[i];
              PL[i + 3] = ratesB[i];
            }
        }

      else if (msc_mode == 1)	/* 16-QAM SM */
        {
          //	  rylcm = RYlcmSM16[multiplex_description.PL_PartA];
          for (i = 0; i < 2; i++) //joma
            {
              ratesA[i] = RatesSM16[multiplex_description.PL_PartA][i] - 1;
            }
          N1 = 0;
          N2 = N_MUX - N1;
          for (i = 0; i < 2; i++) //changed ON4QZ
            {
              ratesB[i] = RatesSM16[multiplex_description.PL_PartB][i] - 1;
            }
          for (i = 0; i < 2; i++)
            {
              L[i] = 2 * N1 * (RX[ratesA[i]] / RY[ratesA[i]]);
              L[i + 2] = (RX[ratesB[i]] * floor((2 * N2 - 12) / RY[ratesB[i]]));
            }
          rowdimL = 2;
          coldimL = 2;
          Lvspp = 0;
          xin1 = 2 * N1;
          xin2 = 2 * N2;
          if (Part_Deinterleaver != NULL) free(Part_Deinterleaver);
          Part_Deinterleaver = deinterleaver(xin1, 13, xin2, 13);
          for (i = 0; i < xin1 + xin2; i++)  Deinterleaver[i] = Part_Deinterleaver[i];
          free(Part_Deinterleaver);
          Part_Deinterleaver = deinterleaver(xin1, 21, xin2, 21);
          for (i = 0; i < xin1 + xin2; i++)  Deinterleaver[i + (xin1 + xin2)] = Part_Deinterleaver[i];
          for (i = 0; i < 2; i++)
            {
              PL[i] = ratesA[i];
              PL[i + 2] = ratesB[i];
            }
        }

      else if (msc_mode == 3)	/* 4-QAM */
        {
          //	  rylcm = (int) RY[multiplex_description.PL_PartA];
          ratesA[0] = (int) RX[multiplex_description.PL_PartA] - 1;
          N1 = 0;
          N2 = N_MUX - N1;
          ratesB[0] = (int) RX[multiplex_description.PL_PartB] - 1;
          if (robustness_mode == 0)
            {
              if (spectrum_occupancy == 0)
                {
                  L[0] = 0;
                  L[1] = 768;
                }
              else
                {
                  L[0] = 0;
                  L[1] = 837;
                }
            }
          if (robustness_mode == 1)
            {
              if (spectrum_occupancy == 0)

                {
                  L[0] = 0;
                  L[1] = 537;
                }
              else
                {
                  L[0] = 0;
                  L[1] = 627;
                }
            }
          if (robustness_mode == 2)
            {
              if (spectrum_occupancy == 0)

                {
                  L[0] = 0;
                  L[1] = 399;
                }
              else
                {
                  L[0] = 0;
                  L[1] = 435;
                }
            }
          Lvspp = 0;
          xin1 = 2 * N1;
          xin2 = 2 * N2;
          if (Part_Deinterleaver != NULL) free(Part_Deinterleaver);
          Part_Deinterleaver = deinterleaver(xin1, 21, xin2, 21);
          for (i = 0; i < xin1 + xin2; i++) Deinterleaver[i] = Part_Deinterleaver[i];
          PL[0] = 0;
          PL[1] = 6;
          rowdimL = 1;
          coldimL = 2;
        }
    }

  if (msc_parameters_valid != 0)
    {
      for (i = 0; i < lMSC; i++)
        {
          trxfrmbufptr = MSC_Demapper[frame_index - 1][i];
          received_real[i] = (double) transmission_frame_buffer[2 * trxfrmbufptr];
          received_imag[i] = (double) transmission_frame_buffer[2 * trxfrmbufptr + 1];

          MSC_cells_sequence[2 * i] = (float) received_real[i];
          MSC_cells_sequence[2 * i + 1] = (float) received_imag[i];
          transfer_function_MSC[i * 2] =  channel_transfer_function_buffer[2 * trxfrmbufptr];
          transfer_function_MSC[i * 2 + 1] = channel_transfer_function_buffer[2 * trxfrmbufptr + 1];
        }
      if (enough_frames == 0)
        {
          for (i = 0; i < lMSC; i++)
            {
              if ((MSC_Demapper[frame_index - 1][i] > max_index_equal_samples) || (MSC_Demapper[frame_index - 1][i] <  min_index_equal_samples))

                {
                  transfer_function_MSC[2 * i] = 0.0;
                  transfer_function_MSC[2 * i + 1] = 0.0;
                }
              SNR_estimation[i] =
                  sqrt(transfer_function_MSC[2 * i] *
                  transfer_function_MSC[i * 2] +
                  transfer_function_MSC[i * 2 + 1] * transfer_function_MSC[i * 2 + 1]);
            }

          n_SPPhard =
              msdhardmsc(received_real, received_imag, lMSC, SNR_estimation, N1,
                         L, rowdimL, coldimL, Lvspp, Deinterleaver, PL,
                         MSD_ITER, 1, SPPhard, VSPPhard, &res_iters,
                         &calc_variance, noise_signal);
          channel_decoded_data_buffer_data_valid = 2;
          length_decoded_data = n_SPPhard;
        }

      else
        {
          MSCAvailable=true;
          SNR_estimation_valid = 0;
          if (SNR_estimation_valid < 1)
            {
              for (i = 0; i < lMSC; i++)
                {
                  SNR_estimation[i] =
                      sqrt(transfer_function_MSC[2 * i] *
                      transfer_function_MSC[i * 2] +
                      transfer_function_MSC[i * 2 + 1] * transfer_function_MSC[i * 2 + 1]);
                }
            }
          else
            {
              for (i = 0; i < 461; i++)
                {
                  if (fabs(noise_power_density[i] - 0.0) < DBL_EPSILON) noise_power_density[i] = 1.0;
                }
              for (i = 0; i < lMSC; i++)
                {
                  if (noise_power_density[MSC_Demapper_symbolwise[frame_index - 1][i]] <= 0.0)
                    {
                      exit(EXIT_FAILURE);
                    }
                  part1 = sqrt(transfer_function_MSC[i * 2] *
                      transfer_function_MSC[i * 2] +
                      transfer_function_MSC[i * 2 +1] * transfer_function_MSC[i *2 + 1]);
                  part2 =sqrt(noise_power_density[MSC_Demapper_symbolwise[frame_index - 1][i]]);
                  SNR_estimation[i] = part1 / part2;
                }
            }


          n_SPPhard =  msdhardmsc(received_real, received_imag, lMSC, SNR_estimation, N1,
                         L, rowdimL, coldimL, Lvspp, Deinterleaver, PL,
                         MSD_ITER, 1, SPPhard, VSPPhard, &res_iters,
                         &calc_variance, noise_signal);
          length_decoded_data = n_SPPhard;
          sum1 = 0.0;
          sum2 = 0.0;
          for (i = 0; i < lMSC; i++)
            {
              squared_noise_signal[i] =
                  noise_signal[2 * i] * noise_signal[2 * i] +
                  noise_signal[2 * i + 1] * noise_signal[2 * i + 1];
              sum1 +=
                  (transfer_function_MSC[2 * i] * transfer_function_MSC[2 * i] +
                  transfer_function_MSC[2 * i +
                  1] * transfer_function_MSC[2 * i +
                  1]) *
                  squared_noise_signal[i];
              sum2 +=
                  (transfer_function_MSC[2 * i] * transfer_function_MSC[2 * i] +
                  transfer_function_MSC[2 * i +
                  1] * transfer_function_MSC[2 * i +
                  1]) *
                  mean_energy_of_used_cells;
            }

          //	  calc_weighted_variance = sum1 / sum2;
          for (i = 0; i < lMSC; i++)
            {
              squared_noise_signal_buffer[MSC_Demapper[frame_index - 1][i]] = squared_noise_signal[i];
            }
          /* now calc the sum of the reshaped "symbol_period" number of columns */
          for (i = 0; i < symbol_period; i++)
            {
              weighted_noise_power_density[i] = 0.0;
              for (j = 0; j < 6 * symbols_per_frame; j++)
                {
                  weighted_noise_power_density[i] += squared_noise_signal_buffer[i + j * symbol_period];
                }
            }

         for (i = 0; i < symbol_period; i++)	/* rows */
            for (j = 0; j < symbols_per_frame; j++)	/* columns */
              samples_resorted[i][j] = 0.0;

          for (i = 0; i < lMSC; i++)
           {
              totindex = (MSC_Demapper[frame_index - 1][i]) % (symbol_period * symbols_per_frame);
              posrow = totindex % symbol_period;
              poscolumn = totindex / symbol_period;
              samples_resorted[posrow][poscolumn] = squared_noise_signal[i] *
                  (transfer_function_MSC[i * 2] * transfer_function_MSC[i * 2] +
                  transfer_function_MSC[i * 2 + 1] * transfer_function_MSC[i * 2 + 1]);
            }
           for (i = 0; i < cnt_MSC_used_carriers; i++)
            {
              sum1 = 0.0;
              for (j = 0; j < symbols_per_frame; j++)
                sum1 += samples_resorted[MSC_used_carriers[i]][j];
              noise_power_density[MSC_used_carriers[i]] =
                  noise_power_density[MSC_used_carriers[i]] * (1.0 - 0.2) +
                  0.2 * sum1 / MSC_carrier_usage[MSC_used_carriers[i]];
            }
          if (SNR_estimation_valid < 1)
            {
              SNR_estimation_valid++;
            }
          channel_decoded_data_buffer_data_valid = 1;
        }
      if (Lvspp != 0)
        {
          VSPPlength =
              multiplex_description.stream_lengths[0][0] * 8 +
              multiplex_description.stream_lengths[1][0] * 8;
          HPPlength = 0;
          for (i = 0; i < no_of_streams; i++)
            {
              HPPlength += 8 * multiplex_description.stream_lengths[0][i];
            }
          n = 0;
          for (i = 0; i < HPPlength; i++) channel_decoded_data_buffer[n++] = SPPhard[i];
          for (i = 0; i < VSPPlength; i++) channel_decoded_data_buffer[n++] = VSPPhard[i];
          for (i = 0; i < n_SPPhard - HPPlength; i++)  channel_decoded_data_buffer[n++] = SPPhard[HPPlength + i];
        }
      else
        {
          for (i = 0; i < n_SPPhard; i++)
            {
              channel_decoded_data_buffer[i] = SPPhard[i];
            }
        }
    }
  frame_index = (frame_index % 6) + 1;
  transmission_frame_buffer_wptr =((transmission_frame_buffer_wptr +symbol_period * symbols_per_frame) % (symbol_period *symbols_per_frame * 6));
  return;
}

char getfacchar(double *facdata)
{
  char karakter;
  int macht, i;
  macht=64;
  karakter =0;
  for (i=0; i < 7 ; i++)
    {
      if (facdata[i] == 1.0)
        {
          karakter += macht;
        }
      macht /= 2;
    }
  return(karakter);
}


