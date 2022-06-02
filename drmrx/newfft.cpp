#include <math.h>
#include <float.h>


/******************************************************************************
 *
 *  MiXViews - an X window system based sound & data editor/processor
 *
 *  Copyright (c) 1993, 1994 Regents of the University of California
 *
 *  Author:     Douglas Scott
 *  Date:       December 13, 1994
 *
 *  Permission to use, copy and modify this software and its documentation
 *  for research and/or educational purposes and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation. The author reserves the right to distribute this
 *  software and its documentation.  The University of California and the author
 *  make no representations about the suitability of this software for any 
 *  purpose, and in no event shall University of California be liable for any
 *  damage, loss of data, or profits resulting from its use.
 *  It is provided "as is" without express or implied warranty.
 *
 ******************************************************************************/
void rfft(float *, int, int);
void cfft(float *, int, int);
static double twopi = M_PI * 2.0;
static double pi = M_PI;



/* If forward is true, rfft replaces 2*N real data points in buf with
	 N complex values representing the positive frequency half of their
	 Fourier spectrum, with *(buf+1) replaced with the real part of the Nyquist
	 frequency value.	If forward is false, rfft expects buf to contain a
	 positive frequency spectrum arranged as before, and replaces it with
	 2*N real values.	N MUST be a power of 2. */
void rfft(float *buf, int N2, int forward)
{
  float c2, h1r, h1i, h2r, h2i, temp;
  float br, bi;
  float theta = (float) (pi / N2);
  float wr = 1.;
  float wi = 0.;
  float c1 = 0.5;
  float wpr;
  float wpi;
  int N2p1;
  int i, i1, i2, i3, i4;


  /* debugging pa0mbo 
     printf("N2 is %d\n",N2); */
  if (forward == 1)
    {
      c2 = -0.5;
      cfft(buf, N2, forward);

      /* debugging pa0mbo 
         printf("na cfft\n"); */
      br = *buf;
      bi = *(buf + 1);

      /* debuging pa0mbo 
         printf(" na br ni = \n"); */
    }

  else
    {
      c2 = 0.5;
      theta = -theta;
      br = *(buf + 1);
      bi = 0.;
      *(buf + 1) = 0.;
    }
  wpr = (float) (-2. * pow(sin(0.5 * theta), 2.));
  wpi = (float) sin(theta);
  N2p1 = (N2 << 1) + 1;

  /* debugging pa0mbo 
     printf(" N2p1 is %d\n",N2p1); */
  for (i = 0; i <= N2 >> 1; i++)
    {
      i1 = i << 1;
      i2 = i1 + 1;
      i3 = N2p1 - i2;
      i4 = i3 + 1;
      if (i == 0)
	{
	  h1r = c1 * (*(buf + i1) + br);
	  h1i = c1 * (*(buf + i2) - bi);
	  h2r = -c2 * (*(buf + i2) + bi);
	  h2i = c2 * (*(buf + i1) - br);
	  *(buf + i1) = h1r + (wr * h2r) - (wi * h2i);
	  *(buf + i2) = h1i + (wr * h2i) + (wi * h2r);
	  br = h1r - (wr * h2r) + (wi * h2i);
	  bi = -h1i + (wr * h2i) + (wi * h2r);
	}

      else
	{
	  h1r = c1 * (*(buf + i1) + *(buf + i3));
	  h1i = c1 * (*(buf + i2) - *(buf + i4));
	  h2r = -c2 * (*(buf + i2) + *(buf + i4));
	  h2i = c2 * (*(buf + i1) - *(buf + i3));
	  *(buf + i1) = h1r + wr * h2r - wi * h2i;
	  *(buf + i2) = h1i + wr * h2i + wi * h2r;
	  *(buf + i3) = h1r - wr * h2r + wi * h2i;
	  *(buf + i4) = -h1i + wr * h2i + wi * h2r;
	}
      wr = ((temp = wr) * wpr) - (wi * wpi) + wr;
      wi = (wi * wpr) + (temp * wpi) + wi;
    }
  if (forward == 1)
    *(buf + 1) = br;

  else
    cfft(buf, N2, forward);
}



/* cfft replaces float array x containing NC complex values
	 (2*NC float values alternating real, imagininary, etc.)
	 by its Fourier transform if forward is true, or by its
	 inverse Fourier transform if forward is false, using a
	 recursive Fast Fourier transform method due to Danielson
	 and Lanczos.	NC MUST be a power of 2. */
void bitreverse(float *, int);
void cfft(float *buf, int N2, int forward)
{
  int delta;
  int ND = N2 << 1;
  int mmax;
  float theta, wpr, wpi, wr, wi;
  float rtemp, itemp;
  int i, j, m;
  float scale;
  float *bi, *be;

  bitreverse(buf, ND);
  for (mmax = 2; mmax < ND; mmax = delta)
    {
      delta = mmax << 1;
      theta = (float) (twopi / ((forward == 1) ? mmax : -mmax));
      wpr = (float) (-2. * pow(sin(0.5 * theta), 2.));
      wpi = (float) sin(theta);
      wr = 1.;
      wi = 0.;
      for (m = 0; m < mmax; m += 2)
	{
	  for (i = m; i < ND; i += delta)
	    {
	      j = i + mmax;
	      rtemp = (wr * *(buf + j)) - (wi * *(buf + j + 1));
	      itemp = (wr * *(buf + j + 1)) + (wi * *(buf + j));
	      *(buf + j) = *(buf + i) - rtemp;
	      *(buf + j + 1) = *(buf + i + 1) - itemp;
	      *(buf + i) += rtemp;
	      *(buf + i + 1) += itemp;
	    }
	  wr = ((rtemp = wr) * wpr) - (wi * wpi) + wr;
	  wi = (wi * wpr) + (rtemp * wpi) + wi;
	}
    }


/* scale output */


/*	scale = forward ? 1./ND : 2.;		 this is the original */
  scale = (float) ((forward == 1) ? 1.0 : 1.0 / ND);
  if ((fabs(scale) - 1.0) < DBL_EPSILON)
    {
      bi = buf;
      be = buf + ND;
      while (bi < be)
	*bi++ *= scale;
    }
}



/* bitreverse places float array x containing N/2 complex values
	 into bit-reversed order   */
void bitreverse(float *buf, int N)
{
  int i, j, m;

  for (i = j = 0; i < N; i += 2, j += m)
    {
      if (j > i)
	{
	  float rtemp = *(buf + j);	/* complex exchange */
	  float itemp = *(buf + j + 1);

	  *(buf + j) = *(buf + i);
	  *(buf + j + 1) = *(buf + i + 1);
	  *(buf + i) = rtemp;
	  *(buf + i + 1) = itemp;
	}
      for (m = N >> 1; m >= 2 && j >= m; m >>= 1)
	j -= m;
    }
}
