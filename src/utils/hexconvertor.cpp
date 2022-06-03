#include "hexconvertor.h"

bool hexFromString(QString s,QByteArray &ba,bool toHex)
{
  int i,j;
  unsigned char res;
  bool ok;
  QString tmp;
  if(toHex)
    {
      if((s.length()&1)!=0) return false;
      ba.resize(s.length()/2);
      for(i=0,j=0;i<s.length();i+=2,j++)
        {
          tmp="0x";
          tmp+=s.mid(i,2);
          res=tmp.toInt(&ok,16);
          if(!ok) return false;
          ba[j]=res;
        }
    }
  else
    {
      ba=s.toLatin1();
    }
  return true;
}

