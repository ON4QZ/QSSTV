#ifndef SUPPORTFUNCTIONS_H
#define SUPPORTFUNCTIONS_H

#include <QString>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QButtonGroup>
#include <QSpinBox>
#include <QFileDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QPlainTextEdit>
#include <QElapsedTimer>


/** \file */

#define OK true
#define NOK false



/** get int value from a QLinedit */ 
bool getValue(int &val, QLineEdit *input);
/** get double value from a QLinedit */ 
bool getValue(double &val, QLineEdit *input);
/** get int value from a QString */ 
bool getValue(int &val, QString input);
bool getValue(double &val, QString input);
void getValue(bool &val, QCheckBox *input);
void getValue(int &val, QSpinBox *input);
void getValue(uint &val, QSpinBox *input);
void getValue(double &val, QDoubleSpinBox *input);
void getValue(QString &s, QLineEdit *input);
void getValue(QString &s, QPlainTextEdit *input);

void getValue(int &s, QComboBox *input);
void getIndex(int &s, QComboBox *input);
void getValue(QString &s, QComboBox *input);
void getValue(bool &val, QPushButton *input);

void getValue(int &s, QButtonGroup *input);

void getValue(bool &val, QRadioButton *input);
void getValue(int &val, QSlider *input);
void getValue(uint &val, QSlider *input);

void setValue(int val, QLineEdit* output);
void setValue(double val, QLineEdit* output);
void setValue(double val, QLineEdit* output,int prec);
void setValue(bool val, QCheckBox *input);
void setValue(int val, QSpinBox *input);
void setValue(uint val, QSpinBox *input);
void setValue(double val, QDoubleSpinBox *input);
void setValue(QString s, QLineEdit *input);
void setValue(QString s, QPlainTextEdit *input);
void setValue(int s, QComboBox *input);
void setIndex(int s, QComboBox *input);
void setValue(QString s, QComboBox *input);
void setValue(bool val, QPushButton *input);
void setValue(int s, QButtonGroup *input);
void setValue(bool val, QRadioButton *input);
void setValue(int val, QSlider *input);



bool browseGetFile(QLineEdit *le,QString deflt,const QString &filter="*");
bool browseSaveFile(QLineEdit *le,QString deflt,const QString &filter="*");
bool browseDir(QLineEdit *le, QString deflt);
void deleteFiles(QString dirPath,QString extension);

bool trash(QString filename,bool forceDelete);





class timingAnalyser
{
public:
  timingAnalyser();
  ~timingAnalyser();
  void start();
  unsigned long result();
private:
  QElapsedTimer tm;
};



#endif

