#ifndef MACROEXPANSION_H
#define MACROEXPANSION_H
#include <QString>
#include <QList>


struct sconvert
{
  QChar tag;
  QString replacement;
};

class macroExpansion
{
public:
  macroExpansion();
  QString convert(QString txt);
  void addConversion(QChar tag,QString value);
  void clear() {convertList.clear();}
private:
  QList<sconvert> convertList;
};
#endif // MACROEXPANSION_H
