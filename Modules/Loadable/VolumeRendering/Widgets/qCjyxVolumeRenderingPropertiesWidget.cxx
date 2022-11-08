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

// qCjyxVolumeRendering includes
#include "qCjyxVolumeRenderingPropertiesWidget.h"
#include "vtkDMMLVolumeRenderingDisplayNode.h"

// DMML includes
#include "vtkDMMLVolumeNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_VolumeRendering
class qCjyxVolumeRenderingPropertiesWidgetPrivate
{
  Q_DECLARE_PUBLIC(qCjyxVolumeRenderingPropertiesWidget);
protected:
  qCjyxVolumeRenderingPropertiesWidget* const q_ptr;

public:
  qCjyxVolumeRenderingPropertiesWidgetPrivate(qCjyxVolumeRenderingPropertiesWidget& object);

  vtkDMMLVolumeRenderingDisplayNode* VolumeRenderingDisplayNode;
  vtkDMMLVolumeNode* VolumeNode;
};

//-----------------------------------------------------------------------------
// qCjyxVolumeRenderingPropertiesWidgetPrivate methods

//-----------------------------------------------------------------------------
qCjyxVolumeRenderingPropertiesWidgetPrivate
::qCjyxVolumeRenderingPropertiesWidgetPrivate(
  qCjyxVolumeRenderingPropertiesWidget& object)
  : q_ptr(&object)
{
  this->VolumeRenderingDisplayNode = nullptr;
  this->VolumeNode = nullptr;
}

//-----------------------------------------------------------------------------
// qCjyxVolumeRenderingPropertiesWidget methods

//-----------------------------------------------------------------------------
qCjyxVolumeRenderingPropertiesWidget
::qCjyxVolumeRenderingPropertiesWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qCjyxVolumeRenderingPropertiesWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qCjyxVolumeRenderingPropertiesWidget::~qCjyxVolumeRenderingPropertiesWidget() = default;

//-----------------------------------------------------------------------------
vtkDMMLNode* qCjyxVolumeRenderingPropertiesWidget::dmmlNode()const
{
  return vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(
    this->dmmlVolumeRenderingDisplayNode());
}

//-----------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode* qCjyxVolumeRenderingPropertiesWidget
::dmmlVolumeRenderingDisplayNode()const
{
  Q_D(const qCjyxVolumeRenderingPropertiesWidget);
  return d->VolumeRenderingDisplayNode;
}

//-----------------------------------------------------------------------------
vtkDMMLVolumeNode* qCjyxVolumeRenderingPropertiesWidget
::dmmlVolumeNode()const
{
  Q_D(const qCjyxVolumeRenderingPropertiesWidget);
  return d->VolumeNode;
}

//-----------------------------------------------------------------------------
void qCjyxVolumeRenderingPropertiesWidget
::setDMMLNode(vtkDMMLNode* node)
{
  this->setDMMLVolumeRenderingDisplayNode(
    vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(node));
}

//-----------------------------------------------------------------------------
void qCjyxVolumeRenderingPropertiesWidget
::setDMMLVolumeRenderingDisplayNode(vtkDMMLVolumeRenderingDisplayNode* displayNode)
{
  Q_D(qCjyxVolumeRenderingPropertiesWidget);
  qvtkReconnect(d->VolumeRenderingDisplayNode, displayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDMML()));

  d->VolumeRenderingDisplayNode = displayNode;
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qCjyxVolumeRenderingPropertiesWidget::updateWidgetFromDMML()
{
  Q_D(qCjyxVolumeRenderingPropertiesWidget);
  vtkDMMLVolumeNode* newVolumeNode =
    d->VolumeRenderingDisplayNode ? d->VolumeRenderingDisplayNode->GetVolumeNode() : nullptr;
  qvtkReconnect(d->VolumeNode, newVolumeNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDMMLVolumeNode()));
  d->VolumeNode = newVolumeNode;
  this->updateWidgetFromDMMLVolumeNode();
}

//-----------------------------------------------------------------------------
void qCjyxVolumeRenderingPropertiesWidget::updateWidgetFromDMMLVolumeNode()
{
}
