/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Adapted for ham sstv use  Ties Bos - PA0MBO
 *
 * Description:
 *	Table of the mapping of OFDM cells.
 *	We build a table of one super-frame where we set flags for each cell to
 *	identify the symbol for this place. E.g. if the flag "CM_MSC" is set for
 *	one table entry this is the cell for a MSC-symbol. The name of the table
 *	is matiMapTab.
 *  We use the table "matcPilotCells" for storing the complex values for the
 *  pilots. For simplicity we allocate memory for all blocks but only the
 *  pilot positions are used.
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later 
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/
#include "../GlobalDefinitions.h"
#include "../tables/TableCarrier.h"
#include "CellMappingTable.h"

//#define _DEBUG_ 1

/* Implementation *************************************************************/
void CCellMappingTable::MakeTable(ERobMode eNewRobustnessMode, 
                                  ESpecOcc eNewSpectOccup)
{
  int				iNoMSCDummyCells; /* Number of MSC dummy cells */
  int				iNumTimePilots; /* Number of time pilots per frame */
  CScatPilots		ScatPilots;
  int				iSym;
  int				iFrameSym;
  int				iCar;
  int				iTimePilotsCounter;
  int				iFreqPilotsCounter;
  int				iScatPilotsCounter;
  int				iMSCCounter;
  int				iFACCounter=0;
  int				iScatPilPhase;
  int				iCarArrInd;
  int				iSpecOccArrayIndex;
  /* Tables */
  const int*		piTableFAC;
  const int*		piTableTimePilots;
  const int*		piTableFreqPilots;


  /* Set Parameters and pointers to the tables ******************************/
  switch (eNewSpectOccup)
    {
    case SO_0:
      iSpecOccArrayIndex = 0;
    break;

    case SO_1:
      iSpecOccArrayIndex = 1;
    break;

    default:
      iSpecOccArrayIndex = 1;
    break;
    }

  // initialize defaults to avoid unitialized warning
  piTableTimePilots = &iTableTimePilRobModA[0][0];
  piTableFreqPilots = &iTableFreqPilRobModA[0][0];
  piTableFAC = &iTableFACRobModA[0][0];
  iNumTimePilots = RMA_NUM_TIME_PIL;

  /* The robust mode defines all other parameters */
  switch (eNewRobustnessMode)
    {
    case RM_ROBUSTNESS_MODE_A:
      iCarrierKmin = iTableCarrierKmin[iSpecOccArrayIndex][0];
      iCarrierKmax = iTableCarrierKmax[iSpecOccArrayIndex][0];

      iFFTSizeN = RMA_FFT_SIZE_N;
      RatioTgTu.iEnum = RMA_ENUM_TG_TU;
      RatioTgTu.iDenom = RMA_DENOM_TG_TU;

      iNumSymPerFrame = RMA_NUM_SYM_PER_FRAME;
      iNumSymbolsPerSuperframe = iNumSymPerFrame * NUM_FRAMES_IN_SUPERFRAME;
      piTableFAC = &iTableFACRobModA[0][0];
      iNumTimePilots = RMA_NUM_TIME_PIL;
      piTableTimePilots = &iTableTimePilRobModA[0][0];
      piTableFreqPilots = &iTableFreqPilRobModA[0][0];
      iScatPilTimeInt = RMA_SCAT_PIL_TIME_INT;
      iScatPilFreqInt = RMA_SCAT_PIL_FREQ_INT;

      /* Scattered pilots phase definition */
      ScatPilots.piConst = iTableScatPilConstRobModA;
      ScatPilots.iColSizeWZ = SIZE_COL_WZ_ROB_MOD_A;
      ScatPilots.piW = &iScatPilWRobModA[0][0];
      ScatPilots.piZ = &iScatPilZRobModA[0][0];
      ScatPilots.iQ = iScatPilQRobModA;

      ScatPilots.piGainTable = &iScatPilGainRobModA[iSpecOccArrayIndex][0];
    break;

    case RM_ROBUSTNESS_MODE_B:
      iCarrierKmin = iTableCarrierKmin[iSpecOccArrayIndex][1];
      iCarrierKmax = iTableCarrierKmax[iSpecOccArrayIndex][1];

      iFFTSizeN = RMB_FFT_SIZE_N;
      RatioTgTu.iEnum = RMB_ENUM_TG_TU;
      RatioTgTu.iDenom = RMB_DENOM_TG_TU;

      iNumSymPerFrame = RMB_NUM_SYM_PER_FRAME;
      iNumSymbolsPerSuperframe = iNumSymPerFrame * NUM_FRAMES_IN_SUPERFRAME;
      piTableFAC = &iTableFACRobModB[0][0];
      iNumTimePilots = RMB_NUM_TIME_PIL;
      piTableTimePilots = &iTableTimePilRobModB[0][0];
      piTableFreqPilots = &iTableFreqPilRobModB[0][0];
      iScatPilTimeInt = RMB_SCAT_PIL_TIME_INT;
      iScatPilFreqInt = RMB_SCAT_PIL_FREQ_INT;

      /* Scattered pilots phase definition */
      ScatPilots.piConst = iTableScatPilConstRobModB;
      ScatPilots.iColSizeWZ = SIZE_COL_WZ_ROB_MOD_B;
      ScatPilots.piW = &iScatPilWRobModB[0][0];
      ScatPilots.piZ = &iScatPilZRobModB[0][0];
      ScatPilots.iQ = iScatPilQRobModB;

      ScatPilots.piGainTable = &iScatPilGainRobModB[iSpecOccArrayIndex][0];
    break;
    case RM_NO_MODE_DETECTED:
    case RM_ROBUSTNESS_MODE_D: // not used in HAM default to E joma
    case RM_ROBUSTNESS_MODE_E:
      iCarrierKmin = iTableCarrierKmin[iSpecOccArrayIndex][2];
      iCarrierKmax = iTableCarrierKmax[iSpecOccArrayIndex][2];

      iFFTSizeN = RME_FFT_SIZE_N;
      RatioTgTu.iEnum = RME_ENUM_TG_TU;
      RatioTgTu.iDenom = RME_DENOM_TG_TU;

      iNumSymPerFrame = RME_NUM_SYM_PER_FRAME;
      iNumSymbolsPerSuperframe = iNumSymPerFrame * NUM_FRAMES_IN_SUPERFRAME;
      piTableFAC = &iTableFACRobModE[0][0];
      iNumTimePilots = RME_NUM_TIME_PIL;
      piTableTimePilots = &iTableTimePilRobModE[0][0];
      piTableFreqPilots = &iTableFreqPilRobModE[0][0];
      iScatPilTimeInt = RME_SCAT_PIL_TIME_INT;
      iScatPilFreqInt = RME_SCAT_PIL_FREQ_INT;

      /* Scattered pilots phase definition */
      ScatPilots.piConst = iTableScatPilConstRobModE;
      ScatPilots.iColSizeWZ = SIZE_COL_WZ_ROB_MOD_E;
      ScatPilots.piW = &iScatPilWRobModE[0][0];
      ScatPilots.piZ = &iScatPilZRobModE[0][0];
      ScatPilots.iQ = iScatPilQRobModE;
      ScatPilots.piGainTable = &iScatPilGainRobModE[iSpecOccArrayIndex][0];
    break;
    }

  /* Get number of carriers with DC */
  iNumCarrier = iCarrierKmax - iCarrierKmin + 1;

  /* Length of guard-interval measured in "time-bins". We do the calculation
     with integer variables -> "/ RatioTgTu.iDenom" MUST be the last
     operation! */
  iGuardSize = iFFTSizeN * RatioTgTu.iEnum / RatioTgTu.iDenom;

  /* Symbol block size is the guard-interval plus the useful part */
  iSymbolBlockSize = iFFTSizeN + iGuardSize;

  /* Calculate the index of the DC carrier in the shifted spectrum */
  iIndexDCFreq = (int) ((_REAL) VIRTUAL_INTERMED_FREQ *
                        iFFTSizeN / SOUNDCRD_SAMPLE_RATE);

  /* Index of minimum useful carrier (shifted) */
  iShiftedKmin = iIndexDCFreq + iCarrierKmin;

  /* Index. of maximum useful carrier (shifted) */
  iShiftedKmax = iIndexDCFreq + iCarrierKmax;

  /* Calculate number of time-interploated frequency pilots. Special case
     with robustness mode D: pilots in all carriers! BUT: DC carrier is
     counted as a pilot in that case!!! Be aware of that! */
  if (iScatPilFreqInt > 1)
    iNumIntpFreqPil = (int) ((_REAL) iNumCarrier / iScatPilFreqInt + 1);
  else
    iNumIntpFreqPil = iNumCarrier;

  // printf("CellMappingTable Maktable iNumCarrier %d iShiftedKmin %d iIndexDCFreq %d iShiftedKmax %d fftsize %d\n",
  //		iNumCarrier, iShiftedKmin, iIndexDCFreq, iShiftedKmax, iFFTSizeN);

  /* Allocate memory for vectors and matrices ----------------------------- */
  /* Allocate memory for mapping table (Matrix) */
  matiMapTab.Init(iNumSymbolsPerSuperframe, iNumCarrier);

  /* Allocate memory for pilot cells definition and set it to zero */
  matcPilotCells.Init(iNumSymbolsPerSuperframe, iNumCarrier,
                      _COMPLEX((_REAL) 0.0, (_REAL) 0.0));

  /* Allocate memory for vectors with number of certain cells */
  veciNumMSCSym.Init(iNumSymbolsPerSuperframe);
  veciNumFACSym.Init(iNumSymbolsPerSuperframe);


  /* Build table ************************************************************/
  /* Some of the definitions at the beginning are overwritten by successive
     definitions! E.g., first define all carriers as MSC cells */
  iFreqPilotsCounter = 0;
  iTimePilotsCounter = 0;
  for (iSym = 0; iSym < iNumSymbolsPerSuperframe; iSym++)
    {
      /* Frame symbol: Counts symbols in one frame, not super frame! */
      iFrameSym = iSym % iNumSymPerFrame;

      /* Reset FAC counter at the beginning of each new frame */
      if (iFrameSym == 0)  iFACCounter = 0;

      /* Calculate the start value of "p" in equation for gain reference
       cells in Table 90 (8.4.4.1) */
      iScatPilotsCounter = (int) ((_REAL) (iCarrierKmin -
                                           (int) ((_REAL) iScatPilFreqInt / 2 + .5) -
                                           iScatPilFreqInt * mod(iFrameSym, iScatPilTimeInt)
                                           ) / (iScatPilFreqInt * iScatPilTimeInt));

      for (iCar = iCarrierKmin; iCar < iCarrierKmax + 1; iCar++)
        {
          /* Set carrier array index (since we do not have negative indices
         in c++) */
          iCarArrInd = iCar - iCarrierKmin;


          /* MSC ---------------------------------------------------------- */
          /* First set all cells to MSC-cells */
          matiMapTab[iSym][iCarArrInd] = CM_MSC;


          /* FAC ---------------------------------------------------------- */
          /* FAC positions are defined in a table */
          if (iFACCounter <= NUM_FAC_CELLS)
            {
              /* piTableFAC[x * 2]: first column; piTableFAC[x * 2 + 1]:
           second column */
              if (piTableFAC[iFACCounter * 2] * iNumCarrier +
                  piTableFAC[iFACCounter * 2 + 1] == iFrameSym *
                  iNumCarrier + iCar)
                {
                  iFACCounter++;
                  matiMapTab[iSym][iCarArrInd] = CM_FAC;
                }
            }


          /* Scattered pilots --------------------------------------------- */
          /* Standard: 8.4.4.3:
         "In some cases gain references fall in locations which coincide
         with those already defined for either frequency or time
         references. In these cases, the phase definitions given in
         clauses 8.4.2 and 8.4.3 take precedence."
         Therefore, Scattered pilots must be definded FIRST here! */

          /* The rule for calculating the scattered pilots is defined in the
         specification in the following form:
         e.g.: k = 2 + 4 * (s mod 5) + 20 * p
         We define a "frequency-" (FreqInt) and "time-interpolation"
         (TimeInt). In this example, "4" is the FreqInt and "5" is the
         TimeInt. The first term "2" is the half of the FreqInt, rounded
         towards infinity. The parameter "20" is FreqInt * TimeInt */
          if (iCar == (int) ((_REAL) iScatPilFreqInt / 2 + .5) +
              iScatPilFreqInt * mod(iFrameSym, iScatPilTimeInt) +
              iScatPilFreqInt * iScatPilTimeInt * iScatPilotsCounter)
            {
              iScatPilotsCounter++;

              /* Set flag in mapping table */
              matiMapTab[iSym][iCarArrInd] = CM_SCAT_PI;

              /* Set complex value for this pilot */
              /* Phase calculation ---------------------------------------- */
              int in, im, ip, i;

              /* Calculations as in drm-standard (8.4.4.3.1) */
              /* "in" is ROW No and "im" is COLUMN No! */
              in = mod(iFrameSym, ScatPilots.piConst[1] /* "y" */);
              im = (int)
                  ((_REAL) iFrameSym / ScatPilots.piConst[1] /* "y" */);
              ip = (int) ((_REAL) (iCar - ScatPilots.piConst[2] /* "k_0" */ -
                                   in * ScatPilots.piConst[0] /* "x" */) / (
                            ScatPilots.piConst[0] /* "x" */ *
                            ScatPilots.piConst[1] /* "y" */));

              /* Phase_1024[s,k] =
               (4Z_256[n,m]pW_1024[n,m] + p^2(1 + s)Q_1024) mod 1024 */
              iScatPilPhase = mod(4 * ScatPilots.piZ[in *
                                  ScatPilots.iColSizeWZ + im] + ip *
                                  ScatPilots.piW[in *
                                  ScatPilots.iColSizeWZ + im] +
                                  ip * ip * (1 + iFrameSym) * ScatPilots.iQ, 1024);


              /* Gain calculation and applying of complex value ----------- */
              /* Test, if current carrier-index is one of the "boosted pilots"
           position */
              _BOOLEAN bIsBoostedPilot = false;
              for (i = 0; i < NUM_BOOSTED_SCAT_PILOTS; i++)
                {
                  /* In case of match set flag */
                  if (ScatPilots.piGainTable[i] == iCar)
                    bIsBoostedPilot = true;
                }

              /* Boosted pilot: Gain = 2, Regular pilot: Gain = sqrt(2) */
              if (bIsBoostedPilot)
                {
                  matcPilotCells[iSym][iCarArrInd] =
                      Polar2Cart(2, iScatPilPhase);

                  /* Add flag for boosted pilot */
                  matiMapTab[iSym][iCarArrInd] |= CM_BOOSTED_PI;
                }
              else
                {
                  matcPilotCells[iSym][iCarArrInd] =
                      Polar2Cart(sqrt((_REAL) 2.0), iScatPilPhase);
                }
            }


          /* Time-reference pilots ---------------------------------------- */
          /* Time refs at the beginning of each frame, we use a table */
          if (iFrameSym == 0)
            {
              /* Use only the first column in piTableTimePilots */
              if (piTableTimePilots[iTimePilotsCounter * 2] == iCar)
                {
                  /* Set flag in mapping table, consider case of both,
             scattered pilot and time pilot at same position */
                  if (_IsScatPil(matiMapTab[iSym][iCarArrInd]))
                    matiMapTab[iSym][iCarArrInd] |= CM_TI_PI;
                  else
                    matiMapTab[iSym][iCarArrInd] = CM_TI_PI;

                  /* Set complex value for this pilot */
                  matcPilotCells[iSym][iCarArrInd] =
                      Polar2Cart(sqrt((_REAL) 2.0),
                                 piTableTimePilots[iTimePilotsCounter * 2 + 1]);

                  if (iTimePilotsCounter == iNumTimePilots - 1)
                    iTimePilotsCounter = 0;
                  else
                    iTimePilotsCounter++;
                }
            }


          /* Frequency-reference pilots ----------------------------------- */
          /* These pilots are in all symbols, the positions are stored in
         a table */
          /* piTableFreqPilots[x * 2]: first column;
         piTableFreqPilots[x * 2 + 1]: second column */
          if (piTableFreqPilots[iFreqPilotsCounter * 2] == iCar)
            {
              /* Set flag in mapping table, consider case of multiple
           definitions of pilot-mapping */
              if (_IsTiPil(matiMapTab[iSym][iCarArrInd]) ||
                  _IsScatPil(matiMapTab[iSym][iCarArrInd]))
                {
                  matiMapTab[iSym][iCarArrInd] |= CM_FRE_PI;
                }
              else
                matiMapTab[iSym][iCarArrInd] = CM_FRE_PI;

              /* Set complex value for this pilot */
              /* Test for "special case" defined in drm-standard */
              _BOOLEAN bIsFreqPilSpeciCase = false;
              if (eNewRobustnessMode == RM_ROBUSTNESS_MODE_E)
                {
                  /* For robustness mode D, carriers 7 and 21 (Means: first
             and second pilot, not No. 28 (NUM_FREQ_PILOTS - 1) */
                  if (iFreqPilotsCounter != NUM_FREQ_PILOTS - 1)
                    {
                      /* Test for odd values of "s" (iSym) */
                      if ((iFrameSym % 2) == 1)
                        bIsFreqPilSpeciCase = true;
                    }
                }

              /* Apply complex value */
              if (bIsFreqPilSpeciCase) matcPilotCells[iSym][iCarArrInd] = Polar2Cart(sqrt((_REAL) 2.0), mod(piTableFreqPilots[iFreqPilotsCounter * 2 + 1] + 512, 1024));
              else matcPilotCells[iSym][iCarArrInd] = Polar2Cart(sqrt((_REAL) 2.0), piTableFreqPilots[iFreqPilotsCounter * 2 + 1]);

              /* Increase counter and wrap if needed */
              if (iFreqPilotsCounter == NUM_FREQ_PILOTS - 1) iFreqPilotsCounter = 0;
              else iFreqPilotsCounter++;
            }


          /* DC-carrier (not used by DRM) --------------------------------- */
          /* Mark DC-carrier. Must be marked after scattered pilots, because
         in one case (Robustness Mode D) some pilots must be overwritten!
         */
          if (iCar == 0) matiMapTab[iSym][iCarArrInd] = CM_DC;

          /* In Robustness Mode A there are three "not used carriers" */
          if (eNewRobustnessMode == RM_ROBUSTNESS_MODE_A)
            {
              if ((iCar == -1) || (iCar == 1)) matiMapTab[iSym][iCarArrInd] = CM_DC;
            }
        }
    }


  /* Count individual cells *************************************************/
  /* We need to count the cells in a symbol for defining how many values from
     each source is needed to generate one symbol in carrier-mapping */
  /* Init all counters */
  iMaxNumMSCSym = 0;
  iMSCCounter = 0;

  rAvPowPerSymbol = (_REAL) 0.0;
  rAvPilPowPerSym = (_REAL) 0.0;

  for (iSym = 0; iSym < iNumSymbolsPerSuperframe; iSym++)
    {
      /* Init all counters */
      veciNumMSCSym[iSym] = 0;
      veciNumFACSym[iSym] = 0;

      for (iCar = 0; iCar < iNumCarrier; iCar++)
        {
          /* MSC */
          if (_IsMSC(matiMapTab[iSym][iCar]))
            {
              veciNumMSCSym[iSym]++;

              /* Count ALL MSC cells per super-frame */
              iMSCCounter++;
            }

          /* FAC */
          if (_IsFAC(matiMapTab[iSym][iCar]))
            veciNumFACSym[iSym]++;

          /* Calculations for average power per symbol (needed for SNR
         estimation and simulation). DC carrier is zero (contributes not
         to the average power) */
          if (!_IsDC(matiMapTab[iSym][iCar]))
            {
              if (_IsData(matiMapTab[iSym][iCar]))
                {
                  /* Data cells have average power of 1 */
                  rAvPowPerSymbol += (_REAL) 1.0;
                }
              else
                {
                  /* All pilots have power of 2 except of the boosted pilots
             at the edges of the spectrum (they have power of 4) */
                  if (_IsBoosPil(matiMapTab[iSym][iCar]))
                    {
                      rAvPowPerSymbol += (_REAL) 4.0;
                      rAvPilPowPerSym += (_REAL) 4.0;
                    }
                  else
                    {
                      /* Regular pilot has power of 2 */
                      rAvPowPerSymbol += (_REAL) 2.0;
                      rAvPilPowPerSym += (_REAL) 2.0;
                    }
                }
            }
        }

      /* Set maximum for symbol */
      /* MSC */
      if (iMaxNumMSCSym < veciNumMSCSym[iSym])
        iMaxNumMSCSym = veciNumMSCSym[iSym];
    }

  /* Set number of useful MSC cells */
  iNumUsefMSCCellsPerFrame =
      (int) (iMSCCounter / NUM_FRAMES_IN_SUPERFRAME);

  /* Calculate dummy cells for MSC */
  iNoMSCDummyCells = iMSCCounter - iNumUsefMSCCellsPerFrame  *
      NUM_FRAMES_IN_SUPERFRAME;

  /* Correct last MSC count (because of dummy cells) */
  veciNumMSCSym[iNumSymbolsPerSuperframe - 1] -= iNoMSCDummyCells;

  /* Normalize the average powers */
  rAvPowPerSymbol /= iNumSymbolsPerSuperframe;
  rAvPilPowPerSym /= iNumSymbolsPerSuperframe;


  /* ########################################################################## */
#ifdef _DEBUG_
  /* Save table in file */
  FILE* pFile = fopen("CarMapTable.txt", "w");

  /* Title */
  fprintf(pFile, "Robustness mode ");
  switch (eNewRobustnessMode)
    {
    case RM_ROBUSTNESS_MODE_A:
      fprintf(pFile, "A");
    break;
    case RM_ROBUSTNESS_MODE_B:
      fprintf(pFile, "B");
    break;
    case RM_ROBUSTNESS_MODE_E:
      fprintf(pFile, "E");
    break;
    }
  fprintf(pFile, " / Spectrum occupancy %d\n\n", iSpecOccArrayIndex);

  /* Actual table */
  for (int i = 0; i < iNumSymbolsPerSuperframe; i++)
    {
      for (int j = 0; j < iNumCarrier; j++)
        {
          if (_IsDC(matiMapTab[i][j]))
            {
              fprintf(pFile, ":");
              continue;
            }
          if (_IsMSC(matiMapTab[i][j]))
            {
              fprintf(pFile, ".");
              continue;
            }
          if (_IsFAC(matiMapTab[i][j]))
            {
              fprintf(pFile, "X");
              continue;
            }
          if (_IsTiPil(matiMapTab[i][j]))
            {
              fprintf(pFile, "T");
              continue;
            }
          if (_IsFreqPil(matiMapTab[i][j]))
            {
              fprintf(pFile, "f");
              continue;
            }
          if (_IsScatPil(matiMapTab[i][j]))
            {
              /* Special mark for boosted pilots */
              if (_IsBoosPil(matiMapTab[i][j]))
                fprintf(pFile, "*");
              else
                fprintf(pFile, "0");
              continue;
            }
        }
      fprintf(pFile, "\n");
    }

  /* Legend */
  fprintf(pFile, "\n\nLegend:\n\t: DC-carrier\n\t. MCS cells");
  fprintf(pFile, "\n\tX FAC cells\n\tT time pilots\n\tf frequency pilots");
  fprintf(pFile, "\n\t0 scattered pilots\n\t* boosted scattered pilots\n");

  fclose(pFile);

  //#ifdef _DEBUG_

  /* Save pilot values in file */
  /* Use following command to plot pilot complex values in Matlab:

  clear all;close all;load PilotCells.dat;subplot(211),mesh(abs(complex(PilotCells(:,1:2:end), PilotCells(:,2:2:end))));subplot(212),mesh(angle(complex(PilotCells(:,1:2:end), PilotCells(:,2:2:end))))

(It plots the absolute of the pilots in the upper plot and angle in 
the lower plot.)
*/
  pFile = fopen("test/PilotCells.dat", "w");
  for (int z = 0; z < iNumSymbolsPerSuperframe; z++)
    {
      for (int v = 0; v < iNumCarrier; v++)
        fprintf(pFile, "%e %e ", matcPilotCells[z][v].real(),
                matcPilotCells[z][v].imag());

      fprintf(pFile, "\n");
    }
  fclose(pFile);
#endif
  /* ########################################################################## */
}

_COMPLEX CCellMappingTable::Polar2Cart(const _REAL rAbsolute, 
                                       const int iPhase) const
{
  /*
  This function takes phases normalized to 1024 as defined in the drm-
  standard.
*/
  return _COMPLEX(rAbsolute * cos((_REAL) 2 * crPi * iPhase / 1024),
                  rAbsolute * sin((_REAL) 2 * crPi * iPhase / 1024));
}

int CCellMappingTable::mod(const int ix, const int iy) const
{
  /* Modulus definition for integer numbers */
  if (ix < 0)
    return ix % iy + iy;
  else
    return ix % iy;
}
