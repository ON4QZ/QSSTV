#ifndef DRM_H
#define DRM_H
/*
*          File drm.h
*      
*          M.Bos - PA0MBO
*          Date feb 21st 2009
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



#include <fftw3.h>
#include "drmdefs.h"
#include "structtemplates.h"

#include <QString>
#include <QList>


class demodulator;
class sourceDecoder;
class ftpInterface;

extern int input_samples_buffer_request;
extern int symbols_per_frame_list[4];
extern int time_ref_cells_k_list[4][21];
extern int time_ref_cells_theta_1024_list[4][21];
extern int y_list[4];
extern int symbols_per_2D_window_list[4];
extern int symbols_to_delay_list[4];
extern int N_symbols_frequency_pilot_search;
extern int K_min_K_max_list[2][24];
extern float samplerate_offset_estimation;
extern float samplerate_offset; //ON4QZ
extern int N_symbols_mode_detection;
extern int time_offset_log_last;
extern int transmission_frame_buffer_data_valid;
extern int fac_valid;
extern int no_of_unused_carriers_list[4];
extern int freq_ref_cells_k_list[4][3];
extern int freq_ref_cells_theta_1024_list[4][3];
extern int x_list[4];
extern int k0_list[4];
extern int dimw1024[4][2]; /* matrix[mode][n][m] */
extern int W_1024_list[4][5][10];
extern int Z_256_list[4][5][10];
extern int Q_1024_list[4];
extern int power_boost_list[4][6][4];
extern int mode_and_occupancy_code_table[14];
extern int robustness_mode, spectrum_occupancy_estimation;
extern int spectrum_occupancy;
extern int symbols_per_frame;extern QString drmCallsign;
extern int time_ref_cells_cnt_list[4];
extern int K_modulo, K_dc;
extern float mean_energy_of_used_cells;
extern int FAC_cells_k[65];
extern float transmission_frame_buffer[82980];        /* complex */
extern float channel_transfer_function_buffer[82980]; /* complex */
extern int transmission_frame_buffer_wptr;
extern int lFAC;
extern float cpsd[513], psd[513];
extern bool drmBusy;

//Display * display;
extern int runstate;
extern struct mplex_desc multiplex_description;
extern struct audio_info audio_information;
extern struct appl_info application_information;
extern struct stream_info stream_information;
extern struct time_info time_and_date;
extern struct dflttmsg default_text_message;
extern struct dfltdunitasmbly default_data_unit_assembly;
extern struct dfltMOTdirasmbly default_MOT_directory_assembly;
extern struct dfltMOTobjasmbly default_MOT_object_assembly;
extern struct dfltMOTobjasmblyinfo default_MOT_object_assembly_information;
extern struct dfltMOTobj default_MOT_object;
extern int channel_decoded_data_buffer_data_valid;
extern double channel_decoded_data_buffer[110000];
extern float WMERFAC;

extern bool callsignValid;



/* char text_message[1000]; */ 
extern int audio_data_flag;
extern int length_decoded_data;
extern int MSC_Demapper[6][2959];
extern long bufaucnt[2048];

extern int spectrum_occupancy_new;
extern int msc_mode_new;
extern int interleaver_depth_new;
extern float freqOffset;
extern float deltaFS;
//extern char drmCallsign[9];
extern demodulator *demodulatorPtr;


enum emscStatus {INVALID,VALID,ALREADYRECEIVED};
extern emscStatus msc_valid;
extern int bodyTotalSegments;
extern int rxSegments;
extern int currentSegmentNumber;
extern unsigned int rxTransportID;
extern QList<short unsigned int> drmBlockList;
extern uint txTransportID;
extern bool stopDRM;
extern float avgSNR;
extern float lastAvgSNR;
extern bool avgSNRAvailable;
extern sourceDecoder *srcDecoder;
extern QString drmCallsign;


#endif


