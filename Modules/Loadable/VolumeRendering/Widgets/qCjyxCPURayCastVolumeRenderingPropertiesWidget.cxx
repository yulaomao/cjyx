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

// qCjyxCPURayCastVolumeRendering includes
#include "qCjyxCPURayCastVolumeRenderingPropertiesWidget.h"
#include "vtkDMMLCPURayCastVolumeRenderingDisplayNode.h"
#include "ui_qCjyxCPURayCastVolumeRenderingPropertiesWidget.h"

// DMML includes
#include "vtkDMMLScene.h"
#include "vtkDMMLViewNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_VolumeRendering
class qCjyxCPURayCastVolumeRenderingPropertiesWidgetPrivate
  : public Ui_qCjyxCPURayCastVolumeRenderingPropertiesWidget
{
  Q_DECLARE_PUBLIC(qCjyxCPURayCastVolumeRenderingPropertiesWidget);
protected:
  qCjyxCPURayCastVolumeRenderingPropertiesWidget* const q_ptr;

public:
  qCjyxCPURayCastVolumeRenderingPropertiesWidgetPrivate(
    qCjyxCPURayCastVolumeRenderingPropertiesWidget& object);
  virtual ~qCjyxCPURayCastVolumeRenderingPropertiesWidgetPrivate();

  virtual void setupUi(qCjyxCPURayCastVolumeRenderingPropertiesWidget*);
  void populateRenderingTechniqueComboBox();
};

// --------------------------------------------------------------------------
qCjyxCPURayCastVolumeRenderingPropertiesWidgetPrivate
::qCjyxCPURayCastVolumeRenderingPropertiesWidgetPrivate(
  qCjyxCPURayCastVolumeRenderingPropertiesWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
qCjyxCPURayCastVolumeRenderingPropertiesWidgetPrivate::
~qCjyxCPURayCastVolumeRenderingPropertiesWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qCjyxCPURayCastVolumeRenderingPropertiesWidgetPrivate
::setupUi(qCjyxCPURayCastVolumeRenderingPropertiesWidget* widget)
{
  this->Ui_qCjyxCPURayCastVolumeRenderingPropertiesWidget::setupUi(widget);
  this->populateRenderingTechniqueComboBox();
  QObject::connect(this->RenderingTechniqueComboBox, SIGNAL(currentIndexChanged(int)),
                   widget, SLOT(setRenderingTechnique(int)));
}

// --------------------------------------------------------------------------
void qCjyxCPURayCastVolumeRenderingPropertiesWidgetPrivate
::populateRenderingTechniqueComboBox()
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
// qCjyxCPURayCastVolumeRenderingPropertiesWidget methods

//-----------------------------------------------------------------------------
qCjyxCPURayCastVolumeRenderingPropertiesWidget
::qCjyxCPURayCastVolumeRenderingPropertiesWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qCjyxCPURayCastVolumeRenderingPropertiesWidgetPrivate(*this) )
{
  Q_D(qCjyxCPURayCastVolumeRenderingPropertiesWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qCjyxCPURayCastVolumeRenderingPropertiesWidget
::~qCjyxCPURayCastVolumeRenderingPropertiesWidget() = default;

//-----------------------------------------------------------------------------
vtkDMMLCPURayCastVolumeRenderingDisplayNode* qCjyxCPURayCastVolumeRenderingPropertiesWidget
::dmmlCPURayCastDisplayNode()
{
  return vtkDMMLCPURayCastVolumeRenderingDisplayNode::SafeDownCast(
    this->dmmlVolumeRenderingDisplayNode());
}

//-----------------------------------------------------------------------------
void qCjyxCPURayCastVolumeRenderingPropertiesWidget::updateWidgetFromDMML()
{
  Q_D(qCjyxCPURayCastVolumeRenderingPropertiesWidget);
  if (!this->dmmlCPURayCastDisplayNode())
    {
    return;
    }
  vtkDMMLViewNode* firstViewNode = this->dmmlCPURayCastDisplayNode()->GetFirstViewNode();
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
  d->RenderingTechniqueComboBox->setCurrentIndex(index);
}

//-----------------------------------------------------------------------------
void qCjyxCPURayCastVolumeRenderingPropertiesWidget::setRenderingTechnique(int index)
{
  Q_D(qCjyxCPURayCastVolumeRenderingPropertiesWidget);
  vtkDMMLCPURayCastVolumeRenderingDisplayNode* displayNode =
    this->dmmlCPURayCastDisplayNode();
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
