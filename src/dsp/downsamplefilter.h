/***************************************************************************
 *   Copyright (C) 2000-2019 by Johan Maes                                 *
 *   on4qz@telenet.be                                                      *
 *   http://users.telenet.be/on4qz                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef DOWNSAMPLEFILTER_H
#define DOWNSAMPLEFILTER_H
#include "appdefs.h"


#define DSAMPLEFILTERLEN 180
#define CONVLENGTH 75
#define CONVDELAY  55

extern const FILTERPARAMTYPE downSampleFilterParam[DSAMPLEFILTERLEN];

class downsampleFilter
{
public:
  downsampleFilter();
  downsampleFilter(unsigned int len, bool scaled);
  ~downsampleFilter();
	void allocate(unsigned int dataLength);
  void setFilterParams(bool scaled);
  void downSample4(short int *data);
  FILTERPARAMTYPE *filteredDataPtr() {return filteredDataBuffer;}
  void init();
  DSPFLOAT avgVolumeDb;
  FILTERPARAMTYPE *getVolumePtr() {return volumeBuffer;}

private:
	void normalizeGain();
  FILTERPARAMTYPE *filteredDataBuffer;
//  FILTERPARAMTYPE *volumeDataBuffer;
  FILTERPARAMTYPE *volumeBuffer;
  FILTERPARAMTYPE *filterParams;
	unsigned int filterLength;
  FILTERPARAMTYPE *samplesI;
  FILTERPARAMTYPE *samplesQ;
  DSPFLOAT *volSamples;
	unsigned int length;
  int zeroes;
  int ssize;
//  unsigned int rxBytes;
  bool first;

};

#endif
