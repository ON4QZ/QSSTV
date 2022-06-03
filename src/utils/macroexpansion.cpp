#include "macroexpansion.h"


macroExpansion::macroExpansion()
{
  convertList.clear();
}



QString macroExpansion::convert(QString txt)
{
  int i,j;
  bool special=false;
  QChar c;
  QString convertedText;
  {
    for (i=0;i<txt.length();i++)
    {
      if (special)
        {
          special=false;
          c=txt.at(i);
          if(c=='%')
          {
            convertedText.append('%');
            continue;
          }
          for (j=0;j<convertList.count();j++)
            {
              if(c==convertList.at(j).tag)
              {
                convertedText.append(convertList.at(j).replacement);
              }
            }
        }
      else
        {
          if(txt.at(i)!='%') convertedText.append(txt.at(i));
          else special=true;
        }
    }

  }
  return convertedText;
}


void macroExpansion::addConversion(QChar tag,QString value)
{
  sconvert cnv;
  cnv.tag=tag;
  cnv.replacement=value;
  convertList.append(cnv);
}
