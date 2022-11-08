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
#include <QDebug>
#include <QInputDialog>

// CTK includes
#include <ctkVTKScalarsToColorsView.h>

// Cjyx includes
#include "qCjyxApplication.h"
#include "qCjyxLayoutManager.h"
#include "qCjyxColorsModuleWidget.h"
#include "ui_qCjyxColorsModuleWidget.h"

// qDMMLWidget includes
#include "qDMMLColorModel.h"
#include "qDMMLThreeDView.h"
#include "qDMMLThreeDWidget.h"
#include "qDMMLSliceView.h"
#include "qDMMLSliceWidget.h"

// qDMMLWidget Colors includes
#include "qDMMLColorLegendDisplayNodeWidget.h"

// Cjyx logic includes
#include <vtkCjyxColorLogic.h>

// DMML includes
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLProceduralColorNode.h>
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLColorLegendDisplayNode.h>
#include <vtkDMMLAbstractViewNode.h>
#include <vtkDMMLAbstractDisplayableManager.h>
#include <vtkDMMLColorLegendDisplayableManager.h>

// VTK includes
#include <vtkBorderRepresentation.h>
#include <vtkColorTransferFunction.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkScalarBarWidget.h>
#include <vtkCjyxScalarBarActor.h>

// STD includes
#include <cstring>

//-----------------------------------------------------------------------------
class qCjyxColorsModuleWidgetPrivate: public Ui_qCjyxColorsModuleWidget
{
  Q_DECLARE_PUBLIC(qCjyxColorsModuleWidget);
protected:
  qCjyxColorsModuleWidget* const q_ptr;

public:
  qCjyxColorsModuleWidgetPrivate(qCjyxColorsModuleWidget& obj);
  virtual ~qCjyxColorsModuleWidgetPrivate();
  vtkCjyxColorLogic* colorLogic()const;
  void setDefaultColorNode();

  vtkWeakPointer<vtkDMMLDisplayableNode> DisplayableNode; /// Current displayable node
  vtkWeakPointer<vtkDMMLColorLegendDisplayNode> ColorLegendNode; /// color legend display node for a current displayable node
};

//-----------------------------------------------------------------------------
qCjyxColorsModuleWidgetPrivate::qCjyxColorsModuleWidgetPrivate(qCjyxColorsModuleWidget& object)
  :
  q_ptr(&object)
{

}

//-----------------------------------------------------------------------------
qCjyxColorsModuleWidgetPrivate::~qCjyxColorsModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkCjyxColorLogic* qCjyxColorsModuleWidgetPrivate::colorLogic()const
{
  Q_Q(const qCjyxColorsModuleWidget);
  return vtkCjyxColorLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
void qCjyxColorsModuleWidgetPrivate::setDefaultColorNode()
{
  Q_Q(qCjyxColorsModuleWidget);
  if (!q->dmmlScene() ||
      !this->ColorTableComboBox ||
      this->ColorTableComboBox->currentNode() != nullptr)
    {
    return;
    }
  const char *defaultID = this->colorLogic()->GetDefaultLabelMapColorNodeID();
  vtkDMMLColorNode *defaultNode = vtkDMMLColorNode::SafeDownCast(
    q->dmmlScene()->GetNodeByID(defaultID));
  this->ColorTableComboBox->setCurrentNode(defaultNode);
}

//-----------------------------------------------------------------------------
qCjyxColorsModuleWidget::qCjyxColorsModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qCjyxColorsModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qCjyxColorsModuleWidget::~qCjyxColorsModuleWidget() = default;

//-----------------------------------------------------------------------------
void qCjyxColorsModuleWidget::setup()
{
  Q_D(qCjyxColorsModuleWidget);

  d->setupUi(this);

  d->CopyColorNodeButton->setIcon(QIcon(":Icons/CjyxCopyColor.png"));

  connect(d->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
          this, SLOT(onDMMLColorNodeChanged(vtkDMMLNode*)));
  connect(d->NumberOfColorsSpinBox, SIGNAL(editingFinished()),
          this, SLOT(updateNumberOfColors()));
  connect(d->LUTRangeWidget, SIGNAL(valuesChanged(double,double)),
          this, SLOT(setLookupTableRange(double,double)));
  connect(d->CopyColorNodeButton, SIGNAL(clicked()),
          this, SLOT(copyCurrentColorNode()));

  double validBounds[4] = {VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, 0., 1.};
  d->ContinuousScalarsToColorsWidget->view()->setValidBounds(validBounds);
  d->ContinuousScalarsToColorsWidget->view()->addColorTransferFunction(nullptr);

  connect( d->DisplayableNodeComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)), this, SLOT(onDisplayableNodeChanged(vtkDMMLNode*)));

  connect(d->CreateColorLegendButton, SIGNAL(clicked()), this, SLOT(createColorLegend()));
  connect(d->DeleteColorLegendButton, SIGNAL(clicked()), this, SLOT(deleteColorLegend()));
  connect(d->UseCurrentColorsButton, SIGNAL(clicked()), this, SLOT(useCurrentColorsForColorLegend()));

  // Select the default color node
  d->setDefaultColorNode();
}

//-----------------------------------------------------------------------------
void qCjyxColorsModuleWidget::setDMMLScene(vtkDMMLScene *scene)
{
  Q_D(qCjyxColorsModuleWidget);
  this->qCjyxAbstractModuleWidget::setDMMLScene(scene);
  d->setDefaultColorNode();
}

//-----------------------------------------------------------------------------
void qCjyxColorsModuleWidget::setCurrentColorNode(vtkDMMLNode* colorNode)
{
  Q_D(qCjyxColorsModuleWidget);
  d->ColorTableComboBox->setCurrentNode(colorNode);
}

//-----------------------------------------------------------------------------
void qCjyxColorsModuleWidget::onDMMLColorNodeChanged(vtkDMMLNode* newColorNode)
{
  Q_D(qCjyxColorsModuleWidget);

  vtkDMMLColorNode* colorNode = vtkDMMLColorNode::SafeDownCast(newColorNode);
  if (!colorNode)
    {
    d->NumberOfColorsSpinBox->setEnabled(false);
    d->NumberOfColorsSpinBox->setValue(0);
    d->LUTRangeWidget->setEnabled(false);
    d->LUTRangeWidget->setValues(0.,0.);
    d->CopyColorNodeButton->setEnabled(false);
    d->ContinuousScalarsToColorsWidget->setEnabled(false);
    return;
    }

  d->CopyColorNodeButton->setEnabled(true);

  vtkDMMLColorTableNode *colorTableNode = vtkDMMLColorTableNode::SafeDownCast(colorNode);
  vtkDMMLProceduralColorNode *procColorNode = vtkDMMLProceduralColorNode::SafeDownCast(colorNode);
  if (colorTableNode && !procColorNode)
    {
    // hide the procedural display, show the color table
    // freesurfer nodes are bit of a special case, they're defined
    // procedurally, but provide a look up table rather than a
    // color transfer function
    d->ContinuousScalarsToColorsWidget->hide();
    d->ColorTableFrame->show();
    d->EditColorsCollapsibleButton->setText(tr("Discrete table"));

    // number of colors
    d->NumberOfColorsSpinBox->setEnabled(
      colorNode->GetType() == vtkDMMLColorTableNode::User);
    d->NumberOfColorsSpinBox->setValue(colorNode->GetNumberOfColors());
    Q_ASSERT(d->NumberOfColorsSpinBox->value() == colorNode->GetNumberOfColors());

    // set the range and the input for the color widget depending on if it's a freesurfer node or a color table node
    double *range = nullptr;
    d->LUTRangeWidget->setEnabled(colorNode->GetType() == vtkDMMLColorTableNode::User);
    if (colorTableNode && colorTableNode->GetLookupTable())
      {
      range = colorTableNode->GetLookupTable()->GetRange();
      }
    disconnect(d->LUTRangeWidget, SIGNAL(valuesChanged(double, double)),
      this, SLOT(setLookupTableRange(double, double)));
    if (range)
      {
      // Make the range a bit (10%) larger than the values to allow some room for
      // adjustment. More adjustment can be done by manually setting the range on the GUI.
      double rangeMargin = (range[1] - range[0])*0.1;
      if (rangeMargin == 0)
        {
        rangeMargin = 10.0;
        }
      d->LUTRangeWidget->setRange(range[0] - rangeMargin, range[1] + rangeMargin);
      d->LUTRangeWidget->setValues(range[0], range[1]);
      }
    else
      {
      d->LUTRangeWidget->setEnabled(false);
      d->LUTRangeWidget->setValues(0.,0.);
      }
    connect(d->LUTRangeWidget, SIGNAL(valuesChanged(double, double)),
      this, SLOT(setLookupTableRange(double, double)));
    // update the annotations from the superclass color node since this is a
    // color table or freesurfer color node
    int numberOfColors = colorNode->GetNumberOfColors();
    vtkNew<vtkIntArray> indexArray;
    indexArray->SetNumberOfValues(numberOfColors);
    vtkNew<vtkStringArray> stringArray;
    stringArray->SetNumberOfValues(numberOfColors);
    for (int colorIndex=0; colorIndex<numberOfColors; ++colorIndex)
      {
      indexArray->SetValue(colorIndex, colorIndex);
      stringArray->SetValue(colorIndex, colorNode->GetColorName(colorIndex));
      }
    }
  else if (procColorNode && !colorTableNode)
    {
    // hide and disable the color table display, show the continuous one
    d->NumberOfColorsSpinBox->setEnabled(false);
    d->NumberOfColorsSpinBox->setValue(0);
    d->LUTRangeWidget->setEnabled(false);
    d->LUTRangeWidget->setValues(0.,0.);
    d->ColorTableFrame->hide();
    d->ContinuousScalarsToColorsWidget->show();
    d->EditColorsCollapsibleButton->setText(tr("Continuous scale"));

    // set the color transfer function to the widget
    bool editable = procColorNode->GetType() == vtkDMMLColorNode::User; // only allow editing of user types
    d->ContinuousScalarsToColorsWidget->view()->setColorTransferFunctionToPlots(procColorNode->GetColorTransferFunction(), editable);
    d->ContinuousScalarsToColorsWidget->setEnabled(editable);
    }
  else
    {
    // not a valid type of color node
    d->LUTRangeWidget->setValues(0.,0.);
    }
}

//-----------------------------------------------------------------------------
void qCjyxColorsModuleWidget::updateNumberOfColors()
{
  Q_D(qCjyxColorsModuleWidget);
  if (!d->NumberOfColorsSpinBox->isEnabled())
    {
    return;
    }
  int newNumber = d->NumberOfColorsSpinBox->value();
  vtkDMMLColorTableNode* colorTableNode = vtkDMMLColorTableNode::SafeDownCast(
    d->ColorTableComboBox->currentNode());
  if (colorTableNode)
    {
    colorTableNode->SetNumberOfColors(newNumber);
    }
  else
    {
    qWarning() << "updateNumberOfColors: please select a discrete color table node to adjust the number of colors";
    }
}

//-----------------------------------------------------------------------------
void qCjyxColorsModuleWidget::setLookupTableRange(double min, double max)
{
  Q_D(qCjyxColorsModuleWidget);

  vtkDMMLNode *currentNode = d->ColorTableComboBox->currentNode();
  if (!currentNode)
    {
    return;
    }

  vtkDMMLColorNode* colorNode = vtkDMMLColorNode::SafeDownCast(currentNode);
  if (colorNode && colorNode->GetLookupTable())
    {
    colorNode->GetLookupTable()->SetRange(min, max);
    }
}

//-----------------------------------------------------------------------------
void qCjyxColorsModuleWidget::copyCurrentColorNode()
{
  Q_D(qCjyxColorsModuleWidget);
  vtkDMMLColorNode* currentNode = vtkDMMLColorNode::SafeDownCast(
    d->ColorTableComboBox->currentNode());
  Q_ASSERT(currentNode);
  QString newColorName = QInputDialog::getText(
    this, "Color node name",
    "Please select a new name for the color node copy",
    QLineEdit::Normal,
    QString(currentNode->GetName()) + QString("Copy"));
  if (newColorName.isEmpty())
    {
    return;
    }

  vtkDMMLColorNode *colorNode = nullptr;
  if (currentNode->IsA("vtkDMMLColorTableNode") ||
      currentNode->IsA("vtkDMMLFreeSurferProceduralColorNode"))
    {
    colorNode = d->colorLogic()->CopyNode(currentNode, newColorName.toUtf8());
    }
  else if (currentNode->IsA("vtkDMMLProceduralColorNode"))
    {
    colorNode = d->colorLogic()->CopyProceduralNode(currentNode, newColorName.toUtf8());
    }
  else
    {
    qWarning() << "CopyCurrentColorNode: current node not of a color node type "
               << "that can be copied. It's a " << currentNode->GetClassName()
               << ", not a procedural or color table node";
    return;
    }
  if (!this->dmmlScene()->AddNode(colorNode))
    {
    qWarning() << "CopyCurrentColorNode: failed to add new node to scene";
    }
  colorNode->Delete();
  if (colorNode->GetID())
    {
    d->ColorTableComboBox->setCurrentNode(colorNode);
    }
}

//-----------------------------------------------------------
void qCjyxColorsModuleWidget::onDisplayableNodeChanged(vtkDMMLNode* node)
{
  Q_D(qCjyxColorsModuleWidget);
  vtkDMMLDisplayableNode* displayableNode = vtkDMMLDisplayableNode::SafeDownCast(node);
  qvtkReconnect(d->DisplayableNode, displayableNode, vtkDMMLDisplayableNode::DisplayModifiedEvent, this, SLOT(updateColorLegendFromDMML()));
  d->DisplayableNode = displayableNode;
  updateColorLegendFromDMML();
}

//-----------------------------------------------------------
void qCjyxColorsModuleWidget::updateColorLegendFromDMML()
{
  Q_D(qCjyxColorsModuleWidget);
  vtkDMMLDisplayNode* displayNode = nullptr;
  if (d->DisplayableNode)
    {
    displayNode = d->DisplayableNode->GetDisplayNode();
    }
  d->ColorLegendNode = vtkCjyxColorLogic::GetColorLegendDisplayNode(d->DisplayableNode);
  d->CreateColorLegendButton->setVisible(displayNode && !d->ColorLegendNode);
  d->UseCurrentColorsButton->setVisible(displayNode && d->ColorLegendNode);
  d->DeleteColorLegendButton->setEnabled(d->ColorLegendNode);
  //d->ColorLegendDisplayNodeWidget->setEnabled(false);
  d->ColorLegendDisplayNodeWidget->setDMMLColorLegendDisplayNode(d->ColorLegendNode);
}

//-----------------------------------------------------------
void qCjyxColorsModuleWidget::createColorLegend()
{
  Q_D(qCjyxColorsModuleWidget);
  if (!d->DisplayableNode || !d->colorLogic() || d->ColorLegendNode)
    {
    return;
    }
  vtkDMMLDisplayNode* displayNode = d->DisplayableNode->GetDisplayNode();
  if (!displayNode)
    {
    return;
    }
  if (!displayNode->GetColorNode())
    {
    // If there is no color node selected in this display node then choose the current color node
    this->useCurrentColorsForColorLegend();
    }
  vtkDMMLColorLegendDisplayNode* colorLegendDisplayNode =
    vtkCjyxColorLogic::AddDefaultColorLegendDisplayNode(d->DisplayableNode);
}

//-----------------------------------------------------------
void qCjyxColorsModuleWidget::deleteColorLegend()
{
  Q_D(qCjyxColorsModuleWidget);
  if (!d->ColorLegendNode || !d->ColorLegendNode->GetScene())
    {
    return;
    }
  d->ColorLegendNode->GetScene()->RemoveNode(d->ColorLegendNode);
}

//-----------------------------------------------------------
void qCjyxColorsModuleWidget::useCurrentColorsForColorLegend()
{
  Q_D(qCjyxColorsModuleWidget);
  vtkDMMLColorNode* colorNode = vtkDMMLColorNode::SafeDownCast(d->ColorTableComboBox->currentNode());
  if (!colorNode)
    {
    return;
    }
  vtkDMMLDisplayNode* displayNode = d->DisplayableNode->GetDisplayNode();
  if (!displayNode)
    {
    return;
    }
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());
}

//-----------------------------------------------------------
bool qCjyxColorsModuleWidget::setEditedNode(vtkDMMLNode* node,
                                              QString role /* = QString()*/,
                                              QString context /* = QString()*/)
{
  Q_D(qCjyxColorsModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkDMMLColorNode::SafeDownCast(node))
    {
    d->ColorTableComboBox->setCurrentNode(node);
    return true;
    }

  return false;
}
