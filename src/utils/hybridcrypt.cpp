#include "hybridcrypt.h"
#include <QDebug>
#include "configparams.h"
#include "QResource"



hybridCrypt::hybridCrypt()
{
  key1=4;
  key2=5;
  key3=7;
  key4=1;
}



bool hybridCrypt::enCrypt(QByteArray *ba)
{
  int i;
  QString string,hstr;
  hcFtpPort=21;
  if(!hybridFtpRemoteHost.isEmpty())
    {
      hcFtpRemoteHost=hybridFtpRemoteHost;
      hcFtpLogin=hybridFtpLogin;
      hcFtpPassword=hybridFtpPassword;
      hcFtpRemoteDirectory=hybridFtpRemoteDirectory; //always relatif to /HybridFiles
    }

  hstr=QChar(63)+hcFtpRemoteHost+QChar(34)+hcFtpLogin+QChar(60)+hcFtpPassword+QChar(62)+hcFtpRemoteDirectory+QChar(58);

  hcFtpRemoteHost.clear();
  hcFtpLogin.clear();
  hcFtpPassword.clear();
  hcFtpRemoteDirectory.clear();
  reverseString(hstr);

  for(i=0;i<hstr.length();i++)
    {
      string.append(charToHex(hstr.at(i)));
    }
  int sc=0;
  short int bufI,  num1, num2, num3, num4, res1, res2, res3, res4;
  unsigned char r1,r2;
  ba->clear();
  while(string.length()%4!=0)
    {
      string.append(QChar(0));
    }
  do
    {
      num1=string.at(sc++).toLatin1();
      num2=string.at(sc++).toLatin1();
      num3=string.at(sc++).toLatin1();
      num4=string.at(sc++).toLatin1();
      res1=num1*key1;
      bufI=num2*key3;
      res1=res1+bufI;
      res2= num1 * key2;
      bufI= num2 * key4;
      res2= res2 + bufI;
      res3= num3 * key1;
      bufI= num4 * key3;
      res3= res3 + bufI;
      res4= num3 * key2;
      bufI= num4 * key4;
      res4= res4 + bufI;
      for(bufI=0;bufI<4;bufI++)
        {
          switch(bufI)
            {
            case 0: r1=res1>>8; r2=res1&0xff; break;
            case 1: r1=res2>>8; r2=res2&0xff; break;
            case 2: r1=res3>>8; r2=res3&0xff; break;
            case 3: r1=res4>>8; r2=res4&0xff; break;
            }
          if((r1==0) &&(r2==0))
            {
              r1=0xFF;
              r2=0xFF;
            }
          if(r1==0xFF) r1=0xFE;
          if(r2==0)
            {
              r2=r1;
              r1=0xFD;
            }
          ba->append(r1);
          ba->append(r2);
        }
    }
  while(sc<string.length());
  ba->append("\r\n");
  return true;
}


bool hybridCrypt::deCrypt(QByteArray *ba)
{
  QString result;
  int baSize;
  bool ok;
  int charCount=0;
  QString tempStr="0x00";
  short int bufI, bufI2, divzr, num1, num2, num3, num4, res1, res2, res3, res4;
  unsigned char r1,r2;
  result="";
  res1=res2=res3=res4=0;
  divzr=key1*key4;
  bufI2=key3*key2;
  divzr-=bufI2;
  if(divzr==0) return false;
  baSize=ba->size();
  if(baSize!=3)
    {
      baSize=ba->size()-2; //drop /r/n
      if(baSize%2!=0) return false;
    }

  if(baSize<20) return false;
  do
    {
      for(bufI=0;bufI<4;bufI++,charCount+=2)
        {
          r1=ba->at(charCount);
          r2=ba->at(charCount+1);
          if(r1==0xFF)
            {
              r1=r2=0;
            }
          if(r1==0xFE)
            {
              r1=0;
            }
          if(r1==0xFD)
            {
              r1=r2;
              r2=0;
            }
          switch(bufI)
            {
            case 0: res1=r1*256+r2; break;
            case 1: res2=r1*256+r2; break;
            case 2: res3=r1*256+r2; break;
            case 3: res4=r1*256+r2; break;
            }
        }

      bufI= res1 * key4;
      bufI2= res2 * key3;
      num1= bufI - bufI2;
      num1= num1 / divzr;
      bufI= res2 * key1;
      bufI2= res1 * key2;
      num2= bufI - bufI2;
      num2 = num2 / divzr;
      bufI= res3 * key4;
      bufI2= res4 * key3;
      num3= bufI - bufI2;
      num3= num3 / divzr;
      bufI= res4 * key1;
      bufI2= res3 * key2;
      num4= bufI - bufI2;
      num4= num4 / divzr;
      tempStr[2]=QChar(num1);
      tempStr[3]=QChar(num2);
      result.append(QChar(tempStr.toInt(&ok,16)));
      tempStr[2]=QChar(num3);
      tempStr[3]=QChar(num4);
      result.append(QChar(tempStr.toInt(&ok,16)));
    }
  while(charCount<baSize);
  reverseString(result);
  getParam(result);
  return true;
}


void hybridCrypt::reverseString(QString & s)
{
  int i,j;
  QString t;
  for (i=0,j=s.length()-1;i<s.length();i++,j--)
    {
      t[j]=s[i];
    }
  s=t;
}

bool hybridCrypt::getParam(QString resultStr)
{
  int a,b,c,d,e;
  QString tempDir;
  a=resultStr.indexOf(QChar(63));
  b=resultStr.indexOf(QChar(34),a+1);
  c=resultStr.indexOf(QChar(60),b+1);
  d=resultStr.indexOf(QChar(62),c+1);
  e=resultStr.indexOf(QChar(58),d+1);
  hcFtpRemoteHost=resultStr.mid(a+1,b-a-1);
  hcFtpLogin=resultStr.mid(b+1,c-b-1);
  hcFtpPassword=resultStr.mid(c+1,d-c-1);
  hcFtpPort=21; // FIXED Port Settings
  tempDir=resultStr.mid(d+1,e-d-1);
  if( !tempDir.isEmpty())
    {
      hcFtpRemoteDirectory=resultStr.mid(d+1,e-d-1);
    }
  else
    {
      hcFtpRemoteDirectory="";
    }
  addToLog(QString("host: %1, login: %2, pwd: %3, dir: %4").arg(hcFtpRemoteHost).arg(hcFtpLogin).arg(hcFtpPassword).arg(hcFtpRemoteDirectory),LOGALL);
  return true;
}

QString hybridCrypt::charToHex(QChar c)
{
  QString hb;
  hb=QString::number(c.toLatin1(),16);
  if(hb.length()<2) hb.prepend("0");
  return hb;
}





