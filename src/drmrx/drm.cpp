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



#include "drm.h"

//float acq_signal[2*DRMBUFSIZE]; //contains complex numbers
int input_samples_buffer_request;
int symbols_per_frame_list[4] = { 15, 15, 20, 24 };
int time_ref_cells_k_list[4][21] =
{
  {6, 7, 11, 12, 15, 16, 23, 29, 30, 33, 34, 38, 39, 41, 45, 46, 0, 0, 0, 0, 0},
  {6, 10, 11, 14, 17, 18, 27, 28, 30, 33, 34, 38, 40, 41, 44, 0, 0, 0, 0, 0, 0}, 
  {7, 8, 13, 14, 21, 22, 26, 27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0, 0, 0, 0, 0}
};
int time_ref_cells_theta_1024_list[4][21] =
  {
  {973, 205, 717, 264, 357, 357, 952, 440, 856, 88,   88, 68, 836, 836, 836, 1008, 0, 0, 0, 0, 0},
  {304, 331, 108, 620, 192, 704,  44, 432, 588, 844, 651,651, 651, 460, 950,    0, 0, 0, 0, 0, 0},
  {432, 331, 108, 620, 192, 704,  44, 304,   0,   0,   0,  0,   0,   0,   0,    0, 0, 0, 0, 0, 0},
  {1,     2,   3,   4,   5,   6,   7,   8,   9,  10,  11, 12,  13,  14,  15,   16, 0, 0, 0, 0, 0}
};

int y_list[4] = { 5, 3, 4, 3 };
int symbols_per_2D_window_list[4] = { 10, 6, 8, 6 };
int symbols_to_delay_list[4] = { 5, 3, 4, 3 };
float cpsd[513], psd[513];
int N_symbols_frequency_pilot_search = 15;
int K_min_K_max_list[2][24] =
  {
    {2, 2, -102, -114, -98, -110, 1, 1, -91, -103, -87, -99, 1, 1, 0, -69, 0, -67, 0, 0, 0, -44, 0, -43},
    {54, 58, 102, 114, 314, 350, 45, 51, 91,  103, 279, 311, 29,31, 0, 69, 0, 213, 0, 0, 0, 44, 0, 135}
};
float samplerate_offset_estimation;
float samplerate_offset;
int N_symbols_mode_detection;
int time_offset_log_last;
int transmission_frame_buffer_data_valid;
int fac_valid=0;
int no_of_unused_carriers_list[4] = { 2, 1, 1, 1 };
int freq_ref_cells_k_list[4][3] = { {9, 27, 36}, {8, 24, 32}, {5, 15, 20}, {5, 15, 20}};
int freq_ref_cells_theta_1024_list[4][3] = { {205, 836, 215}, {331, 651, 555}, {788, 1014, 332}, {788, 1014, 332}
};
int x_list[4] = { 4, 2, 1, 1 };
int k0_list[4] = { 2, 1, 1, 1 };
int dimw1024[4][2] = { {5, 3}, {3, 5}, {2, 10}, {3, 8} };      /* matrix[mode][n][m] */
int W_1024_list[4][5][10] = 
  { {{228, 341, 455, 0, 0, 0, 0, 0, 0, 0}, {455, 569, 683, 0, 0, 0, 0, 0, 0, 0}, {683, 796, 910, 0, 0, 0, 0, 0, 0, 0}, 
     {910, 0, 114, 0, 0, 0, 0, 0, 0, 0}, {114, 228, 341, 0, 0, 0, 0, 0, 0, 0}}, 
  {{512, 0, 512, 0, 512, 0, 0, 0, 0, 0}, {0, 512, 0, 512, 0, 0, 0, 0, 0, 0}, {512, 0, 512, 0, 512, 0, 0, 0, 0, 0}, 
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, 
  {{512, 0, 512, 0, 512, 0, 0, 0, 0, 0}, {0, 512, 0, 512, 0, 0, 0, 0, 0, 0}, 
   {512, 0, 512, 0, 512, 0, 0, 0, 0, 0}, {0, 512, 0, 512, 0, 0, 0, 0, 0, 0}, 
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, 
  {{465, 372, 279, 186, 93, 0, 931, 838, 745, 652}, 
   {931, 838, 745, 652, 559, 465, 372, 279, 186, 93}, 
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
};
int Z_256_list[4][5][10] = 
  { {{0, 81, 248, 0, 0, 0, 0, 0, 0, 0}, 
     {18, 106, 106, 0, 0, 0, 0, 0, 0, 0}, 
     {122, 116, 31, 0, 0, 0, 0, 0, 0, 0}, 
     {129, 129, 39, 0, 0, 0, 0, 0, 0, 0}, 
     {33, 32, 111, 0, 0, 0, 0, 0, 0, 0}}, 
  {{0, 57, 164, 64, 12, 0, 0, 0, 0, 0}, 
   {168, 255, 161, 106, 118, 0, 0, 0, 0, 0}, 
   {25, 232, 132, 233, 38, 0, 0, 0, 0, 0}, 
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, 
  {{0, 57, 164, 64, 12, 0, 0, 0, 0, 0}, {168, 255, 161, 106, 118, 0, 0, 0, 0, 0}, 
   {25, 232, 132, 233, 38, 0, 0, 0, 0, 0}, {168, 255, 161, 106, 118, 0, 0, 0, 0, 0}, 
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, 
  {{0, 240, 17, 60, 220, 38, 151, 101, 0, 0}, 
   {110, 7, 78, 82, 175, 150, 106, 25, 0, 0}, 
   {165, 7, 252, 124, 253, 177, 197, 142, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}} 
};
int Q_1024_list[4] = { 36, 12, 10, 14 };
int power_boost_list[4][6][4] = 
  { {{2, 6, 50, 54}, {2, 6, 54, 58}, {0, 0, 0, 0}, 
     {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, 
  {{1, 3, 43, 45}, {1, 3, 49, 51}, {0, 0, 0, 0}, 
   {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, 
  {{1, 29, 0, 0}, {1, 31, 0, 0}, {0, 0, 0, 0}, 
   {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, 
  {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}} 
};
int mode_and_occupancy_code_table[14] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
int robustness_mode, spectrum_occupancy_estimation;
int spectrum_occupancy;
int symbols_per_frame;
int time_ref_cells_cnt_list[4] = { 16, 15, 8, 16 };
int K_modulo, K_dc;
float mean_energy_of_used_cells;
int FAC_cells_k[65];
float transmission_frame_buffer[82980];        /* complex */
float channel_transfer_function_buffer[82980]; /* complex */
int transmission_frame_buffer_wptr = 0;
int lFAC;

//Display * display;
int runstate;
struct mplex_desc multiplex_description;
struct audio_info audio_information;
struct appl_info application_information;
struct stream_info stream_information;
struct time_info time_and_date;
struct dflttmsg default_text_message;
struct dfltdunitasmbly default_data_unit_assembly;
struct dfltMOTdirasmbly default_MOT_directory_assembly;
struct dfltMOTobjasmbly default_MOT_object_assembly;
struct dfltMOTobjasmblyinfo default_MOT_object_assembly_information;
struct dfltMOTobj default_MOT_object;
int channel_decoded_data_buffer_data_valid;
double channel_decoded_data_buffer[110000];
float WMERFAC;



/* char text_message[1000]; */ 
int audio_data_flag;
int length_decoded_data;
int MSC_Demapper[6][2959];
long bufaucnt[2048];

emscStatus msc_valid;
int bodyTotalSegments;
int rxSegments;
int currentSegmentNumber;
unsigned int rxTransportID;
QList<short unsigned int> drmBlockList;
sourceDecoder *srcDecoder;
uint txTransportID;
bool stopDRM;
float avgSNR;
float lastAvgSNR;
bool avgSNRAvailable;
QString drmCallsign;
bool drmBusy;


