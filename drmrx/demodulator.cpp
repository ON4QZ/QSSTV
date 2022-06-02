#include "demodulator.h"
#include "appglobal.h"
#include "drm.h"
#include <math.h>
#include <float.h>
#include "nrutil.h"
#include "supportfunctions.h"

#include <QDebug>


demodulator *demodulatorPtr;

void ludcmp(float **, int, int *, float *);
void lubksb(float **, int, int *, float *);

int Ts_list[DRMNUMMODES] = { Ts_A, Ts_B, Ts_C, Ts_D };
int Tu_list[DRMNUMMODES] = { Tu_A, Tu_B, Tu_C, Tu_D };
int Tg_list[DRMNUMMODES] = { Tg_A, Tg_B, Tg_C, Tg_D };

float FAC_cells_sequence[200];
bool  FACAvailable;
float deltaFS;
float freqOffset;
float tempbuf[30000];

demodulator::demodulator()
{
   addToLog("fftwf_plan_dft_1d start",LOGFFT);
   p1 = fftwf_plan_dft_1d(256,(fftwf_complex *)ss,(fftwf_complex *)S, FFTW_FORWARD, FFTW_ESTIMATE);
   addToLog("fftwf_plan_dft_1d stop",LOGFFT);
}

demodulator::~demodulator()
{
  fftwf_destroy_plan(p1);
}


void demodulator::init()
{
  int i,j,k;
  FACAvailable=false;
  N_symbols_mode_detection = 20;
  N_symbols_frequency_pilot_search = 15;
  time_offset_log_last = 0;
  symbol_counter = 0;
  N_samples_needed = N_symbols_mode_detection * 320;
  input_samples_buffer_request = N_samples_needed;
  SNR_time_out_counter = SNR_TIMEOUT;
  fac_not_valid_counter = FACVALIDCNTR;
  mode_and_occupancy_code_last = -1;
  timeSyncFlag = false;
  frequencySyncFlag = false;
  frameSyncFlag = false;
  doSynchronize=false;
  rsbufwidx = 0;
  symbufwidx = 0;
  iterationCounter=0;

  samplerate_offset_estimation = 0;
  samplerate_offset=0;
  smp_rate_conv_fft_phase_diff = 0;
  smp_rate_conv_fft_phase_offset = 0;
  smp_rate_conv_in_out_delay = 0;


  transmission_frame_buffer_data_valid = 0;

  counter=0;
  k = 0;
  for (i = 0; i < 4; i++)	// number of robustness modes
    {
      for (j = 0; j < 6; j++)	/* number of spectrumoccupancies */
        {
          no_of_used_cells_per_frame_list[k++] = (K_min_K_max_list[1][j + i * 6] - K_min_K_max_list[0][j + i * 6] + 1 - no_of_unused_carriers_list[i]) * symbols_per_frame_list[i];
        }
    }
  sigmaq_noise_list[0] = (float) pow(10.0, -16.0 / 10.0);
  sigmaq_noise_list[1] = (float) pow(10.0, -14.0 / 10.0);
  sigmaq_noise_list[2] = (float) pow(10.0, -14.0 / 10.0);
  sigmaq_noise_list[3] = (float) pow(10.0, -12.0 / 10.0);
  SNR_timeout_counter=0;
  delta_freq_offset=0;
}

bool demodulator::demodulate(float *sigin,int numSamples)
{
  int i;
  numberOfSamples=numSamples;
  transmission_frame_buffer_data_valid = 0;
  // fac_valid can be -1,0,1


  addToLog(QString("block %1 samples %2" ).arg(iterationCounter).arg(numSamples),LOGDRMDEMOD);
  //  if(iterationCounter<40) arrayDump(QString("DEM %1").arg(iterationCounter),sigin,128,true);
  //  arrayDump(QString("bl%1").arg(iterationCounter),sigin,32,true);
  //  logfile->addToAux(QString("block %1").arg(iterationCounter));
  //  arrayDump(QString("rs"),sigin,numSamples*2,true);
  //  if (fac_valid == 0)
  //    {
  //      fac_not_valid_counter--;
  //      if (fac_not_valid_counter <= 0)
  //        {
  //          doSynchronize = true;
  //          fac_not_valid_counter = FACVALIDCNTR;
  //        }
  //    }
  //  else
  //    {
  //      if (fac_valid == 1)
  //        {
  //          fac_not_valid_counter = FACVALIDCNTR;
  //        }
  //    }
  if (doSynchronize)
    {
      doSynchronize = false;
      init();
    }
  if (numberOfSamples> 0)
    {
      ++iterationCounter;
      //      logfile->addToAux(QString("iterationCounter %1").arg(iterationCounter));
      //      arrayDump("sig1",sigin,RXSTRIPE,true);
      //      logfile->addToAux(QString("rsbufwidx %1 offset %2").arg(rsbufwidx).arg(320 * N_symbols_mode_detection/2));

      //      arrayDump("sigD",&rs_buffer[320 * N_symbols_mode_detection],16,true);
      for(i=0;i<numberOfSamples;i++)
        {
          rs_buffer[(i + rsbufwidx)*2] = sigin[i*2];
          rs_buffer[(i + rsbufwidx) * 2 + 1] = sigin[i*2+1];
        }
      rsbufwidx += numberOfSamples;	/* index of next complex number to fill rs_buffer */
      int block=rsbufwidx/256;
      if(block>20)
        {
          for(i=0;i<(256*block);i++)tempbuf[i]=rs_buffer[2*i];
          psdmean(tempbuf, psd, 256, block);	/* globals   pa0mbo */
          for(i=0;i<(256*block);i++)tempbuf[i]=rs_buffer[2*i+1];
          psdmean(tempbuf, cpsd, 256, block);	/* globals   pa0mbo */
        }

    }
  if(!timeSync())
    {
      return false;
    }
  if(!frequencySync())
    {
      return false;
    }
  if(!frameSync())
    {
      return false;
    }

  if(channelEstimation())
    {
      channel_decoding();
      srcDecoder->decode();
    }
  if(doSynchronize)
    {
      // see if we have to save the data (for BSR and endOfImage)
      //     srcDecoder.checkSaveImage();
    }

  return true;
}

bool demodulator::timeSync()
{
  int i;
  samplerate_offset= samplerate_offset_estimation;
  if (!timeSyncFlag)
    {
      frequencySyncFlag=false;
      // enough data for time sync ?
      N_samples_needed = N_symbols_mode_detection * 320 - rsbufwidx;
      if (N_samples_needed > 0)
        {
          input_samples_buffer_request = N_samples_needed;
          return false;
        }
      spectrum_occupancy = -1;	/* -1 denotes unknown */
      getmode(rs_buffer, N_symbols_mode_detection * 320, &mode_block);

      robustness_mode = mode_block.mode_indx;
      time_offset = mode_block.time_offset;
      samplerate_offset_estimation = mode_block.sample_rate_offset;
      frequency_offset_fractional_init = mode_block.freq_offset_fract;
      time_offset_integer = (int) floor(time_offset + 0.5);
      if(robustness_mode!=99)
        {
          addToLog(QString("numSamples %1, robustmode %2,timeoffset %3,smplrateOffsetEst %4,freqOffsetFracInit %5,timeOffsetInteger %6")
                   .arg(numberOfSamples).arg( robustness_mode).arg(time_offset).arg(samplerate_offset_estimation).arg(frequency_offset_fractional_init).arg(time_offset_integer),LOGDRMDEMOD);
        }

      if ((fabsf(samplerate_offset_estimation) > 200.0E-5) && (robustness_mode != 99))
        {
          // sample_rate offset too large
          N_samples_needed = N_symbols_mode_detection * 320;
          input_samples_buffer_request = N_samples_needed;
          rsbufwidx = 0;
          //          samplerate_offset_estimation=0;
          return false;
        }
      if (robustness_mode != 99)
        {
          //          logfile->addToAux("timesync found");
          addToLog(QString("found robustness_mode: %1").arg(robustness_mode),LOGDRMDEMOD);
          timeSyncFlag = true;
          Ts = Ts_list[robustness_mode];
          Tu = Tu_list[robustness_mode];
          Tg = Tg_list[robustness_mode];
          Tgh = (int) floor(Tg / 2 + 0.5);
          symbols_per_frame = symbols_per_frame_list[robustness_mode];
          K_dc = Tu / 2;
          K_modulo = Tu;
          for (i = 0; i < 21; i++) time_ref_cells_k[i] = time_ref_cells_k_list[robustness_mode][i];
          for (i = 0; i < 21; i++) time_ref_cells_theta_1024[i] = time_ref_cells_theta_1024_list[robustness_mode][i];
          y = y_list[robustness_mode];
          symbols_per_2D_window = symbols_per_2D_window_list[robustness_mode];
          symbols_to_delay = symbols_to_delay_list[robustness_mode];

          //  symbol align rs_buffer
          rsbufwidx = rsbufwidx - time_offset_integer;
          for (i = 0; i < rsbufwidx; i++)
            {
              rs_buffer[i * 2] = rs_buffer[(i + time_offset_integer) * 2];
              rs_buffer[i * 2 + 1] =rs_buffer[(i + time_offset_integer) * 2 + 1];
            }
          counter++;
          (void) getofdm(NULL, 0.0, 0.0, 0.0, Ts, Tu, NULL, NULL, 1, 1, 1);	/* initialisation */
        }

      else
        {
          samplerate_offset_estimation = 0.0;
          int shift=320 * N_symbols_mode_detection;
          //          int shift=512;
          memmove(rs_buffer,&rs_buffer[shift*2],sizeof(float)*2*(rsbufwidx-shift));
          rsbufwidx -= shift;
          //          arrayDump("sig2",rs_buffer,16,true);
          //          for (i = 0; i < rsbufwidx; i++)	/* pa0mbo was rsbufwidx-1 ? */
          //            {
          //              rs_buffer[i * 2] = rs_buffer[(shift + i) * 2];
          //              rs_buffer[i * 2 + 1] = rs_buffer[(shift + i) * 2 + 1];
          //            }
          N_samples_needed = N_symbols_mode_detection * 320 - rsbufwidx;
          if (N_samples_needed > 0)
            {
              input_samples_buffer_request = N_samples_needed;
            }
          else
            {
              input_samples_buffer_request = 0;
            }
          return false;
        }
      addToLog(QString("timeSync found robustness mode:%1").arg(robustness_mode),LOGDRMDEMOD);
    }
  return true;
}



bool demodulator::frequencySync()
{
  int i,j;
  int sp_idx, K_min_, K_max_, K_dc_indx, K_dc_plus2_indx;
  int K_min_indx, K_min_minus4_indx, K_max_indx, K_max_plus1_indx;
  float tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
  //  float energy_ratio_K2_to_K0;
  float energy_ratio_K_max_to_K_max_p1;
  float energy_ratio_K_min_to_K_min_m4;
  float spectrum_occupancy_indicator[6];
  int t_smp;
  struct drmComplex S_buffer[288][20];	/* pa0mbo check */


  if (!frequencySyncFlag)
    {
      frameSyncFlag=false;
      //  enough f=date for pilot search?
      N_samples_needed = (N_symbols_frequency_pilot_search + 1) * Ts - (rsbufwidx);
      if (N_samples_needed > 0)
        {
          input_samples_buffer_request = N_samples_needed;
          return false;
        }
      Zi[0] = -1.0;
      delta_time_offset_integer = 0;
      freq_offset_init = frequency_offset_fractional_init;
      time_offset_fractional_init = time_offset - time_offset_integer;
      delta_time_offset_I_init = samplerate_offset_estimation * Ts;

      t_smp = 0;
      for (i = 0; i < N_symbols_frequency_pilot_search; i++)
        {
          delta_time_offset_integer = getofdm(&rs_buffer[2 * t_smp], time_offset_fractional_init,freq_offset_init, delta_time_offset_I_init, Ts, Tu, Zi,
              symbol_temp, 0, 1,1);
          for (j = 0; j < K_modulo; j++)
            {
              symbol_buffer[(i * K_modulo + j) * 2] = symbol_temp[j * 2];
              symbol_buffer[(i * K_modulo + j) * 2 + 1] = symbol_temp[j * 2 + 1];
            }
          t_smp = t_smp + Ts + delta_time_offset_integer;
        }

      freq_offset_integer = getfoffsint(symbol_buffer, N_symbols_frequency_pilot_search, K_dc,K_modulo, Tu);
      //  prepare for new round
      delta_time_offset_integer = 0;
      freq_offset_init = -freq_offset_integer + frequency_offset_fractional_init;
      time_offset_fractional_init = 0.0;
      Zi[0] = -1.0;

      // detmn frequency occupancy
      // start with reshaping symbol_buffer to S_buffer
      for (i = 0; i < N_symbols_frequency_pilot_search; i++)
        {
          for (j = 0; j < K_modulo; j++)
            {
              (S_buffer[j][i]).re = symbol_buffer[(j + i * K_modulo) * 2];
              (S_buffer[j][i]).im = symbol_buffer[(j + i * K_modulo) * 2 + 1];

            }
        }

      /*
       now per freq occupancy mode
     */
      for (sp_idx = 0; sp_idx < 2; sp_idx++)

        {
          K_min_ = K_min_K_max_list[0][sp_idx + robustness_mode * 6];
          K_max_ = K_min_K_max_list[1][sp_idx + robustness_mode * 6];

          if (K_min_ != K_max_)

            {
              K_dc_indx = (int) floor(freq_offset_integer / (2.0 * M_PI) + 0.5) + Tu / 2;

              K_dc_indx = K_dc_indx % Tu;
              K_dc_plus2_indx = (K_dc_indx + 2 + Tu) % Tu;
              K_min_indx = (K_dc_indx + K_min_ + Tu) % Tu;
              K_min_minus4_indx = (K_min_indx - 4 + Tu) % Tu;
              K_max_indx = (K_dc_indx + K_max_ + Tu) % Tu;
              K_max_plus1_indx = (K_max_indx + 1 + Tu) % Tu;

              //   calc energy ratios
              tmp1 = 0.0;
              tmp2 = 0.0;
              tmp3 = 0.0;
              tmp4 = 0.0;
              tmp5 = 0.0;
              tmp6 = 0.0;
              for (i = 0; i < N_symbols_frequency_pilot_search; i++)

                {
                  tmp1 +=
                      (S_buffer[K_dc_plus2_indx][i]).re *
                      (S_buffer[K_dc_plus2_indx][i]).re +
                      (S_buffer[K_dc_plus2_indx][i]).im *
                      (S_buffer[K_dc_plus2_indx][i]).im;
                  tmp2 +=
                      (S_buffer[K_dc_indx][i]).re *
                      (S_buffer[K_dc_indx][i]).re +
                      (S_buffer[K_dc_indx][i]).im * (S_buffer[K_dc_indx][i]).im;
                  tmp3 +=
                      (S_buffer[K_max_indx][i]).re *
                      (S_buffer[K_max_indx][i]).re +
                      (S_buffer[K_max_indx][i]).im *
                      (S_buffer[K_max_indx][i]).im;
                  tmp4 +=
                      (S_buffer[K_max_plus1_indx][i]).re *
                      (S_buffer[K_max_plus1_indx][i]).re +
                      (S_buffer[K_max_plus1_indx][i]).im *
                      (S_buffer[K_max_plus1_indx][i]).im;
                  tmp5 +=
                      (S_buffer[K_min_indx][i]).re *
                      (S_buffer[K_min_indx][i]).re +
                      (S_buffer[K_min_indx][i]).im *
                      (S_buffer[K_min_indx][i]).im;
                  tmp6 +=
                      (S_buffer[K_min_minus4_indx][i]).re *
                      (S_buffer[K_min_minus4_indx][i]).re +
                      (S_buffer[K_min_minus4_indx][i]).im *
                      (S_buffer[K_min_minus4_indx][i]).im;
                }
              //              energy_ratio_K2_to_K0 = tmp1 / tmp2;
              energy_ratio_K_max_to_K_max_p1 = tmp3 / tmp4;
              energy_ratio_K_min_to_K_min_m4 = tmp5 / tmp6;
              spectrum_occupancy_indicator[sp_idx] =
                  energy_ratio_K_min_to_K_min_m4 +
                  energy_ratio_K_max_to_K_max_p1;
            }

          else  spectrum_occupancy_indicator[sp_idx] = 0.0;
        }

      // detmn max in spectrum_occupancy_indicator and its index

      tmp1 = 0.0;
      for (sp_idx = 0; sp_idx < 2; sp_idx++)
        {
          if (spectrum_occupancy_indicator[sp_idx] > tmp1)

            {
              tmp1 = spectrum_occupancy_indicator[sp_idx];
              spectrum_occupancy_estimation = sp_idx;
            }
        }
      frequencySyncFlag = true;
      addToLog(QString("spectrum occupancy estimation:%1").arg(spectrum_occupancy_estimation),LOGDRMDEMOD);
    }
  return true;
}

bool demodulator::frameSync()
{
  int i,j,k;
  int symbol0;

  int symbol_no_to_equalize;
  int t_smp;
  if (!frameSyncFlag)
    {
      initChannelEstimation=true;
      // enough data ?
      N_symbols_needed = symbols_per_frame + symbols_per_2D_window - 1;
      N_samples_needed = (N_symbols_needed + 1) * Ts - (rsbufwidx);
      if (N_samples_needed > 0)
        {
          input_samples_buffer_request = N_samples_needed;
          return false;
        }

      t_smp = 0;
      for (i = 0; i < symbols_per_frame; i++)
        {
          delta_time_offset_integer =
              getofdm(&rs_buffer[2 * t_smp], time_offset_fractional_init,freq_offset_init, delta_time_offset_I_init, Ts, Tu, Zi,symbol_temp, 0, 1,1);
          for (j = 0; j < K_modulo; j++)
            {
              symbol_buffer[(i * K_modulo + j) * 2] = symbol_temp[j * 2];
              symbol_buffer[(i * K_modulo + j) * 2 + 1] = symbol_temp[j * 2 + 1];
            }
          t_smp = t_smp + Ts + delta_time_offset_integer;
        }

      //  search first symbol of frame using time ref cells
      symbol0 = getsymbolidx(symbol_buffer, symbols_per_frame, time_ref_cells_k,time_ref_cells_theta_1024, K_dc, K_modulo, 21);
      symbol_no_to_equalize =((symbol0 - symbols_to_delay + symbols_per_frame) % symbols_per_frame) + 1;

      frameSyncFlag = true;
      symbol_counter = 0;

      // frame align rs_buffer
      if (symbol_no_to_equalize != 1)
        {
          rsbufwidx -= (symbol_no_to_equalize - 1) * Ts;

          for (j = 0; j < rsbufwidx; j++)	/* pa0mbo was rsbufwidx  22-4-07 now better */
            {
              rs_buffer[j * 2] = rs_buffer[((symbol_no_to_equalize - 1) * Ts + j) * 2];
              rs_buffer[j * 2 + 1] = rs_buffer[((symbol_no_to_equalize - 1) * Ts + j) * 2 + 1];
            }
        }
      symbufwidx = 0;
      Zi[0] = -1.0;
      t_smp = 0;
      for(i=0;i<(Tu_A * 2 * 26);i++)
        {
          symbol_buffer[i]=0;
        }

      for (i = 0; i <symbols_per_2D_window; i++)

        {
          delta_time_offset_integer =  getofdm(&rs_buffer[2 * t_smp], time_offset_fractional_init,freq_offset_init, delta_time_offset_I_init, Ts, Tu, Zi,symbol_temp, 0, 1,1);
          //          arrayDump("sm1",symbol_temp,symbols_per_2D_window*2,true);
          for (j = 0; j < K_modulo; j++)
            {
              symbol_buffer[(i * K_modulo + j) * 2] = symbol_temp[j * 2];
              symbol_buffer[(i * K_modulo + j) * 2 + 1] = symbol_temp[j * 2 + 1];
            }

          symbufwidx++;
          t_smp += Ts + delta_time_offset_integer;
        }
      //       arrayDump("sm1",symbol_buffer,symbols_per_2D_window*K_modulo*2,true);
      // symbol align rs_buffer
      rsbufwidx -= t_smp;

      for (i = 0; i < rsbufwidx; i++)	/* pa0mbo was rxbufwidx 22-4-07 */
        {
          rs_buffer[i * 2] = rs_buffer[(i + t_smp) * 2];
          rs_buffer[i * 2 + 1] = rs_buffer[(i + t_smp) * 2 + 1];
        }
      for (i = 0; i < (2*70); i++)
        {
          next_pilots[i] = 1.1;	/* real part */  // modified joma was j now i
        }
      for (i = 0; i < 458; i++)
        {
          actual_pilots[i] = 0.0;	/* real part */  // modified joma was j now i
        }
      for(i=0;i<5;i++)
        for(j=0;j<208;j++)
          for (k=0;k<205;k++)
            {
              W_pilots_blk[i][j][k]=0;
            }

    }

  return true;
}

#define  MIN_ABS_H (8.0E-5 * 8.0E-5)

bool demodulator::channelEstimation()
{
  int i,j,k,m,p,temp;
  float t1, t2;
  float sigmaq_noise;
  int rndcnt;
  double rest;
  float a;
  int indx;
  int k_index1, k_index2, t1_pos, t2_pos;
  int k1_pos, k2_pos;
  double xsinc1, xsinc2;
  double f_cut_t, f_cut_k;
  //  double f_D_max;
  //  double tau_max;
  int *indxlu;
  float dlu = 0.0, *collu;
  float **amatrix;
  int NP,sortbrkpnt;
  //  float freq_offset_log[100];
  //  float time_offset_log[100];
  int cnt_time_ref_cells;
  int s, nnn, p_min, p_max, theta_1024;
  int sorttmp[1024], sorttmp2[1024];
  int t_smp,nn,ntwee,mtwee,cnt_actual_pilots_rel_indx;
  int ntc_indx;
  int trxbuf_indx, symbuf_indx;
  float hoek,tmpreal,tmpimag;
  float freq_offset;
  //  float h_absq[256];
  //  int lH;
  float tmp1, tmp2, tmp3;
  int gain_ref_cells_per_window;
  float temp1, temp2;
  float sum_MERFAC, sum_WMERFAC, sum_weight_FAC, SNR_dB;
  float MERFAC;
  float FAC_squared_noise_sequence[200];
  float squared_weight_sequence[200];

  //  channel estimation based on pilots

  N_symbols_needed = symbols_per_frame + symbols_per_2D_window - symbufwidx;	// pa0mbo 24-4-07
  N_samples_needed = (N_symbols_needed + 1) * Ts - (rsbufwidx);	// pa0mbo was rsbufwidx-1
  if (N_samples_needed > 0)
    {
      input_samples_buffer_request = N_samples_needed;
      return false;
    }
  //     now set the parameter spectrum_occupancy
  if (spectrum_occupancy <= 0)
    {
      if (spectrum_occupancy_estimation < 0) spectrum_occupancy = 3;
      else spectrum_occupancy = spectrum_occupancy_estimation;
    }
  if (spectrum_occupancy > 3) spectrum_occupancy = 3;	// 4 and 5 not yet supported
  mode_and_occupancy_code = mode_and_occupancy_code_table[robustness_mode * 6 + spectrum_occupancy];
  if (mode_and_occupancy_code < 0)
    {
      spectrum_occupancy = 1;
      mode_and_occupancy_code = mode_and_occupancy_code_table[robustness_mode * 6 + spectrum_occupancy];
    }

  if (mode_and_occupancy_code != mode_and_occupancy_code_last)
    {
      K_min = K_min_K_max_list[0][spectrum_occupancy + robustness_mode * 6];
      K_max = K_min_K_max_list[1][spectrum_occupancy + robustness_mode * 6];
      carrier_per_symbol = K_max - K_min + 1;
      (void) getofdmsync(NULL, Ts, Tu, NULL, K_max - K_min + 1, 0, NULL, NULL, 1, 1, 1);	/* initialisation */
      //  reformat pilot index stuff into th K_dc/K_modulo block
      //  first call listsinit to get gain_ref_cells_k etc
      //  Jan 5th 2009 changed listsinit() call to an include of all code from listsinit.c

      Tu = Tu_list[robustness_mode];
      Ts = Ts_list[robustness_mode];
      Tg = Ts - Tu;
      sigmaq_noise = sigmaq_noise_list[robustness_mode];
      symbols_per_frame = symbols_per_frame_list[robustness_mode];
      for (i = 0; i < 3; i++)
        {
          freq_ref_cells_k[i] = freq_ref_cells_k_list[robustness_mode][i];
          freq_ref_cells_theta_1024[i] = freq_ref_cells_theta_1024_list[robustness_mode][i];
        }
      cnt_time_ref_cells = time_ref_cells_cnt_list[robustness_mode];
      for (i = 0; i < cnt_time_ref_cells; i++)
        {
          time_ref_cells_k[i] = time_ref_cells_k_list[robustness_mode][i];
          time_ref_cells_theta_1024[i] =time_ref_cells_theta_1024_list[robustness_mode][i];
        }
      K_min =
          K_min_K_max_list[0][spectrum_occupancy_estimation +
          robustness_mode * 6];
      K_max =
          K_min_K_max_list[1][spectrum_occupancy_estimation +
          robustness_mode * 6];
      carrier_per_symbol = K_max - K_min + 1;
      for (i = 0; i < 4; i++)
        {
          power_boost[i] = power_boost_list[robustness_mode][spectrum_occupancy_estimation][i];
        }
      x = x_list[robustness_mode];
      y = y_list[robustness_mode];
      k0 = k0_list[robustness_mode];
      Q_1024 = Q_1024_list[robustness_mode];
      mean_energy_of_used_cells =(float) (no_of_used_cells_per_frame_list[spectrum_occupancy_estimation + robustness_mode * 6] + 3 + cnt_time_ref_cells);
      rndcnt = 0;
      for (s = 0; s < symbols_per_frame; s++)
        {
          nnn = s % y;
          m = (int) floor((double) (s / y));
          p_min = (int) ceil((double) ((K_min - k0 - x * nnn) / (x * y)));
          p_max = (int) floor((double) ((K_max - k0 - x * nnn) / (x * y)));
          for (p = p_min; p <= p_max; p++)

            {
              k = k0 + x * nnn + x * y * p;
              theta_1024 = (4*Z_256_list[robustness_mode][nnn][m]+p*W_1024_list[robustness_mode][nnn][m]+p*p*(1 +s)*Q_1024) % 1024;
              a = sqrtf(2.0);
              // power boost
              for (i = 0; i < 4; i++)
                {
                  if (k == power_boost[i]) a = 2;
                }

              // is time ref cell ?

              if (s == 0)
                {
                  for (i = 0; i < cnt_time_ref_cells; i++)
                    {
                      if (k == time_ref_cells_k[i])
                        {
                          indx = i;
                          theta_1024 = time_ref_cells_theta_1024[indx];
                          a = sqrtf(2.0);
                          mean_energy_of_used_cells -= 1.0;
                        }
                    }
                }

              //  is frequence reference cell?
              for (i = 0; i < 3; i++)
                {
                  if (k == freq_ref_cells_k[i])
                    {
                      indx = i;
                      theta_1024 = freq_ref_cells_theta_1024[indx];
                      if (robustness_mode == 3)
                        {
                          theta_1024 = (theta_1024 + 512 * s) % 1024;
                        }
                      a = sqrtf(2.0);
                      mean_energy_of_used_cells -= 1.0;
                    }
                }
              gain_ref_cells_k[rndcnt] = k + s * carrier_per_symbol;
              gain_ref_cells_theta_1024[rndcnt] = theta_1024;
              gain_ref_cells_a[rndcnt++] = a;
              mean_energy_of_used_cells =(float) (mean_energy_of_used_cells - 1.0 + a * a);
            }
        }
      mean_energy_of_used_cells /=no_of_used_cells_per_frame_list[spectrum_occupancy_estimation + robustness_mode * 6];

      addToLog(QString("mean_energy_of_used_cells %1").arg(mean_energy_of_used_cells),LOGDRMDEMOD);

      //   precompute 2-D Wiener filter matrix
      //       uses gain_ref_cells etc
      //       and rndcnt
      symbols_per_2D_window = symbols_per_2D_window_list[robustness_mode];
      symbols_to_delay = symbols_to_delay_list[robustness_mode];
      f_cut_t = 0.0675 / (1.0 * (double) y);
      f_cut_k = 1.75 * (float) Tg / (float) Tu;

      for(i=0;i<5;i++) cnt_tr_cells[i]=0;
      //    start nnn-loop
      for (nnn = 0; nnn < y; nnn++)
        {
          for (i = 0; i < rndcnt; i++)
            {
              training_cells_k[nnn][i] = (gain_ref_cells_k[i] - K_min +(symbols_per_frame -nnn) * carrier_per_symbol) % (symbols_per_frame * carrier_per_symbol) + K_min;

            }

          //          cnt_tr_cells[nnn] = 0;
          for (i = 0; i < rndcnt; i++)
            {
              if ((training_cells_k[nnn][i] - K_min) <(carrier_per_symbol * symbols_per_2D_window))
                {
                  gain_ref_cells_subset_index[nnn][cnt_tr_cells[nnn]] = i;
                  gain_ref_cells_subset[nnn][(cnt_tr_cells[nnn])++] =training_cells_k[nnn][i];
                }
            }
          cnt_next_pilot_cells[nnn] = 0;
          for (i = 0; i < rndcnt; i++)
            {
              if (((training_cells_k[nnn][i] - K_min) >= (carrier_per_symbol * symbols_per_2D_window)) &&
                  ((training_cells_k[nnn][i] - K_min) < carrier_per_symbol * (symbols_per_2D_window + 1)))

                {
                  next_pilot_cells_k_index[nnn][cnt_next_pilot_cells[nnn]] = i;
                  next_pilot_cells_k[nnn][(cnt_next_pilot_cells[nnn])++] = ((training_cells_k[nnn][i]- K_min) % carrier_per_symbol) + K_min;
                }
            }
          //  now sort training cells in subset if necessary
          sortbrkpnt = 0;
          for (i = 1; i < cnt_tr_cells[nnn]; i++)
            {
              if (gain_ref_cells_subset[nnn][i] < gain_ref_cells_subset[nnn][i - 1]) sortbrkpnt = i; // break in data ?
            }
          if (sortbrkpnt > 0)
            {
              // keep first part in sorttmp
              for (i = 0; i < sortbrkpnt; i++)
                {
                  sorttmp[i] = gain_ref_cells_subset[nnn][i];
                  sorttmp2[i] = gain_ref_cells_subset_index[nnn][i];	/* pa0mbo added 22-jan-2009 */
                }
              // now  shift smaller ones to start of vector
              for (i = 0; i < cnt_tr_cells[nnn] - sortbrkpnt; i++)
                {
                  gain_ref_cells_subset[nnn][i] = gain_ref_cells_subset[nnn][i + sortbrkpnt];
                  gain_ref_cells_subset_index[nnn][i] =gain_ref_cells_subset_index[nnn][i + sortbrkpnt];
                }

              //           replace last part from tmp

              for (i = cnt_tr_cells[nnn] - sortbrkpnt; i < cnt_tr_cells[nnn];i++)
                {
                  gain_ref_cells_subset[nnn][i] =sorttmp[i + sortbrkpnt - cnt_tr_cells[nnn]];
                  gain_ref_cells_subset_index[nnn][i] =sorttmp2[i + sortbrkpnt - cnt_tr_cells[nnn]];
                }
              sortbrkpnt = 0;
            }
          //  copy to training_cells_k
          for (i = 0; i < cnt_tr_cells[nnn]; i++)
            {
              training_cells_k[nnn][i] = gain_ref_cells_subset[nnn][i];
            }
          gain_ref_cells_per_window = cnt_tr_cells[nnn];
          for (k_index1 = 0; k_index1 < gain_ref_cells_per_window; k_index1++)
            {
              for (k_index2 = 0; k_index2 < gain_ref_cells_per_window;k_index2++)
                {
                  k1_pos = (((gain_ref_cells_subset[nnn][k_index1] -K_min)) % carrier_per_symbol) + K_min;
                  t1_pos = (gain_ref_cells_subset[nnn][k_index1] - K_min) / carrier_per_symbol;
                  k2_pos = ((gain_ref_cells_subset[nnn][k_index2] -K_min)) % carrier_per_symbol + K_min;
                  t2_pos = ((gain_ref_cells_subset[nnn][k_index2] - K_min)) / carrier_per_symbol;
                  xsinc1 = (k1_pos - k2_pos) * f_cut_k;
                  xsinc2 = (t1_pos - t2_pos) * f_cut_t;
                  if (k1_pos == k2_pos) xsinc1 = 1.0;
                  else
                    {
                      rest = sin(M_PI * xsinc1);
                      xsinc1 = rest / (M_PI * xsinc1);
                    }
                  if (fabs(xsinc2) < DBL_EPSILON) xsinc2 = 1.0;
                  else
                    {
                      rest = sin(M_PI * xsinc2);
                      xsinc2 = rest / (M_PI * xsinc2);
                    }
                  PHI[k_index1][k_index2] = (float) (xsinc1 * xsinc2);
                }
            }
          for (i = 0; i < gain_ref_cells_per_window; i++)
            {
              PHI[i][i] += sigmaq_noise * 2.0 /((gain_ref_cells_a[gain_ref_cells_subset_index[nnn][i]]) * (gain_ref_cells_a[gain_ref_cells_subset_index[nnn][i]]));
            }

          //   now the matrix inversion from numerical recipes
          NP = gain_ref_cells_per_window;
          amatrix = matrix(1, NP, 1, NP);
          indxlu = ivector(1, NP);
          collu = fvector(1, NP);
          for (i = 1; i <= NP; i++)
            for (j = 1; j <= NP; j++)
              amatrix[i][j] = PHI[i - 1][j - 1];
          ludcmp(amatrix, NP, indxlu, &dlu);	/* decompose just once */
          for (j = 1; j <= NP; j++)
            {
              for (i = 1; i <= NP; i++) collu[i] = 0.0;
              collu[j] = 1.0;
              lubksb(amatrix, NP, indxlu, collu);
              for (i = 1; i <= NP; i++) PHI_INV[i - 1][j - 1] = collu[i];
            }
          free_fvector(collu, 1, NP);
          free_ivector(indxlu, 1, NP);
          free_matrix(amatrix, 1, NP, 1, NP);

          for (k_index1 = 0; k_index1 < (K_max - K_min + 1); k_index1++)
            {
              for (k_index2 = 0; k_index2 < gain_ref_cells_per_window; k_index2++)
                {
                  k1_pos = k_index1 + K_min;
                  t1_pos = symbols_to_delay;
                  k2_pos = (training_cells_k[nnn][k_index2] - K_min) % (K_max -K_min + 1) + K_min;
                  t2_pos = (training_cells_k[nnn][k_index2] - K_min) / (K_max -K_min + 1);
                  xsinc1 = (k1_pos - k2_pos) * f_cut_k;
                  xsinc2 = (t1_pos - t2_pos) * f_cut_t;
                  if (k1_pos == k2_pos) xsinc1 = 1.0;
                  else
                    {
                      rest = sin(M_PI * xsinc1);
                      xsinc1 = rest / (M_PI * xsinc1);
                    }
                  if (t1_pos == t2_pos) xsinc2 = 1.0;
                  else
                    {
                      rest = sin(M_PI * xsinc2);
                      xsinc2 = rest / (M_PI * xsinc2);
                    }
                  THETA[k_index2] = (float) (xsinc1 * xsinc2);
                }
              // calc matrix product THETA*PHI_INV
              for (j = 0; j < NP; j++)

                {
                  W_symbol[j] = 0.0;
                  for (k = 0; k < NP; k++) W_symbol[j] += THETA[k] * PHI_INV[k][j];
                }
              for (j = 0; j < NP; j++)
                {
                  W_symbol_blk[nnn][j][k_index1] = W_symbol[j];
                }
            }
          for (k_index1 = 0; k_index1 < cnt_next_pilot_cells[nnn]; k_index1++)
            {
              for (k_index2 = 0; k_index2 < gain_ref_cells_per_window;k_index2++)

                {
                  k1_pos = next_pilot_cells_k[nnn][k_index1];
                  t1_pos = symbols_per_2D_window - 1;
                  k2_pos = (training_cells_k[nnn][k_index2] - K_min) % (K_max -K_min + 1) +K_min;
                  t2_pos = (training_cells_k[nnn][k_index2] - K_min) / (K_max -K_min + 1);
                  xsinc1 = (k1_pos - k2_pos) * f_cut_k;
                  xsinc2 = (t1_pos - t2_pos) * f_cut_t;
                  if (k1_pos == k2_pos) xsinc1 = 1.0;
                  else
                    {
                      rest = sin(M_PI * xsinc1);
                      xsinc1 = rest / (M_PI * xsinc1);
                    }
                  if (t1_pos == t2_pos) xsinc2 = 1.0;
                  else
                    {
                      rest = sin(M_PI * xsinc2);
                      xsinc2 = rest / (M_PI * xsinc2);
                    }
                  THETA[k_index2] = (float) (xsinc1 * xsinc2);
                }		/* end k_index2-loop */

              // calc matrix product THETA*PHI_INV
              for (j = 0; j < NP; j++)
                {
                  W_pilots[j] = 0.0;
                  for (k = 0; k < NP; k++) W_pilots[j] += THETA[k] * PHI_INV[k][j];
                }
              for (j = 0; j < NP; j++)
                {
                  W_pilots_blk[nnn][j][k_index1] = W_pilots[j];
                }
            }			/* end k_index1-loop */
        }			/* end nnn-loop  pa0mbo 26-5-2007 */


      for (i = 0; i < rndcnt; i++)
        {
          temp = ((gain_ref_cells_k[i] - K_min) / (carrier_per_symbol)) * (K_modulo - carrier_per_symbol);
          gain_ref_cells_k[i] += temp + K_dc - 1;
        }
      lFAC = mkfacmap(robustness_mode, K_dc, K_modulo, FAC_cells_k);
      mode_and_occupancy_code_last = mode_and_occupancy_code;

      for (i = 0; i < cnt_tr_cells[y-1]; i++) // joma y-1 was y
        {
          next_pilots[2 * i] = 0.0;	/* real part */  // modified joma was j now i
          next_pilots[2 * i + 1] = 0.0;	/* imag */
        }
      gain_ref_cells_per_frame = rndcnt;
      gain_ref_cells_per_y_symbols = rndcnt / (symbols_per_frame / y);
    }
  //  in matlab code here for display toctic_equalization = 0 etc

  t_smp = 0;

  for (i = 0; i < symbols_per_frame; i++)
    {
      symbol_counter++;
      //  shifted symbol index
      nn = (i - symbols_to_delay + symbols_per_frame) % symbols_per_frame;
      ntwee = nn % y;
      mtwee = nn / y;
      for (j = 0; j < cnt_tr_cells[ntwee]; j++)
        {
          gain_ref_cells_subset_nn[j] =(mtwee * gain_ref_cells_per_y_symbols + gain_ref_cells_subset_index[ntwee][j]) % gain_ref_cells_per_frame;
          training_cells_relative_index[j] =(gain_ref_cells_k[gain_ref_cells_subset_nn[j]] +(symbols_per_frame -nn) * K_modulo) % (K_modulo * symbols_per_frame);
        }

      cnt_actual_pilots_rel_indx = 0;
      //     logfile->addToAux(QString("block %1").arg(iterationCounter));
      for (j = 0; j < cnt_tr_cells[ntwee]; j++)
        {
          if ((training_cells_relative_index[j] - (symbols_per_2D_window-1) * K_modulo) >= 0)
            {
              actual_pilots_relative_index[cnt_actual_pilots_rel_indx++] = j;
            }
          ntc_indx = training_cells_relative_index[j] + i * K_modulo + 1;	/* pa0mbo in  matlab +1 =OK trcrindx 1 lager dan in M   */

          hoek =(float) (2.0 * M_PI *(float)gain_ref_cells_theta_1024[gain_ref_cells_subset_nn[j]] /1024.0);

          tmpreal =(float) (cos(hoek) / gain_ref_cells_a[gain_ref_cells_subset_nn[j]]);
          tmpimag =(float) (-sin(hoek) /gain_ref_cells_a[gain_ref_cells_subset_nn[j]]);
          normalized_training_cells[2 * j] = symbol_buffer[2 * ntc_indx] * tmpreal - symbol_buffer[2 * ntc_indx + 1] * tmpimag;	/* real part */
          normalized_training_cells[2 * j + 1] = symbol_buffer[2 * ntc_indx + 1] * tmpreal + symbol_buffer[2 * ntc_indx] * tmpimag;	/* imag part */
        }


      for (j = 0; j < cnt_actual_pilots_rel_indx; j++)
        {
          actual_pilots[2 * j] = normalized_training_cells[2 * (actual_pilots_relative_index[j])];	/* real part */
          actual_pilots[2 * j + 1] = normalized_training_cells[2 * (actual_pilots_relative_index[j]) + 1];	/* imag */
        }
      temp1 = 0.0;
      temp2 = 0.0;
      for (j = 0; j < cnt_actual_pilots_rel_indx; j++)
        {
          temp1 += actual_pilots[2 * j] * next_pilots[2 * j] + actual_pilots[2 * j + 1] * next_pilots[2 * j + 1];	/* real part */
          temp2 += actual_pilots[2 * j] * next_pilots[2 * j + 1] -actual_pilots[2 * j + 1] * next_pilots[2 * j];

        }
      if (i != 0)  delta_freq_offset = (float) atan2(temp2, temp1 + MIN_ABS_H);

      for (j = 0; j < K_max - K_min + 1; j++)
        {
          H[2 * j] = 0.0;
          H[2 * j + 1] = 0.0;
          for (k = 0; k < cnt_tr_cells[ntwee]; k++)
            {
              H[2 * j] += normalized_training_cells[2 * k] * W_symbol_blk[ntwee][k][j];
              H[2 * j + 1] += normalized_training_cells[2 * k + 1] * W_symbol_blk[ntwee][k][j];
            }
        }



      for (j = 0; j < cnt_actual_pilots_rel_indx; j++)
        {
          next_pilots[2 * j] = 0.0;
          next_pilots[2 * j + 1] = 0.0;
          for (k = 0; k < cnt_tr_cells[ntwee]; k++)
            {
              next_pilots[2 * j] +=  normalized_training_cells[2 * k] * W_pilots_blk[ntwee][k][j];
              next_pilots[2 * j + 1] += normalized_training_cells[2 * k + 1] * W_pilots_blk[ntwee][k][j];

            }
        }



      for (j = K_min; j <= K_max; j++)
        {
          trxbuf_indx = transmission_frame_buffer_wptr + i * K_modulo + K_dc + j;
          symbuf_indx = (i + symbols_to_delay) * K_modulo + K_dc + j;
          tmp1 = H[2 * (j - K_min)] * H[2 * (j - K_min)] + H[2 * (j - K_min) +1] * H[2 * (j -K_min) +1] + MIN_ABS_H;
          tmp2 = H[2 * (j - K_min)] / tmp1;
          tmp3 = -H[2 * (j - K_min) + 1] / tmp1;
          transmission_frame_buffer[2 * trxbuf_indx] =symbol_buffer[2 * symbuf_indx] * tmp2 -symbol_buffer[2 * symbuf_indx + 1] * tmp3;
          transmission_frame_buffer[2 * trxbuf_indx + 1] =symbol_buffer[2 * symbuf_indx] * tmp3 +symbol_buffer[2 * symbuf_indx + 1] * tmp2;
          channel_transfer_function_buffer[2 * trxbuf_indx] =H[2 * (j - K_min)];
          channel_transfer_function_buffer[2 * trxbuf_indx + 1] =H[2 * (j - K_min) + 1];
        }

      //  get next symbol
      if ((iterationCounter>=0) && (iterationCounter<=180))
        {
          //          arrayDump("b179",&rs_buffer[2 * t_smp],Ts,true);
          //          logfile->addToAux(QString("block %1").arg(iterationCounter));
          //          arrayDump("h179",H,K_max - K_min + 1,true);
        }
      delta_time_offset_integer =getofdmsync(&rs_buffer[2 * t_smp], Ts, Tu, H, K_max - K_min + 1,delta_freq_offset, Zi, symbol_temp, 0, 1,1);
      //      arrayDump("symtmp",symbol_temp,K_modulo,true);

      //      addToLog(QString("delta_time_offset_integer %1").arg(delta_time_offset_integer),LOGDRMDEMOD);
      for (j = 0; j < K_modulo; j++)
        {
          symbol_buffer[(symbufwidx * K_modulo + j) * 2] = symbol_temp[j * 2];
          symbol_buffer[(symbufwidx * K_modulo + j) * 2 + 1] = symbol_temp[j * 2 + 1];
        }
      symbufwidx++;
      t_smp += Ts + delta_time_offset_integer;	/* next symbol */
    }				/* end i-loop over symbols_per_frame */


  // fac_valid=0;

  freq_offset = Zi[4];
  time_offset_fractional = Zi[5];
  delta_time_offset_I = Zi[2];
  addToLog("GREEN on channel estimation",LOGDRMDEMOD);

  //  clock too slow or too fast: adjust playing speed

  smp_rate_conv_fft_phase_diff = 4 * (t_smp - N_symbols_needed * Ts);
  smp_rate_conv_fft_phase_offset = 4 * time_offset_fractional;
  smp_rate_conv_in_out_delay += smp_rate_conv_fft_phase_diff;


  addToLog(QString("Rate %1 %2 %3").arg(smp_rate_conv_fft_phase_diff).arg(smp_rate_conv_fft_phase_offset).arg(smp_rate_conv_in_out_delay),LOGDRMDEMOD);
  //    display results
  freqOffset=-freq_offset * 12000.0 / (float) Tu / (2.0 * M_PI);
  deltaFS= (1.0 / (delta_time_offset_I / Ts + 1.0) - 1.0) ;
  samplerate_offset=delta_time_offset_I /Ts;


  //   symbol align rs_buffer



  //     swap symbol buffer

  symbufwidx -= symbols_per_frame;
  for (i = 0; i < symbufwidx * K_modulo; i++)
    {
      symbol_buffer[i * 2] = symbol_buffer[2 * symbols_per_frame * K_modulo + 2 * i];
      symbol_buffer[i * 2 + 1] = symbol_buffer[2 * symbols_per_frame * K_modulo + 2 * i + 1];
    }

  //     SNR estimation using FAC  cells
  //     mean energy of used cells set in listsinit()

  sum_MERFAC = 0.0;
  sum_WMERFAC = 0.0;
  sum_weight_FAC = 0.0;
  for (i = 0; i < lFAC; i++)

    {
      trxbuf_indx = transmission_frame_buffer_wptr + FAC_cells_k[i];	/* pa0mbo 18-5-2007 checked  */
      FAC_cells_sequence[i * 2] = transmission_frame_buffer[2 * trxbuf_indx];
      FAC_cells_sequence[i * 2 + 1] = transmission_frame_buffer[2 * trxbuf_indx + 1];
      t1 = (float) (fabs(FAC_cells_sequence[i * 2]) - sqrt(0.5));
      t1 = t1 * t1;
      t2 = (float) (fabs(FAC_cells_sequence[i * 2 + 1]) - sqrt(0.5));
      t2 = t2 * t2;
      FAC_squared_noise_sequence[i] = t1 + t2;
      t1 = channel_transfer_function_buffer[trxbuf_indx * 2];
      t1 = t1 * t1;
      t2 = channel_transfer_function_buffer[trxbuf_indx * 2 + 1];
      t2 = t2 * t2;
      squared_weight_sequence[i] = t1 + t2;
      sum_MERFAC += FAC_squared_noise_sequence[i];
      sum_WMERFAC += FAC_squared_noise_sequence[i] * (squared_weight_sequence[i] + 1.0E-10);
      sum_weight_FAC += squared_weight_sequence[i];
    }

  FACAvailable=true;
  avgSNRAvailable=true;
  MERFAC = (float) (log(sum_MERFAC / lFAC + 1.0E-10));
  MERFAC /= (log(10.0));
  MERFAC *= -10.0;
  WMERFAC =(float) (log(sum_WMERFAC /(mean_energy_of_used_cells * (sum_weight_FAC + lFAC * 1.0E-10))));
  WMERFAC /= (log(10.0));
  WMERFAC *= -10.0;
  SNR_dB = WMERFAC;

  addToLog(QString("SNR-FAC =%1").arg(SNR_dB),LOGDRMDEMOD);
  //  N_samples_needed = (symbols_per_frame + 1) * Ts - rsbufwidx;

  if (SNR_dB < SNR_MIN_DB)
    {
      SNR_timeout_counter--;
      if (SNR_timeout_counter <= 0)
        {
          doSynchronize = true;
          SNR_timeout_counter = SNR_TIMEOUT;
        }
      transmission_frame_buffer_data_valid = 0;
      fac_not_valid_counter--;
      if (fac_not_valid_counter <= 0)
        {
          doSynchronize = true;
          fac_not_valid_counter = FACVALIDCNTR;
        }
    }
  else
    {
      SNR_timeout_counter = SNR_TIMEOUT;
      transmission_frame_buffer_data_valid = 1;
      fac_valid = 1;
    }
  if (doSynchronize) N_samples_needed = N_symbols_mode_detection * 320 - rsbufwidx;
  else
    {
      rsbufwidx -= t_smp;
      for (i = 0; i < rsbufwidx; i++)
        {
          rs_buffer[i * 2]     = rs_buffer[(i + t_smp) * 2];
          rs_buffer[i * 2 + 1] = rs_buffer[(i + t_smp) * 2 + 1];
        }
      N_samples_needed = (symbols_per_frame + 1) * Ts - rsbufwidx;
    }
  if (N_samples_needed > 0)
    {
      input_samples_buffer_request = N_samples_needed;
    }
  else
    {
      input_samples_buffer_request = 0;
    }
  return true;
}
