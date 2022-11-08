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

==============================================================================*/

// Qt includes
#include <QColor>

// CTK includes
#include <ctkUtils.h>

// qDMML includes
#include "qDMMLMarkupsDisplayNodeWidget.h"
#include "ui_qDMMLMarkupsDisplayNodeWidget.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLColorTableNode.h>
#include <vtkDMMLMarkupsDisplayNode.h>
#include <vtkDMMLMarkupsFiducialNode.h>
#include <vtkDMMLMarkupsNode.h>
#include <vtkDMMLSelectionNode.h>

// VTK includes
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>

//------------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Markupss
class qDMMLMarkupsDisplayNodeWidgetPrivate: public QWidget, public Ui_qDMMLMarkupsDisplayNodeWidget
{
  Q_DECLARE_PUBLIC(qDMMLMarkupsDisplayNodeWidget);

protected:
  qDMMLMarkupsDisplayNodeWidget* const q_ptr;
  typedef QWidget Superclass;

public:
  qDMMLMarkupsDisplayNodeWidgetPrivate(qDMMLMarkupsDisplayNodeWidget& object);
  void init();

  vtkWeakPointer<vtkDMMLMarkupsDisplayNode> MarkupsDisplayNode;
};

//------------------------------------------------------------------------------
qDMMLMarkupsDisplayNodeWidgetPrivate::qDMMLMarkupsDisplayNodeWidgetPrivate(qDMMLMarkupsDisplayNodeWidget& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidgetPrivate::init()
{
  Q_Q(qDMMLMarkupsDisplayNodeWidget);
  this->setupUi(q);

  // use the ctk color dialog on the color picker buttons
  this->selectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);
  this->unselectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);
  this->activeColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

  // set up the display properties
  QObject::connect(this->VisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setVisibility(bool)));
  QObject::connect(this->selectedColorPickerButton, SIGNAL(colorChanged(QColor)),
    q, SLOT(onSelectedColorPickerButtonChanged(QColor)));
  QObject::connect(this->unselectedColorPickerButton, SIGNAL(colorChanged(QColor)),
    q, SLOT(onUnselectedColorPickerButtonChanged(QColor)));
  QObject::connect(this->activeColorPickerButton, SIGNAL(colorChanged(QColor)),
    q, SLOT(onActiveColorPickerButtonChanged(QColor)));
  QObject::connect(this->glyphTypeComboBox, SIGNAL(currentIndexChanged(QString)),
    q, SLOT(onGlyphTypeComboBoxChanged(QString)));
  QObject::connect(this->glyphSizeIsAbsoluteButton, SIGNAL(toggled(bool)),
    q, SLOT(setGlyphSizeIsAbsolute(bool)));
  QObject::connect(this->glyphScaleSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onGlyphScaleSliderWidgetChanged(double)));
  QObject::connect(this->glyphSizeSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onGlyphSizeSliderWidgetChanged(double)));
  QObject::connect(this->curveLineSizeIsAbsoluteButton, SIGNAL(toggled(bool)),
    q, SLOT(setCurveLineSizeIsAbsolute(bool)));
  QObject::connect(this->curveLineThicknessSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onCurveLineThicknessSliderWidgetChanged(double)));
  QObject::connect(this->curveLineDiameterSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onCurveLineDiameterSliderWidgetChanged(double)));
  QObject::connect(this->PropertiesLabelVisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setPropertiesLabelVisibility(bool)));
  QObject::connect(this->PointLabelsVisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setPointLabelsVisibility(bool)));
  QObject::connect(this->textScaleSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onTextScaleSliderWidgetChanged(double)));

  QObject::connect(this->opacitySliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onOpacitySliderWidgetChanged(double)));

  QObject::connect(this->FillVisibilityCheckBox, SIGNAL(toggled(bool)), q, SLOT(setFillVisibility(bool)));
  QObject::connect(this->OutlineVisibilityCheckBox, SIGNAL(toggled(bool)), q, SLOT(setOutlineVisibility(bool)));
  QObject::connect(this->FillOpacitySliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onFillOpacitySliderWidgetChanged(double)));
  QObject::connect(this->OutlineOpacitySliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onOutlineOpacitySliderWidgetChanged(double)));

  this->SnapModeComboBox->addItem(tr("unconstrained"), vtkDMMLMarkupsDisplayNode::SnapModeUnconstrained);
  this->SnapModeComboBox->addItem(tr("snap to visible surface"), vtkDMMLMarkupsDisplayNode::SnapModeToVisibleSurface);
  QObject::connect(this->SnapModeComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(onSnapModeWidgetChanged()));

  QObject::connect(this->OccludedVisibilityCheckBox, SIGNAL(toggled(bool)), q, SLOT(setOccludedVisibility(bool)));
  QObject::connect(this->OccludedOpacitySliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(setOccludedOpacity(double)));

  QObject::connect(this->OccludedVisibilityCheckBox, SIGNAL(toggled(bool)), q, SLOT(setOccludedVisibility(bool)));

  this->TextFontFamilyComboBox->addItem(vtkTextProperty::GetFontFamilyAsString(VTK_ARIAL), VTK_ARIAL);
  this->TextFontFamilyComboBox->addItem(vtkTextProperty::GetFontFamilyAsString(VTK_COURIER), VTK_COURIER);
  this->TextFontFamilyComboBox->addItem(vtkTextProperty::GetFontFamilyAsString(VTK_TIMES), VTK_TIMES);

  QObject::connect(this->TextFontFamilyComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(onTextPropertyWidgetsChanged()));
  QObject::connect(this->TextBoldCheckBox,   SIGNAL(toggled(bool)), q, SLOT(onTextPropertyWidgetsChanged()));
  QObject::connect(this->TextItalicCheckBox, SIGNAL(toggled(bool)), q, SLOT(onTextPropertyWidgetsChanged()));
  QObject::connect(this->TextShadowCheckBox, SIGNAL(toggled(bool)), q, SLOT(onTextPropertyWidgetsChanged()));
  QObject::connect(this->TextBackgroundColorPickerButton, SIGNAL(colorChanged(QColor)), q, SLOT(onTextPropertyWidgetsChanged()));
  QObject::connect(this->TextBackgroundOpacitySlider, SIGNAL(valueChanged(double)), q, SLOT(onTextPropertyWidgetsChanged()));

  // populate the glyph type combo box
  if (this->glyphTypeComboBox->count() == 0)
    {
    vtkNew<vtkDMMLMarkupsDisplayNode> displayNode;
    int min = displayNode->GetMinimumGlyphType();
    int max = displayNode->GetMaximumGlyphType();
    this->glyphTypeComboBox->setEnabled(false);
    for (int i = min; i <= max; i++)
      {
      this->glyphTypeComboBox->addItem(displayNode->GetGlyphTypeAsString(i), displayNode->GetGlyphTypeAsString(i));
      }
    this->glyphTypeComboBox->setEnabled(true);
    }
  // set the default value if not set
  if (this->glyphTypeComboBox->currentIndex() == 0)
    {
    vtkNew<vtkDMMLMarkupsDisplayNode> displayNode;
    QString glyphType = QString(displayNode->GetGlyphTypeAsString());
    this->glyphTypeComboBox->setEnabled(false);
    int index =  this->glyphTypeComboBox->findData(glyphType);
    if (index != -1)
      {
      this->glyphTypeComboBox->setCurrentIndex(index);
      }
    else
      {
      // glyph types start at 1, combo box is 0 indexed
      this->glyphTypeComboBox->setCurrentIndex(displayNode->GetGlyphType() - 1);
      }
    this->glyphTypeComboBox->setEnabled(true);
    }

  if (this->MarkupsDisplayNode.GetPointer())
    {
    q->setEnabled(true);
    q->setDMMLMarkupsDisplayNode(this->MarkupsDisplayNode);
    }

  this->glyphSizeSliderWidget->setVisible(this->glyphSizeIsAbsoluteButton->isChecked());
  this->glyphScaleSliderWidget->setHidden(this->glyphSizeIsAbsoluteButton->isChecked());

  this->curveLineDiameterSliderWidget->setVisible(this->glyphSizeIsAbsoluteButton->isChecked());
  this->curveLineThicknessSliderWidget->setHidden(this->glyphSizeIsAbsoluteButton->isChecked());

  q->connect(this->ScalarsDisplayWidget, SIGNAL(scalarRangeModeValueChanged(vtkDMMLDisplayNode::ScalarRangeFlagType)),
    q, SIGNAL(scalarRangeModeValueChanged(vtkDMMLDisplayNode::ScalarRangeFlagType)));
  q->connect(this->ScalarsDisplayWidget, SIGNAL(displayNodeChanged()),
    q, SIGNAL(displayNodeChanged()));

  // Disable until a valid display node is set
  this->setEnabled(false);
}

//------------------------------------------------------------------------------
qDMMLMarkupsDisplayNodeWidget::qDMMLMarkupsDisplayNodeWidget(QWidget *_parent)
  : qDMMLWidget(_parent)
  , d_ptr(new qDMMLMarkupsDisplayNodeWidgetPrivate(*this))
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLMarkupsDisplayNodeWidget::~qDMMLMarkupsDisplayNodeWidget()
{
  this->setDMMLMarkupsDisplayNode((vtkDMMLMarkupsDisplayNode*)nullptr);
}

//------------------------------------------------------------------------------
vtkDMMLMarkupsDisplayNode* qDMMLMarkupsDisplayNodeWidget::dmmlMarkupsDisplayNode()const
{
  Q_D(const qDMMLMarkupsDisplayNodeWidget);
  return d->MarkupsDisplayNode;
}

//------------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setDMMLMarkupsDisplayNode(vtkDMMLNode* node)
{
  this->setDMMLMarkupsDisplayNode(vtkDMMLMarkupsDisplayNode::SafeDownCast(node));
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setDMMLMarkupsNode(vtkDMMLMarkupsNode* node)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  this->setDMMLMarkupsDisplayNode(
    node ? vtkDMMLMarkupsDisplayNode::SafeDownCast(node->GetDisplayNode()) : nullptr);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setDMMLMarkupsNode(vtkDMMLNode* node)
{
  this->setDMMLMarkupsNode(vtkDMMLMarkupsNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setDMMLMarkupsDisplayNode(vtkDMMLMarkupsDisplayNode* markupsDisplayNode)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (d->MarkupsDisplayNode == markupsDisplayNode)
    {
    return;
    }

  qvtkReconnect(d->MarkupsDisplayNode, markupsDisplayNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromDMML()));
  d->MarkupsDisplayNode = markupsDisplayNode;

  // Set display node to scalars display widget
  d->ScalarsDisplayWidget->setDMMLDisplayNode(markupsDisplayNode);

  d->InteractionHandleWidget->setDMMLDisplayNode(markupsDisplayNode);

  this->updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  this->setEnabled(d->MarkupsDisplayNode != nullptr);
  d->DisplayNodeViewComboBox->setDMMLDisplayNode(d->MarkupsDisplayNode);
  d->pointFiducialProjectionWidget->setDMMLMarkupsDisplayNode(d->MarkupsDisplayNode);
  d->VisibilityCheckBox->setChecked(d->MarkupsDisplayNode ? d->MarkupsDisplayNode->GetVisibility() : false);

  // update the display properties from the markups display node
  vtkSmartPointer<vtkDMMLMarkupsDisplayNode> markupsDisplayNode = d->MarkupsDisplayNode.GetPointer();
  if (!markupsDisplayNode)
    {
    // Create a temporary markups display node that we can retrieve defaults from.
    // We do not need the exact default display node properties, these are just placeholder
    // values shown in the disabled widget.
    markupsDisplayNode = vtkSmartPointer<vtkDMMLMarkupsDisplayNode>::New();
    }

  double* color = markupsDisplayNode->GetSelectedColor();
  d->selectedColorPickerButton->setColor(QColor::fromRgbF(color[0], color[1], color[2]));
  color = markupsDisplayNode->GetColor();
  d->unselectedColorPickerButton->setColor(QColor::fromRgbF(color[0], color[1], color[2]));
  color = markupsDisplayNode->GetActiveColor();
  d->activeColorPickerButton->setColor(QColor::fromRgbF(color[0], color[1], color[2]));
  d->opacitySliderWidget->setValue(markupsDisplayNode->GetOpacity());

  // glyph type
  QString glyphTypeStr = QString(markupsDisplayNode->GetGlyphTypeAsString());
  int glyphTypeIndex = d->glyphTypeComboBox->findData(glyphTypeStr);
  if (glyphTypeIndex>=0)
    {
    d->glyphTypeComboBox->setCurrentIndex(glyphTypeIndex);
    }

  d->glyphSizeIsAbsoluteButton->setChecked(!markupsDisplayNode->GetUseGlyphScale());

  // glyph scale
  double glyphScale = markupsDisplayNode->GetGlyphScale();
  // make sure that the slider can accommodate this scale
  if (glyphScale > d->glyphScaleSliderWidget->maximum())
    {
    d->glyphScaleSliderWidget->setMaximum(glyphScale);
    }
  d->glyphScaleSliderWidget->setValue(glyphScale);

  // glyph size
  double glyphSize = markupsDisplayNode->GetGlyphSize();
  // make sure that the slider can accommodate this scale
  if (glyphSize > d->glyphSizeSliderWidget->maximum())
    {
    d->glyphSizeSliderWidget->setMaximum(glyphSize);
    }
  d->glyphSizeSliderWidget->setValue(glyphSize);
  d->glyphSizeSliderWidget->setDMMLScene(markupsDisplayNode->GetScene());

  d->curveLineSizeIsAbsoluteButton->setChecked(markupsDisplayNode->GetCurveLineSizeMode() == vtkDMMLMarkupsDisplayNode::UseLineDiameter);

  // curve thickness
  double lineThicknessPercentage = markupsDisplayNode->GetLineThickness() * 100.0;
  // make sure that the slider can accommodate this scale
  if (lineThicknessPercentage > d->curveLineThicknessSliderWidget->maximum())
    {
    d->curveLineThicknessSliderWidget->setMaximum(lineThicknessPercentage);
    }
  d->curveLineThicknessSliderWidget->setValue(lineThicknessPercentage);

  // line diameter
  double lineDiameter = markupsDisplayNode->GetLineDiameter();
  // make sure that the slider can accommodate this scale
  if (lineDiameter > d->curveLineDiameterSliderWidget->maximum())
    {
    d->curveLineDiameterSliderWidget->setMaximum(lineDiameter);
    }
  d->curveLineDiameterSliderWidget->setValue(lineDiameter);

  // Only enable line size editing if not fiducial node
  bool lineSizeEnabled = (vtkDMMLMarkupsFiducialNode::SafeDownCast(markupsDisplayNode->GetDisplayableNode()) == nullptr);
  d->curveLineSizeIsAbsoluteButton->setEnabled(lineSizeEnabled);
  d->curveLineDiameterSliderWidget->setEnabled(lineSizeEnabled);
  d->curveLineThicknessSliderWidget->setEnabled(lineSizeEnabled);
  d->curveLineDiameterSliderWidget->setDMMLScene(markupsDisplayNode->GetScene());

  d->PropertiesLabelVisibilityCheckBox->setChecked(markupsDisplayNode->GetPropertiesLabelVisibility());

  d->PointLabelsVisibilityCheckBox->setChecked(markupsDisplayNode->GetPointLabelsVisibility());

  // text scale
  double textScale = markupsDisplayNode->GetTextScale();
  // make sure that the slider can accommodate this scale
  if (textScale > d->textScaleSliderWidget->maximum())
    {
    d->textScaleSliderWidget->setMaximum(textScale);
    }
  d->textScaleSliderWidget->setValue(textScale);

  bool wasBlocking = false;
  wasBlocking = d->FillVisibilityCheckBox->blockSignals(true);
  d->FillVisibilityCheckBox->setChecked(markupsDisplayNode->GetFillVisibility());
  d->FillVisibilityCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->OutlineVisibilityCheckBox->blockSignals(true);
  d->OutlineVisibilityCheckBox->setChecked(markupsDisplayNode->GetOutlineVisibility());
  d->OutlineVisibilityCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->FillOpacitySliderWidget->blockSignals(true);
  d->FillOpacitySliderWidget->setValue(markupsDisplayNode->GetFillOpacity());
  d->FillOpacitySliderWidget->blockSignals(wasBlocking);

  wasBlocking = d->OutlineOpacitySliderWidget->blockSignals(true);
  d->OutlineOpacitySliderWidget->setValue(markupsDisplayNode->GetOutlineOpacity());
  d->OutlineOpacitySliderWidget->blockSignals(wasBlocking);

  wasBlocking = d->OccludedVisibilityCheckBox->blockSignals(true);
  d->OccludedVisibilityCheckBox->setChecked(markupsDisplayNode->GetOccludedVisibility());
  d->OccludedVisibilityCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->SnapModeComboBox->blockSignals(true);
  int snapModeIndex = d->SnapModeComboBox->findData(markupsDisplayNode->GetSnapMode());
  d->SnapModeComboBox->setCurrentIndex(snapModeIndex);
  d->SnapModeComboBox->blockSignals(wasBlocking);

  wasBlocking = d->OccludedOpacitySliderWidget->blockSignals(true);
  d->OccludedOpacitySliderWidget->setValue(markupsDisplayNode->GetOccludedOpacity());
  d->OccludedOpacitySliderWidget->blockSignals(wasBlocking);

  vtkTextProperty* property = markupsDisplayNode->GetTextProperty(); // always returns valid pointer

  wasBlocking = d->TextFontFamilyComboBox->blockSignals(true);
  int fontFamilyIndex = d->TextFontFamilyComboBox->findData(property->GetFontFamily());
  d->TextFontFamilyComboBox->setCurrentIndex(fontFamilyIndex);
  d->TextFontFamilyComboBox->blockSignals(wasBlocking);

  wasBlocking = d->TextBoldCheckBox->blockSignals(true);
  d->TextBoldCheckBox->setChecked(property->GetBold());
  d->TextBoldCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->TextItalicCheckBox->blockSignals(true);
  d->TextItalicCheckBox->setChecked(property->GetItalic());
  d->TextItalicCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->TextShadowCheckBox->blockSignals(true);
  d->TextShadowCheckBox->setChecked(property->GetShadow());
  d->TextShadowCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->TextBackgroundOpacitySlider->blockSignals(true);
  d->TextBackgroundOpacitySlider->setValue(property->GetBackgroundOpacity());
  d->TextBackgroundOpacitySlider->blockSignals(wasBlocking);

  wasBlocking = d->TextBackgroundColorPickerButton->blockSignals(true);
  double textBackgroundColorF[3] = { 0.0, 0.0, 0.0 };
  property->GetBackgroundColor(textBackgroundColorF);
  d->TextBackgroundColorPickerButton->setColor(QColor::fromRgbF(textBackgroundColorF[0], textBackgroundColorF[1], textBackgroundColorF[2]));
  d->TextBackgroundColorPickerButton->blockSignals(wasBlocking);

  // Scalars
  d->ScalarsDisplayWidget->updateWidgetFromDMML();

  emit displayNodeChanged();
}

//------------------------------------------------------------------------------
vtkDMMLSelectionNode* qDMMLMarkupsDisplayNodeWidget::getSelectionNode(vtkDMMLScene *dmmlScene)
{
  vtkDMMLSelectionNode* selectionNode = nullptr;
  if (dmmlScene)
    {
    selectionNode =
      vtkDMMLSelectionNode::SafeDownCast(dmmlScene->GetNodeByID("vtkDMMLSelectionNodeSingleton"));
    }
  return selectionNode;
}

//------------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setVisibility(bool visible)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode.GetPointer())
    {
    return;
    }
  d->MarkupsDisplayNode->SetVisibility(visible);
}

//------------------------------------------------------------------------------
bool qDMMLMarkupsDisplayNodeWidget::visibility()const
{
  Q_D(const qDMMLMarkupsDisplayNodeWidget);
  return d->VisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setPropertiesLabelVisibility(bool visible)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode.GetPointer())
    {
    return;
    }
  d->MarkupsDisplayNode->SetPropertiesLabelVisibility(visible);
}

//------------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setPointLabelsVisibility(bool visible)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode.GetPointer())
    {
    return;
    }
  d->MarkupsDisplayNode->SetPointLabelsVisibility(visible);
}

//------------------------------------------------------------------------------
bool qDMMLMarkupsDisplayNodeWidget::propertiesLabelVisibility()const
{
  Q_D(const qDMMLMarkupsDisplayNodeWidget);
  return d->PropertiesLabelVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
bool qDMMLMarkupsDisplayNodeWidget::pointLabelsVisibility()const
{
  Q_D(const qDMMLMarkupsDisplayNodeWidget);
  return d->PointLabelsVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setGlyphSizeIsAbsolute(bool absolute)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode.GetPointer())
    {
    return;
    }
  d->MarkupsDisplayNode->SetUseGlyphScale(!absolute);
}

//------------------------------------------------------------------------------
bool qDMMLMarkupsDisplayNodeWidget::glyphSizeIsAbsolute()const
{
  Q_D(const qDMMLMarkupsDisplayNodeWidget);
  return d->glyphSizeIsAbsoluteButton->isChecked();
}

//------------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setCurveLineSizeIsAbsolute(bool absolute)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode.GetPointer())
    {
    return;
    }
  d->MarkupsDisplayNode->SetCurveLineSizeMode(absolute ?
    vtkDMMLMarkupsDisplayNode::UseLineDiameter : vtkDMMLMarkupsDisplayNode::UseLineThickness);
}

//------------------------------------------------------------------------------
bool qDMMLMarkupsDisplayNodeWidget::curveLineSizeIsAbsolute()const
{
  Q_D(const qDMMLMarkupsDisplayNodeWidget);
  return d->curveLineSizeIsAbsoluteButton->isChecked();
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onSelectedColorPickerButtonChanged(QColor color)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetSelectedColor(color.redF(), color.greenF(), color.blueF());
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onUnselectedColorPickerButtonChanged(QColor color)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetColor(color.redF(), color.greenF(), color.blueF());
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onActiveColorPickerButtonChanged(QColor color)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetActiveColor(color.redF(), color.greenF(), color.blueF());
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onGlyphTypeComboBoxChanged(QString value)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (value.isEmpty())
    {
    return;
    }
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetGlyphTypeFromString(value.toUtf8());
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onGlyphScaleSliderWidgetChanged(double value)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetGlyphScale(value);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onGlyphSizeSliderWidgetChanged(double value)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetGlyphSize(value);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onCurveLineThicknessSliderWidgetChanged(double percentValue)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetLineThickness(percentValue * 0.01);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onCurveLineDiameterSliderWidgetChanged(double value)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetLineDiameter(value);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onTextScaleSliderWidgetChanged(double value)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetTextScale(value);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onOpacitySliderWidgetChanged(double value)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetOpacity(value);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setMaximumMarkupsScale(double maxScale)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);

  if (maxScale > d->glyphScaleSliderWidget->maximum())
    {
    d->glyphScaleSliderWidget->setMaximum(maxScale);
    }
  if (maxScale > d->textScaleSliderWidget->maximum())
    {
    d->textScaleSliderWidget->setMaximum(maxScale);
    }
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setMaximumMarkupsSize(double maxSize)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);

  if (maxSize > d->glyphSizeSliderWidget->maximum())
    {
    d->glyphSizeSliderWidget->setMaximum(maxSize);
    }
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onInteractionCheckBoxChanged(int state)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetHandlesInteractive(state);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setFillVisibility(bool visibility)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetFillVisibility(visibility);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setOutlineVisibility(bool visibility)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetOutlineVisibility(visibility);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onFillOpacitySliderWidgetChanged(double opacity)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetFillOpacity(opacity);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onOutlineOpacitySliderWidgetChanged(double opacity)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetOutlineOpacity(opacity);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setOccludedVisibility(bool visibility)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetOccludedVisibility(visibility);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::setOccludedOpacity(double OccludedOpacity)
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetOccludedOpacity(OccludedOpacity);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onTextPropertyWidgetsChanged()
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }

  int fontFamily = d->TextFontFamilyComboBox->currentData().toInt();
  bool textBold = d->TextBoldCheckBox->isChecked();
  bool textItalic = d->TextItalicCheckBox->isChecked();
  bool textShadow = d->TextShadowCheckBox->isChecked();
  double backgroundOpacity = d->TextBackgroundOpacitySlider->value();
  QColor backgroundColor = d->TextBackgroundColorPickerButton->color();
  double backgroundColorF[3] = { backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF() };

  DMMLNodeModifyBlocker blocker(d->MarkupsDisplayNode);
  vtkSmartPointer<vtkTextProperty> textProperty = d->MarkupsDisplayNode->GetTextProperty(); // always returns valid pointer
  textProperty->SetFontFamily(fontFamily);
  textProperty->SetBold(textBold);
  textProperty->SetItalic(textItalic);
  textProperty->SetShadow(textShadow);
  textProperty->SetBackgroundOpacity(backgroundOpacity);
  textProperty->SetBackgroundColor(backgroundColorF);
}

//-----------------------------------------------------------------------------
void qDMMLMarkupsDisplayNodeWidget::onSnapModeWidgetChanged()
{
  Q_D(qDMMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  int snapMode = d->SnapModeComboBox->currentData().toInt();
  d->MarkupsDisplayNode->SetSnapMode(snapMode);
}
