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

/// Qt includes
#include <QFileInfo>

// CTK includes
#include <ctkFlowLayout.h>
#include <ctkUtils.h>

// VTK includes
#include <vtkNew.h>

/// Volumes includes
#include "qCjyxIOOptions_p.h"
#include "qCjyxVolumesIOOptionsWidget.h"
#include "ui_qCjyxVolumesIOOptionsWidget.h"

/// Cjyx includes
#include "qCjyxCoreApplication.h"
#include "vtkDMMLColorLogic.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLVolumeArchetypeStorageNode.h"
#include "vtkCjyxApplicationLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Volumes
class qCjyxVolumesIOOptionsWidgetPrivate
  : public qCjyxIOOptionsPrivate
  , public Ui_qCjyxVolumesIOOptionsWidget
{
public:
};

//-----------------------------------------------------------------------------
qCjyxVolumesIOOptionsWidget::qCjyxVolumesIOOptionsWidget(QWidget* parentWidget)
  : qCjyxIOOptionsWidget(new qCjyxVolumesIOOptionsWidgetPrivate, parentWidget)
{
  Q_D(qCjyxVolumesIOOptionsWidget);
  d->setupUi(this);

  ctkFlowLayout::replaceLayout(this);

  connect(d->NameLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(updateProperties()));
  connect(d->LabelMapCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
  connect(d->CenteredCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
  connect(d->SingleFileCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
  connect(d->OrientationCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
  connect(d->ShowCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
  connect(d->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
          this, SLOT(updateProperties()));

  // need to update the color selector when the label map check box is toggled
  connect(d->LabelMapCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateColorSelector()));

  // Single file by default
  d->SingleFileCheckBox->setChecked(true);
}

//-----------------------------------------------------------------------------
qCjyxVolumesIOOptionsWidget::~qCjyxVolumesIOOptionsWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxVolumesIOOptionsWidget::updateProperties()
{
  Q_D(qCjyxVolumesIOOptionsWidget);
  if (!d->NameLineEdit->text().isEmpty())
    {
    QStringList names = d->NameLineEdit->text().split(';');
    for (int i = 0; i < names.count(); ++i)
      {
      names[i] = names[i].trimmed();
      }
    d->Properties["name"] = names;
    }
  else
    {
    d->Properties.remove("name");
    }
  d->Properties["labelmap"] = d->LabelMapCheckBox->isChecked();
  d->Properties["center"] = d->CenteredCheckBox->isChecked();
  d->Properties["singleFile"] = d->SingleFileCheckBox->isChecked();
  d->Properties["discardOrientation"] = d->OrientationCheckBox->isChecked();
  d->Properties["show"] = d->ShowCheckBox->isChecked();
  d->Properties["colorNodeID"] = d->ColorTableComboBox->currentNodeID();
}

//-----------------------------------------------------------------------------
void qCjyxVolumesIOOptionsWidget::setFileName(const QString& fileName)
{
  this->setFileNames(QStringList(fileName));
}

//-----------------------------------------------------------------------------
void qCjyxVolumesIOOptionsWidget::setFileNames(const QStringList& fileNames)
{
  Q_D(qCjyxVolumesIOOptionsWidget);
  QStringList names;
  bool onlyNumberInName = false;
  bool onlyNumberInExtension = false;
  bool hasLabelMapName = false;

  vtkSmartPointer<vtkDMMLVolumeArchetypeStorageNode> snode;
  if (this->dmmlScene())
    {
    // storage node must be added to the scene to have access to supported file extensions
    // (known file extensions are used to determine node name accurately when there are
    // multiple '.' characters in the filename.
    snode = vtkDMMLVolumeArchetypeStorageNode::SafeDownCast(
      this->dmmlScene()->AddNewNodeByClass("vtkDMMLVolumeArchetypeStorageNode"));
    }
  if (snode.GetPointer() == nullptr)
    {
    qWarning("qCjyxVolumesIOOptionsWidget::setFileNames: dmmlScene is invalid, node name may not be determined accurately");
    snode = vtkSmartPointer<vtkDMMLVolumeArchetypeStorageNode>::New();
    }
 foreach(const QString& fileName, fileNames)
    {
    QFileInfo fileInfo(fileName);
    QString fileBaseName = fileInfo.baseName();
    if (fileInfo.isFile())
      {
      std::string fileNameStd = fileInfo.fileName().toStdString();
      std::string filenameWithoutExtension = snode->GetFileNameWithoutExtension(fileNameStd.c_str());
      fileBaseName = QString(filenameWithoutExtension.c_str());
      names << fileBaseName;
      // Single file
      // If the name (or the extension) is just a number, then it must be a 2D
      // slice from a 3D volume, so uncheck Single File.
      onlyNumberInName = QRegExp("[0-9\\.\\-\\_\\@\\(\\)\\~]+").exactMatch(fileBaseName);
      fileInfo.suffix().toInt(&onlyNumberInExtension);
      }
    // Because '_' is considered as a word character (\w), \b
    // doesn't consider '_' as a word boundary.
    QRegExp labelMapName("(\\b|_)([Ll]abel(s)?)(\\b|_)");
    QRegExp segName("(\\b|_)([Ss]eg)(\\b|_)");
    if (fileBaseName.contains(labelMapName) ||
      fileBaseName.contains(segName))
      {
      hasLabelMapName = true;
      }
    }
  if (snode->GetScene())
    {
    snode->GetScene()->RemoveNode(snode);
    }
  d->NameLineEdit->setText( names.join("; ") );
  d->SingleFileCheckBox->setChecked(!onlyNumberInName && !onlyNumberInExtension);
  d->LabelMapCheckBox->setChecked(hasLabelMapName);
  this->qCjyxIOOptionsWidget::setFileNames(fileNames);

  // update the color selector since the label map check box may not
  // have changed on setting this new name
  this->updateColorSelector();
}

//-----------------------------------------------------------------------------
void qCjyxVolumesIOOptionsWidget::updateColorSelector()
{
  Q_D(qCjyxVolumesIOOptionsWidget);

  if (qCjyxCoreApplication::application() != nullptr)
    {
    // access the color logic which has information about default color nodes
    vtkCjyxApplicationLogic* appLogic = qCjyxCoreApplication::application()->applicationLogic();
    if (appLogic && appLogic->GetColorLogic())
      {
      if (d->LabelMapCheckBox->isChecked())
        {
        d->ColorTableComboBox->setCurrentNodeID(appLogic->GetColorLogic()->GetDefaultLabelMapColorNodeID());
        }
      else
        {
        d->ColorTableComboBox->setCurrentNodeID(appLogic->GetColorLogic()->GetDefaultVolumeColorNodeID());
        }
      }
    }
}

//------------------------------------------------------------------------------
void qCjyxVolumesIOOptionsWidget::updateGUI(const qCjyxIO::IOProperties& ioProperties)
{
  Q_D(qCjyxVolumesIOOptionsWidget);
  qCjyxIOOptionsWidget::updateGUI(ioProperties);
  if (ioProperties.contains("singleFile"))
    {
    d->SingleFileCheckBox->setChecked(ioProperties["singleFile"].toBool());
    }
}
