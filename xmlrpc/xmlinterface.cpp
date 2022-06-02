#include "xmlinterface.h"
#include "appglobal.h"

xmlInterface::xmlInterface(QObject *parent) : QObject(parent)
{
  rigInfo.trxState="RX";
  rpcServer = new MaiaXmlRpcServer(7362, this);
  rpcServer->addMethod("main.get_trx_state", this, "getTrxState");
  rpcServer->addMethod("rig.take_control", this, "takeControl");
  rpcServer->addMethod("rig.set_name", this, "setName");
  rpcServer->addMethod("rig.set_modes", this, "setModes");
  rpcServer->addMethod("rig.set_mode", this, "setMode");
  rpcServer->addMethod("rig.set_bandwidths", this, "setBandwidths");
  rpcServer->addMethod("rig.set_bandwidth", this, "setBandwidth");
  rpcServer->addMethod("main.set_wf_sideband", this, "setWfSideband");
  rpcServer->addMethod("rig.set_frequency", this, "setFrequency");
  rpcServer->addMethod("system.multicall", this, "systemMulticall");
  rpcServer->addMethod("main.get_frequency", this, "getFrequency");
  rpcServer->addMethod("rig.get_mode", this, "getMode");
  rpcServer->addMethod("rig.get_bandwidth", this, "getBandwidth");
  rpcServer->addMethod("rig.get_notch", this, "getNotch");
}


void xmlInterface::takeControl() { log("takeControl",""); }

void xmlInterface::setName(QString t)
{
    rigInfo.rigName=t;
    log("setName",t);
}

void xmlInterface::setMode(QString t)
{
    rigInfo.mode=t;
    log("setMode",t);
}

void xmlInterface::setModes(QVariantList t)
{
    log("setModes",t);

}

void xmlInterface::setBandwidths(QVariantList t)
{
    log("setBandwidths",t);
}

void xmlInterface::setBandwidth(QString t)
{
    rigInfo.bandWidth=t;
    log("setBandwidth",t);
}

void xmlInterface::setWfSideband(QString t)
{
    log("setWfSideband",t);
}
void xmlInterface::setFrequency(double d)
{
    rigInfo.frequency=d;
    log("setFrequency",QString::number(d,'g',9));
}

QString xmlInterface::getTrxState()
{
//  log("getTrxSate",rigInfo.trxState);
  return rigInfo.trxState;
}

double  xmlInterface::getFrequency()
{
  log("getFrequency",QString::number(rigInfo.frequency,'g',9));
  return rigInfo.frequency;
}

QString  xmlInterface::getMode()
{
    log("getMode",rigInfo.mode);
    return rigInfo.mode;
}

int xmlInterface::getNotch(int t)
{
    log("getNotch",QString::number(rigInfo.notch));
    rigInfo.notch=t;
    return 1;
}

QString  xmlInterface::getBandwidth()
{
    log("getBandwidth",rigInfo.bandWidth);
    return rigInfo.bandWidth;
}


QVariantList xmlInterface::systemMulticall(QVariantList s)
{
    QVariant ret;
    QVariantMap m;
    QVariantList args;
        QVariantList tmp;
    QVariantList results;

    //    QString response;
    QObject *responseObject;
    const char *responseSlot;
    int i,j;
    log("systemMulticall",s);
    for(i=0;i<s.length();i++)
    {
        tmp.clear();
        m=s.at(i).toMap();

        emit rpcServer->getMethod(m["methodName"].toString(), &responseObject, &responseSlot);
        if(responseObject!=0)
        {

            args=m["params"].toList();
            for(j=0;j<args.count();)
            {
                if(args.at(j).type()==QVariant::Invalid) args.takeAt(j);
                else j++;
            }
            if(!invokeMethodWithVariants(responseObject, responseSlot, args, &ret))
            { /* error invoking... */
                continue;
//                return QVariantList ();
            }
            tmp.append(ret);
            results << (QVariant)tmp;
        }
    }

    return  results;
}

void xmlInterface::activatePTT(bool b)
{
    if(b)
    {
        rigInfo.trxState="TX";
    }
    else
      {
        rigInfo.trxState="RX";
    }
}


void xmlInterface::log(QString cmd,QString t)
{
  Q_UNUSED(cmd);
  Q_UNUSED(t);

  addToLog(cmd+": "+t,LOGXML);
}



void xmlInterface::log(QString cmd,QVariantList t)
{
    int i;
    QString tmp;
    for(i=0;i<t.length();i++) tmp+=t.at(i).toString();
    log(cmd,tmp);
}
