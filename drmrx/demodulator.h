#ifndef DEMODULATOR_H
#define DEMODULATOR_H
#include <fftw3.h>
#include "drmproto.h"
#include "drmdefs.h"
#include "sourcedecoder.h"


#define SNR_TIMEOUT  10
#define FACVALIDCNTR 10
#define SNR_MIN_DB 3
#define DRMNUMMODES 4


extern int Ts_list[DRMNUMMODES];
extern int Tu_list[DRMNUMMODES];
extern int Tg_list[DRMNUMMODES];
extern bool FACAvailable;

class demodulator
{
public:
  demodulator();
  ~demodulator();
  void init();
  bool demodulate(float *sigin, int  numSamples);
  bool isTimeSync() {return timeSyncFlag;}
  bool isFrequencySync() {return frequencySyncFlag;}
  bool isFrameSync() {return frameSyncFlag;}
  bool isFACAvailable() {return FACAvailable;}
private:
  int iterationCounter;
  bool timeSync();
  bool frequencySync();
  bool frameSync();
  bool channelEstimation();
  int symbol_counter;
  int N_samples_needed;
  int SNR_time_out_counter;
  int fac_not_valid_counter;
  int mode_and_occupancy_code_last;
  int rsbufwidx;
  int symbufwidx;
  int smp_rate_conv_fft_phase_diff;
  float smp_rate_conv_fft_phase_offset;
  int smp_rate_conv_in_out_delay;
  fftwf_plan p1;
  drmComplex ss[256], S[256];
//  float rs_buffer[DRMBUFSIZE];
  float rs_buffer[10*8000];
  bool doSynchronize;
  bool timeSyncFlag;
  bool frequencySyncFlag;
  bool frameSyncFlag;
  int numberOfSamples;
  smode_info mode_block;
  float time_offset;
  float frequency_offset_fractional_init;
  int time_offset_integer;
  int counter;
  int Ts, Tu, Tg, Tgh;
  float freq_offset_integer;
  int x,y, k0;
  int symbols_per_2D_window;
  int delta_time_offset_integer;
  float time_offset_fractional_init;
  float freq_offset_init;
  float delta_time_offset_I_init;
  float Zi[6];
  float symbol_temp[2 * Tu_A];
  float symbol_buffer[Tu_A * 2 * 26];
  int time_ref_cells_k[21];
  int time_ref_cells_theta_1024[21];
  int symbols_to_delay;
  int N_symbols_needed;
  int no_of_used_cells_per_frame_list[24];
  float sigmaq_noise_list[4];
  int gain_ref_cells_k[712];
  int gain_ref_cells_theta_1024[712];
  float gain_ref_cells_a[712];
  int training_cells_k[5][712];
  int cnt_tr_cells[5];
  int gain_ref_cells_per_frame;
  int gain_ref_cells_per_y_symbols;
  float next_pilots[2*70];	/* complex */   //joma
  int K_min,K_max;
  float W_symbol_blk[5][208][229];
  float W_symbol[208];
  float W_pilots_blk[5][208][205];
  float W_pilots[208];
  int SNR_timeout_counter;
  float delta_freq_offset;
  int mode_and_occupancy_code;
  int carrier_per_symbol;
  int freq_ref_cells_k[3];
  int freq_ref_cells_theta_1024[3];
  int power_boost[4];
  int Q_1024;
  int gain_ref_cells_subset[5][209];
  int gain_ref_cells_subset_index[5][209];
  int cnt_next_pilot_cells[5];
  int next_pilot_cells_k_index[5][40];
  int next_pilot_cells_k[5][40];
  float PHI[208][208];
  float PHI_INV[208][208];
  float THETA[208];
  int gain_ref_cells_subset_nn[209];
  int training_cells_relative_index[209];
  int actual_pilots_relative_index[209];
  float normalized_training_cells[1424];
  float actual_pilots[458];
  float H[458];
  float delta_time_offset_I;
  float time_offset_fractional;
  bool initChannelEstimation;
};

#endif // DEMODULATOR_H
