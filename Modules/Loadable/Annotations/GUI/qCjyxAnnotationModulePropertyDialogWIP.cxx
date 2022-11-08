#include "qCjyxAnnotationModulePropertyDialog.h"
#include "ui_qCjyxAnnotationModulePropertyDialog.h"

// Qt includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QComboBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QMotifStyle>

#include "Logic/vtkCjyxAnnotationModuleLogic.h"
#include "vtkDMMLAnnotationNode.h"
#include "vtkDMMLAnnotationPointDisplayNode.h"
#include "vtkDMMLAnnotationLineDisplayNode.h"
#include "vtkDMMLAnnotationTextDisplayNode.h"
#include "vtkDMMLAnnotationControlPointsNode.h"
#include "vtkDMMLAnnotationFiducialNode.h"
#include "vtkDMMLAnnotationRulerNode.h"
#include "vtkDMMLAnnotationLinesNode.h"
#include "vtkDMMLAnnotationROINode.h"

//------------------------------------------------------------------------------
qCjyxAnnotationModulePropertyDialog::~qCjyxAnnotationModulePropertyDialog()
{
  this->m_id = 0;
  this->m_logic = 0;
}

//------------------------------------------------------------------------------
qCjyxAnnotationModulePropertyDialog::qCjyxAnnotationModulePropertyDialog(const char * id, vtkCjyxAnnotationModuleLogic* logic)
{
  this->m_id = vtkStdString(id);
  this->m_logic = logic;

  // now build the user interface
  ui.setupUi(this);

  if (this->m_logic->IsAnnotationHierarchyNode(id))
    {
    // hierarchies

    // TODO show the display widget

    //ui.tabWidget->hide();

    }
  else
    {
    // annotations

    this->initialize();

    }
  ui.toolBox->setStyle(new QMotifStyle());

  // create the slot and signal connections
  this->createConnection();


}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::initialize()
{
  // backup the current annotationNode
  this->m_logic->BackupAnnotationNode(this->m_id);

  // build the typeLabelText including name and id of the annotation
  QString * typeLabelText = new QString("Name: ");
  typeLabelText->append(this->m_logic->GetAnnotationName(this->m_id));
  typeLabelText->append(" (");
  typeLabelText->append(this->m_id);
  typeLabelText->append(")");

  ui.typeLabel->setText(*typeLabelText);

  // update the typeIcon according to the annotation type
  QIcon icon = QIcon(this->m_logic->GetAnnotationIcon(this->m_id.c_str()));
  QPixmap pixmap = icon.pixmap(32, 32);

  ui.typeIcon->setPixmap(pixmap);

  // load the current annotation text
  vtkStdString text = this->m_logic->GetAnnotationText(this->m_id.c_str());

  ui.annotationTextEdit->setText(text.c_str());

  // load the current measurement
  const char * measurement = this->m_logic->GetAnnotationMeasurement(
      this->m_id.c_str(), true);

  ui.measurementLineEdit->setText(measurement);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::createConnection()
{
  this->connect(this, SIGNAL(rejected()), this, SLOT(onDialogRejected()));
  this->connect(this, SIGNAL(accepted()), this, SLOT(onDialogAccepted()));

  // text
  this->connect(ui.annotationTextEdit, SIGNAL(textChanged()), this,
      SLOT(onTextChanged()));

  this->connect(ui.textScaleSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
      this, SLOT(onTextScaleChanged(double)));

  this->connect(ui.textSelectedColorPickerButton, SIGNAL(colorChanged(QColor)),
      this, SLOT(onTextSelectedColorChanged(QColor)));

  this->connect(ui.textUnselectedColorPickerButton, SIGNAL(colorChanged(QColor)),
      this, SLOT(onTextUnselectedColorChanged(QColor)));

  this->connect(ui.textOpacitySliderSpinBoxWidget, SIGNAL(valueChanged(double)),
      this, SLOT(onTextOpacityChanged(double)));

  // point
  //this->connect(ui.pointsTableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
  //              this, SLOT(onPointsTableWidgetChanged(QTableWidgetItem*)));
  this->connect(ui.pointUnselectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onPointColorChanged(QColor)));
  this->connect(ui.pointSelectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onPointSelectedColorChanged(QColor)));

  this->connect(ui.pointSizeSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onPointSizeChanged(double)));

  this->connect(ui.pointGlyphTypeComboBox, SIGNAL(currentIndexChanged(QString)),
                this, SLOT(onPointGlyphChanged(QString)));

//  this->connect(ui.pointOpacitySliderSpinBoxWidget, SIGNAL(valueChanged(double)),
  //              this, SLOT(onPointOpacityChanged(double)));
  //this->connect(ui.pointAmbientSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
  //              this, SLOT(onPointAmbientChanged(double)));
  //this->connect(ui.pointDiffuseSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
   //             this, SLOT(onPointDiffuseChanged(double)));
  //this->connect(ui.pointSpecularSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
  //              this, SLOT(onPointSpecularChanged(double)));

  // line
  this->connect(ui.lineUnselectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onLineColorChanged(QColor)));
  this->connect(ui.lineSelectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onLineSelectedColorChanged(QColor)));

  this->connect(ui.lineLabelPositionSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onLineLabelPositionChanged(double)));
  this->connect(ui.lineLabelVisibilityCheckBox, SIGNAL(stateChanged(int)),
                this, SLOT(onLineLabelVisibilityStateChanged(int)));
  this->connect(ui.lineTickSpacingLineEdit, SIGNAL(textChanged(QString)),
                this, SLOT(onLineTickSpacingChanged()));

  this->connect(ui.lineMaxTicksSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onLineMaxTicksChanged(double)));

  // line material properties
  //this->connect(ui.lineOpacitySliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
  //              this, SLOT(onLineOpacityChanged(double)));
  //this->connect(ui.lineAmbientSliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
//                this, SLOT(onLineAmbientChanged(double)));
 // this->connect(ui.lineDiffuseSliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
  //              this, SLOT(onLineDiffuseChanged(double)));
  //this->connect(ui.lineSpecularSliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
   //             this, SLOT(onLineSpecularChanged(double)));

  this->connect(ui.lockUnlockButton, SIGNAL(clicked()), this, SLOT(onLockUnlockButtonClicked()));
  this->connect(ui.visibleInvisibleButton, SIGNAL(clicked()), this, SLOT(onVisibleInvisibleButtonClicked()));
   /*
   this->connect(
   ui.annotationTextEdit,
   SIGNAL(textChanged()),
   this,
   SLOT(onTextChanged()));
   this->connect(
   this,
   SIGNAL(rejected()),
   this,
   SLOT(onDialogRejected()));
   this->connect(
   this,
   SIGNAL(accepted()),
   this,
   SLOT(onDialogAccepted()));

   this->connect(
   ui.textUnselectedColorPickerButton,
   SIGNAL(colorChanged(QColor)),
   this,
   SLOT(onTextColorChanged(QColor)));
   this->connect(
   ui.textSelectedColorPickerButton,
   SIGNAL(colorChanged(QColor)),
   this,
   SLOT(onTextSelectedColorChanged(QColor)));
   this->connect(
   ui.textScaleSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onTextScaleChanged(double)));

   this->connect(
   ui.pointUnselectedColorPickerButton,
   SIGNAL(colorChanged(QColor)),
   this,
   SLOT(onPointColorChanged(QColor)));
   this->connect(
   ui.pointSelectedColorPickerButton,
   SIGNAL(colorChanged(QColor)),
   this,
   SLOT(onPointSelectedColorChanged(QColor)));
   this->connect(
   ui.pointSizeSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onPointSizeChanged(double)));
   this->connect(
   ui.pointOpacitySliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onPointOpacityChanged(double)));
   this->connect(
   ui.pointAmbientSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onPointAmbientChanged(double)));
   this->connect(
   ui.pointDiffuseSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onPointDiffuseChanged(double)));
   this->connect(
   ui.pointSpecularSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onPointSpecularChanged(double)));

   this->connect(
   ui.lineUnselectedColorPickerButton,
   SIGNAL(colorChanged(QColor)),
   this,
   SLOT(onLineColorChanged(QColor)));
   this->connect(
   ui.lineSelectedColorPickerButton,
   SIGNAL(colorChanged(QColor)),
   this,
   SLOT(onLineSelectedColorChanged(QColor)));
   this->connect(
   ui.lineWidthSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onLineWidthChanged(double)));
   this->connect(
   ui.lineOpacitySliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onLineOpacityChanged(double)));
   this->connect(
   ui.lineAmbientSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onLineAmbientChanged(double)));
   this->connect(
   ui.lineDiffuseSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onLineDiffuseChanged(double)));
   this->connect(
   ui.lineSpecularSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onLineSpecularChanged(double)));

   this->connect(
   ui.CTKCollapsibleGroupBox_4,
   SIGNAL(clicked()),
   this,
   SLOT(onCollapsibleGroupBoxClicked()));
   */
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onCoordinateChanged(QString text)
{
  Q_UNUSED(text)
  /*
   if (this->m_isUpdated)
   {
   return;
   }

   std::vector<double> positions;
   QString valueString;
   std::vector<double> thevalue;
   const char* format;

   for (int i = 0; i < m_lineEditList.size(); ++i)
   {
   positions.push_back(m_lineEditList[i]->text().toDouble());
   }

   // update widget
   vtkDMMLNode* node = this->m_logic->GetDMMLScene()->GetNodeByID(m_nodeId);
   int num = positions.size() / 3;
   double pos[3];
   for (int id = 0; id < num; ++id)
   {
   pos[0] = positions[id * 3];
   pos[1] = positions[id * 3 + 1];
   pos[2] = positions[id * 3 + 2];
   this->m_logic->SetAnnotationControlPointsCoordinate(node, pos, id);
   }

   // update value in the property dialog
   thevalue = this->m_logic->GetAnnotationMeasurement(node);
   format = this->m_logic->GetAnnotationTextFormatProperty(node);
   this->FormatValueToChar(format, thevalue, valueString);
   this->updateValue(valueString);

   emit coordinateChanged(valueString, m_nodeId);
   */
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onTextChanged()
{
  QString text = ui.annotationTextEdit->toPlainText();
  this->m_logic->SetAnnotationText(this->m_id.c_str(), text.toUtf8());
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::updateTextFromTable(QString text)
{
  Q_UNUSED(text)
  // Text Properties
  // ui.annotationTextEdit->setText(
  //    text);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::updateValue(QString valueString)
{
  QString valueStr;
  valueStr.append("<p>Value: <b>").append(valueString).append("</b></p>");
  // ui.annotationValueBrowser->setHtml(
  //    valueStr);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::SaveLinesNode(vtkDMMLAnnotationLinesNode* node)
{
  Q_UNUSED(node);

  /*
 if (!node)
 {
 return;
 }
 if (!this->m_lineDispCopy)
 {
 this->m_lineDispCopy = vtkDMMLAnnotationLineDisplayNode::New();
 }

 node->CreateAnnotationLineDisplayNode();
 this->m_lineDispCopy->Copy(node->GetAnnotationLineDisplayNode());
 this->SaveControlPoints(node);
 */
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::SaveControlPoints(vtkDMMLAnnotationControlPointsNode* node)
{
  Q_UNUSED(node);
  /*
   if (!node)
   {
   return;
   }

   if (!this->m_pointDispCopy)
   {
   this->m_pointDispCopy = vtkDMMLAnnotationPointDisplayNode::New();
   }
   node->CreateAnnotationPointDisplayNode();
   this->m_pointDispCopy->Copy(node->GetAnnotationPointDisplayNode());
   this->SaveAnnotationNode((vtkDMMLAnnotationNode*) node);
   */
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::SaveAnnotationNode(vtkDMMLAnnotationNode* node)
{
  Q_UNUSED(node);
  /*
   if (!node)
   {
   return;
   }

   if (!this->m_textDispCopy)
   {
   this->m_textDispCopy = vtkDMMLAnnotationTextDisplayNode::New();
   }
   node->CreateAnnotationTextDisplayNode();
   this->m_textDispCopy->Copy(node->GetAnnotationTextDisplayNode());
   */
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::SaveStateForUndo(vtkDMMLNode* node)
{
  Q_UNUSED(node);
  /*
   if (node->IsA("vtkDMMLAnnotationRulerNode"))
   {
   vtkDMMLAnnotationRulerNode* mynode =
   vtkDMMLAnnotationRulerNode::SafeDownCast(node);
   if (!this->m_rulerCopy)
   {
   this->m_rulerCopy = vtkDMMLAnnotationRulerNode::New();
   }
   this->m_rulerCopy->Copy(mynode);
   this->SaveLinesNode(mynode);
   }
   else if (node->IsA("vtkDMMLAnnotationFiducialNode"))
   {
   vtkDMMLAnnotationFiducialNode* fiducialNode =
   vtkDMMLAnnotationFiducialNode::SafeDownCast(node);
   fiducialNode->CreateAnnotationTextDisplayNode();
   fiducialNode->CreateAnnotationPointDisplayNode();
   fiducialNode->GetScene()->SaveStateForUndo();
   }
   */
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::UndoLinesNode(vtkDMMLAnnotationLinesNode* node)
{
  Q_UNUSED(node);
  /*
   if (!node)
   {
   return;
   }
   node->CreateAnnotationLineDisplayNode();
   node->GetAnnotationLineDisplayNode()->Copy(m_lineDispCopy);
   this->UndoControlPoints(node);
   */
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::UndoControlPoints(vtkDMMLAnnotationControlPointsNode* node)
{
  Q_UNUSED(node);
  /*
   if (!node)
   {
   return;
   }
   node->CreateAnnotationPointDisplayNode();
   node->GetAnnotationPointDisplayNode()->Copy(m_pointDispCopy);
   this->UndoAnnotationNode((vtkDMMLAnnotationNode*) node);
   */
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::UndoAnnotationNode(vtkDMMLAnnotationNode* node)
{
  Q_UNUSED(node);
  /*
   if (!node)
   {
   return;
   }
   node->CreateAnnotationTextDisplayNode();
   node->GetAnnotationTextDisplayNode()->Copy(m_textDispCopy);
   */
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::Undo(vtkDMMLNode* node)
{
  Q_UNUSED(node);
  /*
   if (node->IsA("vtkDMMLAnnotationRulerNode"))
   {
   vtkDMMLAnnotationRulerNode* rnode =
   vtkDMMLAnnotationRulerNode::SafeDownCast(node);
   rnode->Copy(m_rulerCopy);
   this->UndoLinesNode(rnode);
   }
   else if (node->IsA("vtkDMMLAnnotationFiducialNode"))
   {
   //ToDo
   }
   */
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onTextUnselectedColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(color, qcolor);

  this->m_logic->SetAnnotationTextUnselectedColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onTextSelectedColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(color, qcolor);

  this->m_logic->SetAnnotationTextSelectedColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onTextScaleChanged(double value)
{
  this->m_logic->SetAnnotationTextScale(this->m_id.c_str(), value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onTextOpacityChanged(double value)
{
  // get the text display node
  vtkDMMLAnnotationTextDisplayNode *textDisplayNode = this->m_logic->GetTextDisplayNode(this->m_id.c_str());
  if (!textDisplayNode)
    {
    return;
    }
  textDisplayNode->SetOpacity(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLockUnlockButtonClicked()
{
  // toggle the lock flag
  this->m_logic->SetAnnotationLockedUnlocked(this->m_id.c_str());

  int locked = this->m_logic->GetAnnotationLockedUnlocked(this->m_id.c_str());

  if (!locked)
    {
    ui.lockUnlockButton->setIcon(QIcon(":/Icons/AnnotationUnlock.png"));
    ui.lockUnlockButton->setToolTip(QString("Click to lock this annotation"));
    }
  else
    {
    ui.lockUnlockButton->setIcon(QIcon(":/Icons/AnnotationLock.png"));
    ui.lockUnlockButton->setToolTip(QString("This annotation is locked. Click to unlock!"));
    }

  this->lockUnlockInterface(locked);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onVisibleInvisibleButtonClicked()
{
  this->m_logic->SetAnnotationVisibility(this->m_id.c_str());

  // load the visibility status
  int visible = this->m_logic->GetAnnotationVisibility(this->m_id.c_str());

  if (!visible)
    {
    ui.visibleInvisibleButton->setIcon(QIcon(":/Icons/AnnotationInvisible.png"));
    ui.visibleInvisibleButton->setToolTip(QString("Click to show this annotation"));
    }
  else
    {
    ui.visibleInvisibleButton->setIcon(QIcon(":/Icons/AnnotationVisibility.png"));
    ui.visibleInvisibleButton->setToolTip(QString("Click to hide this annotation"));
    }
}
/*
//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointsTableWidgetChanged(QTableWidgetItem *tableItem)
{
  if (tableItem == nullptr)
    {
    return;
    }
  int row = tableItem->row();
  int col = tableItem->column();
  QString newString = tableItem->text();
  double newValue = newString.toDouble();
  vtkDMMLNode *node = this->m_logic->GetDMMLScene()->GetNodeByID(this->m_id.c_str());
  if (!node)
    {
    return;
    }
  vtkDMMLAnnotationControlPointsNode *pointsNode = vtkDMMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!pointsNode)
    {
    return;
    }
//  std::cout << "onPointsTableWidgetChanged: row = " << row << ", col = " << col << ", newValue = " << newValue << std::endl;
  // get the point coordinates corresponding to this row
  double *oldCoords = pointsNode->GetControlPointCoordinates(row);
  double newCoords[3];
  newCoords[0] = oldCoords[0];
  newCoords[1] = oldCoords[1];
  newCoords[2] = oldCoords[2];
  if (!oldCoords)
    {
    return;
    }
  if (col == 1)
    {
    // x
    if (newCoords[0] != newValue)
      {
      newCoords[0] = newValue;
      }
    }
  else if (col == 2)
    {
    // y
    if (newCoords[1] != newValue)
      {
      newCoords[1] = newValue;
      }
    }
  else if (col == 3)
    {
    // z
    if (newCoords[2] != newValue)
      {
      newCoords[2] = newValue;
      }
    }
  if (newCoords[0] != oldCoords[0] ||
      newCoords[1] != oldCoords[1] ||
      newCoords[2] != oldCoords[2])
    {
    //std::cout << "Setting control point for point " << row << ", to " << newCoords[0] << ", " << newCoords[1] << ", " << newCoords[2] << std::endl;
    pointsNode->SetControlPoint(row, newCoords);
    }
}*/
//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(color, qcolor);

  this->m_logic->SetAnnotationPointUnselectedColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointSelectedColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(color, qcolor);

  this->m_logic->SetAnnotationPointColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointSizeChanged(double value)
{
  vtkDMMLNode* node = this->m_logic->GetDMMLScene()->GetNodeByID(this->m_id.c_str());
  if (!node)
    {
    return;
    }
  vtkDMMLAnnotationControlPointsNode *pointsNode = vtkDMMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!pointsNode)
    {
    return;
    }
  // get the point display node
  vtkDMMLAnnotationPointDisplayNode *pointDisplayNode = pointsNode->GetAnnotationPointDisplayNode();
  if (!pointDisplayNode)
    {
    return;
    }
  pointDisplayNode->SetGlyphScale(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointOpacityChanged(double value)
{
  // get the point display node
  vtkDMMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(this->m_id.c_str());
  if (!pointDisplayNode)
    {
    return;
    }
  pointDisplayNode->SetOpacity(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointAmbientChanged(double value)
{
  // get the point display node
  vtkDMMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(this->m_id.c_str());
  if (!pointDisplayNode)
    {
    return;
    }
  pointDisplayNode->SetAmbient(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointDiffuseChanged(double value)
{
  // get the point display node
  vtkDMMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(this->m_id.c_str());
  if (!pointDisplayNode)
    {
    return;
    }
  pointDisplayNode->SetDiffuse(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointSpecularChanged(double value)
{
  // get the point display node
  vtkDMMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(this->m_id.c_str());
  if (!pointDisplayNode)
    {
    return;
    }
  pointDisplayNode->SetSpecular(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointGlyphChanged(QString value)
{
//  std::cout << "OnPointGlyphChanged: " << qPrintable(value) << std::endl;
  this->m_logic->SetAnnotationPointGlyphTypeFromString(this->m_id.c_str(),value.toUtf8());
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(color, qcolor);

  this->m_logic->SetAnnotationLineUnselectedColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineSelectedColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(color, qcolor);

  this->m_logic->SetAnnotationLineColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineWidthChanged(double value)
{
  // get the line display node
  vtkDMMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetLineThickness(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineLabelPositionChanged(double value)
{
  // get the line display node
  vtkDMMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetLabelPosition(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineLabelVisibilityStateChanged(int state)
{
  // get the line display node
  vtkDMMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  if (state)
    {
    lineDisplayNode->LabelVisibilityOn();
    }
  else
    {
    lineDisplayNode->LabelVisibilityOff();
    }
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineTickSpacingChanged()
{
  // get the line display node
  vtkDMMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  QString plainText = ui.lineTickSpacingLineEdit->text();
  double value = plainText.toDouble();
  lineDisplayNode->SetTickSpacing(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineMaxTicksChanged(double value)
{
  // get the line display node
  vtkDMMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetMaxTicks(int(value));
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineOpacityChanged(double value)
{
  // get the line display node
  vtkDMMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetOpacity(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineAmbientChanged(double value)
{
  // get the line display node
  vtkDMMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetAmbient(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineDiffuseChanged(double value)
{
  // get the line display node
  vtkDMMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetDiffuse(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineSpecularChanged(double value)
{
  // get the line display node
  vtkDMMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetSpecular(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::UpdateLockUnlockStatus(bool isLock)
{
  Q_UNUSED(isLock);
  /* if (isLock)
   {
   ui.annotationTextEdit->setEnabled(
   false);
   for (int i = 0; i < m_lineEditList.size(); ++i)
   {
   m_lineEditList[i]->setEnabled(
   false);
   }
   //   ui.textTab->setEnabled(
   //       false);
   //   ui.pointTab->setEnabled(
   //       false);
   //  ui.lineTab->setEnabled(
   //    false);
   }
   else
   {
   ui.annotationTextEdit->setEnabled(
   true);
   for (int i = 0; i < m_lineEditList.size(); ++i)
   {
   m_lineEditList[i]->setEnabled(
   true);
   }
   ui.textTab->setEnabled(
   true);
   ui.pointTab->setEnabled(
   true);
   ui.lineTab->setEnabled(
   true);
   }*/

}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::TurnColorArrayToQColor(double* color, QColor &qcolor)
{
  qcolor.setRed(color[0] * 255.0001);
  qcolor.setGreen(color[1] * 255.0001);
  qcolor.setBlue(color[2] * 255.0001);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::FormatValueToChar(const char* format, std::vector<
    double> vv, QString &valueString)
{
  char valuechar[100];
  QString tempString;
  valueString = "";
  foreach(double v, vv)
      {
      sprintf(valuechar, format, v);
      tempString = valuechar;
      tempString.append(" ");
      valueString.append(tempString);
      }

  //valueString = valuechar;
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::TurnQColorToColorArray(double* color, QColor &qcolor)
{
  color[0] = qcolor.red() / 255.0;
  color[1] = qcolor.green() / 255.0;
  color[2] = qcolor.blue() / 255.0;
}

//-----------------------------------------------------------------------------
// Methods for closing the property dialog
//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onDialogRejected()
{
  // the user clicked cancel, now restore the backuped node
  this->m_logic->RestoreAnnotationNode(this->m_id.c_str());

  // delete all backups
  this->m_logic->DeleteBackupNodes(this->m_id.c_str());


  emit dialogRejected();
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onDialogAccepted()
{

  // delete all backups
  this->m_logic->DeleteBackupNodes(this->m_id.c_str());

  emit dialogAccepted();
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::lockUnlockInterface(bool lock)
{
  lock = !lock;
/*
  ui.annotationTextEdit->setEnabled(lock);
  ui.pointsTableWidget->setEnabled(lock);
  ui.measurementLineEdit->setEnabled(lock);
  ui.textSelectedColorPickerButton->setEnabled(lock);
  ui.textUnselectedColorPickerButton->setEnabled(lock);
  ui.textScaleSliderSpinBoxWidget->setEnabled(lock);
  ui.textOpacitySliderSpinBoxWidget->setEnabled(lock);
  ui.visibleInvisibleButton->setEnabled(lock);
  ui.pointSelectedColorPickerButton->setEnabled(lock);
  ui.pointUnselectedColorPickerButton->setEnabled(lock);
  ui.pointGlyphTypeComboBox->setEnabled(lock);
  ui.lineSelectedColorPickerButton->setEnabled(lock);
  ui.lineUnselectedColorPickerButton->setEnabled(lock);
  ui.pointAmbientSliderSpinBoxWidget->setEnabled(lock);
  ui.pointDiffuseSliderSpinBoxWidget->setEnabled(lock);
  ui.pointOpacitySliderSpinBoxWidget->setEnabled(lock);
  ui.pointSizeSliderSpinBoxWidget->setEnabled(lock);
  ui.pointSpecularSliderSpinBoxWidget->setEnabled(lock);
  ui.lineAmbientSliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineDiffuseSliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineOpacitySliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineSpecularSliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineWidthSliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineLabelPositionSliderSpinBoxWidget->setEnabled(lock);
  ui.lineLabelVisibilityCheckBox->setEnabled(lock);
  ui.lineTickSpacingLineEdit->setEnabled(lock);
  ui.lineMaxTicksSliderSpinBoxWidget->setEnabled(lock);
  */
}

