#include "logbook.h"
#include "appglobal.h"
#include "xmlrpc/xmlinterface.h"
#include "xmlrpc/ipcmessage.h"
#include "configparams.h"
#include "rigcontrol.h"
#include <QDateTime>



slogParam logParamArray[NUMLOGPARAMS]=
{
    {"program","QSSTV 9"},
    {"version", "1"},
    {"date","" },
    {"time",""  },
    {"endTime",""  },
    {"call","" , },
    {"mhz",""  },
    {"mode",""  },
    {"tx",""  },
    {"rx",""  },
    {"name","" },
    {"qth","" },
    {"state","" },
    {"province","" },
    {"country","" },
    {"locator","" },
    {"serialout","" },
    {"serialin","" },
    {"free1","" },
    {"notes","" },
    {"power","" }
};

// to be independent from localization
QString monthArray[12]=
{
  "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};


logBook::logBook()
{
  ipcQueue=new ipcMessage(1238);

}


void logBook::logQSO(QString call,QString mode,QString comment)
{
  int i;
  QDateTime dt(QDateTime::currentDateTimeUtc());
  QString tmp;
  getFrequency();
  if(frequency!=-1) setParam(LFREQ,QString::number(frequency/1000000.,'g',9));
  setParam(LCALL,call);
  setParam(LNOTES,comment);
  tmp=QString::number(dt.date().day())+" "+monthArray[dt.date().month()-1]+" "+QString::number(dt.date().year());
  setParam(LDATE,tmp);
  tmp=QString::number(dt.time().hour()*100+dt.time().minute()).rightJustified(4,'0');
  setParam(LTIME,tmp);
  setParam(LENDTIME,tmp);
  setParam(LMODE,mode);
  setParam(LNOTES,comment);
  tmp.clear();
  for(i=0;i<NUMLOGPARAMS;i++)
    {
      tmp+=logParamArray[i].tag+":"+logParamArray[i].val+QChar(0x01);
    }
  ipcQueue->sendMessage(tmp);
}





void logBook::getFrequency()
{
  frequency=-1;

  if(rigControllerPtr->params()->enableXMLRPC) // we get the frequency from flrig or alike
    {
      frequency=xmlIntfPtr->getFrequency();
    }
  else if(rigControllerPtr->params()->enableCAT) // we get the frequency from hamlib
    {
      if(!rigControllerPtr->getFrequency(frequency))
        {
          frequency=-1;
        }
    }


}

void logBook::setParam(eIndex tag,QString value)
{
   logParamArray[tag].val=value;
}

