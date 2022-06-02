#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>

class fileWatcher : public QFileSystemWatcher
{
public:
  fileWatcher();
  void init();

signals:

public slots:
};

#endif // FILEWATCHER_H
