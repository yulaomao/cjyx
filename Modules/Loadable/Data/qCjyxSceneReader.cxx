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

// QtCore includes
#include <QDebug>
#include <QMessageBox>
#include <QVersionNumber>

#include "qCjyxSceneReader.h"
#include "qCjyxSceneIOOptionsWidget.h"

// Logic includes
#include <vtkCjyxCamerasModuleLogic.h>

// DMML includes
#include <vtkDMMLMessageCollection.h>
#include <vtkDMMLScene.h>

// CTK includes
#include <ctkUtils.h>

// Cjyx includes
#include "qCjyxCoreApplication.h"
#include "qCjyxCoreIOManager.h"

class qCjyxSceneReaderPrivate
{
public:
  vtkSmartPointer<vtkCjyxCamerasModuleLogic> CamerasLogic;
};

//-----------------------------------------------------------------------------
qCjyxSceneReader::qCjyxSceneReader(vtkCjyxCamerasModuleLogic* camerasLogic,
                               QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qCjyxSceneReaderPrivate)
{
  Q_D(qCjyxSceneReader);
  d->CamerasLogic = camerasLogic;
}

//-----------------------------------------------------------------------------
qCjyxSceneReader::~qCjyxSceneReader() = default;

//-----------------------------------------------------------------------------
QString qCjyxSceneReader::description()const
{
  return "DMML Scene";
}

//-----------------------------------------------------------------------------
qCjyxIO::IOFileType qCjyxSceneReader::fileType()const
{
  return QString("SceneFile");
}

//-----------------------------------------------------------------------------
QStringList qCjyxSceneReader::extensions()const
{
  return QStringList() << "*.dmml";
}

//-----------------------------------------------------------------------------
qCjyxIOOptions* qCjyxSceneReader::options()const
{
  return new qCjyxSceneIOOptionsWidget;
}

//-----------------------------------------------------------------------------
bool qCjyxSceneReader::load(const qCjyxIO::IOProperties& properties)
{
  Q_D(qCjyxSceneReader);
  Q_ASSERT(properties.contains("fileName"));
  QString file = properties["fileName"].toString();
  this->dmmlScene()->SetURL(file.toUtf8());
  bool clear = properties.value("clear", false).toBool();
  bool success = false;
  if (clear)
    {
    qDebug("Clear and import into main DMML scene");
    success = this->dmmlScene()->Connect(this->userMessages());
    if (success)
      {
      // Set default scene file format to .dmml
      qCjyxCoreIOManager* coreIOManager = qCjyxCoreApplication::application()->coreIOManager();
      coreIOManager->setDefaultSceneFileType("DMML Scene (.dmml)");
      }
    }
  else
    {
    if (properties.value("copyCameras", true).toBool() == false)
      {
      qWarning() << Q_FUNC_INFO << ": copyCameras=false property is ignored, cameras are now always replaced in the scene";
      }
    success = this->dmmlScene()->Import(this->userMessages());
    }

  // Display warning message if scene file was created with a different application or with a future application version
  std::string currentApplication;
  int currentMajor = 0;
  int currentMinor = 0;
  int currentPatch = 0;
  int currentRevision = 0;
  std::string loadedApplication;
  int loadedMajor = 0;
  int loadedMinor = 0;
  int loadedPatch = 0;
  int loadedRevision = 0;
  if (vtkDMMLScene::ParseVersion(this->dmmlScene()->GetVersion(), currentApplication,
    currentMajor, currentMinor, currentPatch, currentRevision)
    && vtkDMMLScene::ParseVersion(this->dmmlScene()->GetLastLoadedVersion(), loadedApplication,
      loadedMajor, loadedMinor, loadedPatch, loadedRevision))
    {
    QStringList sceneVersionWarningMessages;
    if (loadedApplication != currentApplication)
      {
      sceneVersionWarningMessages << tr("The scene file was saved with %1 application (this application is %2).")
        .arg(QString::fromUtf8(loadedApplication.c_str()))
        .arg(QString::fromUtf8(currentApplication.c_str()));
      }
    QVersionNumber loadedSceneVersion(loadedMajor, loadedMinor, loadedPatch);
    QVersionNumber currentVersion(currentMajor, currentMinor, currentPatch);
    if (loadedSceneVersion > currentVersion)
      {
      sceneVersionWarningMessages << tr("The scene file was created with a newer version of the application (%1) than the current version (%2).")
        .arg(loadedSceneVersion.toString())
        .arg(currentVersion.toString());
      }
    if (!sceneVersionWarningMessages.isEmpty())
      {
      sceneVersionWarningMessages.push_front(tr("The scene may not load correctly.").arg(file));
      this->userMessages()->AddMessage(vtkCommand::WarningEvent, sceneVersionWarningMessages.join(" ").toStdString());
      }
    }

  // If there were scene loading errors then log the list of extensions that were installed when the scene was saved.
  // It may provide useful hints for why the extension load failed.
  if (!success
    || this->userMessages()->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) > 0
    || this->userMessages()->GetNumberOfMessagesOfType(vtkCommand::WarningEvent) > 0)
    {
    std::string extensions = this->dmmlScene()->GetExtensions() ? this->dmmlScene()->GetExtensions() : "";
    std::string lastLoadedExtensions = this->dmmlScene()->GetLastLoadedExtensions() ? this->dmmlScene()->GetLastLoadedExtensions() : "";
    if (extensions != lastLoadedExtensions)
      {
      QStringList extensionsList = QString::fromStdString(extensions).split(";");
      QStringList lastLoadedExtensionsList = QString::fromStdString(lastLoadedExtensions).split(";");
      QSet<QString> notInstalledExtensions = ctk::qStringListToQSet(lastLoadedExtensionsList).subtract(ctk::qStringListToQSet(extensionsList));
      if (!notInstalledExtensions.isEmpty())
        {
        QString extensionsInformation =
          tr("These extensions were installed when the scene was saved but not installed now: %1."
             " These extensions may be required for successful loading of the scene.")
          .arg(ctk::qSetToQStringList(notInstalledExtensions).join(", "));
        this->userMessages()->AddMessage(vtkCommand::MessageEvent, extensionsInformation.toStdString());
        }
      }
    }

  return success;
}
