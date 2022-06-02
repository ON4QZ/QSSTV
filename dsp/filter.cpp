#include "filter.h"
#include "nco.h"
#include <QDebug>
#include "arraydumper.h"

filter::filter(efilterType fType, uint dataLenght)
{
  filterType=fType;
  resetPointers();
  dataLen=dataLenght;
  coefZPtrNewed=false;
}


filter::~filter()
{
  deleteBuffers();
}


void filter::init()
{
  deleteBuffers();
  nZeroes=0;
  nPoles=0;
  gain=1;
//  avgVolume=0;
  volumeAttackIntegrator=1;
  volumeDecayIntegrator=1;
  prevTemp=0;
  angleToFc=SAMPLERATE/(2*M_PI);
  frCenter=0;
  fltrIndex=0;
  coefIndex=0;
  resIprev=0;
  resQprev=0;

}

void filter::resetPointers()
{
  coefZPtr=NULL;
  coefPPtr=NULL;
  sampleBufferIPtr=NULL;
  sampleBufferQPtr=NULL;
  sampleBufferYIPtr=NULL;
  filteredPtr=NULL;
  volumePtr=NULL;
  demodPtr=NULL;
}

void filter::deleteBuffers()
{
  if(coefZPtr!=NULL && coefZPtrNewed)
      {
//          qDebug() <<"delete coefZptr" << this << coefZPtr;
          delete [] coefZPtr;
          coefZPtr=NULL;
          coefZPtrNewed=false;
      }
  if(sampleBufferIPtr!=NULL)  delete [] sampleBufferIPtr;
  if(sampleBufferQPtr!=NULL)  delete [] sampleBufferQPtr;
  if(sampleBufferYIPtr!=NULL) delete [] sampleBufferYIPtr;
  if(filteredPtr!=NULL)       delete [] filteredPtr;
  if(volumePtr!=NULL)         delete [] volumePtr;
  if(demodPtr!=NULL)          delete [] demodPtr;
  resetPointers();
}




void filter::allocate()
{
  uint i;
  bufSize=nZeroes*sizeof(FILTERPARAMTYPE);
  sampleBufferIPtr=new FILTERPARAMTYPE[nZeroes+1];
  sampleBufferQPtr=new FILTERPARAMTYPE[nZeroes+1];

  for(i=0;i<=nZeroes;i++)
    {
      sampleBufferIPtr[i]=0;
      sampleBufferQPtr[i]=0;
    }

  if(nPoles>0)
    {
      sampleBufferYIPtr=new FILTERPARAMTYPE[nPoles+1];
      for(i=0;i<=nPoles;i++)  sampleBufferYIPtr[i] =0;
    }
  volumePtr=new FILTERPARAMTYPE[dataLen];
  demodPtr=new quint16[dataLen];
  filteredPtr=new FILTERPARAMTYPE[dataLen];
  for(i=0;i<dataLen;i++)
    {
      volumePtr[i]=0;
      demodPtr[i]=0;
      filteredPtr[i]=0;
    }
  if(frCenter>0)
    {
      nco.init(frCenter/SAMPLERATE);
    }
  fltrIndex=nZeroes;
  coefIndex = nZeroes+1;
}


void filter::processFIR(FILTERPARAMTYPE *dataPtr, double *dataOutputPtr)
{
   FILTERPARAMTYPE resI;
  const FILTERPARAMTYPE *cf1;
  unsigned int i,k;
  int fi;
  for (k=0;k<dataLen;k++)
    {
      sampleBufferIPtr[fltrIndex]=dataPtr[k];
      fi=fltrIndex--;
      if(fltrIndex<0)
        fltrIndex=nZeroes;
      resI=0;
      cf1 = coefZPtr;
      for(i=0;i<=nZeroes;i++,cf1++)
        {
          if ((fi+i) >nZeroes)
            {
              fi -=(nZeroes+1);
            }

          resI+=sampleBufferIPtr[fi+i]*(*cf1);
        }

      dataOutputPtr[k]=resI/gain;
    }

}

void filter::processFIRInt(FILTERPARAMTYPE *dataPtr, quint16 *dataOutputPtr)
{
  //  FILTERPARAMTYPE resI=0;
  //  FILTERPARAMTYPE *fp1;
  //  const FILTERPARAMTYPE *cf1;
  //  unsigned int i,k;
  //  for (k=0;k<dataLen;k++)
  //    {
  //      resI=0;
  //      cf1 = coefZPtr;
  //      fp1 = sampleBufferIPtr;
  //      memmove(sampleBufferIPtr+1, sampleBufferIPtr, bufSize);
  //      sampleBufferIPtr[0]=dataPtr[k];
  //      for(i=0;i<=nZeroes;i++,fp1++,cf1++)
  //        {
  //          resI+=(*fp1)*(*cf1);
  //        };
  //      dataOutputPtr[k]=(quint16)rint(resI/gain);
  //    }

  FILTERPARAMTYPE resI=0;
  const FILTERPARAMTYPE *cf1;
  unsigned int i,k;
  int fi;
  for (k=0;k<dataLen;k++)
    {
      sampleBufferIPtr[fltrIndex]=dataPtr[k];
      fi=fltrIndex--;
      if(fltrIndex<0)
        fltrIndex=nZeroes;
      resI=0;
      cf1 = coefZPtr;
      for(i=0;i<=nZeroes;i++,cf1++)
        {
          if ((fi+i) >nZeroes)
            {
              fi -=(nZeroes+1);
            }

          resI+=sampleBufferIPtr[fi+i]*(*cf1);
        }
      dataOutputPtr[k]=(quint16)rint(resI/gain);
    }
}


void filter::processFIRDemod(FILTERPARAMTYPE *dataPtr,FILTERPARAMTYPE *dataOutputPtr)
{
  FILTERPARAMTYPE resI,resQ;
  const FILTERPARAMTYPE *cf1;
  FILTERPARAMTYPE discRe,discIm;
  FILTERPARAMTYPE  temp;
  unsigned int i,k;
  int fi;
  for (k=0;k<dataLen;k++)
    {
      nco.multiply(sampleBufferIPtr[fltrIndex],sampleBufferQPtr[fltrIndex],dataPtr[k]);
      fi=fltrIndex--;
      if(fltrIndex<0)
        fltrIndex=nZeroes;
      resI=0;
      resQ=0;
      cf1 = coefZPtr;
      for(i=0;i<=nZeroes;i++,cf1++)
        {
          if ((fi+i) >nZeroes)
            {
              fi -=(nZeroes+1);
            }

          resI+=sampleBufferIPtr[fi+i]*(*cf1);
          resQ+=sampleBufferQPtr[fi+i]*(*cf1);
        }
      resI/=gain;
      resQ/=gain;
      discRe=resI*resIprev+resQ*resQprev;
      discIm=-resQ*resIprev+resQprev*resI;
      resIprev=resI;
      resQprev=resQ;
      if(discRe==0) discRe=0.0001;
      temp=frCenter-atan2(discIm,discRe)*angleToFc;
      if(temp<500) temp=prevTemp;
      if(temp>2600) temp=prevTemp;
      prevTemp=temp;
      dataOutputPtr[k]=temp;
//      double vol=sqrt(resI*resI+resQ*resQ);


//      if (vol>avgVolume) avgVolume=avgVolume*(1-volumeAttackIntegrator)+(volumeAttackIntegrator)*vol;
//      else if (vol<avgVolume) avgVolume=avgVolume*(1-volumeDecayIntegrator)+(volumeDecayIntegrator)*vol;
//      volumePtr[k]=avgVolume;
    }
//  arrayDump("Volume",volumePtr,RXSTRIPE,true,false);
}




void filter::processHILBVolume(FILTERPARAMTYPE *dataPtr)
{
  FILTERPARAMTYPE resI;
  FILTERPARAMTYPE resQ;
  const FILTERPARAMTYPE *cf1;
  FILTERPARAMTYPE *fp1;
  unsigned int i;
  uint k;
  for (k=0;k<dataLen;k++)
    {
      resQ=0;
      cf1 = coefZPtr+1;
      fp1 = sampleBufferIPtr+1;
      memmove(sampleBufferIPtr+1, sampleBufferIPtr,bufSize);
      sampleBufferIPtr[0]=dataPtr[k];
      for(i=1;i<=nZeroes;i+=2,fp1+=2,cf1+=2)
        {
          resQ+=(*fp1)*(*cf1);
        };
      resQ/=gain;
      resI=sampleBufferIPtr[nZeroes/2];
      volumePtr[k]=sqrt(resQ*resQ+resI*resI);
    }
}



void filter::processIIRRectified(double *dataPtr)
{
  unsigned int i,j;
  double resx;
//  arrayDump("Inp",dataPtr,dataLen,true,true);
  for (i=0;i<dataLen;i++)
    {
      resx=0;
      memmove(sampleBufferIPtr,sampleBufferIPtr+1,bufSize);
      memmove(sampleBufferYIPtr,sampleBufferYIPtr+1,nPoles*sizeof(FILTERPARAMTYPE));
      sampleBufferIPtr[nZeroes]=dataPtr[i]/gain;
      if(!(nZeroes&1))
        {
          resx=sampleBufferIPtr[nZeroes/2]*coefZPtr[nZeroes/2];
        }
      for (j=0;j<((nZeroes+1)/2);j++)
        {
          resx+=(sampleBufferIPtr[j]+sampleBufferIPtr[nZeroes-j])*coefZPtr[j];
        }
      for (j=0;j<nPoles;j++)
        {
          resx+=sampleBufferYIPtr[j]*coefPPtr[j];
        }
      sampleBufferYIPtr[nPoles]=resx;
      filteredPtr[i]=abs(resx);
    }
//   arrayDump("SYNCFIL",filteredPtr,dataLen,true,true);
}

void filter::processIQ(FILTERPARAMTYPE *dataPtr, float *dataOutputPtr)
{
  FILTERPARAMTYPE resQ=0;
  const FILTERPARAMTYPE *cf1;
  FILTERPARAMTYPE *fp1;
  uint i,k;
  for (k=0;k<dataLen;k++)
    {
      resQ=0;
      cf1 = coefZPtr;
      fp1 = sampleBufferIPtr;
      memmove(sampleBufferIPtr+1, sampleBufferIPtr,bufSize); // newest at index 0
      sampleBufferIPtr[0]=dataPtr[k];
      for(i=0;i<=nZeroes;i++,fp1++,cf1++)
        {
          resQ+=(*fp1)*(*cf1);
        }
      dataOutputPtr[2*k+1]=sampleBufferIPtr[(nZeroes+1)/2]; // just delay
      dataOutputPtr[2*k]=resQ/gain;
    }
}


void filter::setupMatchedFilter(FILTERPARAMTYPE freq, uint numTaps)
{
  uint i;
  init();
  nZeroes=numTaps-1;
  if(coefZPtr!=NULL && coefZPtrNewed)
      {
          delete [] coefZPtr;
      }
  coefZPtr=new FILTERPARAMTYPE[nZeroes+1];
  coefZPtrNewed=true;
//  qDebug() <<"new coefZptr" << this << coefZPtr;

  for(i=0;i<=nZeroes;i++)
    {
      if(freq==0)
        {
          coefZPtr[i]=1;
        }
      else
        {
          coefZPtr[i]=sin(i*2*M_PI/(SAMPLERATE/freq));
        }
    }
  if(freq==0) gain=numTaps;
  else gain=numTaps/2;
  allocate();
}


