
/******************************************************************************/  

/*                                                                            */ 

/*  University of Kaiserslautern, Institute of Communications Engineering     */ 

/*  Copyright (C) 2004 Torsten Schorr                                         */ 

/*                                                                            */ 

/*  Author(s)    : Torsten Schorr (schorr@eit.uni-kl.de)                      */ 

/*  Project start: 15.06.2004                                                 */ 

/*  Last change  : 30.06.2004                                                 */ 

/*                                                                            */ 

/******************************************************************************/ 

/*                                                                            */ 

/*  This program is free software; you can redistribute it and/or modify      */ 

/*  it under the terms of the GNU General Public License as published by      */ 

/*  the Free Software Foundation; either version 2 of the License, or         */ 

/*  (at your option) any later version.                                       */ 

/*                                                                            */ 

/*  This program is distributed in the hope that it will be useful,           */ 

/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */ 

/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */ 

/*  GNU General Public License for more details.                              */ 

/*                                                                            */ 

/*  You should have received a copy of the GNU General Public License         */ 

/*  along with this program; if not, write to the Free Software               */ 

/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */ 

/*                                                                            */ 

/******************************************************************************/ 
  

/******************************************************************************/ 

/*                                                                            */ 

/*  viterbi_decode.h (part of msd_hard)                                       */ 

/*                                                                            */ 

/******************************************************************************/ 

/*  Description:                                                              */ 

/*  function headers and constants for viterbi_decode.c                       */ 

/*                                                                            */ 

/******************************************************************************/ 
  
#ifndef	STATES
#define		STATES 64			/* number of states */
#endif  /*  */
#ifndef	NOOFBF
#define		NOOFBF 32			/* number of butterflies */
#endif  /*  */
#ifndef QOFB
#define		QOFB 8				/* number of different output patterns per input symbol */
#endif  /*  */
#ifndef NOPB
#define		NOPB 4				/* number of parity bits */
#endif  /*  */
  

/* decoder output for zero input, states 1:32  */ 
const char CODER_OUTPUT[] = { 0, 6, 3, 5, 3, 5, 0, 6, 4, 2, 7, 1, 7, 1, 4, 2, 1, 7, 2, 4, 2, 4, 1, 7, 5, 3, 6, 0, 6, 0, 5, 3 };
int viterbi_decode (float *llr, int N, int N_PartA, signed char *puncturing1, signed char *puncturing2,
                       signed char *puncturing3, char *infoout, char *cwout, int bitpos, int *Deinterleaver, int L, int N_tail,
                       char *memory_ptr);

