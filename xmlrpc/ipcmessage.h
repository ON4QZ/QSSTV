#ifndef IPCMESSAGE_H
#define IPCMESSAGE_H
#include <QString>

#define MSGTXMAXLEN 2048
#define MSGPERM 0666    // msg queue permission
#define MTYPE 88


struct smessageBuf
{
  long mtype;
  char mtext[MSGTXMAXLEN];
};



class ipcMessage
{
public:
  ipcMessage(int messageKey);
  ~ipcMessage();
  void essage();
  bool sendMessage(QString t);
  bool receiveMessage(QString &t);
  bool closeQueue();
private:
  smessageBuf msgBuf;
  int key;
  int messageQId, rc;
  int done;


};

#endif // IPCMESSAGE_H
