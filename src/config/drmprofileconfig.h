#ifndef DRMPROFILECONFIG_H
#define DRMPROFILECONFIG_H
#include "drmparams.h"


#include "baseconfig.h"

#define NUMBEROFPROFILES 3



struct sprofile
{
    QString name;
    drmTxParams params;

};

extern sprofile drmPFArray[NUMBEROFPROFILES];

bool getDRMParams(int idx, drmTxParams &d);
bool getName(int idx,QString &n);




namespace Ui {
class drmProfileConfig;
}

class drmProfileConfig : public baseConfig
{
    Q_OBJECT

public:
    explicit drmProfileConfig(QWidget *parent = 0);
    ~drmProfileConfig();
    void readSettings();
    void writeSettings();
    void getParams();
    void setParams();
    bool getDRMParams(int idx,drmTxParams &d);
    bool getName(int idx, QString &n);

private:
    Ui::drmProfileConfig *ui;
    bool diff(sprofile a,sprofile b);

};

extern drmProfileConfig *drmProfileConfigPtr;



#endif // DRMPROFILECONFIG
