/**************************************************************************
*   Copyright (C) 2000-2019 by Johan Maes                                 *
*   on4qz@telenet.be                                                      *
*   https://www.qsl.net/o/on4qz                                           *
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

#ifndef BUFFERMANAG_H
#define BUFFERMANAG_H
#include <string.h>

#include <QMutex>
#include <QDebug>


template <class T,unsigned int N> class buffer
{
public:
  buffer()
  {
    memblock=new T[1<<N];
    reset();
  }
  ~buffer()
  {
    delete [] memblock;
  }

  unsigned int count()
  {

    return ((writeIndex-readIndex)& ((1<<N)-1));
  }
  unsigned int spaceLeft()
  {
    return ((1<<N)-count()-1);
  }
  unsigned int getBufferSize() {return (1<<N);}
  bool get(T &v)
  {
    mutex.lock();
    if(writeIndex==readIndex)
      {
        mutex.unlock();
        return false;
      }
    v=memblock[readIndex++];
    readIndex&= ((1<<N)-1);
    mutex.unlock();
    return true;
  }
  bool get(T *cp,unsigned int len)
  {
    int i;
    mutex.lock();
    if(len>count())
      {
        mutex.unlock();
        return false;
      }
    for(i=0;i<len;i++)
      {
        cp[i]=memblock[readIndex++];
        readIndex&= ((1<<N)-1);
      }

    mutex.unlock();
    return true;
  }
  bool put(T v)
  {
    mutex.lock();
    if(((writeIndex+1)&((1<<N)-1))==readIndex)
      {
        mutex.unlock();
        return false;
      }
    memblock[writeIndex++]=v;
    writeIndex&= ((1<<N)-1);
    mutex.unlock();
    return true;
  }

  bool put(T *cp,unsigned int len)
  {
    unsigned int i;
    mutex.lock();

    if(len>spaceLeft())
      {
        mutex.unlock();
        return false;
      }
    for(i=0;i<len;i++)
      {
        memblock[writeIndex++]=cp[i];
        writeIndex&= ((1<<N)-1);
      }
    mutex.unlock();
    return true;
  }

  void putNoCheck(T *cp,unsigned int len) // no boundary check performed
  {
    mutex.lock();
    memcpy(&memblock[writeIndex],cp,len*sizeof(T));
    writeIndex+=len;
    writeIndex&= ((1<<N)-1);
    mutex.unlock();
  }

  void reset()
  {
    mutex.lock();
    readIndex=0;
    writeIndex=0;
    mutex.unlock();
  }

  void fill(T f)
  {
    mutex.lock();
    for(int i=0; i< (1<<N) ;i++) memblock[i]=f;
    mutex.unlock();
  }
  T *readPointer() { return &memblock[readIndex];}
  T *writePointer() { return &memblock[writeIndex];}
  T *basePointer() { return memblock;}

  unsigned int getReadIndex() { return readIndex;}
  unsigned int getWriteIndex() { return writeIndex;}
  bool skip(unsigned int s)
  {
    mutex.lock();
    if(s>count())
      {
        mutex.unlock();
        return false;
      }
    readIndex+=s;
    readIndex&= ((1<<N)-1);
    mutex.unlock();
    return true;
  }
  bool rewind(unsigned int s)
  {
    mutex.lock();
    if(s>spaceLeft())
      {
        mutex.unlock();
        return false;
      }

    readIndex-=s;
    readIndex&= ((1<<N)-1);
    mutex.unlock();
    return true;
  }
  // no check is made if space available
  bool advance(unsigned int s)
  {
    mutex.lock();
    writeIndex+=s;
    writeIndex&= ((1<<N)-1);
    mutex.unlock();
    return true;
  }

  void setReadIndex(unsigned int idx)
  {
    idx&= ((1<<N)-1);
    readIndex=idx;
  }


  T at(unsigned int i)
  {
    i&=((1<<N)-1);
    return memblock[i];
  }
  void copy(T *dst,int len)
  {
    mutex.lock();
    for(int i=0; i< len ;i++) dst[i]=memblock[(readIndex+i)&((1<<N)-1)];
    mutex.unlock();
  }
  void copyNoCheck(T *dst,int len)
  {
    mutex.lock();
    memcpy(dst,&memblock[readIndex],len*sizeof(T));
    readIndex+=len;
    readIndex&= ((1<<N)-1);
    mutex.unlock();
  }

private:
  T *memblock;
  unsigned int readIndex;
  unsigned int writeIndex;
  QMutex mutex;
};


#endif
