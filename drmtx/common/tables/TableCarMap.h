/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Adapted for ham sttv use Ties Bos - PA0MBO
 *
 * Description:
 *	Tables for carrier mapping
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

#if !defined(TABLE_CAR_MAP_H__3B0_CA63_4344_BB2B_23E7912__INCLUDED_)
#define TABLE_CAR_MAP_H__3B0_CA63_4344_BB2B_23E7912__INCLUDED_

#include "../GlobalDefinitions.h"
#include "../matlib/Matlib.h"
#include "TableQAMMapping.h"


/* Global functions ***********************************************************/
/*
	----------------------------------------------------------------------------
	Implementation of distance to nearest constellation point (symbol) for all
	QAM types
*/


inline CComplex MinDist4QAM(const CComplex cI)
{
	/* Return vector pointing to nearest signal point of this constellation.
	   2 possible constellation points for real and imaginary axis */
	return CComplex(
		/* Real axis minimum distance */
		Min(Abs(rTableQAM4[0][0] - Real(cI)), Abs(rTableQAM4[1][0] - Real(cI))),
		/* Imaginary axis minimum distance */
		Min(Abs(rTableQAM4[0][1] - Imag(cI)), Abs(rTableQAM4[1][1] - Imag(cI))));
}

inline CComplex MinDist16QAM(const CComplex cI)
{
	/* Return vector pointing to nearest signal point of this constellation.
	   4 possible constellation points for real and imaginary axis */
	return CComplex(
		/* Real axis minimum distance */
		Min(Abs(rTableQAM16[0][0] - Real(cI)), Abs(rTableQAM16[1][0] - Real(cI)),
			Abs(rTableQAM16[2][0] - Real(cI)), Abs(rTableQAM16[3][0] - Real(cI))),
		/* Imaginary axis minimum distance */
		Min(Abs(rTableQAM16[0][1] - Imag(cI)), Abs(rTableQAM16[1][1] - Imag(cI)),
			Abs(rTableQAM16[2][1] - Imag(cI)), Abs(rTableQAM16[3][1] - Imag(cI))));
}

inline CComplex MinDist64QAM(const CComplex cI)
{
	/* Return vector pointing to nearest signal point of this constellation.
	   8 possible constellation points for real and imaginary axis */
	return CComplex(
		/* Real axis minimum distance */
		Min(Abs(rTableQAM64SM[0][0] - Real(cI)), Abs(rTableQAM64SM[1][0] - Real(cI)),
			Abs(rTableQAM64SM[2][0] - Real(cI)), Abs(rTableQAM64SM[3][0] - Real(cI)),
			Abs(rTableQAM64SM[4][0] - Real(cI)), Abs(rTableQAM64SM[5][0] - Real(cI)),
			Abs(rTableQAM64SM[6][0] - Real(cI)), Abs(rTableQAM64SM[7][0] - Real(cI))),
		/* Imaginary axis minimum distance */
		Min(Abs(rTableQAM64SM[0][1] - Imag(cI)), Abs(rTableQAM64SM[1][1] - Imag(cI)),
			Abs(rTableQAM64SM[2][1] - Imag(cI)), Abs(rTableQAM64SM[3][1] - Imag(cI)),
			Abs(rTableQAM64SM[4][1] - Imag(cI)), Abs(rTableQAM64SM[5][1] - Imag(cI)),
			Abs(rTableQAM64SM[6][1] - Imag(cI)), Abs(rTableQAM64SM[7][1] - Imag(cI))));
}




/* Time pilots ****************************************************************/
/* The two numbers are: {carrier no, phase} (Phases are normalized to 1024) */
#define RMA_NUM_TIME_PIL	16
const int iTableTimePilRobModA[RMA_NUM_TIME_PIL][2] = {
	{ 6, 973},
	{ 7, 205},
	{11, 717},
	{12, 264},
	{15, 357},
	{16, 357},
	{23, 952},
	{29, 440},
	{30, 856},
	{33,  88},
	{34,  88},
	{38,  68},
	{39, 836},
	{41, 836},
	{45, 836},
	{46, 1008},
};

#define RMB_NUM_TIME_PIL	15

const int iTableTimePilRobModB[RMB_NUM_TIME_PIL][2] = {
	{ 6, 304},
	{10, 331},
	{11, 108},
	{14, 620},
	{17, 192},
	{18, 704},
	{27,  44},
	{28, 432},
	{30, 588},
	{33, 844},
	{34, 651},
	{38, 651},
	{40, 651},
	{41, 460},
	{44, 944},
};

#define RME_NUM_TIME_PIL	8
const int iTableTimePilRobModE[RME_NUM_TIME_PIL][2] = {
	{ 7, 432},
	{ 8, 331},
	{13, 108},
	{14, 620},
	{21, 192},
	{22, 704},
	{26,  44},
	{27, 304},
};


/* Scattered pilots ***********************************************************/
/* Definitions for the positions of scattered pilots */
#define RMA_SCAT_PIL_FREQ_INT	4
#define RMA_SCAT_PIL_TIME_INT	5

#define RMB_SCAT_PIL_FREQ_INT	2
#define RMB_SCAT_PIL_TIME_INT	3

#define RME_SCAT_PIL_FREQ_INT	1
#define RME_SCAT_PIL_TIME_INT	4

/* Phase definitions of scattered pilots ------------------------------------ */
const int iTableScatPilConstRobModA[3] = {4, 5, 2};

const int iTableScatPilConstRobModB[3] = {2, 3, 1};

const int iTableScatPilConstRobModE[3] = {1, 4, 1};

#define SIZE_ROW_WZ_ROB_MOD_A	5
#define SIZE_COL_WZ_ROB_MOD_A	3
const int iScatPilWRobModA[SIZE_ROW_WZ_ROB_MOD_A][SIZE_COL_WZ_ROB_MOD_A] = {
	{228, 341, 455},
	{455, 569, 683},
	{683, 796, 910},
	{910,   0, 114},
	{114, 228, 341}
};
const int iScatPilZRobModA[SIZE_ROW_WZ_ROB_MOD_A][SIZE_COL_WZ_ROB_MOD_A] = {
	{0,    81, 248},
	{18,  106, 106},
	{122, 116,  31},
	{129, 129,  39},
	{33,   32, 111}
};
const int iScatPilQRobModA = 36;

#define SIZE_ROW_WZ_ROB_MOD_B	3
#define SIZE_COL_WZ_ROB_MOD_B	5
const int iScatPilWRobModB[SIZE_ROW_WZ_ROB_MOD_B][SIZE_COL_WZ_ROB_MOD_B] = {
	{512,   0, 512,   0, 512},
	{0,   512,   0, 512,   0},
	{512,   0, 512,   0, 512}
};
const int iScatPilZRobModB[SIZE_ROW_WZ_ROB_MOD_B][SIZE_COL_WZ_ROB_MOD_B] = {
	{0,    57, 164,  64,  12},
	{168, 255, 161, 106, 118},
	{25,  232, 132, 233,  38}
};
const int iScatPilQRobModB = 12;

#define SIZE_ROW_WZ_ROB_MOD_E	4
#define SIZE_COL_WZ_ROB_MOD_E	5
const int iScatPilWRobModE[SIZE_ROW_WZ_ROB_MOD_E][SIZE_COL_WZ_ROB_MOD_E] = {
	{512,   0, 512,   0, 512},
	{0,   512,   0, 512,   0},
	{512,   0, 512,   0, 512},
	{0,   512,   0, 512,   0}
};
const int iScatPilZRobModE[SIZE_ROW_WZ_ROB_MOD_E][SIZE_COL_WZ_ROB_MOD_E] = {
	{0,    57, 164,  64,  12},
	{168, 255, 161, 106, 118},
	{25,  232, 132, 233,  38},
	{168, 255, 161, 106, 118}
};
const int iScatPilQRobModE = 10;

/* Gain definitions of scattered pilots ------------------------------------- */
#define NUM_BOOSTED_SCAT_PILOTS		4
const int iScatPilGainRobModA[2][NUM_BOOSTED_SCAT_PILOTS] = {
	{2, 4, 50, 54},
	{2, 6, 54, 58}
};

const int iScatPilGainRobModB[2][NUM_BOOSTED_SCAT_PILOTS] = {
	{1, 3, 43, 45},
	{1, 3, 49, 51}
};



const int iScatPilGainRobModE[2][NUM_BOOSTED_SCAT_PILOTS] = {
	{1,29, 0, 0},
	{1,31, 0, 0}
};

#endif // !defined(TABLE_CAR_MAP_H__3B0_CA63_4344_BB2B_23E7912__INCLUDED_)
