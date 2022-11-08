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

// qCjyxGPURayCastVolumeRendering includes
#include "qCjyxMultiVolumeRenderingPropertiesWidget.h"
#include "vtkDMMLMultiVolumeRenderingDisplayNode.h"
#include "ui_qCjyxMultiVolumeRenderingPropertiesWidget.h"

// DMML includes
#include "vtkDMMLScene.h"
#include "vtkDMMLViewNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_VolumeRendering
class qCjyxMultiVolumeRenderingPropertiesWidgetPrivate
  : public Ui_qCjyxMultiVolumeRenderingPropertiesWidget
{
  Q_DECLARE_PUBLIC(qCjyxMultiVolumeRenderingPropertiesWidget);
protected:
  qCjyxMultiVolumeRenderingPropertiesWidget* const q_ptr;

public:
  qCjyxMultiVolumeRenderingPropertiesWidgetPrivate(
    qCjyxMultiVolumeRenderingPropertiesWidget& object);
  virtual ~qCjyxMultiVolumeRenderingPropertiesWidgetPrivate();

  virtual void setupUi(qCjyxMultiVolumeRenderingPropertiesWidget*);
  void populateRenderingTechniqueComboBox();
};

// --------------------------------------------------------------------------
qCjyxMultiVolumeRenderingPropertiesWidgetPrivate
::qCjyxMultiVolumeRenderingPropertiesWidgetPrivate(
  qCjyxMultiVolumeRenderingPropertiesWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
qCjyxMultiVolumeRenderingPropertiesWidgetPrivate::
~qCjyxMultiVolumeRenderingPropertiesWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qCjyxMultiVolumeRenderingPropertiesWidgetPrivate
::setupUi(qCjyxMultiVolumeRenderingPropertiesWidget* widget)
{
  this->Ui_qCjyxMultiVolumeRenderingPropertiesWidget::setupUi(widget);
  this->populateRenderingTechniqueComboBox();
  QObject::connect(this->RenderingTechniqueComboBox, SIGNAL(currentIndexChanged(int)),
                   widget, SLOT(setRenderingTechnique(int)));
  QObject::connect(this->SurfaceSmoothingCheckBox, SIGNAL(toggled(bool)),
                   widget, SLOT(setSurfaceSmoothing(bool)));
}

// --------------------------------------------------------------------------
void qCjyxMultiVolumeRenderingPropertiesWidgetPrivate::populateRenderingTechniqueComboBox()
{
  this->RenderingTechniqueComboBox->clear();
  this->RenderingTechniqueComboBox->addItem(
    "Composite With Shading", vtkDMMLViewNode::Composite);
  this->RenderingTechniqueComboBox->addItem(
    "Maximum Intensity Projection", vtkDMMLViewNode::MaximumIntensityProjection);
  this->RenderingTechniqueComboBox->addItem(
    "Minimum Intensity Projection", vtkDMMLViewNode::MinimumIntensityProjection);
}

//-----------------------------------------------------------------------------
// qCjyxMultiVolumeRenderingPropertiesWidget methods

//-----------------------------------------------------------------------------
qCjyxMultiVolumeRenderingPropertiesWidget
::qCjyxMultiVolumeRenderingPropertiesWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qCjyxMultiVolumeRenderingPropertiesWidgetPrivate(*this) )
{
  Q_D(qCjyxMultiVolumeRenderingPropertiesWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qCjyxMultiVolumeRenderingPropertiesWidget::~qCjyxMultiVolumeRenderingPropertiesWidget() = default;

//-----------------------------------------------------------------------------
vtkDMMLMultiVolumeRenderingDisplayNode* qCjyxMultiVolumeRenderingPropertiesWidget
::dmmlDisplayNode()
{
  return vtkDMMLMultiVolumeRenderingDisplayNode::SafeDownCast(
    this->dmmlVolumeRenderingDisplayNode());
}

//-----------------------------------------------------------------------------
void qCjyxMultiVolumeRenderingPropertiesWidget::updateWidgetFromDMML()
{
  Q_D(qCjyxMultiVolumeRenderingPropertiesWidget);

  vtkDMMLMultiVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  vtkDMMLViewNode* firstViewNode = displayNode->GetFirstViewNode();
  if (!firstViewNode)
    {
    return;
    }

  int technique = firstViewNode->GetRaycastTechnique();
  int index = d->RenderingTechniqueComboBox->findData(QVariant(technique));
  if (index == -1)
    {
    index = 0;
    }
  bool wasBlocked = d->RenderingTechniqueComboBox->blockSignals(true);
  d->RenderingTechniqueComboBox->setCurrentIndex(index);
  d->RenderingTechniqueComboBox->blockSignals(wasBlocked);

  wasBlocked = d->SurfaceSmoothingCheckBox->blockSignals(true);
  d->SurfaceSmoothingCheckBox->setChecked(firstViewNode->GetVolumeRenderingSurfaceSmoothing());
  d->SurfaceSmoothingCheckBox->blockSignals(wasBlocked);
}

//-----------------------------------------------------------------------------
void qCjyxMultiVolumeRenderingPropertiesWidget::setRenderingTechnique(int index)
{
  Q_D(qCjyxMultiVolumeRenderingPropertiesWidget);
  vtkDMMLMultiVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  int technique = d->RenderingTechniqueComboBox->itemData(index).toInt();

  std::vector<vtkDMMLNode*> viewNodes;
  displayNode->GetScene()->GetNodesByClass("vtkDMMLViewNode", viewNodes);
  for (std::vector<vtkDMMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkDMMLViewNode* viewNode = vtkDMMLViewNode::SafeDownCast(*it);
    if (displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      viewNode->SetRaycastTechnique(technique);
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxMultiVolumeRenderingPropertiesWidget::setSurfaceSmoothing(bool on)
{
  Q_D(qCjyxMultiVolumeRenderingPropertiesWidget);
  vtkDMMLMultiVolumeRenderingDisplayNode* displayNode = this->dmmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  std::vector<vtkDMMLNode*> viewNodes;
  displayNode->GetScene()->GetNodesByClass("vtkDMMLViewNode", viewNodes);
  for (std::vector<vtkDMMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkDMMLViewNode* viewNode = vtkDMMLViewNode::SafeDownCast(*it);
    if (displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      viewNode->SetVolumeRenderingSurfaceSmoothing(on);
      }
    }
}
