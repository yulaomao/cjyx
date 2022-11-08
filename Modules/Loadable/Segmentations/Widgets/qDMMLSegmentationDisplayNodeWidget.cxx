/*==============================================================================

  Program: 3D Cjyx

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

==============================================================================*/

// Segmentations includes
#include "qDMMLSegmentationDisplayNodeWidget.h"

#include "ui_qDMMLSegmentationDisplayNodeWidget.h"

#include "qDMMLSegmentsTableView.h"

#include "vtkDMMLSegmentationDisplayNode.h"
#include "vtkDMMLSegmentationNode.h"

// SegmentationCore includes
#include "vtkSegmentation.h"
#include "vtkSegment.h"

// VTK includes
#include <vtkWeakPointer.h>

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
class qDMMLSegmentationDisplayNodeWidgetPrivate: public Ui_qDMMLSegmentationDisplayNodeWidget
{
  Q_DECLARE_PUBLIC(qDMMLSegmentationDisplayNodeWidget);

protected:
  qDMMLSegmentationDisplayNodeWidget* const q_ptr;
public:
  qDMMLSegmentationDisplayNodeWidgetPrivate(qDMMLSegmentationDisplayNodeWidget& object);
  void init();

public:
  /// Segmentation DMML display node
  vtkWeakPointer<vtkDMMLSegmentationDisplayNode> SegmentationDisplayNode;

  /// Selected segment ID
  QString SelectedSegmentID;
};

//-----------------------------------------------------------------------------
qDMMLSegmentationDisplayNodeWidgetPrivate::qDMMLSegmentationDisplayNodeWidgetPrivate(qDMMLSegmentationDisplayNodeWidget& object)
  : q_ptr(&object)
{
  this->SegmentationDisplayNode = nullptr;
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidgetPrivate::init()
{
  Q_Q(qDMMLSegmentationDisplayNodeWidget);
  this->setupUi(q);

  // Make connections
  QObject::connect(this->checkBox_Visible, SIGNAL(stateChanged(int)),
    q, SLOT(onVisibilityChanged(int)) );
  QObject::connect(this->SliderWidget_Opacity, SIGNAL(valueChanged(double)),
    q, SLOT(onOpacityChanged(double)));

  // Segmentation visibility and opacity settings
  QObject::connect(this->checkBox_VisibilitySliceFill, SIGNAL(stateChanged(int)),
    q, SLOT(onVisibilitySliceFillChanged(int)) );
  QObject::connect(this->checkBox_VisibilitySliceOutline, SIGNAL(stateChanged(int)),
    q, SLOT(onVisibilitySliceOutlineChanged(int)) );
  QObject::connect(this->checkBox_Visibility3D, SIGNAL(stateChanged(int)),
    q, SLOT(onVisibility3DChanged(int)) );

  QObject::connect(this->SliderWidget_OpacitySliceFill, SIGNAL(valueChanged(double)),
    q, SLOT(onOpacitySliceFillChanged(double)) );
  QObject::connect(this->SliderWidget_OpacitySliceOutline, SIGNAL(valueChanged(double)),
    q, SLOT(onOpacitySliceOutlineChanged(double)) );
  QObject::connect(this->SliderWidget_Opacity3D, SIGNAL(valueChanged(double)),
    q, SLOT(onOpacity3DChanged(double)) );

  // Advanced options
  QObject::connect(this->spinBox_SliceIntersectionThickness, SIGNAL(valueChanged(int)),
    q, SLOT(onSliceIntersectionThicknessChanged(int)) );
  QObject::connect(this->comboBox_DisplayedRepresentation3D, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onRepresentation3DChanged(int)) );
  QObject::connect(this->comboBox_DisplayedRepresentation2D, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onRepresentation2DChanged(int)) );

  // Selected segment visibility and opacity settings
  QObject::connect(this->checkBox_VisibilitySliceFill_SelectedSegment, SIGNAL(stateChanged(int)),
    q, SLOT(onSegmentVisibilitySliceFillChanged(int)) );
  QObject::connect(this->checkBox_VisibilitySliceOutline_SelectedSegment, SIGNAL(stateChanged(int)),
    q, SLOT(onSegmentVisibilitySliceOutlineChanged(int)) );
  QObject::connect(this->checkBox_Visibility3D_SelectedSegment, SIGNAL(stateChanged(int)),
    q, SLOT(onSegmentVisibility3DChanged(int)) );

  QObject::connect(this->SliderWidget_OpacitySliceFill_SelectedSegment, SIGNAL(valueChanged(double)),
    q, SLOT(onSegmentOpacitySliceFillChanged(double)) );
  QObject::connect(this->SliderWidget_OpacitySliceOutline_SelectedSegment, SIGNAL(valueChanged(double)),
    q, SLOT(onSegmentOpacitySliceOutlineChanged(double)) );
  QObject::connect(this->SliderWidget_Opacity3D_SelectedSegment, SIGNAL(valueChanged(double)),
    q, SLOT(onSegmentOpacity3DChanged(double)) );

  q->setEnabled(this->SegmentationDisplayNode.GetPointer());

  q->updateSelectedSegmentSection();
}


//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// qDMMLSegmentationDisplayNodeWidget methods

//-----------------------------------------------------------------------------
qDMMLSegmentationDisplayNodeWidget::qDMMLSegmentationDisplayNodeWidget(QWidget* _parent)
  : qDMMLWidget(_parent)
  , d_ptr(new qDMMLSegmentationDisplayNodeWidgetPrivate(*this))
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);
  d->init();
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
qDMMLSegmentationDisplayNodeWidget::~qDMMLSegmentationDisplayNodeWidget() = default;

//-----------------------------------------------------------------------------
vtkDMMLSegmentationDisplayNode* qDMMLSegmentationDisplayNodeWidget::segmentationDisplayNode() const
{
  Q_D(const qDMMLSegmentationDisplayNodeWidget);
  return d->SegmentationDisplayNode;
}

//-----------------------------------------------------------------------------
QString qDMMLSegmentationDisplayNodeWidget::segmentationDisplayNodeID()
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);
  return (d->SegmentationDisplayNode.GetPointer() ? d->SegmentationDisplayNode->GetID() : QString());
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::setSegmentationDisplayNode(vtkDMMLSegmentationDisplayNode* node)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (d->SegmentationDisplayNode == node)
    {
    return;
    }

  qvtkReconnect(d->SegmentationDisplayNode, node, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromDMML()));

  vtkDMMLSegmentationDisplayNode* displayNode = vtkDMMLSegmentationDisplayNode::SafeDownCast(node);
  d->SegmentationDisplayNode = displayNode;

  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::setSegmentationNode(vtkDMMLSegmentationNode* node)
{
  this->setSegmentationDisplayNode(
    node ? vtkDMMLSegmentationDisplayNode::SafeDownCast(node->GetDisplayNode()) : nullptr );
}

//-----------------------------------------------------------------------------
QString qDMMLSegmentationDisplayNodeWidget::currentSegmentID()
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  return d->SelectedSegmentID;
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::setCurrentSegmentID(QString segmentID)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  d->SelectedSegmentID = segmentID;
  this->updateSelectedSegmentSection();
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::updateSelectedSegmentSection()
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  d->groupBox_SelectedSegment->setEnabled(!d->SelectedSegmentID.isEmpty());
  if (!d->SegmentationDisplayNode || d->SelectedSegmentID.isEmpty())
    {
    d->groupBox_SelectedSegment->setTitle("Selected segment: none");
    return;
    }

  // Get segment display properties
  vtkDMMLSegmentationDisplayNode::SegmentDisplayProperties properties;
  if (d->SegmentationDisplayNode)
    {
    if (!d->SegmentationDisplayNode->GetSegmentDisplayProperties(d->SelectedSegmentID.toUtf8().constData(), properties))
      {
      qCritical() << Q_FUNC_INFO << ": No display properties found for segment ID " << d->SelectedSegmentID;
      return;
      }
    }

  d->checkBox_VisibilitySliceFill_SelectedSegment->blockSignals(true);
  d->checkBox_VisibilitySliceFill_SelectedSegment->setChecked(properties.Visible2DFill);
  d->checkBox_VisibilitySliceFill_SelectedSegment->blockSignals(false);

  d->checkBox_VisibilitySliceOutline_SelectedSegment->blockSignals(true);
  d->checkBox_VisibilitySliceOutline_SelectedSegment->setChecked(properties.Visible2DOutline);
  d->checkBox_VisibilitySliceOutline_SelectedSegment->blockSignals(false);

  d->checkBox_Visibility3D_SelectedSegment->blockSignals(true);
  d->checkBox_Visibility3D_SelectedSegment->setChecked(properties.Visible3D);
  d->checkBox_Visibility3D_SelectedSegment->blockSignals(false);

  d->SliderWidget_OpacitySliceFill_SelectedSegment->blockSignals(true);
  d->SliderWidget_OpacitySliceFill_SelectedSegment->setValue(properties.Opacity2DFill);
  d->SliderWidget_OpacitySliceFill_SelectedSegment->blockSignals(false);

  d->SliderWidget_OpacitySliceOutline_SelectedSegment->blockSignals(true);
  d->SliderWidget_OpacitySliceOutline_SelectedSegment->setValue(properties.Opacity2DOutline);
  d->SliderWidget_OpacitySliceOutline_SelectedSegment->blockSignals(false);

  d->SliderWidget_Opacity3D_SelectedSegment->blockSignals(true);
  d->SliderWidget_Opacity3D_SelectedSegment->setValue(properties.Opacity3D);
  d->SliderWidget_Opacity3D_SelectedSegment->blockSignals(false);

  // Set groupbox header to include segment name
  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(d->SegmentationDisplayNode->GetDisplayableNode());
  if (!segmentationNode || !segmentationNode->GetSegmentation())
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentation node!";
    return;
    }
  vtkSegment* selectedSegment = segmentationNode->GetSegmentation()->GetSegment(d->SelectedSegmentID.toUtf8().constData());
  if (!selectedSegment)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to access segment " << d->SelectedSegmentID << " in segmentation " << segmentationNode->GetName();
    return;
    }
  QString newTitle = QString("Selected segment: %1").arg(selectedSegment->GetName());
  d->groupBox_SelectedSegment->setTitle(newTitle);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  this->setEnabled(d->SegmentationDisplayNode.GetPointer());
  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }
  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(d->SegmentationDisplayNode->GetDisplayableNode());
  if (!segmentationNode)
    {
    return;
    }

  // Set overall visibility
  d->checkBox_Visible->setChecked( d->SegmentationDisplayNode->GetVisibility() );

  bool wasBlocked = d->SliderWidget_Opacity->blockSignals(true);
  d->SliderWidget_Opacity->setValue(d->SegmentationDisplayNode->GetOpacity());
  d->SliderWidget_Opacity->blockSignals(wasBlocked);

  // Set visibility and opacity settings
  wasBlocked = d->checkBox_VisibilitySliceFill->blockSignals(true);
  d->checkBox_VisibilitySliceFill->setChecked(d->SegmentationDisplayNode->GetVisibility2DFill());
  d->checkBox_VisibilitySliceFill->blockSignals(wasBlocked);

  wasBlocked = d->checkBox_VisibilitySliceOutline->blockSignals(true);
  d->checkBox_VisibilitySliceOutline->setChecked(d->SegmentationDisplayNode->GetVisibility2DOutline());
  d->checkBox_VisibilitySliceOutline->blockSignals(wasBlocked);

  wasBlocked = d->checkBox_Visibility3D->blockSignals(true);
  d->checkBox_Visibility3D->setChecked(d->SegmentationDisplayNode->GetVisibility3D());
  d->checkBox_Visibility3D->blockSignals(wasBlocked);

  wasBlocked = d->SliderWidget_OpacitySliceFill->blockSignals(true);
  d->SliderWidget_OpacitySliceFill->setValue(d->SegmentationDisplayNode->GetOpacity2DFill());
  d->SliderWidget_OpacitySliceFill->blockSignals(wasBlocked);

  wasBlocked = d->SliderWidget_OpacitySliceOutline->blockSignals(true);
  d->SliderWidget_OpacitySliceOutline->setValue(d->SegmentationDisplayNode->GetOpacity2DOutline());
  d->SliderWidget_OpacitySliceOutline->blockSignals(wasBlocked);

  wasBlocked = d->SliderWidget_Opacity3D->blockSignals(true);
  d->SliderWidget_Opacity3D->setValue(d->SegmentationDisplayNode->GetOpacity3D());
  d->SliderWidget_Opacity3D->blockSignals(wasBlocked);

  // Set visibility checkbox states

  // Set slice intersection thickness
  d->spinBox_SliceIntersectionThickness->setValue( d->SegmentationDisplayNode->GetSliceIntersectionThickness() );

  // Populate representations comboboxes
  this->populate3DRepresentationsCombobox();
  this->populate2DRepresentationsCombobox();

  // Set displayed representation selections
  std::string displayRepresentation3D = d->SegmentationDisplayNode->GetDisplayRepresentationName3D();
  if (!displayRepresentation3D.empty())
    {
    d->comboBox_DisplayedRepresentation3D->setCurrentIndex( d->comboBox_DisplayedRepresentation3D->findText(
      displayRepresentation3D.c_str() ) );
    }
  std::string displayRepresentation2D = d->SegmentationDisplayNode->GetDisplayRepresentationName2D();
  if (!displayRepresentation2D.empty())
    {
    d->comboBox_DisplayedRepresentation2D->setCurrentIndex( d->comboBox_DisplayedRepresentation2D->findText(
      displayRepresentation2D.c_str() ) );
    }

  // Set display node to display widgets
  d->DisplayNodeViewComboBox->setDMMLDisplayNode(d->SegmentationDisplayNode);

  // Update selected segment visibility and opacity section
  this->updateSelectedSegmentSection();
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::populate3DRepresentationsCombobox()
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);
  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  // Note: This function used to collect existing poly data representations from
  // the segmentation and was connected to events like this:
  // qvtkConnect( segmentationNode, vtkSegmentation::ContainedRepresentationNamesModified, this, SLOT( populateRepresentationsCombobox() ) );
  // It was then decided that a preferred poly data representation can be selected
  // regardless its existence, thus the combobox is populated only once at initialization.

  // Prevent selecting incrementally added representations thus changing DMML properties
  d->comboBox_DisplayedRepresentation3D->blockSignals(true);
  d->comboBox_DisplayedRepresentation3D->clear();

  // Populate 3D representations combobox with only poly data representations
  std::set<std::string> modelRepresentationNames;
  d->SegmentationDisplayNode->GetPolyDataRepresentationNames(modelRepresentationNames);
  for (std::set<std::string>::iterator reprIt = modelRepresentationNames.begin();
    reprIt != modelRepresentationNames.end(); ++reprIt)
    {
    d->comboBox_DisplayedRepresentation3D->addItem(reprIt->c_str());
    }

  // Unblock signals
  d->comboBox_DisplayedRepresentation3D->blockSignals(false);

  // Set selection from display node
  std::string displayRepresentation3D = d->SegmentationDisplayNode->GetDisplayRepresentationName3D();
  if (!displayRepresentation3D.empty())
    {
    d->comboBox_DisplayedRepresentation3D->setCurrentIndex( d->comboBox_DisplayedRepresentation3D->findText(
      displayRepresentation3D.c_str() ) );
    }
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::populate2DRepresentationsCombobox()
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);
  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  // Prevent selecting incrementally added representations thus changing DMML properties
  d->comboBox_DisplayedRepresentation2D->blockSignals(true);
  d->comboBox_DisplayedRepresentation2D->clear();

  vtkDMMLSegmentationNode* segmentationNode = vtkDMMLSegmentationNode::SafeDownCast(d->SegmentationDisplayNode->GetDisplayableNode());
  if (!segmentationNode)
    {
    d->comboBox_DisplayedRepresentation2D->blockSignals(false);
    return;
    }

  // Populate 2D representations combobox with all available representations
  std::set<std::string> representationNames;
  segmentationNode->GetSegmentation()->GetAvailableRepresentationNames(representationNames);
  for (std::set<std::string>::iterator reprIt = representationNames.begin();
    reprIt != representationNames.end(); ++reprIt)
    {
    d->comboBox_DisplayedRepresentation2D->addItem(reprIt->c_str());
    }

  // Unblock signals
  d->comboBox_DisplayedRepresentation2D->blockSignals(false);

  // Set selection from display node
  std::string displayRepresentation2D = d->SegmentationDisplayNode->GetDisplayRepresentationName2D();
  if (!displayRepresentation2D.empty())
    {
    d->comboBox_DisplayedRepresentation2D->setCurrentIndex( d->comboBox_DisplayedRepresentation2D->findText(
      displayRepresentation2D.c_str() ) );
    }
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onVisibilityChanged(int visible)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  d->SegmentationDisplayNode->SetVisibility(visible > 0 ? 1 : 0);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onOpacityChanged(double opacity)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  d->SegmentationDisplayNode->SetOpacity(opacity);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onVisibilitySliceFillChanged(int visibility)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  d->SegmentationDisplayNode->SetVisibility2DFill(visibility);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onVisibilitySliceOutlineChanged(int visibility)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  d->SegmentationDisplayNode->SetVisibility2DOutline(visibility);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onVisibility3DChanged(int visibility)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  d->SegmentationDisplayNode->SetVisibility3D(visibility);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onOpacitySliceFillChanged(double opacity)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  d->SegmentationDisplayNode->SetOpacity2DFill(opacity);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onOpacitySliceOutlineChanged(double opacity)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  d->SegmentationDisplayNode->SetOpacity2DOutline(opacity);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onOpacity3DChanged(double opacity)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  d->SegmentationDisplayNode->SetOpacity3D(opacity);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onSliceIntersectionThicknessChanged(int thickness)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  d->SegmentationDisplayNode->SetSliceIntersectionThickness(thickness);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onRepresentation3DChanged(int index)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  // Get representation name from index
  QString representationName = d->comboBox_DisplayedRepresentation3D->itemText(index);

  d->SegmentationDisplayNode->SetPreferredDisplayRepresentationName3D(representationName.toUtf8().constData());
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onRepresentation2DChanged(int index)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer())
    {
    return;
    }

  // Get representation name from index
  QString representationName = d->comboBox_DisplayedRepresentation2D->itemText(index);

  d->SegmentationDisplayNode->SetPreferredDisplayRepresentationName2D(representationName.toUtf8().constData());
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onSegmentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);

  qDMMLSegmentsTableView* senderSegmentsTable = qobject_cast<qDMMLSegmentsTableView*>(sender());
  if (!senderSegmentsTable)
    {
    return;
    }

  QStringList selectedSegmentIDs = senderSegmentsTable->selectedSegmentIDs();
  if (selectedSegmentIDs.size() == 0)
    {
    this->setCurrentSegmentID(QString());
    }
  else
    {
    this->setCurrentSegmentID(selectedSegmentIDs[0]);
    }
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onSegmentVisibilitySliceFillChanged(int visibility)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer() || d->SelectedSegmentID.isEmpty())
    {
    return;
    }

  d->SegmentationDisplayNode->SetSegmentVisibility2DFill(d->SelectedSegmentID.toUtf8().constData(), visibility);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onSegmentVisibilitySliceOutlineChanged(int visibility)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer() || d->SelectedSegmentID.isEmpty())
    {
    return;
    }

  d->SegmentationDisplayNode->SetSegmentVisibility2DOutline(d->SelectedSegmentID.toUtf8().constData(), visibility);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onSegmentVisibility3DChanged(int visibility)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer() || d->SelectedSegmentID.isEmpty())
    {
    return;
    }

  d->SegmentationDisplayNode->SetSegmentVisibility3D(d->SelectedSegmentID.toUtf8().constData(), visibility);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onSegmentOpacitySliceFillChanged(double opacity)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer() || d->SelectedSegmentID.isEmpty())
    {
    return;
    }

  d->SegmentationDisplayNode->SetSegmentOpacity2DFill(d->SelectedSegmentID.toUtf8().constData(), opacity);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onSegmentOpacitySliceOutlineChanged(double opacity)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer() || d->SelectedSegmentID.isEmpty())
    {
    return;
    }

  d->SegmentationDisplayNode->SetSegmentOpacity2DOutline(d->SelectedSegmentID.toUtf8().constData(), opacity);
}

//-----------------------------------------------------------------------------
void qDMMLSegmentationDisplayNodeWidget::onSegmentOpacity3DChanged(double opacity)
{
  Q_D(qDMMLSegmentationDisplayNodeWidget);

  if (!d->SegmentationDisplayNode.GetPointer() || d->SelectedSegmentID.isEmpty())
    {
    return;
    }

  d->SegmentationDisplayNode->SetSegmentOpacity3D(d->SelectedSegmentID.toUtf8().constData(), opacity);
}
