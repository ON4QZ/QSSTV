#ifndef ARRAYDUMPER_H
#define ARRAYDUMPER_H
#include <QString>
//#include "vector.h"

void arrayDump(QString label, short int *data, unsigned int len, bool toAux, bool singleColumn);
void arrayDump(QString label, quint16 *data, unsigned int len,bool inHex,bool toAux);
void arrayDump(QString label, int *data, unsigned int len, bool toAux);
void arrayDump(QString label, float *data, unsigned int len, bool toAux, bool singleColumn);
void arrayDump(QString label, double *data, unsigned int len, bool toAux, bool singleColumn);
void arrayDump(QString label, quint32 *data, unsigned int len, bool inHex, bool toAux);
//void arrayBinDump(QString label, CVector<_BINARY> data, unsigned int len, bool toAux);
//void arrayComplexDump(QString label,CVectorEx<_COMPLEX> data,unsigned int len,bool toAux);

#endif // ARRAYDUMPER_H
