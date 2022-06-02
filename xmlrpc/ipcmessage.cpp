#include "ipcmessage.h"

#include <QDebug>
#include "appglobal.h"


#include<string.h>
#include<time.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/wait.h>
#include<sys/errno.h>


//extern int errno;       // error NO.


ipcMessage::ipcMessage(int messageKey)
{
  key=messageKey;
  messageQId = msgget(key, MSGPERM|IPC_CREAT);
  if(messageQId<0) errorOut() << "IPC Error" << strerror(errno);
}


ipcMessage::~ipcMessage()
{
  closeQueue();
}


bool ipcMessage::sendMessage(QString t)
{
  if(messageQId<0) return false;
  int len;
  // message to send
  msgBuf.mtype = MTYPE; // set the type of message
  strncpy(msgBuf.mtext,t.toLatin1().data(),MSGTXMAXLEN-1);
  len=strlen(msgBuf.mtext);

  // send the message to queue
  rc = msgsnd(messageQId, &msgBuf, len+1, IPC_NOWAIT);
  if (rc < 0)
    {
      if(rc<0) errorOut() << "IPC Error" << strerror(errno);
      return false;
    }

  return true;
}

bool ipcMessage::receiveMessage(QString &t)
{
  if(messageQId<0) return false;
  // read the message from queue
  rc = msgrcv(messageQId, &msgBuf, sizeof(msgBuf.mtext), 0, IPC_NOWAIT);
  if (rc < -1)
    {
      if(rc<0) errorOut() << "IPC Error" << strerror(errno);
      return false;
    }
  if(rc>=0)
    {
      t=msgBuf.mtext;
      return true;
    }
  return false;
}

bool ipcMessage::closeQueue()
{
  // remove the queue
  rc=msgctl(messageQId,IPC_RMID,NULL);
  if (rc < 0)
    {
      if(rc<0) errorOut() << "IPC Error" << strerror(errno);
      return false;
    }
  return 0;
}
