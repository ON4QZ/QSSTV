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
  addPath(txStockImagesPath);
  addPath(templatesPath);
  connect(this,SIGNAL(directoryChanged(QString)),galleryWidgetPtr,SLOT(slotDirChanged(QString)));
}
