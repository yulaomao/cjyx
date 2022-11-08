/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMainWindow>
#include <QPixmap>

// CTK includes
#include <ctkMessageBox.h>
#include <ctkUtils.h>

// QtCore includes
#include "qDMMLUtils.h"
#include "qCjyxCoreApplication.h"
#include "qCjyxApplication.h"
#include "qCjyxSceneWriter.h"
#include "vtkCjyxApplicationLogic.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLMessageCollection.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
qCjyxSceneWriter::qCjyxSceneWriter(QObject* parentObject)
  : Superclass(parentObject)
{
}

//----------------------------------------------------------------------------
qCjyxSceneWriter::~qCjyxSceneWriter() = default;

//----------------------------------------------------------------------------
QString qCjyxSceneWriter::description()const
{
  return tr("DMML Scene");
}

//----------------------------------------------------------------------------
qCjyxIO::IOFileType qCjyxSceneWriter::fileType()const
{
  return QString("SceneFile");
}

//----------------------------------------------------------------------------
bool qCjyxSceneWriter::canWriteObject(vtkObject* object)const
{
  return vtkDMMLScene::SafeDownCast(object);
}

//----------------------------------------------------------------------------
QStringList qCjyxSceneWriter::extensions(vtkObject* object)const
{
  Q_UNUSED(object);
  return QStringList()
    << tr("DMML Scene (.dmml)")
    << tr("Medical Reality Bundle (.mrb)")
    << tr("Cjyx Data Bundle (*)");
}

//----------------------------------------------------------------------------
bool qCjyxSceneWriter::write(const qCjyxIO::IOProperties& properties)
{
  this->setWrittenNodes(QStringList());

  Q_ASSERT(!properties["fileName"].toString().isEmpty());
  QFileInfo fileInfo(properties["fileName"].toString());
  QString baseDir = fileInfo.absolutePath();
  if (!QFileInfo(baseDir).isWritable())
    {
    qWarning() << "Failed to save" << fileInfo.absoluteFilePath() << ":"
      << "Path" << baseDir << "is not writable";
    this->userMessages()->AddMessage(vtkCommand::ErrorEvent,
      tr("Failed to save scene as %1 (path %2 is not writeable)").arg(fileInfo.absoluteFilePath()).arg(baseDir).toStdString());
    return 0;
    }
  bool res = false;
  if (fileInfo.suffix() == "dmml")
    {
    res = this->writeToDMML(properties);
    }
  else if (fileInfo.suffix() == "mrb")
    {
    res = this->writeToMRB(properties);
    }
  else
    {
    res = this->writeToDirectory(properties);
    }
  return res;
}

//----------------------------------------------------------------------------
bool qCjyxSceneWriter::writeToDMML(const qCjyxIO::IOProperties& properties)
{
  // set the dmml scene url first
  Q_ASSERT(!properties["fileName"].toString().isEmpty());
  QFileInfo fileInfo(properties["fileName"].toString());
  QString baseDir = fileInfo.absolutePath();
  QString fullPath = fileInfo.absoluteFilePath();

  this->dmmlScene()->SetURL(fullPath.toUtf8());
  this->dmmlScene()->SetRootDirectory(baseDir.toUtf8());

  if (properties.contains("screenShot"))
    {
    // screenshot is provided, save along with the scene dmml file
    QImage screenShot = properties["screenShot"].value<QImage>();
    // convert to vtkImageData
    vtkNew<vtkImageData> imageData;
    qDMMLUtils::qImageToVtkImageData(screenShot, imageData.GetPointer());
    vtkCjyxApplicationLogic* applicationLogic = qCjyxCoreApplication::application()->applicationLogic();
    Q_ASSERT(this->dmmlScene() == applicationLogic->GetDMMLScene());
    applicationLogic->SaveSceneScreenshot(imageData.GetPointer());
    }

  // write out the dmml file
  bool res = this->dmmlScene()->Commit();
  if (!res)
    {
    this->userMessages()->AddMessage(vtkCommand::ErrorEvent,
      tr("Failed to save scene as %1").arg(fileInfo.absoluteFilePath()).toStdString());
    }
  return res;
}

//----------------------------------------------------------------------------
bool qCjyxSceneWriter::writeToMRB(const qCjyxIO::IOProperties& properties)
{
  //
  // make a temp directory to save the scene into - this will
  // be a uniquely named directory that contains a directory
  // named based on the user's selection.
  //

  QFileInfo fileInfo(properties["fileName"].toString());
  QString baseDir = fileInfo.absolutePath();
  QString fullPath = fileInfo.absoluteFilePath();

  // Save URL and root directory so next time when the scene is saved,
  // again, the same folder and filename is used by default.
  this->dmmlScene()->SetURL(fullPath.toUtf8());
  this->dmmlScene()->SetRootDirectory(baseDir.toUtf8());

  if (!QFileInfo(baseDir).isWritable())
    {
    qWarning() << "Failed to save" << fileInfo.absoluteFilePath() << ":"
               << "Path" << baseDir << "is not writable";
    this->userMessages()->AddMessage(vtkCommand::ErrorEvent,
      tr("Failed to save scene as %1 (path %2 is not writeable)").arg(fileInfo.absoluteFilePath()).arg(baseDir).toStdString());
    return false;
    }

  vtkSmartPointer<vtkImageData> thumbnail;
  if (properties.contains("screenShot"))
    {
    QPixmap screenShot = properties["screenShot"].value<QPixmap>();
    // convert to vtkImageData
    thumbnail = vtkSmartPointer<vtkImageData>::New();
    qDMMLUtils::qImageToVtkImageData(screenShot.toImage(), thumbnail);
    }

  bool success = this->dmmlScene()->WriteToMRB(fullPath.toUtf8(), thumbnail, this->userMessages());
  if (!success)
    {
    this->userMessages()->AddMessage(vtkCommand::ErrorEvent,
      tr("Failed to save scene as %1").arg(fileInfo.absoluteFilePath()).toStdString());
    return false;
    }

  // Mark the storable nodes as modified since read, since that flag was reset
  // when the files were written out. If there was newly generated data in the
  // scene that only got saved to the MRB bundle directory, it would be marked
  // as unmodified since read when saving as a DMML file + data. This will not
  // disrupt multiple MRB saves.
  this->dmmlScene()->SetStorableNodesModifiedSinceRead();

  qDebug() << "saved " << fileInfo.absoluteFilePath();
  return true;
}

//---------------------------------------------------------------------------
bool qCjyxSceneWriter::writeToDirectory(const qCjyxIO::IOProperties& properties)
{
  // open a file dialog to let the user choose where to save
  QString saveDirName = properties["fileName"].toString();

  QDir saveDir(saveDirName);
  if (!saveDir.exists())
    {
    QDir().mkpath(saveDir.absolutePath());
    }
  int numFiles = saveDir.count() - 2;
  if (numFiles != 0)
    {
    qCjyxApplication* app = qCjyxApplication::application();
    QWidget* mainWindow = app ? app->mainWindow() : nullptr;
    ctkMessageBox *emptyMessageBox = new ctkMessageBox(mainWindow);
    QString error;
    switch(numFiles)
      {
      case -2:
        VTK_FALLTHROUGH;
      case -1:
        error = tr("fails to be created");
        break;
      case 1:
        error = tr("contains 1 file or directory");
        break;
      default:
        error = tr("contains %1 files or directories").arg(numFiles);
        break;
      }
    QString message = tr("Selected directory\n\"%1\"\n%2.\n"
                         "Please choose an empty directory.")
                         .arg(saveDirName)
                         .arg(error);
    emptyMessageBox->setIcon(QMessageBox::Warning);
    emptyMessageBox->setText(message);
    emptyMessageBox->exec();
    emptyMessageBox->deleteLater();
    return false;
    }

  vtkSmartPointer<vtkImageData> imageData;
  if (properties.contains("screenShot"))
    {
    QPixmap screenShot = properties["screenShot"].value<QPixmap>();
    // convert to vtkImageData
    imageData = vtkSmartPointer<vtkImageData>::New();
    qDMMLUtils::qImageToVtkImageData(screenShot.toImage(), imageData);
    }

  vtkCjyxApplicationLogic* applicationLogic =
    qCjyxCoreApplication::application()->applicationLogic();
  Q_ASSERT(this->dmmlScene() == applicationLogic->GetDMMLScene());
  bool retval = applicationLogic->SaveSceneToCjyxDataBundleDirectory(
    saveDirName.toUtf8(), imageData);
  if (retval)
    {
    qDebug() << "Saved scene to dir" << saveDirName;
    }
  else
    {
    qDebug() << "Error saving scene to file!";
    }
  return retval ? true : false;
}
