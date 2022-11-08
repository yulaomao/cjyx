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
#include <QColor>

// CTK includes
#include <ctkUtils.h>

// qDMML includes
#include "qDMMLModelDisplayNodeWidget.h"
#include "ui_qDMMLModelDisplayNodeWidget.h"

// Subject hierarchy includes
#include <qCjyxSubjectHierarchyFolderPlugin.h>
#include <qCjyxSubjectHierarchyPluginHandler.h>

// DMML include
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>

static const int REPRESENTATION_POINTS = 0;
static const int REPRESENTATION_WIREFRAME = 1;
static const int REPRESENTATION_SURFACE = 2;

//------------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Models
class qDMMLModelDisplayNodeWidgetPrivate: public QWidget, public Ui_qDMMLModelDisplayNodeWidget
{
  Q_DECLARE_PUBLIC(qDMMLModelDisplayNodeWidget);

protected:
  qDMMLModelDisplayNodeWidget* const q_ptr;
  typedef QWidget Superclass;

public:
  qDMMLModelDisplayNodeWidgetPrivate(qDMMLModelDisplayNodeWidget& object);
  void init();

  bool IsUpdatingWidgetFromDMML{ false };

  QList<vtkDMMLModelDisplayNode*> modelDisplayNodesFromSelection()const;
  QList<vtkDMMLDisplayNode*> displayNodesFromSelection()const;

  // Current display nodes, used to display the current display properties in the widget.
  // They are the first display node that belong to the first current subject hierarchy item.
  // - The model-specific display node pointer is a quick accessor to the model specific
  //   features if the first subject hierarchy item belongs to a model node
  // - The generic display node is an accessor to the generic display properties. it is
  //   needed because selection of both folders and models are supported
  vtkWeakPointer<vtkDMMLModelDisplayNode> CurrentModelDisplayNode;
  vtkWeakPointer<vtkDMMLDisplayNode> CurrentDisplayNode;

  vtkSmartPointer<vtkProperty> Property;
  QList<vtkIdType> CurrentSubjectHierarchyItemIDs;
};

//------------------------------------------------------------------------------
qDMMLModelDisplayNodeWidgetPrivate::qDMMLModelDisplayNodeWidgetPrivate(qDMMLModelDisplayNodeWidget& object)
  : q_ptr(&object)
{
  this->Property = vtkSmartPointer<vtkProperty>::New();
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidgetPrivate::init()
{
  Q_Q(qDMMLModelDisplayNodeWidget);
  this->setupUi(q);

  this->MaterialPropertyWidget->setProperty(this->Property);
  q->qvtkConnect(this->Property, vtkCommand::ModifiedEvent,
    q, SLOT(updateDisplayNodesFromProperty()));

  q->connect(this->VisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setVisibility(bool)));
  q->connect(this->ClippingCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setClipping(bool)));
  q->connect(this->ConfigureClippingPushButton, SIGNAL(clicked()),
    q, SIGNAL(clippingConfigurationButtonClicked()));
  this->ConfigureClippingPushButton->setVisible(false);

  q->connect(this->SliceIntersectionVisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setSliceIntersectionVisible(bool)));
  q->connect(this->SliceDisplayModeComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setSliceDisplayMode(int)));
  q->connect(this->SliceIntersectionThicknessSpinBox, SIGNAL(valueChanged(int)),
    q, SLOT(setSliceIntersectionThickness(int)));
  q->connect(this->SliceIntersectionOpacitySlider, SIGNAL(valueChanged(double)),
    q, SLOT(setSliceIntersectionOpacity(double)));
  q->connect(this->DistanceToColorNodeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    q, SLOT(setDistanceToColorNode(vtkDMMLNode*)));

  q->connect(this->RepresentationComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setRepresentation(int)));
  q->connect(this->PointSizeSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(setPointSize(double)));
  q->connect(this->LineWidthSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(setLineWidth(double)));
  q->connect(this->ShowFacesComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setShowFaces(int)));
  q->connect(this->ColorPickerButton, SIGNAL(colorChanged(QColor)),
    q, SLOT(setColor(QColor)));
  q->connect(this->OpacitySliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(setOpacity(double)));
  q->connect(this->EdgeColorPickerButton, SIGNAL(colorChanged(QColor)),
    q, SLOT(setEdgeColor(QColor)));

  q->connect(this->BackfaceHueOffsetSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setBackfaceHueOffset(double)));
  q->connect(this->BackfaceSaturationOffsetSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setBackfaceSaturationOffset(double)));
  q->connect(this->BackfaceBrightnessOffsetSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setBackfaceBrightnessOffset(double)));

  q->connect(this->LightingCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setLighting(bool)));
  q->connect(this->InterpolationComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setInterpolation(int)));

  q->connect(this->ScalarsDisplayWidget, SIGNAL(scalarRangeModeValueChanged(vtkDMMLDisplayNode::ScalarRangeFlagType)),
    q, SIGNAL(scalarRangeModeValueChanged(vtkDMMLDisplayNode::ScalarRangeFlagType)));
  q->connect(this->ScalarsDisplayWidget, SIGNAL(displayNodeChanged()),
    q, SIGNAL(displayNodeChanged()));

  if (this->CurrentModelDisplayNode.GetPointer())
    {
    q->setEnabled(true);
    q->setDMMLModelDisplayNode(this->CurrentModelDisplayNode);
    }
}

//------------------------------------------------------------------------------
QList<vtkDMMLModelDisplayNode*> qDMMLModelDisplayNodeWidgetPrivate::modelDisplayNodesFromSelection()const
{
  Q_Q(const qDMMLModelDisplayNodeWidget);
  QList<vtkDMMLModelDisplayNode*> modelDisplayNodes;
  if (q->dmmlScene() == nullptr)
    {
    return modelDisplayNodes;
    }
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(q->dmmlScene());
  if (!shNode)
    {
    return modelDisplayNodes;
    }

  foreach (vtkIdType itemID, this->CurrentSubjectHierarchyItemIDs)
    {
    // Can be set from model or folder
    vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (modelNode)
      {
      // Note: Formerly the last display node was chosen that was model display node type (or the proper fiber type)
      vtkDMMLModelDisplayNode* modelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(
        modelNode->GetDisplayNode());
      if (modelDisplayNode)
        {
        modelDisplayNodes << modelDisplayNode;
        }
      }
    }
  return modelDisplayNodes;
}

//------------------------------------------------------------------------------
QList<vtkDMMLDisplayNode*> qDMMLModelDisplayNodeWidgetPrivate::displayNodesFromSelection()const
{
  Q_Q(const qDMMLModelDisplayNodeWidget);
  QList<vtkDMMLDisplayNode*> displayNodes;
  if (q->dmmlScene() == nullptr)
    {
    return displayNodes;
    }
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(q->dmmlScene());
  if (!shNode)
    {
    return displayNodes;
    }

  foreach (vtkIdType itemID, this->CurrentSubjectHierarchyItemIDs)
    {
    // Can be set from model or folder
    vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    vtkDMMLDisplayNode* displayNode = vtkDMMLDisplayNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (displayNode)
      {
      displayNodes << displayNode;
      }
    else if (displayableNode)
      {
      // Note: Formerly the last display node was chosen that was model display node type (or the proper fiber type)
      displayNode = displayableNode->GetDisplayNode();
      if (displayNode)
        {
        displayNodes << displayNode;
        }
      }
    }
  return displayNodes;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qDMMLModelDisplayNodeWidget::qDMMLModelDisplayNodeWidget(QWidget* _parent)
  : qDMMLWidget(_parent)
  , d_ptr(new qDMMLModelDisplayNodeWidgetPrivate(*this))
{
  Q_D(qDMMLModelDisplayNodeWidget);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLModelDisplayNodeWidget::~qDMMLModelDisplayNodeWidget() = default;

//---------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setDMMLScene(vtkDMMLScene* newScene)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  if (this->dmmlScene() == newScene)
    {
    return;
    }

  this->Superclass::setDMMLScene(newScene);
  this->qvtkReconnect(this->dmmlScene(), newScene, vtkDMMLScene::EndBatchProcessEvent,
    this, SLOT(updateWidgetFromDMML()));

  if (this->dmmlScene())
    {
    this->updateWidgetFromDMML();
    }
}

//------------------------------------------------------------------------------
vtkDMMLModelDisplayNode* qDMMLModelDisplayNodeWidget::dmmlModelDisplayNode()const
{
  Q_D(const qDMMLModelDisplayNodeWidget);
  return d->CurrentModelDisplayNode;
}

//------------------------------------------------------------------------------
vtkDMMLDisplayNode* qDMMLModelDisplayNodeWidget::dmmlDisplayNode()const
{
  Q_D(const qDMMLModelDisplayNodeWidget);
  return d->CurrentDisplayNode;
}

//------------------------------------------------------------------------------
vtkIdType qDMMLModelDisplayNodeWidget::currentSubjectHierarchyItemID()const
{
  Q_D(const qDMMLModelDisplayNodeWidget);
  if (d->CurrentSubjectHierarchyItemIDs.empty())
    {
    return vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  return d->CurrentSubjectHierarchyItemIDs[0];
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setCurrentSubjectHierarchyItemID(vtkIdType currentItemID)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  if ( d->CurrentSubjectHierarchyItemIDs.size() == 1
    && d->CurrentSubjectHierarchyItemIDs[0] == currentItemID )
    {
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->dmmlScene());
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  d->CurrentSubjectHierarchyItemIDs.clear();
  d->CurrentSubjectHierarchyItemIDs << currentItemID;

  if (!currentItemID)
    {
    return;
    }

  vtkDMMLDisplayNode* displayNode = nullptr;

  // Can be set from model or folder
  vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (modelNode)
    {
    // Note: Formerly the last display node was chosen that was model display node type (or the proper fiber type)
    displayNode = modelNode->GetDisplayNode();
    }

  // get folder plugin (can fail if subject hierarchy logic is not instantiated)
  qCjyxSubjectHierarchyFolderPlugin* folderPlugin = qobject_cast<qCjyxSubjectHierarchyFolderPlugin*>(
    qCjyxSubjectHierarchyPluginHandler::instance()->pluginByName("Folder") );
  if (folderPlugin && folderPlugin->canOwnSubjectHierarchyItem(currentItemID) > 0.0)
    {
    displayNode = folderPlugin->createDisplayNodeForItem(currentItemID);
    }

  this->setDMMLDisplayNode(displayNode);
}

//------------------------------------------------------------------------------
QList<vtkIdType> qDMMLModelDisplayNodeWidget::currentSubjectHierarchyItemIDs()const
{
  Q_D(const qDMMLModelDisplayNodeWidget);
  return d->CurrentSubjectHierarchyItemIDs;
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setCurrentSubjectHierarchyItemIDs(QList<vtkIdType> currentItemIDs)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  // Set first item as current item (that the widget displays)
  if (currentItemIDs.size() > 0)
    {
    this->setCurrentSubjectHierarchyItemID(currentItemIDs[0]);
    }

  d->CurrentSubjectHierarchyItemIDs = currentItemIDs;

  // Set display nodes to scalars display widget
  d->ScalarsDisplayWidget->setDMMLDisplayNodes(d->displayNodesFromSelection());
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setDMMLModelDisplayNode(vtkDMMLNode* node)
{
  this->setDMMLModelDisplayNode(vtkDMMLModelDisplayNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setDMMLModelDisplayNode(vtkDMMLModelDisplayNode* modelDisplayNode)
{
  this->setDMMLDisplayNode(modelDisplayNode);
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setDMMLDisplayNode(vtkDMMLDisplayNode* displayNode)
{
  Q_D(qDMMLModelDisplayNodeWidget);
  if (d->CurrentDisplayNode == displayNode)
    {
    return;
    }

  qvtkReconnect(d->CurrentDisplayNode, displayNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromDMML()));
  d->CurrentDisplayNode = displayNode;
  d->CurrentModelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(displayNode);

  // Set display node to scalars display widget
  d->ScalarsDisplayWidget->setDMMLDisplayNode(displayNode);

  this->updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLModelDisplayNodeWidget);
  this->setEnabled(d->CurrentDisplayNode.GetPointer() != nullptr);
  if (!d->CurrentDisplayNode.GetPointer())
    {
    emit displayNodeChanged();
    return;
    }

  if (d->IsUpdatingWidgetFromDMML)
    {
    return;
    }
  d->IsUpdatingWidgetFromDMML = true;

  d->VisibilityCheckBox->setChecked(d->CurrentDisplayNode->GetVisibility());
  d->DisplayNodeViewComboBox->setDMMLDisplayNode(d->CurrentDisplayNode);
  d->ClippingCheckBox->setChecked(d->CurrentDisplayNode->GetClipping());
  d->SliceIntersectionVisibilityCheckBox->setChecked(d->CurrentDisplayNode->GetVisibility2D());
  d->SliceIntersectionThicknessSpinBox->setValue(d->CurrentDisplayNode->GetSliceIntersectionThickness());
  bool showSliceIntersectionThickness =
    (d->CurrentModelDisplayNode ? d->CurrentModelDisplayNode->GetSliceDisplayMode() == vtkDMMLModelDisplayNode::SliceDisplayIntersection : true);
  d->SliceIntersectionThicknessSpinBox->setEnabled(showSliceIntersectionThickness);
  d->SliceIntersectionOpacitySlider->setValue(d->CurrentDisplayNode->GetSliceIntersectionOpacity());

  d->SliceDisplayModeComboBox->setEnabled(d->CurrentModelDisplayNode);
  if (d->CurrentModelDisplayNode)
    {
    d->SliceDisplayModeComboBox->setCurrentIndex(d->CurrentModelDisplayNode->GetSliceDisplayMode());
    }

  bool wasBlocking = d->DistanceToColorNodeComboBox->blockSignals(true);
  if (d->DistanceToColorNodeComboBox->dmmlScene() != this->dmmlScene())
    {
    d->DistanceToColorNodeComboBox->setDMMLScene(this->dmmlScene());
    }
  if ( d->CurrentModelDisplayNode
    && d->DistanceToColorNodeComboBox->currentNodeID() != d->CurrentModelDisplayNode->GetDistanceEncodedProjectionColorNodeID() )
    {
    d->DistanceToColorNodeComboBox->setCurrentNodeID(d->CurrentModelDisplayNode->GetDistanceEncodedProjectionColorNodeID());
    }
  d->DistanceToColorNodeComboBox->setEnabled( d->CurrentModelDisplayNode &&
    d->CurrentModelDisplayNode->GetSliceDisplayMode() == vtkDMMLModelDisplayNode::SliceDisplayDistanceEncodedProjection );
  d->DistanceToColorNodeComboBox->blockSignals(wasBlocking);

  // Representation
  switch (d->CurrentDisplayNode->GetRepresentation())
    {
    case REPRESENTATION_POINTS:
      d->RepresentationComboBox->setCurrentIndex(0);
      break;
    case REPRESENTATION_WIREFRAME:
      d->RepresentationComboBox->setCurrentIndex(1);
      break;
    case REPRESENTATION_SURFACE:
      if (d->CurrentDisplayNode->GetEdgeVisibility())
        {
        d->RepresentationComboBox->setCurrentIndex(3);
        }
      else
        {
        d->RepresentationComboBox->setCurrentIndex(2);
        }
      break;
    }

  d->PointSizeSliderWidget->setValue(d->CurrentDisplayNode->GetPointSize());
  bool showPointSize = d->CurrentDisplayNode->GetRepresentation() == REPRESENTATION_POINTS;
  d->PointSizeSliderWidget->setEnabled(showPointSize);

  d->LineWidthSliderWidget->setValue(d->CurrentDisplayNode->GetLineWidth());
  // Enable line width editing in REPRESENTATION_SURFACE mode regardless of edge visibility,
  // because if the model consists only of lines then line width will make a difference
  // even if edge visibility is disabled.
  bool showLineWidth = (d->CurrentDisplayNode->GetRepresentation() == REPRESENTATION_WIREFRAME
    || d->CurrentDisplayNode->GetRepresentation() == REPRESENTATION_SURFACE);
  d->LineWidthSliderWidget->setEnabled(showLineWidth);

  if (!d->CurrentDisplayNode->GetFrontfaceCulling() && d->CurrentDisplayNode->GetBackfaceCulling())
    {
    // show only front faces
    d->ShowFacesComboBox->setCurrentIndex(1);
    }
  else if (d->CurrentDisplayNode->GetFrontfaceCulling() && !d->CurrentDisplayNode->GetBackfaceCulling())
    {
    // show only back faces
    d->ShowFacesComboBox->setCurrentIndex(2);
    }
  else
    {
    // show all faces
    d->ShowFacesComboBox->setCurrentIndex(0);
    }

  double* c = d->CurrentDisplayNode->GetColor();
  bool wasBlocked = d->ColorPickerButton->blockSignals(true);
  d->ColorPickerButton->setColor(QColor::fromRgbF(qMin(c[0], 1.), qMin(c[1], 1.), qMin(c[2], 1.)));
  d->ColorPickerButton->blockSignals(wasBlocked);

  d->OpacitySliderWidget->setValue(d->CurrentDisplayNode->GetOpacity());
  double* ec = d->CurrentDisplayNode->GetEdgeColor();
  d->EdgeColorPickerButton->setColor(
    QColor::fromRgbF(qMin(ec[0], 1.), qMin(ec[1], 1.), qMin(ec[2], 1.)));
  bool showEdgeColor =
    (d->CurrentDisplayNode->GetRepresentation() == REPRESENTATION_SURFACE && d->CurrentDisplayNode->GetEdgeVisibility());
  d->EdgeColorPickerButton->setEnabled(showEdgeColor);

  if (d->CurrentModelDisplayNode)
    {
    double hsvOffset[3];
    d->CurrentModelDisplayNode->GetBackfaceColorHSVOffset(hsvOffset);
    QSignalBlocker blocker1(d->BackfaceHueOffsetSpinBox);
    QSignalBlocker blocker2(d->BackfaceSaturationOffsetSpinBox);
    QSignalBlocker blocker3(d->BackfaceSaturationOffsetSpinBox);
    d->BackfaceHueOffsetSpinBox->setValue(hsvOffset[0]);
    d->BackfaceSaturationOffsetSpinBox->setValue(hsvOffset[1]);
    d->BackfaceBrightnessOffsetSpinBox->setValue(hsvOffset[2]);
    }
  d->BackfaceHueOffsetSpinBox->setEnabled(d->CurrentModelDisplayNode != nullptr);
  d->BackfaceSaturationOffsetSpinBox->setEnabled(d->CurrentModelDisplayNode != nullptr);
  d->BackfaceBrightnessOffsetSpinBox->setEnabled(d->CurrentModelDisplayNode != nullptr);

  d->LightingCheckBox->setChecked(d->CurrentDisplayNode->GetLighting());
  d->InterpolationComboBox->setCurrentIndex(d->CurrentDisplayNode->GetInterpolation());

  // Material
  d->Property->SetAmbient(d->CurrentDisplayNode->GetAmbient());
  d->Property->SetDiffuse(d->CurrentDisplayNode->GetDiffuse());
  d->Property->SetSpecular(d->CurrentDisplayNode->GetSpecular());
  d->Property->SetSpecularPower(d->CurrentDisplayNode->GetPower());
  d->Property->SetMetallic(d->CurrentDisplayNode->GetMetallic());
  d->Property->SetRoughness(d->CurrentDisplayNode->GetRoughness());

  // Scalars
  d->ScalarsDisplayWidget->updateWidgetFromDMML();

  d->IsUpdatingWidgetFromDMML = false;

  emit displayNodeChanged();
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setVisibility(bool visible)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetVisibility(visible);
    }
}

//------------------------------------------------------------------------------
bool qDMMLModelDisplayNodeWidget::visibility()const
{
  Q_D(const qDMMLModelDisplayNodeWidget);
  return d->VisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setClipping(bool clip)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetClipping(clip);
    }
  emit clippingToggled(clip);
}

//------------------------------------------------------------------------------
bool qDMMLModelDisplayNodeWidget::clipping()const
{
  Q_D(const qDMMLModelDisplayNodeWidget);
  return d->ClippingCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setSliceIntersectionVisible(bool visible)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetVisibility2D(visible);
    }
}

//------------------------------------------------------------------------------
bool qDMMLModelDisplayNodeWidget::sliceIntersectionVisible()const
{
  Q_D(const qDMMLModelDisplayNodeWidget);
  return d->SliceIntersectionVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setSliceIntersectionThickness(int thickness)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetSliceIntersectionThickness(thickness);
    }
}

//------------------------------------------------------------------------------
int qDMMLModelDisplayNodeWidget::sliceIntersectionThickness()const
{
  Q_D(const qDMMLModelDisplayNodeWidget);
  return d->SliceIntersectionThicknessSpinBox->value();
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setSliceIntersectionOpacity(double opacity)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetSliceIntersectionOpacity(opacity);
    }
}

//------------------------------------------------------------------------------
double qDMMLModelDisplayNodeWidget::sliceIntersectionOpacity()const
{
  Q_D(const qDMMLModelDisplayNodeWidget);
  return d->SliceIntersectionOpacitySlider->value();
}

//------------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::updateDisplayNodesFromProperty()
{
  Q_D(qDMMLModelDisplayNodeWidget);

  if (d->IsUpdatingWidgetFromDMML)
    {
    return;
    }

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    int wasModifying = displayNode->StartModify();
    // Lighting
    displayNode->SetLighting(d->Property->GetLighting());
    displayNode->SetInterpolation(d->Property->GetInterpolation());
    displayNode->SetShading(d->Property->GetShading());
    // Material
    displayNode->SetAmbient(d->Property->GetAmbient());
    displayNode->SetDiffuse(d->Property->GetDiffuse());
    displayNode->SetSpecular(d->Property->GetSpecular());
    displayNode->SetPower(d->Property->GetSpecularPower());
    displayNode->SetMetallic(d->Property->GetMetallic());
    displayNode->SetRoughness(d->Property->GetRoughness());
    displayNode->EndModify(wasModifying);
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setRepresentation(int newRepresentation)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    switch (newRepresentation)
      {
      case 0: // points
        displayNode->SetRepresentation(REPRESENTATION_POINTS);
        break;
      case 1: // wireframe
        displayNode->SetRepresentation(REPRESENTATION_WIREFRAME);
        break;
      case 2: // surface
      case 3: // surface with edges
        {
        int wasModified = displayNode->StartModify();
        displayNode->SetRepresentation(REPRESENTATION_SURFACE);
        displayNode->SetEdgeVisibility(newRepresentation == 3);
        displayNode->EndModify(wasModified);
        break;
        }
      }
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setSliceDisplayMode(int newMode)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLModelDisplayNode*> modelDisplayNodesInSelection = d->modelDisplayNodesFromSelection();
  foreach (vtkDMMLModelDisplayNode* modelDisplayNode, modelDisplayNodesInSelection)
    {
    int wasModified = modelDisplayNode->StartModify();
    // Select a color node if none is selected yet
    if (modelDisplayNode->GetSliceDisplayMode()
      != vtkDMMLModelDisplayNode::SliceDisplayDistanceEncodedProjection
      && newMode == vtkDMMLModelDisplayNode::SliceDisplayDistanceEncodedProjection
      && modelDisplayNode->GetDistanceEncodedProjectionColorNodeID() == nullptr)
      {
      modelDisplayNode->SetAndObserveDistanceEncodedProjectionColorNodeID("vtkDMMLProceduralColorNodeRedGreenBlue");
      }
    modelDisplayNode->SetSliceDisplayMode(newMode);
    modelDisplayNode->EndModify(wasModified);
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setPointSize(double newPointSize)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetPointSize(newPointSize);
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setLineWidth(double newLineWidth)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetLineWidth(newLineWidth);
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setBackfaceHueOffset(double newOffset)
{
  Q_D(qDMMLModelDisplayNodeWidget);
  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach(vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    vtkDMMLModelDisplayNode* modelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(displayNode);
    if (!modelDisplayNode)
      {
      continue;
      }
    double hsvOffset[3];
    modelDisplayNode->GetBackfaceColorHSVOffset(hsvOffset);
    modelDisplayNode->SetBackfaceColorHSVOffset(newOffset, hsvOffset[1], hsvOffset[2]);
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setBackfaceSaturationOffset(double newOffset)
{
  Q_D(qDMMLModelDisplayNodeWidget);
  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach(vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    vtkDMMLModelDisplayNode* modelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(displayNode);
    if (!modelDisplayNode)
      {
      continue;
      }
    double hsvOffset[3];
    modelDisplayNode->GetBackfaceColorHSVOffset(hsvOffset);
    modelDisplayNode->SetBackfaceColorHSVOffset(hsvOffset[0], newOffset, hsvOffset[2]);
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setBackfaceBrightnessOffset(double newOffset)
{
  Q_D(qDMMLModelDisplayNodeWidget);
  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach(vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    vtkDMMLModelDisplayNode* modelDisplayNode = vtkDMMLModelDisplayNode::SafeDownCast(displayNode);
    if (!modelDisplayNode)
      {
      continue;
      }
    double hsvOffset[3];
    modelDisplayNode->GetBackfaceColorHSVOffset(hsvOffset);
    modelDisplayNode->SetBackfaceColorHSVOffset(hsvOffset[0], hsvOffset[1], newOffset);
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setShowFaces(int newShowFaces)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    int wasModified = displayNode->StartModify();
    switch (newShowFaces)
      {
      case 0: // show all
        displayNode->SetFrontfaceCulling(false);
        displayNode->SetBackfaceCulling(false);
        break;
      case 1: // show front only
        displayNode->SetFrontfaceCulling(false);
        displayNode->SetBackfaceCulling(true);
        break;
      case 2: // show back only
        displayNode->SetFrontfaceCulling(true);
        displayNode->SetBackfaceCulling(false);
        break;
      }
    displayNode->EndModify(wasModified);
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setColor(const QColor& newColor)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    double* oldColorArray = displayNode->GetColor();
    QColor oldColor = QColor::fromRgbF(oldColorArray[0], oldColorArray[1], oldColorArray[2]);
    if (oldColor != newColor)
      {
      displayNode->SetColor(newColor.redF(), newColor.greenF(), newColor.blueF());
      // Solid color is set, therefore disable scalar visibility
      // (otherwise color would come from the scalar value and colormap).
      displayNode->SetScalarVisibility(false);
      }
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setOpacity(double newOpacity)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetOpacity(newOpacity);
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setEdgeVisibility(bool newEdgeVisibility)
{
  Q_D(qDMMLModelDisplayNodeWidget);
  if (!d->CurrentDisplayNode.GetPointer())
  {
    return;
  }
  d->Property->SetEdgeVisibility(newEdgeVisibility);
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setEdgeColor(const QColor& newColor)
{
  Q_D(qDMMLModelDisplayNodeWidget);

  QList<vtkDMMLDisplayNode*> displayNodesInSelection = d->displayNodesFromSelection();
  foreach (vtkDMMLDisplayNode* displayNode, displayNodesInSelection)
    {
    displayNode->SetEdgeColor(newColor.redF(), newColor.greenF(), newColor.blueF());
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setLighting(bool newLighting)
{
  Q_D(qDMMLModelDisplayNodeWidget);
  if (!d->CurrentDisplayNode.GetPointer())
  {
    return;
  }
  d->Property->SetLighting(newLighting);
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setInterpolation(int newInterpolation)
{
  Q_D(qDMMLModelDisplayNodeWidget);
  if (!d->CurrentDisplayNode.GetPointer())
    {
    return;
    }
  switch (newInterpolation)
    {
    case vtkDMMLDisplayNode::FlatInterpolation: d->Property->SetInterpolationToFlat(); break;
    case vtkDMMLDisplayNode::GouraudInterpolation: d->Property->SetInterpolationToGouraud(); break;
    case vtkDMMLDisplayNode::PhongInterpolation: d->Property->SetInterpolationToPhong(); break;
    case vtkDMMLDisplayNode::PBRInterpolation: d->Property->SetInterpolationToPBR(); break;
    default:
      qWarning() << Q_FUNC_INFO << " failed: invalid interpolation mode " << newInterpolation;
    }
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setDistanceToColorNode(vtkDMMLNode* colorNode)
{
  Q_D(qDMMLModelDisplayNodeWidget);
  if (!d->CurrentModelDisplayNode.GetPointer())
    {
    return;
    }
  d->CurrentModelDisplayNode->SetAndObserveDistanceEncodedProjectionColorNodeID(colorNode ? colorNode->GetID() : nullptr);
}

// --------------------------------------------------------------------------
bool qDMMLModelDisplayNodeWidget::clippingConfigurationButtonVisible()const
{
  Q_D(const qDMMLModelDisplayNodeWidget);
  return d->ConfigureClippingPushButton->isVisible();
}

// --------------------------------------------------------------------------
void qDMMLModelDisplayNodeWidget::setClippingConfigurationButtonVisible(bool show)
{
  Q_D(qDMMLModelDisplayNodeWidget);
  d->ConfigureClippingPushButton->setVisible(show);
}
