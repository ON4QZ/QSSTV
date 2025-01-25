#include "dirdialog.h"
#include <QDir>
#include <QFileDialog>

static QString lastPath;

dirDialog::dirDialog(QWidget * parent,QString title)
{
  parentPtr=parent;
  dialogTitle=title;
}

dirDialog::~dirDialog()
{
}


/*!
    \fn dirDialog::openFileName(const QString &path, const QString &filter)
    \brief selection of a file

    \param path  directory to open (preselected) if empty, the last accessed directory will be used
    \param filter  types to select from (e.g. mydirs*)
    \return if canceled or no selection then returns an empty string else return string containing absolute filename
*/


QString dirDialog::openFileName(const QString &path, const QString &filter)
{
  QString fn;
  if (path.isEmpty() && lastPath.isEmpty())
    {
      lastPath=QDir::homePath();
    }
  else if (!path.isEmpty())
    {
      lastPath=path;
    }

  fn=QFileDialog::getOpenFileName(parentPtr,dialogTitle,lastPath,filter);
  if(!fn.isEmpty())
    {
      QFileInfo  fi(fn);
      lastPath=fi.absolutePath();
    }
  return fn;
}

/*!
    \fn dirDialog::openDirName(const QString &path)
    \brief selection of a directory

    \param path directory to open (preselected)
    \return if canceled or no selection then return an empty string else return string containing absolute dirname
*/

QStringList dirDialog::openFileNames(const QString &path, const QString &filter)
{
  QStringList fl;
  if (path.isEmpty() && lastPath.isEmpty())
    {
      lastPath=QDir::homePath();
    }
  else if (!path.isEmpty())
    {
      lastPath=path;
    }

  fl=QFileDialog::getOpenFileNames(parentPtr,dialogTitle,lastPath,filter);
  if(fl.count()>0)
    {
      QFileInfo  fi(fl.at(0));
      lastPath=fi.absolutePath();
    }
  return fl;
}



/*!
    \fn dirDialog::openDirName(const QString &path)
    \brief selection of a directory

    \param path directory to open (preselected)
    \return if canceled or no selection then return an empty string else return string containing absolute dirname
*/

QString dirDialog::openDirName(const QString &path)
{
  QString fn;
  if ((path.isEmpty()) && lastPath.isEmpty())
    {
      lastPath=QDir::homePath();
    }
  else if (!path.isEmpty())
    {
      lastPath=path;
    }
  fn=QFileDialog::getExistingDirectory(parentPtr,dialogTitle,lastPath);
  if(!fn.isEmpty())
    {
      lastPath=fn;
    }
  return fn;
}

/*!
    \fn dirDialog::saveFileName(const QString &path, const QString &filter,QString extension)
    \brief Save a file to disk

    Saves a file to disk. A dialogbox is opened with \a startWith directory (or /dir/subdir/..../filename) preselected
    \param path directory to open (can include filename to preselect)
    \param filter    file types to select from (e.g. *.txt *.doc)
    \param extension if extension is not empty or NULL, then this string will be appended to the filename. A dot will automatically be inserted (i.e specify "txt" not ".txt").
    \return if canceled or no selection then return an empty string else return string containing absolute filename.
*/

QString dirDialog::saveFileName(const QString &path, const QString &filter, QString extension)
{
  QString fn;
  if ((path.isEmpty()) && lastPath.isEmpty())
    {
      lastPath=QDir::currentPath();
    }
  else if (!path.isEmpty())
    {
      lastPath=path;
    }
  QString exten(extension);
  fn=QFileDialog::getSaveFileName(parentPtr,dialogTitle,lastPath,filter);
  if(fn.isEmpty()) return fn;
  QFileInfo  fi(fn);
  if(!exten.isEmpty())
    {
      if(fi.suffix()=="")
        {
          fi.setFile(fi.absoluteFilePath()+"."+exten);
        }
    }
  lastPath=fi.absolutePath();
  return fi.absoluteFilePath();
}
