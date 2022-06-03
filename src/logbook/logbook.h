#ifndef LOGBOOK_H
#define LOGBOOK_H

#include <QString>

class ipcMessage;

#define NUMLOGPARAMS 21

struct slogParam
{
  QString tag;
  QString val;
};



class logBook
{
public:
  enum eIndex {LPROG,LVER,LDATE,LTIME,LENDTIME,LCALL,LFREQ,LMODE,LTX,LRX,LNAME,LQTH,LSTATE,LPROV,LCNTRY,LLOC,LSO,LSI,LFREE,LNOTES,LPWR};
  logBook();
  void logQSO(QString call, QString mode, QString comment);
private:
  void getFrequency();
  double frequency;
  void setParam(eIndex tag,QString value);
  ipcMessage *ipcQueue;
};

#endif // LOGBOOK_H
