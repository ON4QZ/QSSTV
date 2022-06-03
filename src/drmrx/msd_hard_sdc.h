
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

/*  msd_hard.h (part of msd_hard)                                             */ 

/*                                                                            */ 

/******************************************************************************/ 

/*  Description:                                                              */ 

/*  constants and tables for msd_hard.c                                       */ 

/*                                                                            */ 

/******************************************************************************/ 
extern signed char puncturing[13][17];



/*   int RX[13] = {1, 3, 1, 4, 1, 4, 3, 2, 8, 3, 4, 7, 8}; */ 
extern int RY[13];
extern signed char tailpuncturing[12][13];


#define SQRT2	1.41421356237310F
#define SQRT10	3.16227766016838F
#define SQRT42	6.48074069840786F
extern float partitioning[4][8];


#undef SQRT2
#undef SQRT10
#undef SQRT42
