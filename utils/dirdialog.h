#ifndef DIRDIALOG_H
#define DIRDIALOG_H
#include <QWidget>
#include <QString>

class dirDialog
{
public:
    dirDialog(QWidget *parent,QString title="");
    ~dirDialog();
    QString openFileName(const QString &startWith, const QString &filter="*");
    QString openDirName(const QString &path);
    QString saveFileName(const QString &path, const QString &filter,QString extension);
    QStringList openFileNames(const QString &path, const QString &filter);
private:
    QWidget * parentPtr;
    QString  dialogTitle;
};

#endif // DIRDIALOG_H
