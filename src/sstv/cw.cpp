/***************************************************************************
                          cw.cpp  -  QSSTV
                             -------------------
    begin                : Tue Apr 17 22:27:58 CEST 2001
    copyright            : (C) 2001 by Johan Maes ON1MH
    email                : on1mh@pandora.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "cw.h"
#include <ctype.h>
#include "appglobal.h"
#include "configparams.h"




enum eCWState {CWNEW,CWNEXTDOT,CWDOTSPACING,CWNEXTCHAR,CWCHARSPACING,CWWORDSPACING,CWEND,CWFINISHED};
enum  eCWResult {CWIDLE,CWfalse,CWtrue};

static int dotIndex;
static const char *dotPtr;

static int charIndex;
static float dotSpacing;

static  eCWState cwState;
static  eCWResult result;
//  const char *charLookupCW(char a);
//  bool sendChar(float &duration);
//  const char *s;

static struct {
  char key;
  const char	*cw;
} charTable[] = 
{
  {'A',	".-"	}, {'B',"-..."  },{'C',	"-.-." },
  {'D',	"-.."	}, {'E',"."	},{'F',"..-."  },
  {'G',	"--."	}, {'H',"...."	},{'I',".."    },
  {'J',	".---"	}, {'K',"-.-"	},{'L',".-.."  },
  {'M',	"--"	}, {'N',"-."	},{'O',"---"   },
  {'P',	".--."	}, {'Q',"--.-"	},{'R',".-."   },
  {'S',	"..."	}, {'T',"-"	},{'U',"..-"   },
  {'V',	"...-"	}, {'W',".--"	},{'X',"-..-"  },
  {'Y',	"-.--"	}, {'Z',"--.."	},
  {'0',	"-----"	}, {'1',".----"	},{'2',	"..---"},
  {'3',	"...--"	}, {'4',"....-"	},{'5',"....." },
  {'6',	"-...."	}, {'7',"--..."	},{'8',"---.." },
  {'9',	"----."	},
  {'"',	".-..-."}, {'\'', ".----." },{'$',"...-..-"},
  {'(',	"-.--." }, { ')', "-.--.-" },{'+',".-.-."},
  {',',	"--..--"}, {'-',  "-....-" },{'.',".-.-.-"},
  {'/',	"-..-." }, { ':', "---..." },{';',"-.-.-."},
  {'=',	"-...-" }, { '?', "..--.." },{'_',"..--.-"},
  {0,	""	}
};

static QString cwString;
void initCW(QString cwTxt)
{ 
  cwState=CWNEW;
  dotSpacing=1.2/(float)cwWPM;
  cwString=cwTxt;
}

const char *charLookupCW(const char a)
{
  char b;
  int i=0;;
  b=toupper(a);
  dotIndex=0;
  while (charTable[i].key!=0)
    {
      if(charTable[i].key==b)
        {

          return (charTable[i].cw);
        }
      i++;
    }
  return NULL;
}

bool nextSymbolCW(float &duration)
{
  if (dotPtr[dotIndex]==0)
    {
      return false;
    }
  else if(dotPtr[dotIndex]=='.')
    {
      duration=dotSpacing;
    }
  else
    {
      duration=3*dotSpacing;
    }
  dotIndex++;
  return true;
}

float getCWDuration()
{
  float tim=0;
  float tone,duration;
  tone=0;
  while(sendTextCW(tone,duration))
    {
      tim+=duration;
    }
  return tim;
}

bool sendTextCW(float &tone,float &duration)
{
  result=CWIDLE;
  do
    {
      switch (cwState)
        {
        case CWNEW:
          {
            charIndex=0;
            if (cwString[0]==0)
              {
                result=CWfalse;
              }
            cwState=CWNEXTCHAR;
          }
        break;
        case CWNEXTCHAR:
          {
            if(cwString[charIndex]==' ')
              {
                charIndex++;
                cwState=CWWORDSPACING;
              }
            else
              {
                dotPtr=charLookupCW(cwString[charIndex++].toLatin1());
                if (dotPtr==NULL)
                  {
                    cwState=CWEND;
                  }
                else
                  {
                    dotIndex=0;
                    cwState=CWNEXTDOT;
                  }
              }
          }
        break;
        case CWNEXTDOT:
          {
            if(nextSymbolCW(duration))
              {
                tone=(float)cwTone;
                cwState=CWDOTSPACING;
                result=CWtrue;
              }
            else
              {
                cwState=CWCHARSPACING;
              }
          }
        break;
        case CWDOTSPACING:
          {
            tone=0;
            duration=dotSpacing;
            cwState=CWNEXTDOT;
            result=CWtrue;
          }
        break;
        case CWCHARSPACING:
          {
            tone=0;
            duration=2*dotSpacing;  // we already had a dotspace
            cwState=CWNEXTCHAR;
            result=CWtrue;
          }
        break;
        case CWWORDSPACING:
          {
            tone=0;
            duration=4*dotSpacing; // we already had a charspace
            cwState=CWNEXTCHAR;
            result=CWtrue;
          }
        break;

        case CWEND:
          {
            tone=0;
            duration=7*dotSpacing;
            cwState=CWFINISHED;
            result=CWtrue;
          }
        break;
        case CWFINISHED:
          {
            result=CWfalse;
          }
        break;

        }
    }
  while(result==CWIDLE);
  return (result==CWtrue);
}












