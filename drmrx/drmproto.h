#ifndef DRMPROTO_H
#define DRMPROTO_H
/*
*   file drmproto.h
*
*   defines the protoypes for project RXAMADRM
*
*   PA0MBO - M.BOS
*
*   Date Feb 21st 2009
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

struct smode_info
{
  int mode_indx;
   float time_offset;
   float sample_rate_offset;
   float freq_offset_fract;
};

struct soutblock_ofdm
{
  int OKflag;
   float phifcl;
   int dtoffsI;
   int dfoffsI;
   float foffs;
   float toffsfr;
 };

void init_audio (void);
//int monorec (short int *,int n);
//void drmfilter (float *, float *, float *, float *, int, int);
int demodulate ( /*@null@ */ float *, int, int);
void drmfilter1c (float *, float *, float *, int, int);
void drmfilter1 (float *, float *, float *, int, int);
void initGetmode(int n);
void getmode (/*@null@ */ float *, int, /*@null@ *//*@out@ */ smode_info *);
float getfoffsint (float *, int, int, int, int);
int resample (float *, /*@out@ */ float *, float, int, int);
int getofdm ( /*@null@ */ float *, float, float, float, int, int, 
              /*@null@ */ float *, /*@null@ */ float *, int, int, int);
int getsymbolidx (float *, int, int *, int *, int, int, int);
int getofdmsync ( /*@null@ */ float *, int, int, /*@null@ */ float *, int, 
                  float, /*@null@ */ float *, /*@null@ */ float *, int, int, int);
int mkfacmap (int, int, int, int /*@out@ */ *);
void showFAC ( /*@null@ */ float *, int, /*@null@ */ char argv[], int argc, int);
int *deinterleaver (int, int, int, int);
int msdhardfac ( /*@out@ */ double *, /*@out@ */ double *, int, /*@out@ */ double *, int, double *, 
                 int, int, int *, int *, int, int, double *);
int mkmscmap (int, int, int, int, int);
int msdhardsdc (double *, double *, int, double *, int, double *, int, int, int, int *, int *, int, int, double *);
//void crc16_c (double *, double *, int);



/*void copy_mplex_desc(struct mplex_desc *, struct mplex_desc *);
int cmp_mplex_desc(struct mplex_desc *, struct mplex_desc *); */ 
int msdhardmsc (double *, double *, int, double *, int, double *, int, int, int, int *, int *, int, int, /*@out@ */ double *,
                /*@out@ */ double *, 
                double *, double *, double *);
void bits2bytes (double *, int, unsigned char /*@out@ */ *);
void crc16_bytewise (double /*@out@ */ *, unsigned char *, long);
int deflate_uncompress (unsigned char *, int, char /*@out@ */ *, int, 
                         unsigned char /*@out@ */ *, int *, double /*@out@ */ *);
float **matrix (long, long, long, long);
int *ivector (long, long);
float *fvector (long, long);
void channel_decoding (void);
void source_decoding (void);
void crc8_c (double *, double *, int);
void psdmean (float *, float *, int, int);
void psdcmean (float *, float *, int, int);
struct drmComplex
{
  float re;
  float im;
};

#endif


