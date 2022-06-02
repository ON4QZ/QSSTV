#ifndef HYBRIDCRYPT_H
#define HYBRIDCRYPT_H
#include <QByteArray>
#include <QString>

class hybridCrypt
{
public:
    hybridCrypt();
    bool enCrypt(QByteArray *ba);
    bool deCrypt(QByteArray *ba);
    QString host()  {return hcFtpRemoteHost;}
    QString user()  {return hcFtpLogin;}
    QString passwd(){return hcFtpPassword;}
    QString dir()   {return hcFtpRemoteDirectory;}
    int port() {return hcFtpPort;}
private:
    QString hcFtpRemoteHost;
    QString hcFtpLogin;
    QString hcFtpPassword;
    QString hcFtpRemoteDirectory;
    int hcFtpPort;
    short int key1,key2,key3,key4;
    void reverseString(QString & s);
    bool getParam(QString result);
    QString charToHex(QChar c);
};

#endif // HYBRIDCRYPT_H
