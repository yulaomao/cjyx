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

#include "qCjyxDiffusionTensorVolumeDisplayWidget.h"
#include "ui_qCjyxDiffusionTensorVolumeDisplayWidget.h"

// Qt includes

// DMML includes
#include "vtkDMMLDiffusionTensorVolumeNode.h"
#include "vtkDMMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkDMMLGlyphableVolumeSliceDisplayNode.h"

// VTK includes

// STD includes

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Volumes
class qCjyxDiffusionTensorVolumeDisplayWidgetPrivate
  : public Ui_qCjyxDiffusionTensorVolumeDisplayWidget
{
  Q_DECLARE_PUBLIC(qCjyxDiffusionTensorVolumeDisplayWidget);
protected:
  qCjyxDiffusionTensorVolumeDisplayWidget* const q_ptr;
public:
  qCjyxDiffusionTensorVolumeDisplayWidgetPrivate(qCjyxDiffusionTensorVolumeDisplayWidget& object);
  ~qCjyxDiffusionTensorVolumeDisplayWidgetPrivate();
  void init();
  void glyphsOnSlicesDisplaySetEnabled(bool enabled);
  vtkWeakPointer<vtkDMMLDiffusionTensorVolumeNode> VolumeNode;
};

//-----------------------------------------------------------------------------
qCjyxDiffusionTensorVolumeDisplayWidgetPrivate
::qCjyxDiffusionTensorVolumeDisplayWidgetPrivate(
  qCjyxDiffusionTensorVolumeDisplayWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qCjyxDiffusionTensorVolumeDisplayWidgetPrivate
::~qCjyxDiffusionTensorVolumeDisplayWidgetPrivate() = default;

//-----------------------------------------------------------------------------
void qCjyxDiffusionTensorVolumeDisplayWidgetPrivate::init()
{
  Q_Q(qCjyxDiffusionTensorVolumeDisplayWidget);

  this->setupUi(q);
  this->DTISliceDisplayWidget->setVisibilityHidden(true);

  QObject::connect(this->ScalarInvariantComboBox, SIGNAL(scalarInvariantChanged(int)),
                   q, SLOT(setVolumeScalarInvariant(int)));
  QObject::connect(this->RedSliceCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setRedSliceVisible(bool)));
  QObject::connect(this->YellowSliceCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setYellowSliceVisible(bool)));
  QObject::connect(this->GreenSliceCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setGreenSliceVisible(bool)));
}

//-----------------------------------------------------------------------------
void qCjyxDiffusionTensorVolumeDisplayWidgetPrivate::glyphsOnSlicesDisplaySetEnabled(bool enabled)
{
  this->GlyphsOnSlicesDisplayCollapsibleGroupBox->setEnabled(enabled);
  if (!enabled)
    {
    this->RedSliceCheckBox->setCheckState(Qt::Unchecked);
    this->YellowSliceCheckBox->setCheckState(Qt::Unchecked);
    this->GreenSliceCheckBox->setCheckState(Qt::Unchecked);
    }
}

// --------------------------------------------------------------------------
qCjyxDiffusionTensorVolumeDisplayWidget
::qCjyxDiffusionTensorVolumeDisplayWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qCjyxDiffusionTensorVolumeDisplayWidgetPrivate(*this))
{
  Q_D(qCjyxDiffusionTensorVolumeDisplayWidget);
  d->init();

  // disable as there is not DMML Node associated with the widget
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qCjyxDiffusionTensorVolumeDisplayWidget
::~qCjyxDiffusionTensorVolumeDisplayWidget() = default;

// --------------------------------------------------------------------------
vtkDMMLDiffusionTensorVolumeNode* qCjyxDiffusionTensorVolumeDisplayWidget
::volumeNode()const
{
  Q_D(const qCjyxDiffusionTensorVolumeDisplayWidget);
  return d->VolumeNode;
}

// --------------------------------------------------------------------------
vtkDMMLDiffusionTensorVolumeDisplayNode* qCjyxDiffusionTensorVolumeDisplayWidget::volumeDisplayNode()const
{
  vtkDMMLDiffusionTensorVolumeNode* volumeNode = this->volumeNode();
  return volumeNode ? vtkDMMLDiffusionTensorVolumeDisplayNode::SafeDownCast(
    volumeNode->GetVolumeDisplayNode()) : nullptr;
}

// --------------------------------------------------------------------------
QList<vtkDMMLGlyphableVolumeSliceDisplayNode*> qCjyxDiffusionTensorVolumeDisplayWidget::sliceDisplayNodes()const
{
  Q_D(const qCjyxDiffusionTensorVolumeDisplayWidget);
  vtkDMMLDiffusionTensorVolumeDisplayNode* displayNode = this->volumeDisplayNode();
  if (!displayNode)
    {
    return QList<vtkDMMLGlyphableVolumeSliceDisplayNode*>();
    }
  QList<vtkDMMLGlyphableVolumeSliceDisplayNode*> res
    = QList<vtkDMMLGlyphableVolumeSliceDisplayNode*>::fromVector(
      QVector<vtkDMMLGlyphableVolumeSliceDisplayNode*>::fromStdVector(
        displayNode->GetSliceGlyphDisplayNodes(d->VolumeNode)));
  return res;
}

// --------------------------------------------------------------------------
void qCjyxDiffusionTensorVolumeDisplayWidget::setDMMLVolumeNode(vtkDMMLNode* node)
{
  this->setDMMLVolumeNode(vtkDMMLDiffusionTensorVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qCjyxDiffusionTensorVolumeDisplayWidget::setDMMLVolumeNode(vtkDMMLDiffusionTensorVolumeNode* volumeNode)
{
  Q_D(qCjyxDiffusionTensorVolumeDisplayWidget);

  vtkDMMLDiffusionTensorVolumeDisplayNode* oldVolumeDisplayNode = this->volumeDisplayNode();

  qvtkReconnect(oldVolumeDisplayNode, volumeNode ? volumeNode->GetVolumeDisplayNode() :nullptr,
                vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDMML()));
  d->VolumeNode = volumeNode;
  d->ScalarVolumeDisplayWidget->setDMMLVolumeNode(volumeNode);
  vtkDMMLDiffusionTensorVolumeDisplayNode* newVolumeDisplayNode = this->volumeDisplayNode();
  vtkDMMLGlyphableVolumeSliceDisplayNode* glyphableVolumeSliceNode = nullptr;
  if (newVolumeDisplayNode)
    {
    std::vector< vtkDMMLGlyphableVolumeSliceDisplayNode*> dtiSliceDisplayNodes =
      newVolumeDisplayNode->GetSliceGlyphDisplayNodes(d->VolumeNode);
    if (dtiSliceDisplayNodes.size() == 0)
      {
      newVolumeDisplayNode->AddSliceGlyphDisplayNodes(d->VolumeNode);
      dtiSliceDisplayNodes =
        newVolumeDisplayNode->GetSliceGlyphDisplayNodes(d->VolumeNode);
      }
    Q_ASSERT(dtiSliceDisplayNodes.size());
    d->RedSliceCheckBox->setChecked(dtiSliceDisplayNodes[0]->GetVisibility());
    if (dtiSliceDisplayNodes.size() > 1)
      {
      d->YellowSliceCheckBox->setChecked(dtiSliceDisplayNodes[1]->GetVisibility());
      }
    if (dtiSliceDisplayNodes.size() > 2)
      {
      d->GreenSliceCheckBox->setChecked(dtiSliceDisplayNodes[1]->GetVisibility());
      }

    glyphableVolumeSliceNode = dtiSliceDisplayNodes[0];
    }
  // The update tasks are also needed when scene is closed (newVolumeDisplayNode is nullptr)
  d->DTISliceDisplayWidget->setDMMLDTISliceDisplayNode(glyphableVolumeSliceNode);
  qvtkDisconnect(nullptr, vtkCommand::ModifiedEvent, this, SLOT(synchronizeSliceDisplayNodes()));
  qvtkConnect(glyphableVolumeSliceNode, vtkCommand::ModifiedEvent,
              this, SLOT(synchronizeSliceDisplayNodes()));
  this->synchronizeSliceDisplayNodes();
  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qCjyxDiffusionTensorVolumeDisplayWidget::updateWidgetFromDMML()
{
  Q_D(qCjyxDiffusionTensorVolumeDisplayWidget);
  this->setEnabled(d->VolumeNode != nullptr);
  vtkDMMLDiffusionTensorVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (!displayNode)
    {
    return;
    }
  d->ScalarInvariantComboBox->setScalarInvariant(displayNode->GetScalarInvariant());
  if (
    displayNode->GetScalarInvariant() == vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorOrientation ||
    displayNode->GetScalarInvariant() == vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMiddleEigenvector ||
    displayNode->GetScalarInvariant() == vtkDMMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMinEigenvector
    )
  {
    d->ScalarVolumeDisplayWidget->setColorTableComboBoxEnabled(false);
    d->ScalarVolumeDisplayWidget->setDMMLWindowLevelWidgetEnabled(false);
    displayNode->AutoScalarRangeOn();
    displayNode->AutoWindowLevelOn();
  } else {
    d->ScalarVolumeDisplayWidget->setColorTableComboBoxEnabled(true);
    d->ScalarVolumeDisplayWidget->setDMMLWindowLevelWidgetEnabled(true);
  }
}

// --------------------------------------------------------------------------
void qCjyxDiffusionTensorVolumeDisplayWidget::synchronizeSliceDisplayNodes()
{
  QList<vtkDMMLGlyphableVolumeSliceDisplayNode*> sliceDisplayNodes = this->sliceDisplayNodes();
  if (sliceDisplayNodes.count() != 3)
    {
    return;
    }
  sliceDisplayNodes[1]->SetColorMode(sliceDisplayNodes[0]->GetColorMode());
  sliceDisplayNodes[1]->SetOpacity(sliceDisplayNodes[0]->GetOpacity());
  sliceDisplayNodes[1]->SetAndObserveColorNodeID(sliceDisplayNodes[0]->GetColorNodeID());
  sliceDisplayNodes[1]->SetAutoScalarRange(sliceDisplayNodes[0]->GetAutoScalarRange());
  sliceDisplayNodes[1]->SetScalarRange(sliceDisplayNodes[0]->GetScalarRange()[0],
                                       sliceDisplayNodes[0]->GetScalarRange()[1]);
  sliceDisplayNodes[2]->SetColorMode(sliceDisplayNodes[0]->GetColorMode());
  sliceDisplayNodes[2]->SetOpacity(sliceDisplayNodes[0]->GetOpacity());
  sliceDisplayNodes[2]->SetAndObserveColorNodeID(sliceDisplayNodes[0]->GetColorNodeID());
  sliceDisplayNodes[2]->SetAutoScalarRange(sliceDisplayNodes[0]->GetAutoScalarRange());
  sliceDisplayNodes[2]->SetScalarRange(sliceDisplayNodes[0]->GetScalarRange()[0],
                                       sliceDisplayNodes[0]->GetScalarRange()[1]);
}

//----------------------------------------------------------------------------
void qCjyxDiffusionTensorVolumeDisplayWidget::setVolumeScalarInvariant(int scalarInvariant)
{
  vtkDMMLDiffusionTensorVolumeDisplayNode* volumeDisplayNode = this->volumeDisplayNode();
  if (!volumeDisplayNode)
    {
    return;
    }
  volumeDisplayNode->SetScalarInvariant(scalarInvariant);
}

//----------------------------------------------------------------------------
void qCjyxDiffusionTensorVolumeDisplayWidget::setRedSliceVisible(bool visible)
{
  QList<vtkDMMLGlyphableVolumeSliceDisplayNode*> sliceDisplayNodes = this->sliceDisplayNodes();
  if (sliceDisplayNodes.count() != 3)
    {
    return;
    }
  sliceDisplayNodes[0]->SetVisibility(visible);
}

//----------------------------------------------------------------------------
void qCjyxDiffusionTensorVolumeDisplayWidget::setYellowSliceVisible(bool visible)
{
  QList<vtkDMMLGlyphableVolumeSliceDisplayNode*> sliceDisplayNodes = this->sliceDisplayNodes();
  if (sliceDisplayNodes.count() != 3)
    {
    return;
    }
  sliceDisplayNodes[1]->SetVisibility(visible);
}

//----------------------------------------------------------------------------
void qCjyxDiffusionTensorVolumeDisplayWidget::setGreenSliceVisible(bool visible)
{
  QList<vtkDMMLGlyphableVolumeSliceDisplayNode*> sliceDisplayNodes = this->sliceDisplayNodes();
  if (sliceDisplayNodes.count() != 3)
    {
    return;
    }
  sliceDisplayNodes[2]->SetVisibility(visible);
}
