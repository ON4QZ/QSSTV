/*
 * Reed-Solomon coding and decoding
 * Phil Karn (karn@ka9q.ampr.org) September 1996
 * Separate CCSDS version create Dec 1998, merged into this version May 1999
 * 
 * This file is derived from my generic RS encoder/decoder, which is
 * in turn based on the program "new_rs_erasures.c" by Robert
 * Morelos-Zaragoza (robert@spectra.eng.hawaii.edu) and Hari Thirumoorthy
 * (harit@spectra.eng.hawaii.edu), Aug 1995
 
 * Copyright 1999 Phil Karn, KA9Q
 * May be used under the terms of the GNU public license
 */
#include <stdio.h>
#include "rs.h"

static int KK;

/* MM, KK, B0, PRIM are user-defined in rs.h */

/* Primitive polynomials - see Lin & Costello, Appendix A,
 * and  Lee & Messerschmitt, p. 453.
 */

/* 1+x^2+x^3+x^4+x^8 */
int Pp[MM+1] = { 1, 0, 1, 1, 1, 0, 0, 0, 1 };



/* This defines the type used to store an element of the Galois Field
 * used by the code. Make sure this is something larger than a char if
 * if anything larger than GF(256) is used.
 *
 * Note: unsigned char will work up to GF(256) but int seems to run
 * faster on the Pentium.
 */
typedef int gf;

/* index->polynomial form conversion table */
static gf Alpha_to[NN + 1];

/* Polynomial->index form conversion table */
static gf Index_of[NN + 1];

/* No legal value in index form represents zero, so
 * we need a special value for this purpose
 */
#define A0	(NN)

/* Generator polynomial g(x) in index form */
//static gf Gg[NN - KK + 1];
static gf Gg[NN-RSDSIZERS4+1]; //worst case
static int RS_init=0; /* Initialization flag */

/* Compute x % NN, where NN is 2**MM - 1,
 * without a slow divide
 */
static  gf
modnn(int x)
{
  while (x >= NN) {
    x -= NN;
    x = (x >> MM) + (x & NN);
  }
  return x;
}

#define	min(a,b)	((a) < (b) ? (a) : (b))

#define	CLEAR(a,n) {\
int ci;\
for(ci=(n)-1;ci >=0;ci--)\
(a)[ci] = 0;\
}

#define	COPY(a,b,n) {\
int ci;\
for(ci=(n)-1;ci >=0;ci--)\
(a)[ci] = (b)[ci];\
}

#define	COPYDOWN(a,b,n) {\
int ci;\
for(ci=(n)-1;ci >=0;ci--)\
(a)[ci] = (b)[ci];\
}

#define Ldec 1


/* generate GF(2**m) from the irreducible polynomial p(X) in Pp[0]..Pp[m]
   lookup tables:  index->polynomial form   alpha_to[] contains j=alpha**i;
                   polynomial form -> index form  index_of[j=alpha**i] = i
   alpha=2 is the primitive element of GF(2**m)
   HARI's COMMENT: (4/13/94) alpha_to[] can be used as follows:
        Let @ represent the primitive element commonly called "alpha" that
   is the root of the primitive polynomial p(x). Then in GF(2^m), for any
   0 <= i <= 2^m-2,
        @^i = a(0) + a(1) @ + a(2) @^2 + ... + a(m-1) @^(m-1)
   where the binary vector (a(0),a(1),a(2),...,a(m-1)) is the representation
   of the integer "alpha_to[i]" with a(0) being the LSB and a(m-1) the MSB. Thus for
   example the polynomial representation of @^5 would be given by the binary
   representation of the integer "alpha_to[5]".
                   Similarily, index_of[] can be used as follows:
        As above, let @ represent the primitive element of GF(2^m) that is
   the root of the primitive polynomial p(x). In order to find the power
   of @ (alpha) that has the polynomial representation
        a(0) + a(1) @ + a(2) @^2 + ... + a(m-1) @^(m-1)
   we consider the integer "i" whose binary representation with a(0) being LSB
   and a(m-1) MSB is (a(0),a(1),...,a(m-1)) and locate the entry
   "index_of[i]". Now, @^index_of[i] is that element whose polynomial 
    representation is (a(0),a(1),a(2),...,a(m-1)).
   NOTE:
        The element alpha_to[2^m-1] = 0 always signifying that the
   representation of "@^infinity" = 0 is (0,0,0,...,0).
        Similarily, the element index_of[0] = A0 always signifying
   that the power of alpha which has the polynomial representation
   (0,0,...,0) is "infinity".
 
*/

static void
generate_gf(void)
{
  int i, mask;

  mask = 1;
  Alpha_to[MM] = 0;
  for (i = 0; i < MM; i++) {
    Alpha_to[i] = mask;
    Index_of[Alpha_to[i]] = i;
    /* If Pp[i] == 1 then, term @^i occurs in poly-repr of @^MM */
    if (Pp[i] != 0)
      Alpha_to[MM] ^= mask;	/* Bit-wise EXOR operation */
    mask <<= 1;	/* single left-shift */
  }
  Index_of[Alpha_to[MM]] = MM;
  /*
   * Have obtained poly-repr of @^MM. Poly-repr of @^(i+1) is given by
   * poly-repr of @^i shifted left one-bit and accounting for any @^MM
   * term that may occur when poly-repr of @^i is shifted.
   */
  mask >>= 1;
  for (i = MM + 1; i < NN; i++) {
    if (Alpha_to[i - 1] >= mask)
      Alpha_to[i] = Alpha_to[MM] ^ ((Alpha_to[i - 1] ^ mask) << 1);
    else
      Alpha_to[i] = Alpha_to[i - 1] << 1;
    Index_of[Alpha_to[i]] = i;
  }
  Index_of[0] = A0;
  Alpha_to[NN] = 0;
}

/*
 * Obtain the generator polynomial of the TT-error correcting, length
 * NN=(2**MM -1) Reed Solomon code from the product of (X+@**(B0+i)), i = 0,
 * ... ,(2*TT-1)
 *
 * Examples:
 *
 * If B0 = 1, TT = 1. deg(g(x)) = 2*TT = 2.
 * g(x) = (x+@) (x+@**2)
 *
 * If B0 = 0, TT = 2. deg(g(x)) = 2*TT = 4.
 * g(x) = (x+1) (x+@) (x+@**2) (x+@**3)
 */
static void
gen_poly(void)
{
  int i, j;

  Gg[0] = 1;
  for (i = 0; i < NN - KK; i++) {
    Gg[i+1] = 1;
    /*
     * Below multiply (Gg[0]+Gg[1]*x + ... +Gg[i]x^i) by
     * (@**(B0+i)*PRIM + x)
     */
    for (j = i; j > 0; j--)
      if (Gg[j] != 0)
	Gg[j] = Gg[j - 1] ^ Alpha_to[modnn((Index_of[Gg[j]]) + (B0 + i) *PRIM)];
      else
	Gg[j] = Gg[j - 1];
    /* Gg[0] can never be zero */
    Gg[0] = Alpha_to[modnn(Index_of[Gg[0]] + (B0 + i) * PRIM)];
  }
  /* convert Gg[] to index form for quicker encoding */
  for (i = 0; i <= NN - KK; i++)
    Gg[i] = Index_of[Gg[i]];
}


/*
 * take the string of symbols in data[i], i=0..(k-1) and encode
 * systematically to produce NN-KK parity symbols in bb[0]..bb[NN-KK-1] data[]
 * is input and bb[] is output in polynomial form. Encoding is done by using
 * a feedback shift register with appropriate connections specified by the
 * elements of Gg[], which was generated above. Codeword is   c(X) =
 * data(X)*X**(NN-KK)+ b(X)
 */
int
encode_rs(dtype data[], dtype bb[])
{
  int i, j;
  gf feedback;
  CLEAR(bb,NN-KK);


  for(i = KK - 1; i >= 0; i--) {
    feedback = Index_of[data[i] ^ bb[NN - KK - 1]];
    if (feedback != A0) {	/* feedback term is non-zero */
      for (j = NN - KK - 1; j > 0; j--)
	if (Gg[j] != A0)
	  bb[j] = bb[j - 1] ^ Alpha_to[modnn(Gg[j] + feedback)];
	else
	  bb[j] = bb[j - 1];
      bb[0] = Alpha_to[modnn(Gg[0] + feedback)];
    } else {	/* feedback term is zero. encoder becomes a
		 * single-byte shifter */
      for (j = NN - KK - 1; j > 0; j--)
	bb[j] = bb[j - 1];
      bb[0] = 0;
    }
  }
  return 0;
}

/*
 * Performs ERRORS+ERASURES decoding of RS codes. If decoding is successful,
 * writes the codeword into data[] itself. Otherwise data[] is unaltered.
 *
 * Return number of symbols corrected, or -1 if codeword is illegal
 * or uncorrectable. If eras_pos is non-null, the detected error locations
 * are written back. NOTE! This array must be at least NN-KK elements long.
 * 
 * First "no_eras" erasures are declared by the calling program. Then, the
 * maximum # of errors correctable is t_after_eras = floor((NN-KK-no_eras)/2).
 * If the number of channel errors is not greater than "t_after_eras" the
 * transmitted codeword will be recovered. Details of algorithm can be found
 * in R. Blahut's "Theory ... of Error-Correcting Codes".

 * Warning: the eras_pos[] array must not contain duplicate entries; decoder failure
 * will result. The decoder *could* check for this condition, but it would involve
 * extra time on every decoding operation.
 */
int
eras_dec_rs(dtype data[], int eras_pos[], int no_eras)
{
  int deg_lambda, el, deg_omega;
  int i, j, r,k;

  gf u,q,tmp,num1,num2,den,discr_r;
  gf lambda[NN-KK + 1], s[NN-KK + 1];	/* Err+Eras Locator poly
           * and syndrome poly */
  gf b[NN-KK + 1], t[NN-KK + 1], omega[NN-KK + 1];
  gf root[NN-KK], reg[NN-KK + 1], loc[NN-KK];
  int syn_error, count;


  /* form the syndromes; i.e., evaluate data(x) at roots of g(x)
   * namely @**(B0+i)*PRIM, i = 0, ... ,(NN-KK-1)
   */
  for(i=1;i<=NN-KK;i++)
    {
      s[i] = data[0];
    }
  for(j=1;j<NN;j++)
    {
      if(data[j] == 0) continue;
      tmp = Index_of[data[j]];

      /*	s[i] ^= Alpha_to[modnn(tmp + (B0+i-1)*j)]; */
      for(i=1;i<=NN-KK;i++)
        s[i] ^= Alpha_to[modnn(tmp + (B0+i-1)*PRIM*j)];
    }
  /* Convert syndromes to index form, checking for nonzero condition */
  syn_error = 0;
  for(i=1;i<=NN-KK;i++)
    {
      syn_error |= s[i];
      s[i] = Index_of[s[i]];
    }
  
  if (!syn_error) {
      /* if syndrome is zero, data[] is a codeword and there are no
     * errors to correct. So return data[] unmodified
     */
      count = 0;
      goto finish;
    }
  CLEAR(&lambda[1],NN-KK);
  lambda[0] = 1;

  if (no_eras > 0) {
      /* Init lambda to be the erasure locator polynomial */
      lambda[1] = Alpha_to[modnn(PRIM * eras_pos[0])];
      for (i = 1; i < no_eras; i++)
        {
          u = modnn(PRIM*eras_pos[i]);
          for (j = i+1; j > 0; j--)
            {
              tmp = Index_of[lambda[j - 1]];
              if(tmp != A0)
                {
                  lambda[j] ^= Alpha_to[modnn(u + tmp)];
                }
            }
        }
     }
  for(i=0;i<NN-KK+1;i++)
    b[i] = Index_of[lambda[i]];
  
  /*
   * Begin Berlekamp-Massey algorithm to determine error+erasure
   * locator polynomial
   */
  r = no_eras;
  el = no_eras;
  while (++r <= NN-KK) {	/* r is the step number */
      /* Compute discrepancy at the r-th step in poly-form */
      discr_r = 0;
      for (i = 0; i < r; i++){
          if ((lambda[i] != 0) && (s[r - i] != A0)) {
              discr_r ^= Alpha_to[modnn(Index_of[lambda[i]] + s[r - i])];
            }
        }
      discr_r = Index_of[discr_r];	/* Index form */
      if (discr_r == A0) {
          /* 2 lines below: B(x) <-- x*B(x) */
          COPYDOWN(&b[1],b,NN-KK);
          b[0] = A0;
        } else {
          /* 7 lines below: T(x) <-- lambda(x) - discr_r*x*b(x) */
          t[0] = lambda[0];
          for (i = 0 ; i < NN-KK; i++) {
              if(b[i] != A0)
                t[i+1] = lambda[i+1] ^ Alpha_to[modnn(discr_r + b[i])];
              else
                t[i+1] = lambda[i+1];
            }
          if (2 * el <= r + no_eras - 1) {
              el = r + no_eras - el;
              /*
   * 2 lines below: B(x) <-- inv(discr_r) *
   * lambda(x)
   */
              for (i = 0; i <= NN-KK; i++)
                b[i] = (lambda[i] == 0) ? A0 : modnn(Index_of[lambda[i]] - discr_r + NN);
            } else {
              /* 2 lines below: B(x) <-- x*B(x) */
              COPYDOWN(&b[1],b,NN-KK);
              b[0] = A0;
            }
          COPY(lambda,t,NN-KK+1);
        }
    }

  /* Convert lambda to index form and compute deg(lambda(x)) */
  deg_lambda = 0;
  for(i=0;i<NN-KK+1;i++){
      lambda[i] = Index_of[lambda[i]];
      if(lambda[i] != A0)
        deg_lambda = i;
    }
  /*
   * Find roots of the error+erasure locator polynomial by Chien
   * Search
   */
  COPY(&reg[1],&lambda[1],NN-KK);
  count = 0;		/* Number of roots of lambda(x) */
  for (i = 1,k=NN-Ldec; i <= NN; i++,k = modnn(NN+k-Ldec)) {
      q = 1;
      for (j = deg_lambda; j > 0; j--){
          if (reg[j] != A0) {
              reg[j] = modnn(reg[j] + j);
              q ^= Alpha_to[reg[j]];
            }
        }
      if (q != 0)
        continue;
      /* store root (index-form) and error location number */
      root[count] = i;
      loc[count] = k;
      /* If we've already found max possible roots,
     * abort the search to save time
     */
      if(++count == deg_lambda)
        break;
    }
  if (deg_lambda != count) {
      /*
     * deg(lambda) unequal to number of roots => uncorrectable
     * error detected
     */
      count = -1;
      goto finish;
    }
  /*
   * Compute err+eras evaluator poly omega(x) = s(x)*lambda(x) (modulo
   * x**(NN-KK)). in index form. Also find deg(omega).
   */
  deg_omega = 0;
  for (i = 0; i < NN-KK;i++){
      tmp = 0;
      j = (deg_lambda < i) ? deg_lambda : i;
      for(;j >= 0; j--){
          if ((s[i + 1 - j] != A0) && (lambda[j] != A0))
            tmp ^= Alpha_to[modnn(s[i + 1 - j] + lambda[j])];
        }
      if(tmp != 0)
        deg_omega = i;
      omega[i] = Index_of[tmp];
    }
  omega[NN-KK] = A0;
  
  /*
   * Compute error values in poly-form. num1 = omega(inv(X(l))), num2 =
   * inv(X(l))**(B0-1) and den = lambda_pr(inv(X(l))) all in poly-form
   */
  for (j = count-1; j >=0; j--) {
      num1 = 0;
      for (i = deg_omega; i >= 0; i--) {
          if (omega[i] != A0)
            num1  ^= Alpha_to[modnn(omega[i] + i * root[j])];
        }
      num2 = Alpha_to[modnn(root[j] * (B0 - 1) + NN)];
      den = 0;

      /* lambda[i+1] for i even is the formal derivative lambda_pr of lambda[i] */
      for (i = min(deg_lambda,NN-KK-1) & ~1; i >= 0; i -=2) {
          if(lambda[i+1] != A0)
            den ^= Alpha_to[modnn(lambda[i+1] + i * root[j])];
        }
      if (den == 0) {

          /* Convert to dual- basis */
          count = -1;
          goto finish;
        }
      /* Apply error to data */
      if (num1 != 0) {
          data[loc[j]] ^= Alpha_to[modnn(Index_of[num1] + Index_of[num2] + NN - Index_of[den])];
        }
    }
finish:
  if(eras_pos != NULL){
      for(i=0;i<count;i++){
          if(eras_pos!= NULL)
            eras_pos[i] = loc[i];
        }
    }
  return count;
}
/* Encoder/decoder initialization - call this first! */
void init_rs(int kk)
{
  KK=kk;
  generate_gf();
  gen_poly();
  RS_init = 1;
}
