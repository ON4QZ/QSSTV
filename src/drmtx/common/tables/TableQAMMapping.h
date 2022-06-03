/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Adapted for ham sstv use Ties Bos - PA0MBO
 *
 * Description:
 *	Tables for QAM mapping (Mapping is already normalized)
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

#if !defined(QAM_MAPPING_H__3B0_CA63_4344_BB2B_23E7912__INCLUDED_)
#define QAM_MAPPING_H__3B0_CA63_4344_BB2B_23E7912__INCLUDED_

#include "../GlobalDefinitions.h"


/* Definitions ****************************************************************/
/* Input bits are collected in bytes separately for imaginary and real part. 
   The order is: [i_0, i_1, i_2] and [q_0, q_1, q_2] -> {i, q} 
   All entries are normalized according to the DRM-standard */
const _REAL rTableQAM64SM[8][2] = {
	{ 1.0801234497f,  1.0801234497f},
	{-0.1543033499f, -0.1543033499f},
	{ 0.4629100498f,  0.4629100498f},
	{-0.7715167498f, -0.7715167498f},
	{ 0.7715167498f,  0.7715167498f},
	{-0.4629100498f, -0.4629100498f},
	{ 0.1543033499f,  0.1543033499f},
	{-1.0801234497f, -1.0801234497f}
};

const _REAL rTableQAM64HMsym[8][2] = {
	{ 1.0801234497f,  1.0801234497f},
	{ 0.4629100498f,  0.4629100498f},
	{ 0.7715167498f,  0.7715167498f},
	{ 0.1543033499f,  0.1543033499f},
	{-0.1543033499f, -0.1543033499f},
	{-0.7715167498f, -0.7715167498f},
	{-0.4629100498f, -0.4629100498f},
	{-1.0801234497f, -1.0801234497f}
};

const _REAL rTableQAM64HMmix[8][2] = {
	{ 1.0801234497f,  1.0801234497f},
	{ 0.4629100498f, -0.1543033499f},
	{ 0.7715167498f,  0.4629100498f},
	{ 0.1543033499f, -0.7715167498f},
	{-0.1543033499f,  0.7715167498f},
	{-0.7715167498f, -0.4629100498f},
	{-0.4629100498f,  0.1543033499f},
	{-1.0801234497f, -1.0801234497f}
};

const _REAL rTableQAM16[4][2] = {
	{ 0.9486832980f,  0.9486832980f},
	{-0.3162277660f, -0.3162277660f},
	{ 0.3162277660f,  0.3162277660f},
	{-0.9486832980f, -0.9486832980f}
};

const _REAL rTableQAM4[2][2] = {
	{ 0.7071067811f,  0.7071067811f},
	{-0.7071067811f, -0.7071067811f}
};


#endif // !defined(QAM_MAPPING_H__3B0_CA63_4344_BB2B_23E7912__INCLUDED_)
