
/*
*   File crc8_c.c
*
*   from diorama-1.1.1 by A. Dittrich & T. Schorr
*
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
void crc8_c( /*@out@ */ double checksum[], double in[], int N)
{
  int i;
  unsigned int b = 0xFF;
  unsigned int x = 0x1D;	/* (1) 00011101 */
  unsigned int y;

  for (i = 0; i < N; i++)

    {
      y = ((b >> 7) + (int) floor(in[i] + 0.5)) & 0x01;
      if (y == 1)
	b = ((b << 1) ^ x);

      else
	b = (b << 1);
    }
  *checksum = (double) (b & 0xFF);
}
