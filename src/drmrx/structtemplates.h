/*
*    File structtemplates.h
*    part of package RXAMADRM
*    M.Bos - PA0MBO
*    Date feb 21st 2009
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


struct mplex_desc
{
  int stream_lengths[2][6];
   int PL_PartA;
   int PL_PartB;
   int HM_length;
   int PL_HM;
 };
struct audio_info
{
  int ID[4];
   int stream_ID[4];
   int audio_coding[4];
   int SBR_flag[4];
   int audio_mode[4];
   int sampling_rate[4];
   int text_flag[4];
   int enhancement_flag[4];
   int coder_field[4];
   int bytes_per_frame[4];
 };
struct appl_info
{
  int ID[4];
   int stream_ID[4];
   int packet_mode[4];
   int data_unit_indicator[4];
   int packet_ID[4];
   int enhancement_flag[4];
   int application_domain[4];
   int packet_length[4];
   int application_data[4][100];       /* pa0mbo checken for max later */
   int appl_data_length[4];
   int user_application_type[4];
   int user_application_identifier[4];
   char label[4][100];         /*  pa0mbo check max 100 */
   char language[4][100];      /* ibid */
   char country[4][100];       /* inid */
   int language_code[4];
   int programme_type_code[4];
   int bytes_per_frame[4];
 };
struct stream_info
{
  int number_of_audio_services;
   int number_of_data_services;
   int number_of_streams;
   int number_of_audio_streams;
   int number_of_data_streams;
   int audio_streams[4];
   int data_streams[4];
   int audio_services[4];
   int data_services[4];
 };
struct time_info
{
  int day;
   int month;
   int year;
   int hours;
   int minutes;
 };
struct dflttmsg
{
  int stream_no;
   int current_toggle;
   int first_last_flag;
   int command_flag;
   double field1;
   double field2;
   char segments[8][256];      /* pa0mbo check max dim 100 later */
   char string[256];
   int CRC_error;
   int first_seg_received;
   char current_segment[256];  /* check dim 256 later pa0mbo */
   int current_segment_no;
   int current_segment_length;
 };
struct dfltdunitasmbly
{
  int ID;
   int first_packet_received;
   int packet_ID;
   int continuity_index;
   int application_domain;
   char application_data[120000];      /* pa0mbo check max dim 1200 */
   int CRC_error;
   char data_unit[120000];
   int cnt_data_unit;
 };
struct dfltMOTdirasmbly
{
  int transport_ID;
   int continuity_index;
   int current_segment_no;
   int body_complete;
   char body[120000];          /* pa0mbo check max dim 12000 */
   int bodycnt;
 };
struct dfltMOTobjasmbly
{
  int transport_ID;
   int header_continuity_index;
   int current_header_segment_no;
   int body_continuity_index;
   int current_body_segment_no;
   int header_complete;
   int body_complete;
   int doDelete;
   char header[900];           /* pa0mbo check max 900 dim */
   int hdrcnt;
   //char body[120000];          /* pa0mbo check max dim 100 */
   char  body[140000];       //joma   /* pa0mbo check max dim 100 */
   int bodycnt;
 };
struct dfltMOTobjasmblyinfo
{
  int transport_ID[120000];    /* pa0mbo check max dim 100 */
   int cntID;
 };
struct dfltMOTobj
{
  int ID;
   int content_type;
   int content_subtype;
   int creation_time;
   int start_validity;
   int expire_time;
   int trigger_time;
   int version_number;
   unsigned char repetition_distance[100];     /* pa0mbo check max 100 dim */
   unsigned char group_reference[100]; /* pa0mbo check */
   int priority;
   char label[16];
   char content_name[30];      /* pa0mbo check max dim 30 */
   char content_description[100];      /* pa0mbo check later */
   unsigned char mime_type[100];       /* pa0mbo chack 100 dim later */
   unsigned char compression_type;
   unsigned char header[900];
   int hdrcnt;
   unsigned char body[1200000];        /* chck these dims 100 later */
   int bodycnt;
   int error;
 };


struct shadowMOTobj
{
   int transport_ID;
   int hdr_segment_count ;
   int hdr_highest_segm ;
   int hdr_segment_lengths[10];
   int body_segment_lengths[2000];
   char header [10][250];
   int body_segment_count ;
   int body_highest_segm;
   int body_cnt;
   int header_cnt;
   char body[2000][250];
   int body_complete;
   int header_complete;
};

