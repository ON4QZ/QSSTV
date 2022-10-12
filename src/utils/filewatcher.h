#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>

class fileWatcher : public QFileSystemWatcher
{
public:
  fileWatcher();
  void init();

public slots:
  void addPathRecursive(QString path);
};

#endif // FILEWATCHER_H
