#include "filewatcher.h"
#include "configparams.h"
#include "gallerywidget.h"

fileWatcher::fileWatcher()
{

}

void fileWatcher::init()
{
  if(directories().count()>0)
    {
      removePaths(directories());
    }

  addPathRecursive(txStockImagesPath);
  addPathRecursive(templatesPath);

  connect(this,SIGNAL(directoryChanged(QString)),galleryWidgetPtr,SLOT(slotDirChanged(QString)));
}

void fileWatcher::addPathRecursive(QString path)
{
  addPath(path);
  if(recursiveScanDirs) {
    QDirIterator it(path, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while (it.hasNext()) {
      it.next();
      QString f(it.filePath());
      if(!f.endsWith("cache")) {
        addPath(f);
      }
    }
  }
}
