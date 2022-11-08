/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care
  and CANARIE.

==============================================================================*/
// Segmentations includes
#include "qDMMLSegmentationGeometryDialog.h"

#include "vtkDMMLSegmentationNode.h"

#include "qCjyxApplication.h"

// Segmentations logic includes
#include "vtkCjyxSegmentationsModuleLogic.h"

// CTK includes
#include <ctkMessageBox.h>

// Segmentation core includes
#include <vtkOrientedImageData.h>

// Qt includes
#include <QDialog>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Segmentations_Widgets
class qDMMLSegmentationGeometryDialogPrivate : public QDialog
{
  Q_DECLARE_PUBLIC(qDMMLSegmentationGeometryDialog);
protected:
  qDMMLSegmentationGeometryDialog* const q_ptr;
public:
  qDMMLSegmentationGeometryDialogPrivate(qDMMLSegmentationGeometryDialog& object);
  ~qDMMLSegmentationGeometryDialogPrivate() override;
public:
  void init();
private:
  vtkDMMLSegmentationNode* SegmentationNode;
  bool ResampleLabelmaps;

  qDMMLSegmentationGeometryWidget* GeometryWidget;
  QPushButton* OKButton;
  QPushButton* CancelButton;

};

//-----------------------------------------------------------------------------
qDMMLSegmentationGeometryDialogPrivate::qDMMLSegmentationGeometryDialogPrivate(qDMMLSegmentationGeometryDialog& object)
  : q_ptr(&object)
  , ResampleLabelmaps(false)
{
}

//-----------------------------------------------------------------------------
qDMMLSegmentationGeometryDialogPrivate::~qDMMLSegmentationGeometryDialogPrivate()
{
  delete this->GeometryWidget;
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationGeometryDialogPrivate::init()
{
  Q_Q(qDMMLSegmentationGeometryDialog);

  // Set up UI
  this->setWindowTitle("Segmentation geometry");

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setSpacing(4);
  layout->setContentsMargins(4, 4, 4, 6);

  this->GeometryWidget = new qDMMLSegmentationGeometryWidget();
  this->GeometryWidget->setSegmentationNode(this->SegmentationNode);
  layout->addWidget(this->GeometryWidget);

  //layout->addStretch(1);

  QHBoxLayout* buttonsLayout = new QHBoxLayout();
  buttonsLayout->setSpacing(4);
  buttonsLayout->setContentsMargins(4, 0, 4, 0);

  this->OKButton = new QPushButton("OK");
  buttonsLayout->addWidget(this->OKButton);

  this->CancelButton = new QPushButton("Cancel");
  buttonsLayout->addWidget(this->CancelButton);
  this->CancelButton->setVisible(this->GeometryWidget->editEnabled());

  layout->addLayout(buttonsLayout);

  // Make connections
  connect(this->OKButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(this->CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

//-----------------------------------------------------------------------------
// qDMMLSegmentationGeometryDialog methods

//-----------------------------------------------------------------------------
qDMMLSegmentationGeometryDialog::qDMMLSegmentationGeometryDialog(vtkDMMLSegmentationNode* segmentationNode, QObject* parent)
  : QObject(parent)
  , d_ptr(new qDMMLSegmentationGeometryDialogPrivate(*this))
{
  Q_D(qDMMLSegmentationGeometryDialog);
  d->SegmentationNode = segmentationNode;

  d->init();
}

//-----------------------------------------------------------------------------
qDMMLSegmentationGeometryDialog::~qDMMLSegmentationGeometryDialog() = default;

//-----------------------------------------------------------------------------
bool qDMMLSegmentationGeometryDialog::editEnabled()const
{
  Q_D(const qDMMLSegmentationGeometryDialog);
  return d->GeometryWidget->editEnabled();
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationGeometryDialog::setEditEnabled(bool aEditEnabled)
{
  Q_D(qDMMLSegmentationGeometryDialog);
  d->GeometryWidget->setEditEnabled(aEditEnabled);
  d->CancelButton->setVisible(aEditEnabled);
}

//-----------------------------------------------------------------------------
bool qDMMLSegmentationGeometryDialog::resampleLabelmaps()const
{
  Q_D(const qDMMLSegmentationGeometryDialog);
  return d->ResampleLabelmaps;
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationGeometryDialog::setResampleLabelmaps(bool aResampleLabelmaps)
{
  Q_D(qDMMLSegmentationGeometryDialog);
  d->ResampleLabelmaps = aResampleLabelmaps;
  if (aResampleLabelmaps)
    {
    d->OKButton->setToolTip("Set reference image geometry and resample all segment labelmaps");
    }
  else
    {
    d->OKButton->setToolTip("Set reference image geometry (do not resample)");
    }
}

//-----------------------------------------------------------------------------
bool qDMMLSegmentationGeometryDialog::exec()
{
  Q_D(qDMMLSegmentationGeometryDialog);

  // Initialize dialog
  d->GeometryWidget->setSegmentationNode(d->SegmentationNode);

  qCjyxApplication* app = qCjyxApplication::application();
  QWidget* mainWindow = app ? app->mainWindow() : nullptr;
  if (mainWindow)
    {
    // setParent resets window flags, so save them and then restore
    Qt::WindowFlags windowFlags = d->windowFlags();
    d->setParent(mainWindow);
    d->setWindowFlags(windowFlags);
    }

  // Show dialog
  bool result = false;
  if (d->exec() != QDialog::Accepted)
    {
    return result;
    }

  DMMLNodeModifyBlocker blocker(d->SegmentationNode);

  // Apply geometry after clean exit
  if (d->GeometryWidget->editEnabled())
    {
    d->GeometryWidget->setReferenceImageGeometryForSegmentationNode();
    if (d->ResampleLabelmaps)
      {
      d->GeometryWidget->resampleLabelmapsInSegmentationNode();
      }
    }
  return true;
}
