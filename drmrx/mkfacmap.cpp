
/*
*    File mkfacmap.c
*
*
*    produces FAC mapping data
*
*    Author M.Bos
*    Date Feb 21st 2009
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
#include <malloc.h>
int mkfacmap(int robustness_mode, int K_dc, int K_modulo, int /*@out@ */ *FACmap)
{
  int elem_cnt, i;

  switch (robustness_mode)

    {
    case 0:
      elem_cnt = 45;
      i = 0;
      FACmap[i++] = K_dc + K_modulo + 10;
      FACmap[i++] = K_dc + K_modulo + 22;
      FACmap[i++] = K_dc + K_modulo + 30;
      FACmap[i++] = K_dc + K_modulo + 50;
      FACmap[i++] = K_dc + 2 * K_modulo + 14;
      FACmap[i++] = K_dc + 2 * K_modulo + 26;
      FACmap[i++] = K_dc + 2 * K_modulo + 34;
      FACmap[i++] = K_dc + 3 * K_modulo + 18;
      FACmap[i++] = K_dc + 3 * K_modulo + 30;
      FACmap[i++] = K_dc + 3 * K_modulo + 38;
      FACmap[i++] = K_dc + 4 * K_modulo + 22;
      FACmap[i++] = K_dc + 4 * K_modulo + 34;
      FACmap[i++] = K_dc + 4 * K_modulo + 42;
      FACmap[i++] = K_dc + 5 * K_modulo + 18;
      FACmap[i++] = K_dc + 5 * K_modulo + 26;
      FACmap[i++] = K_dc + 5 * K_modulo + 38;
      FACmap[i++] = K_dc + 5 * K_modulo + 46;
      FACmap[i++] = K_dc + 6 * K_modulo + 22;
      FACmap[i++] = K_dc + 6 * K_modulo + 30;
      FACmap[i++] = K_dc + 6 * K_modulo + 42;
      FACmap[i++] = K_dc + 6 * K_modulo + 50;
      FACmap[i++] = K_dc + 7 * K_modulo + 26;
      FACmap[i++] = K_dc + 7 * K_modulo + 34;
      FACmap[i++] = K_dc + 7 * K_modulo + 46;
      FACmap[i++] = K_dc + 8 * K_modulo + 10;
      FACmap[i++] = K_dc + 8 * K_modulo + 30;
      FACmap[i++] = K_dc + 8 * K_modulo + 38;
      FACmap[i++] = K_dc + 8 * K_modulo + 50;
      FACmap[i++] = K_dc + 9 * K_modulo + 14;
      FACmap[i++] = K_dc + 9 * K_modulo + 34;
      FACmap[i++] = K_dc + 9 * K_modulo + 42;
      FACmap[i++] = K_dc + 10 * K_modulo + 18;
      FACmap[i++] = K_dc + 10 * K_modulo + 38;
      FACmap[i++] = K_dc + 10 * K_modulo + 46;
      FACmap[i++] = K_dc + 11 * K_modulo + 10;
      FACmap[i++] = K_dc + 11 * K_modulo + 22;
      FACmap[i++] = K_dc + 11 * K_modulo + 42;
      FACmap[i++] = K_dc + 11 * K_modulo + 50;
      FACmap[i++] = K_dc + 12 * K_modulo + 14;
      FACmap[i++] = K_dc + 12 * K_modulo + 26;
      FACmap[i++] = K_dc + 12 * K_modulo + 46;
      FACmap[i++] = K_dc + 13 * K_modulo + 18;
      FACmap[i++] = K_dc + 13 * K_modulo + 30;
      FACmap[i++] = K_dc + 14 * K_modulo + 22;
      FACmap[i++] = K_dc + 14 * K_modulo + 34;
      break;
    case 1:
      elem_cnt = 45;
      i = 0;
      FACmap[i++] = K_dc + 21;
      FACmap[i++] = K_dc + K_modulo + 11;
      FACmap[i++] = K_dc + K_modulo + 23;
      FACmap[i++] = K_dc + K_modulo + 35;
      FACmap[i++] = K_dc + 2 * K_modulo + 13;
      FACmap[i++] = K_dc + 2 * K_modulo + 25;
      FACmap[i++] = K_dc + 2 * K_modulo + 37;
      FACmap[i++] = K_dc + 3 * K_modulo + 15;
      FACmap[i++] = K_dc + 3 * K_modulo + 27;
      FACmap[i++] = K_dc + 3 * K_modulo + 39;
      FACmap[i++] = K_dc + 4 * K_modulo + 5;
      FACmap[i++] = K_dc + 4 * K_modulo + 17;
      FACmap[i++] = K_dc + 4 * K_modulo + 29;
      FACmap[i++] = K_dc + 4 * K_modulo + 41;
      FACmap[i++] = K_dc + 5 * K_modulo + 7;
      FACmap[i++] = K_dc + 5 * K_modulo + 19;
      FACmap[i++] = K_dc + 5 * K_modulo + 31;
      FACmap[i++] = K_dc + 6 * K_modulo + 9;
      FACmap[i++] = K_dc + 6 * K_modulo + 21;
      FACmap[i++] = K_dc + 6 * K_modulo + 33;
      FACmap[i++] = K_dc + 7 * K_modulo + 11;
      FACmap[i++] = K_dc + 7 * K_modulo + 23;
      FACmap[i++] = K_dc + 7 * K_modulo + 35;
      FACmap[i++] = K_dc + 8 * K_modulo + 13;
      FACmap[i++] = K_dc + 8 * K_modulo + 25;
      FACmap[i++] = K_dc + 8 * K_modulo + 37;
      FACmap[i++] = K_dc + 9 * K_modulo + 15;
      FACmap[i++] = K_dc + 9 * K_modulo + 27;
      FACmap[i++] = K_dc + 9 * K_modulo + 39;
      FACmap[i++] = K_dc + 10 * K_modulo + 5;
      FACmap[i++] = K_dc + 10 * K_modulo + 17;
      FACmap[i++] = K_dc + 10 * K_modulo + 29;
      FACmap[i++] = K_dc + 10 * K_modulo + 41;
      FACmap[i++] = K_dc + 11 * K_modulo + 7;
      FACmap[i++] = K_dc + 11 * K_modulo + 19;
      FACmap[i++] = K_dc + 11 * K_modulo + 31;
      FACmap[i++] = K_dc + 12 * K_modulo + 9;
      FACmap[i++] = K_dc + 12 * K_modulo + 21;
      FACmap[i++] = K_dc + 12 * K_modulo + 33;
      FACmap[i++] = K_dc + 13 * K_modulo + 11;
      FACmap[i++] = K_dc + 13 * K_modulo + 23;
      FACmap[i++] = K_dc + 13 * K_modulo + 35;
      FACmap[i++] = K_dc + 14 * K_modulo + 13;
      FACmap[i++] = K_dc + 14 * K_modulo + 25;
      FACmap[i++] = K_dc + 14 * K_modulo + 37;
      break;
    case 2:
      elem_cnt = 45;
      i = 0;
      FACmap[i++] = K_dc + K_modulo + 7;
      FACmap[i++] = K_dc + K_modulo + 23;
      FACmap[i++] = K_dc + 2 * K_modulo + 8;
      FACmap[i++] = K_dc + 2 * K_modulo + 16;
      FACmap[i++] = K_dc + 2 * K_modulo + 24;
      FACmap[i++] = K_dc + 3 * K_modulo + 9;
      FACmap[i++] = K_dc + 3 * K_modulo + 17;
      FACmap[i++] = K_dc + 4 * K_modulo + 10;
      FACmap[i++] = K_dc + 4 * K_modulo + 18;
      FACmap[i++] = K_dc + 5 * K_modulo + 11;
      FACmap[i++] = K_dc + 5 * K_modulo + 19;
      FACmap[i++] = K_dc + 6 * K_modulo + 4;
      FACmap[i++] = K_dc + 6 * K_modulo + 12;
      FACmap[i++] = K_dc + 7 * K_modulo + 13;
      FACmap[i++] = K_dc + 7 * K_modulo + 21;
      FACmap[i++] = K_dc + 8 * K_modulo + 6;
      FACmap[i++] = K_dc + 8 * K_modulo + 14;
      FACmap[i++] = K_dc + 8 * K_modulo + 22;
      FACmap[i++] = K_dc + 9 * K_modulo + 7;
      FACmap[i++] = K_dc + 9 * K_modulo + 23;
      FACmap[i++] = K_dc + 10 * K_modulo + 8;
      FACmap[i++] = K_dc + 10 * K_modulo + 16;
      FACmap[i++] = K_dc + 10 * K_modulo + 24;
      FACmap[i++] = K_dc + 11 * K_modulo + 9;
      FACmap[i++] = K_dc + 11 * K_modulo + 13;
      FACmap[i++] = K_dc + 11 * K_modulo + 17;
      FACmap[i++] = K_dc + 12 * K_modulo + 10;
      FACmap[i++] = K_dc + 12 * K_modulo + 18;
      FACmap[i++] = K_dc + 13 * K_modulo + 11;
      FACmap[i++] = K_dc + 13 * K_modulo + 19;
      FACmap[i++] = K_dc + 14 * K_modulo + 4;
      FACmap[i++] = K_dc + 14 * K_modulo + 12;
      FACmap[i++] = K_dc + 14 * K_modulo + 16;
      FACmap[i++] = K_dc + 15 * K_modulo + 13;
      FACmap[i++] = K_dc + 15 * K_modulo + 21;
      FACmap[i++] = K_dc + 16 * K_modulo + 6;
      FACmap[i++] = K_dc + 16 * K_modulo + 14;
      FACmap[i++] = K_dc + 16 * K_modulo + 22;
      FACmap[i++] = K_dc + 17 * K_modulo + 7;
      FACmap[i++] = K_dc + 17 * K_modulo + 23;
      FACmap[i++] = K_dc + 18 * K_modulo + 8;
      FACmap[i++] = K_dc + 18 * K_modulo + 16;
      FACmap[i++] = K_dc + 18 * K_modulo + 24;
      FACmap[i++] = K_dc + 19 * K_modulo + 9;
      FACmap[i++] = K_dc + 19 * K_modulo + 17;
      break;
    case 3:			/* does not exist in amateur mode */
      elem_cnt = 45;
      i = 0;
      FACmap[i++] = K_dc + 3 * K_modulo + 9;
      FACmap[i++] = K_dc + 3 * K_modulo + 18;
      FACmap[i++] = K_dc + 3 * K_modulo + 27;
      FACmap[i++] = K_dc + 4 * K_modulo + 10;
      FACmap[i++] = K_dc + 4 * K_modulo + 19;
      FACmap[i++] = K_dc + 5 * K_modulo + 11;
      FACmap[i++] = K_dc + 5 * K_modulo + 20;
      FACmap[i++] = K_dc + 5 * K_modulo + 29;
      FACmap[i++] = K_dc + 6 * K_modulo + 12;
      FACmap[i++] = K_dc + 6 * K_modulo + 30;
      FACmap[i++] = K_dc + 7 * K_modulo + 13;
      FACmap[i++] = K_dc + 7 * K_modulo + 22;
      FACmap[i++] = K_dc + 7 * K_modulo + 31;
      FACmap[i++] = K_dc + 8 * K_modulo + 5;
      FACmap[i++] = K_dc + 8 * K_modulo + 14;
      FACmap[i++] = K_dc + 8 * K_modulo + 23;
      FACmap[i++] = K_dc + 8 * K_modulo + 32;
      FACmap[i++] = K_dc + 9 * K_modulo + 6;
      FACmap[i++] = K_dc + 9 * K_modulo + 15;
      FACmap[i++] = K_dc + 9 * K_modulo + 24;
      FACmap[i++] = K_dc + 9 * K_modulo + 33;
      FACmap[i++] = K_dc + 10 * K_modulo + 16;
      FACmap[i++] = K_dc + 10 * K_modulo + 25;
      FACmap[i++] = K_dc + 10 * K_modulo + 34;
      FACmap[i++] = K_dc + 11 * K_modulo + 8;
      FACmap[i++] = K_dc + 11 * K_modulo + 17;
      FACmap[i++] = K_dc + 11 * K_modulo + 26;
      FACmap[i++] = K_dc + 11 * K_modulo + 35;
      FACmap[i++] = K_dc + 12 * K_modulo + 9;
      FACmap[i++] = K_dc + 12 * K_modulo + 18;
      FACmap[i++] = K_dc + 12 * K_modulo + 27;
      FACmap[i++] = K_dc + 12 * K_modulo + 36;
      FACmap[i++] = K_dc + 13 * K_modulo + 10;
      FACmap[i++] = K_dc + 13 * K_modulo + 19;
      FACmap[i++] = K_dc + 13 * K_modulo + 37;
      FACmap[i++] = K_dc + 14 * K_modulo + 11;
      FACmap[i++] = K_dc + 14 * K_modulo + 20;
      FACmap[i++] = K_dc + 14 * K_modulo + 29;
      FACmap[i++] = K_dc + 15 * K_modulo + 12;
      FACmap[i++] = K_dc + 15 * K_modulo + 30;
      FACmap[i++] = K_dc + 16 * K_modulo + 13;
      FACmap[i++] = K_dc + 16 * K_modulo + 22;
      FACmap[i++] = K_dc + 16 * K_modulo + 31;
      FACmap[i++] = K_dc + 17 * K_modulo + 5;
      FACmap[i++] = K_dc + 17 * K_modulo + 14;
      FACmap[i++] = K_dc + 17 * K_modulo + 23;
      FACmap[i++] = K_dc + 17 * K_modulo + 32;
      FACmap[i++] = K_dc + 18 * K_modulo + 6;
      FACmap[i++] = K_dc + 18 * K_modulo + 15;
      FACmap[i++] = K_dc + 18 * K_modulo + 24;
      FACmap[i++] = K_dc + 18 * K_modulo + 33;
      FACmap[i++] = K_dc + 19 * K_modulo + 16;
      FACmap[i++] = K_dc + 19 * K_modulo + 25;
      FACmap[i++] = K_dc + 19 * K_modulo + 34;
      FACmap[i++] = K_dc + 20 * K_modulo + 8;
      FACmap[i++] = K_dc + 20 * K_modulo + 17;
      FACmap[i++] = K_dc + 20 * K_modulo + 26;
      FACmap[i++] = K_dc + 20 * K_modulo + 35;
      FACmap[i++] = K_dc + 21 * K_modulo + 9;
      FACmap[i++] = K_dc + 21 * K_modulo + 18;
      FACmap[i++] = K_dc + 21 * K_modulo + 27;
      FACmap[i++] = K_dc + 21 * K_modulo + 36;
      FACmap[i++] = K_dc + 22 * K_modulo + 10;
      FACmap[i++] = K_dc + 22 * K_modulo + 19;
      FACmap[i++] = K_dc + 22 * K_modulo + 37;
      break;
    default:
      printf("wrong robustness mode in cal to mkfacmap\n");
      exit(EXIT_FAILURE);
    }
  return elem_cnt;
}
