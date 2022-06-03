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
#include "downsamplefilter.h"
#include "appglobal.h"
#include "arraydumper.h"
#include "logging.h"

#include <QDebug>



#include <math.h>

#define VOLINTEGRATOR 0.5



downsampleFilter::downsampleFilter()
{
  filteredDataBuffer=NULL;
  volumeBuffer=NULL;
  filterParams=NULL;
  samplesI=NULL;
  samplesQ=NULL;
  volSamples=NULL;
  filterLength=0;

}

downsampleFilter::downsampleFilter(unsigned int len, bool scaled)
{
  filteredDataBuffer=NULL;
  volumeBuffer=NULL;
  filterParams=NULL;
  samplesI=NULL;
  samplesQ=NULL;
  volSamples=NULL;
  filterLength=0;
  setFilterParams(scaled);
  allocate(len);
  init();
}

downsampleFilter::~downsampleFilter()
{
  if(filteredDataBuffer) delete [] filteredDataBuffer;
  if(filterParams) delete [] filterParams;
  if(samplesI) delete [] samplesI;
  if(samplesQ) delete [] samplesQ;
  if(volSamples) delete [] volSamples;
  if(volumeBuffer) delete [] volumeBuffer;
//  if(volumeDataBuffer) delete [] volumeDataBuffer;
}



void downsampleFilter::allocate(unsigned int len)
{
  length=len;
  if(filteredDataBuffer) delete [] filteredDataBuffer;
  filteredDataBuffer=new FILTERPARAMTYPE [length/4];
  volumeBuffer=new FILTERPARAMTYPE [length/4+CONVDELAY];
}


void downsampleFilter::init()
{
  unsigned int i;
  first=true;
  for(i=0;i<filterLength;i++)
    {
      samplesI[i]=0;
      samplesQ[i]=0;
    }
  for(i=0;i<length/4;i++)
    {
      filteredDataBuffer[i]=0;
    }
  for(i=0;i<CONVLENGTH;i++)
    {
      volSamples[i]=0;
    }
  for(i=0;i<length/4+CONVDELAY;i++)
    {
      volumeBuffer[i]=0.;
    }
  avgVolumeDb=0;
}

void downsampleFilter::setFilterParams(bool scaled)
{
  unsigned int i;

  filterLength=DSAMPLEFILTERLEN;
  if(filterParams) delete [] filterParams;
  filterParams=new FILTERPARAMTYPE [filterLength];
  if(samplesI) delete [] samplesI;
  if(samplesQ) delete [] samplesQ;
  samplesI=new FILTERPARAMTYPE[filterLength];
  samplesQ=new FILTERPARAMTYPE[filterLength];
  volSamples=new DSPFLOAT[CONVLENGTH];
  zeroes=filterLength-1;
  ssize=(zeroes)*sizeof(DSPFLOAT);


  DSPFLOAT gain=0;
  for(i=0;i<filterLength;i++)
    {
      gain+=downSampleFilterParam[i];
    }
  for(i=0;i<filterLength;i++)
    {
      if(scaled) filterParams[i]=downSampleFilterParam[i]/gain;
      else filterParams[i]=downSampleFilterParam[i];
    }
  addToLog(QString("filtergain:=%1").arg(gain),LOGPERFORM);
}



void downsampleFilter::downSample4(short int *data)
{
  unsigned int i,k;
  DSPFLOAT tmpVol;
  FILTERPARAMTYPE res;
  const FILTERPARAMTYPE *cf1;
  FILTERPARAMTYPE *fp1;
  FILTERPARAMTYPE res0,res1,res2,res3;
//  arrayDump("DownIn",data,RXSTRIPE,true,false);


  memmove(volumeBuffer,volumeBuffer+length/4,CONVDELAY*sizeof(FILTERPARAMTYPE));

  for (k=0;k<length;k+=4)
    {
      res0=res1=res2=res3=0;
      cf1 = filterParams;
      fp1 = samplesI;
      memmove(samplesI+4, samplesI, (filterLength-4)*sizeof(FILTERPARAMTYPE));
      samplesI[3]=data[k];
      samplesI[2]= data[k+1];
      samplesI[1]= data[k+2];
      samplesI[0]= data[k+3];
      for(i=0;i<filterLength;i+=4,fp1+=4,cf1+=4)
        {
          res0+=(*fp1)*(*cf1);
          res1+=(*(fp1+1))*(*(cf1+1));
          res2+=(*(fp1+2))*(*(cf1+2));
          res3+=(*(fp1+3))*(*(cf1+3));
        }
      res=res0+res1+res2+res3;
      filteredDataBuffer[k/4]=res;
      memmove(volSamples+1,volSamples,(CONVLENGTH-1)*sizeof(DSPFLOAT));
      volSamples[0]=abs(res)*1.35;

      tmpVol=0;
      for(i=0;i<CONVLENGTH;i++)
        {
          tmpVol+=volSamples[i];
        }
      volumeBuffer[k/4+CONVDELAY]=tmpVol/CONVLENGTH;
//      volumeDataBuffer[k/4]=tmpVol/(CONVLENGTH);
    }
  avgVolumeDb=20*log(volumeBuffer[length/8])-110;
//  arrayDump("DownOut",filteredDataBuffer,length/4,true,true);
//  arrayDump("VolOut",volumeDataBuffer,length/4,true,true);
}

// rFIR
// localSamplingrate = 48000.000000
// number of taps = 180
// Band0 Lower=0.000000, Upper=2800.000000, Desired=1.000000, Weight=1.000000
// Band1 Lower=3500.000000, Upper=24000.000000, Desired=0.000000, Weight=10.000000
// window applied
//Coefficients:
const FILTERPARAMTYPE downSampleFilterParam[DSAMPLEFILTERLEN]=
{
   -3.7745373873035e-5,
   1.2821993095221e-5,
   2.311774607022e-5,
   3.93416585698818e-5,
   5.96630768741091e-5,
   8.15536238549509e-5,
   0.000101722709767542,
   0.000116276271064301,
   0.000121134249908642,
   0.000112667967400144,
   8.85061965447603e-5,
   4.83892511055542e-5,
   -5.12050973476878e-6,
   -6.63000241955111e-5,
   -0.000126498754490435,
   -0.000175019484683051,
   -0.000200634430912931,
   -0.000193612975094613,
   -0.000147565501362106,
   -6.29787053004485e-5,
   5.3151532907392e-5,
   0.000185663591404659,
   0.000312869274430059,
   0.000409285568879863,
   0.000449756444030134,
   0.000414496951656379,
   0.000294105750555632,
   9.35912229123051e-5,
   -0.000165601564791902,
   -0.00044640802132058,
   -0.000700237166667507,
   -0.000874165824697474,
   -0.000920325965202881,
   -0.000806050005626634,
   -0.000522886928400522,
   -9.25367983951586e-5,
   0.000431289193698061,
   0.000968555754911369,
   0.00142243885570497,
   0.00169530324453739,
   0.00170682284447917,
   0.00141183031333547,
   0.000814531791130871,
   -2.45069535733128e-5,
   -0.00099078573814898,
   -0.00192873057602086,
   -0.00266417331498039,
   -0.0030336497628345,
   -0.00291602413460463,
   -0.00226082329124683,
   -0.00110762621769599,
   0.000408348452134145,
   0.00206727405426683,
   0.00359246937521952,
   0.00469322329756861,
   0.00511431509758501,
   0.00468722876609756,
   0.00337177582297895,
   0.00128034801536449,
   -0.00132164430054919,
   -0.00404279527077636,
   -0.00641821805936274,
   -0.00798175134076984,
   -0.00834762501082702,
   -0.00728844360368625,
   -0.00479554097560457,
   -0.00110917359991169,
   0.00329009088110941,
   0.00773195237700219,
   0.0114452598882855,
   0.0136749644542385,
   0.0138115533747118,
   0.0115118605606838,
   0.00679329350542433,
   8.1748290004832e-5,
   -0.0077978972841613,
   -0.0156927727462633,
   -0.0222502345991312,
   -0.026088061124922,
   -0.0259857248583191,
   -0.0210702338547245,
   -0.0109692249861873,
   0.00409269475786721,
   0.0232714919148808,
   0.0451623194183613,
   0.0679352326825397,
   0.0895327926436653,
   0.107905219007286,
   0.121252681749066,
   0.128250977806601,
   0.128241891841668,
   0.121183971401713,
   0.107782931581642,
   0.089380577086183,
   0.0677811957851725,
   0.0450342640538684,
   0.0231922642804605,
   0.00407642705530391,
   -0.0109193514192932,
   -0.0209623465780798,
   -0.0258377065514141,
   -0.0259243772593719,
   -0.0220977075791061,
   -0.0155760373514757,
   -0.00773531334234096,
   8.10439299597243e-5,
   0.00673072404465227,
   0.0113989423580145,
   0.0136677521637495,
   0.0135242773076766,
   0.0113121310281208,
   0.00763723779722783,
   0.00324773606385114,
   -0.00109419621738977,
   -0.00472773521304448,
   -0.00718070408259266,
   -0.00821880222557047,
   -0.00785332662004067,
   -0.0063106795715127,
   -0.00397233394884528,
   -0.00129770774231779,
   0.00125627414397684,
   0.00330601442656137,
   0.00459247977024611,
   0.00500724473373418,
   0.0045915336446142,
   0.0035119604617914,
   0.00201938511639171,
   0.000398575790043235,
   -0.0010802549654419,
   -0.00220316299747724,
   -0.00283930339342042,
   -0.00295134721483293,
   -0.0025896723210505,
   -0.00187315829857408,
   -0.000961382358312621,
   -2.37581136811773e-5,
   0.000788913665704397,
   0.00136614000435156,
   0.00165000316065111,
   0.00163726749497039,
   0.00137237896324327,
   0.00093352423400739,
   0.000415262312175493,
   -8.90049238507674e-5,
   -0.000502395727301656,
   -0.0007736279266968,
   -0.000882343815877056,
   -0.00083716454123059,
   -0.00066985195239054,
   -0.000426559350805018,
   -0.000158060915641661,
   8.92295205720884e-5,
   0.000280087438665955,
   0.000394307002607022,
   0.000427389303447503,
   0.000388526242968403,
   0.000296704742725585,
   0.000175906711695273,
   5.03152557592893e-5,
   -5.95729463836399e-5,
   -0.000139497224417446,
   -0.000182939261691597,
   -0.00018951796313575,
   -0.00016531066549151,
   -0.000119504934298439,
   -6.26663462287718e-5,
   -4.84410009996915e-6,
   4.58362969798013e-5,
   8.39847177922791e-5,
   0.000107157181539094,
   0.000115539629094478,
   0.00011129193114192,
   9.77629608902248e-5,
   7.87516808137608e-5,
   5.79226786300601e-5,
   3.84207370527109e-5,
   2.27214590093519e-5,
   1.26877050578184e-5,
   -0.000470151901610343
};






