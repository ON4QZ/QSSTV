#include "arraydumper.h"
#include "appglobal.h"





void arrayDump(QString label,short int *data, unsigned int len,bool toAux,bool singleColumn)
{
  unsigned int i,j;
  QString dumpStr,tmp;
  if(!singleColumn)
    {
      for( i=0;i<len;i+=16)
        {
          dumpStr=label+" ";
          for(j=0;(j<16)&&((i+j)<len);j++)
            {
              tmp=QString::number(data[i+j]);
//              while(tmp.length()<4) tmp.prepend("0");
              dumpStr+=tmp+" ";
            }
          dumpStr.chop(1);
          if(toAux)
            {
              logFilePtr->addToAux(dumpStr);
            }
          else
            {
              addToLog(dumpStr,LOGALL);
            }
        }
    }
  else
    {
      dumpStr=label+" ";
      if(toAux) logFilePtr->addToAux(dumpStr);
      else   addToLog(dumpStr,LOGALL);
      for(i=0;i<len;i++)
        {
          tmp=QString::number(data[i]);
          if(toAux) logFilePtr->addToAux(tmp);
          else   addToLog(tmp,LOGALL);
        }
    }
}

void arrayDump(QString label,int *data, unsigned int len,bool toAux)
{
  unsigned int i,j;
  QString dumpStr,tmp;
  for( i=0;i<len;i+=16)
    {
      dumpStr=label+" ";
      for(j=0;(j<16)&&((i+j)<len);j++)
        {
          tmp=QString::number(data[i+j],16);
          while(tmp.length()<4) tmp.prepend("0");
          dumpStr+=tmp+" ";
        }
      dumpStr.chop(1);
      if(toAux)
        {
          logFilePtr->addToAux(dumpStr);
        }
      else
        {
          addToLog(dumpStr,LOGALL);
        }
    }
}

void arrayDump(QString label,quint16 *data, unsigned int len,bool inHex,bool toAux)
{
  unsigned int i,j;
  QString dumpStr,tmp;
  for( i=0;i<len;i+=16)
    {
      dumpStr=label+" ";
      for(j=0;(j<16)&&((i+j)<len);j++)
        {
          if(inHex)
            {
              tmp=QString::number(data[i+j],16);
              while(tmp.length()<4) tmp.prepend("0");
            }
          else
            {
              tmp=QString::number(data[i+j]).rightJustified(5);
            }
          dumpStr+=tmp+" ";
        }
      dumpStr.chop(1);
      if(toAux)
        {
          logFilePtr->addToAux(dumpStr);
        }
      else
        {
          addToLog(dumpStr,LOGALL);
        }
    }
}

void arrayDump(QString label, quint32* data, unsigned int len,bool inHex, bool toAux)
{
  unsigned int i,j;
  QString dumpStr,tmp;
  for( i=0;i<len;i+=16)
    {
      dumpStr=label+" ";
      for(j=0;(j<16)&&((i+j)<len);j++)
        {
          if(inHex)
            {
              tmp=QString::number(data[i+j],16);
              while(tmp.length()<8) tmp.prepend("0");
            }
          else
            {
              tmp=QString::number(data[i+j]).rightJustified(10);
            }

          dumpStr+=tmp+" ";
        }
      dumpStr.chop(1);
      if(toAux)
        {
          logFilePtr->addToAux(dumpStr);
        }
      else
        {
          addToLog(dumpStr,LOGALL);
        }
    }
}

void arrayDump(QString label, float *data, unsigned int len, bool toAux, bool singleColumn)
{
  unsigned int i,j;
  QString dumpStr,tmp;
  if(!singleColumn)
    {
      for( i=0;i<len;i+=16)
        {
          dumpStr=label+" ";
          for(j=0;(j<16)&&((i+j)<len);j++)
            {
              tmp=QString::number(data[i+j],'E',9);
              while(tmp.length()<8) tmp.prepend(" ");
              dumpStr+=tmp+" ";
            }
          dumpStr.chop(1);
          if(toAux)
            {
              logFilePtr->addToAux(dumpStr);
            }
          else
            {
              addToLog(dumpStr,LOGALL);
            }
        }
    }
  else
    {
      dumpStr=label+" ";
      if(toAux) logFilePtr->addToAux(dumpStr);
      else   addToLog(dumpStr,LOGALL);
      for(i=0;i<len;i++)
        {
          tmp=QString::number(data[i],'E',9);
          if(toAux) logFilePtr->addToAux(tmp);
          else   addToLog(tmp,LOGALL);
        }
    }
}

void arrayDump(QString label, double *data, unsigned int len, bool toAux, bool singleColumn)
{
  unsigned int i,j;
  QString dumpStr,tmp;
  if(!singleColumn)
    {
      for( i=0;i<len;i+=16)
        {
          dumpStr=label+" ";
          for(j=0;(j<16)&&((i+j)<len);j++)
            {
              tmp=QString::number(data[i+j],'E',9);
              while(tmp.length()<8) tmp.prepend(" ");
              dumpStr+=tmp+" ";
            }
          dumpStr.chop(1);
          if(toAux)
            {
              logFilePtr->addToAux(dumpStr);
            }
          else
            {
              addToLog(dumpStr,LOGALL);
            }
        }
    }
  else
    {
      dumpStr=label+" ";
      if(toAux) logFilePtr->addToAux(dumpStr);
      else   addToLog(dumpStr,LOGALL);
      for(i=0;i<len;i++)
        {
           tmp=QString::number(data[i],'E',9);
          if(toAux) logFilePtr->addToAux(tmp);
          else   addToLog(tmp,LOGALL);
        }
    }
}



