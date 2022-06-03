/*
*    File deinterleaver.c
*
*    follows deinterleaver.m by
*    Torsten Schorr
*
*    Author M.Bos - PA0MBO
*    Date Feb 21st 2009
*
*
*    N.B. separate routines for different return arguments
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
#include <math.h>
int *deinterleaver(int xinA, int tA, int xinB, int tB)
{
  int *deinterl;
  int i;
  int sA, sB, qA, qB;
  int PIofi;
  double part1, part2;
  double part3;
  deinterl = (int *) malloc((xinA + xinB) * sizeof(int));
  if (deinterl == NULL)

    {
      printf("Cannot malloc space for deinterl in routine deinterleaver\n");
      exit(EXIT_FAILURE);
    }
  if (xinA < 0)

    {
      printf("deinterleaver: xinA must be > = 0!\n");
      free(deinterl);
      exit(EXIT_FAILURE);
    }
  if (tA < 1)

    {
      printf("deinterleaver: tA must be a natural number!\n");
      free(deinterl);
      exit(EXIT_FAILURE);
    }
  if (xinB < 6)

    {
      printf("deinterleaver: xinB must be >= 6!\n");
      free(deinterl);
      exit(EXIT_FAILURE);
    }
  if (tB < 1)

    {
      printf("deinterleaver: tB must be a natural number\n");
      free(deinterl);
      exit(EXIT_FAILURE);
    }
  if (xinA == 0)
    sA = 0;

  else

    {
      part1 = log((double) xinA);
      part2 = log(2.0);
      part3 = ceil(part1 / part2);
      sA = (int) pow(2, part3);
    } qA = sA / 4 - 1;
  if (xinB == 0)
    sB = 0;

  else

    {
      part1 = log((double) xinB);
      part2 = log(2.0);
      part3 = ceil(part1 / part2);
      sB = (int) pow(2, part3);
    } qB = sB / 4 - 1;
  deinterl[0] = 0;
  PIofi = 0;
  for (i = 1; i <= xinA - 1; i++)

    {
      PIofi = (tA * PIofi + qA) % sA;
      while (PIofi >= xinA)

        {
          PIofi = (tA * PIofi + qA) % sA;
        }
      deinterl[PIofi] = i;
    }
  deinterl[xinA] = xinA;
  PIofi = 0;
  for (i = 1; i <= xinB - 1; i++)

    {
      PIofi = (tB * PIofi + qB) % sB;
      while (PIofi >= xinB)

        {
          PIofi = (tB * PIofi + qB) % sB;
        }
      deinterl[PIofi + xinA] = i + xinA;
    }
  return (deinterl);
}
