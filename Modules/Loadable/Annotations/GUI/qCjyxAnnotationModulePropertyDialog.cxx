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

// qDMML includes
#include <qDMMLUtils.h>

// Annotations includes
#include "vtkDMMLAnnotationControlPointsNode.h"
#include "vtkDMMLAnnotationFiducialNode.h"
#include "vtkDMMLAnnotationHierarchyNode.h"
#include "vtkDMMLAnnotationLineDisplayNode.h"
#include "vtkDMMLAnnotationLinesNode.h"
#include "vtkDMMLAnnotationNode.h"
#include "vtkDMMLAnnotationPointDisplayNode.h"
#include "vtkDMMLAnnotationROINode.h"
#include "vtkDMMLAnnotationRulerNode.h"
#include "vtkDMMLAnnotationTextDisplayNode.h"
#include "vtkCjyxAnnotationModuleLogic.h"

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>

//------------------------------------------------------------------------------
qCjyxAnnotationModulePropertyDialog::~qCjyxAnnotationModulePropertyDialog()
{
  this->m_logic = nullptr;
}

//------------------------------------------------------------------------------
qCjyxAnnotationModulePropertyDialog::qCjyxAnnotationModulePropertyDialog(const char * id, vtkCjyxAnnotationModuleLogic* logic)
{
  this->m_id = vtkStdString(id);
  this->m_logic = logic;

  // now build the user interface
  ui.setupUi(this);

  this->setAttribute(Qt::WA_DeleteOnClose);

  ui.DescriptionLabel->setVisible(true);
  ui.DescriptionTextEdit->setVisible(true);
  ui.lineTickSpacingSlider->setSynchronizeSiblings(ctkSliderWidget::SynchronizeWidth);

  if (this->m_logic->IsAnnotationHierarchyNode(id))
    {
    // hierarchies

    // TODO show the display widget
    ui.advancedCollapsibleButton->hide();

    // init so it updates the general properties
    this->initialize();

    }
  else
    {
    // annotations

    this->initialize();

    }

  // create the slot and signal connections
  this->createConnection();


}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::initialize()
{
  // backup the current annotationNode
  this->m_logic->BackupAnnotationNode(this->m_id);

  this->updateNameText();
  this->updateTypeLabelText();
  this->updateIDLabelText();

  // update the typeIcon according to the annotation type
  QIcon icon = QIcon(this->m_logic->GetAnnotationIcon(this->m_id.c_str()));
  QPixmap pixmap = icon.pixmap(32, 32);

  ui.typeIcon->setPixmap(pixmap);

  // hide the visib if it's a hierarchy, or else update it
  if (this->m_logic->IsAnnotationHierarchyNode(this->m_id))
    {
    ui.visibleInvisibleButton->hide();
    }
  else
    {
    ui.visibleInvisibleButton->show();
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

  // customise the all color picker button
  ui.allColorPickerButton->setToolTip(tr("Set unselected color of whole annotation (points, text, lines), use advanced pane to set individual colors"));
  ui.allColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);
  // if all the display nodes are the same color, update the color on the
  // button to show it, otherwise will be black
  this->updateAllColorButton();

  vtkDMMLNode *dmmlNode = this->m_logic->GetDMMLScene()->GetNodeByID(this->m_id.c_str());
  if (!dmmlNode)
    {
    return;
    }

  // hide the RAS if it's not a fiducial
  ui.RASCoordinatesWidget->setDMMLScene(this->m_logic->GetDMMLScene());
  if (!dmmlNode->IsA("vtkDMMLAnnotationFiducialNode"))
    {
    ui.RASLabel->setVisible(false);
    ui.RASCoordinatesWidget->setVisible(false);
    }
  else
    {
    ui.RASLabel->setVisible(true);
    ui.RASCoordinatesWidget->setVisible(true);
    // update
    vtkDMMLAnnotationFiducialNode *fidNode = vtkDMMLAnnotationFiducialNode::SafeDownCast(dmmlNode);
    double *pos = fidNode->GetFiducialCoordinates();
    if (pos)
      {
      ui.RASCoordinatesWidget->setCoordinates(pos);
      }
    // disable if locked
    if (fidNode->GetLocked())
      {
      ui.RASCoordinatesWidget->setEnabled(false);
      }
    else
      {
      ui.RASCoordinatesWidget->setEnabled(true);
      }
    }

  // if it's a hierarchy node, don't show the size or lock buttons
  if (this->m_logic->IsAnnotationHierarchyNode(this->m_id))
    {
    ui.hierarchyPushCheckBox->show();

    ui.hierarchyTextScaleLabel->show();
    ui.hierarchyTextScaleSlider->show();
    ui.hierarchyPointSizeDefaultButton->show();
    ui.hierarchyPointSizeLabel->show();
    ui.hierarchyPointSizeSlider->show();
    ui.hierarchyPointSizeDefaultButton->show();
    ui.hierarchyGlyphTypeLabel->show();
    ui.hierarchyPointGlyphTypeComboBox->show();
    ui.hierarchyPointGlyphTypeDefaultButton->show();

    // fill in types if empty
    if (ui.hierarchyPointGlyphTypeComboBox->count() == 0)
      {
      vtkDMMLAnnotationPointDisplayNode *displayNode = vtkDMMLAnnotationPointDisplayNode::New();
      int min = displayNode->GetMinimumGlyphType();
      int max = displayNode->GetMaximumGlyphType();
      bool enabled =  ui.pointGlyphTypeComboBox->isEnabled();
      ui.hierarchyPointGlyphTypeComboBox->setEnabled(false);
      for (int i = min; i <= max; i++)
        {
        displayNode->SetGlyphType(i);
        ui.hierarchyPointGlyphTypeComboBox->addItem(displayNode->GetGlyphTypeAsString());
        }
      ui.hierarchyPointGlyphTypeComboBox->setEnabled(enabled);
      displayNode->Delete();
      }
    // set default values if not set
    vtkDMMLAnnotationPointDisplayNode *pdNode = vtkDMMLAnnotationPointDisplayNode::New();

    if (ui.hierarchyPointGlyphTypeComboBox->currentIndex() == 0)
      {
      QString glyphType = QString(pdNode->GetGlyphTypeAsString());
      bool enabled = ui.hierarchyPointGlyphTypeComboBox->isEnabled();
      ui.hierarchyPointGlyphTypeComboBox->setEnabled(false);
      int index =  ui.hierarchyPointGlyphTypeComboBox->findData(glyphType);
      if (index != -1)
        {
        ui.hierarchyPointGlyphTypeComboBox->setCurrentIndex(index);
        }
      else
        {
        // glyph types start at 1, combo box is 0 indexed
        ui.hierarchyPointGlyphTypeComboBox->setCurrentIndex(pdNode->GetGlyphType() - 1);
        }
      ui.hierarchyPointGlyphTypeComboBox->setEnabled(enabled);
      }
    if (ui.hierarchyPointSizeSlider->value() == 0)
      {
      double glyphScale = pdNode->GetGlyphScale();
      bool enabled = ui.hierarchyPointSizeSlider->isEnabled();
      ui.hierarchyPointSizeSlider->setEnabled(false);
      ui.hierarchyPointSizeSlider->setValue(glyphScale);
      ui.hierarchyPointSizeSlider->setEnabled(enabled);
      }
    pdNode->Delete();
    if (ui.hierarchyTextScaleSlider->value() == 0)
      {
      vtkDMMLAnnotationTextDisplayNode *tdNode = vtkDMMLAnnotationTextDisplayNode::New();
      double textScale = tdNode->GetTextScale();
      bool enabled =  ui.hierarchyTextScaleSlider->isEnabled();
      ui.hierarchyTextScaleSlider->setEnabled(false);
      ui.hierarchyTextScaleSlider->setValue(textScale);
      ui.hierarchyTextScaleSlider->setEnabled(enabled);

      tdNode->Delete();
    }

    ui.sizeLabel->hide();
    ui.sizeSmallPushButton->hide();
    ui.sizeMediumPushButton->hide();
    ui.sizeLargePushButton->hide();

    ui.lockUnlockButton->hide();

    // the rest is hidden, so just return
    return;
    }
  else
    {
    ui.hierarchyPushCheckBox->hide();

    ui.hierarchyTextScaleLabel->hide();
    ui.hierarchyTextScaleSlider->hide();
    ui.hierarchyTextScaleDefaultButton->hide();
    ui.hierarchyPointSizeLabel->hide();
    ui.hierarchyPointSizeSlider->hide();
    ui.hierarchyPointSizeDefaultButton->hide();
    ui.hierarchyGlyphTypeLabel->hide();
    ui.hierarchyPointGlyphTypeComboBox->hide();
    ui.hierarchyPointGlyphTypeDefaultButton->hide();

    }


  // load the current annotation text
  vtkStdString text = this->m_logic->GetAnnotationText(this->m_id.c_str());

  ui.annotationTextEdit->setText(text.c_str());
  ui.DescriptionTextEdit->setText(text.c_str());

  // load the current annotation text scale
  vtkDMMLAnnotationTextDisplayNode *textDisplayNode = this->m_logic->GetTextDisplayNode(this->m_id.c_str());
  if (textDisplayNode)
    {
    double textScale = textDisplayNode->GetTextScale();
    ui.textScaleSliderSpinBoxWidget->setValue(textScale);

    double opacity = textDisplayNode->GetOpacity();
    ui.textOpacitySliderSpinBoxWidget->setValue(opacity);

    int visibility = textDisplayNode->GetVisibility();
    ui.textVisibilityCheckBox->setChecked(visibility);
    }

  // load the current measurement
  const char * measurement = this->m_logic->GetAnnotationMeasurement(
      this->m_id.c_str(), true);

  ui.measurementLineEdit->setText(measurement);

  // load the unselected text color
  double * unselectedColor = this->m_logic->GetAnnotationTextUnselectedColor(
      this->m_id.c_str());
  QColor unselectedQColor;
  qDMMLUtils::colorToQColor(unselectedColor,unselectedQColor);

  ui.textUnselectedColorPickerButton->setDisplayColorName(false);
  ui.textUnselectedColorPickerButton->setColor(unselectedQColor);
  ui.textUnselectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

  // load the selected text color
  double * selectedColor = this->m_logic->GetAnnotationTextSelectedColor(
      this->m_id.c_str());
  QColor selectedQColor;
  qDMMLUtils::colorToQColor(selectedColor,selectedQColor);

  ui.textSelectedColorPickerButton->setDisplayColorName(false);
  ui.textSelectedColorPickerButton->setColor(selectedQColor);
  ui.textSelectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

  // load the lock/unlock status
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



  vtkDMMLAnnotationControlPointsNode *pointsNode = vtkDMMLAnnotationControlPointsNode::SafeDownCast(dmmlNode);
  vtkDMMLAnnotationPointDisplayNode *pointDisplayNode = nullptr;
  if (pointsNode)
    {
    // get the point display node
    pointDisplayNode = pointsNode->GetAnnotationPointDisplayNode();

    // point values
    // clear out the table first?
    //ui.pointsTableWidget->clear();
    for (int p = 0; p < pointsNode->GetNumberOfControlPoints(); p++)
      {
      ui.pointsTableWidget->insertRow(p);
      double *coord = pointsNode->GetControlPointCoordinates(p);

      if (coord)
        {
        QTableWidgetItem* xItem = new QTableWidgetItem();
        xItem->setData(Qt::DisplayRole, QVariant(coord[0]));
        ui.pointsTableWidget->setItem(p, 0, xItem);
        QTableWidgetItem* yItem = new QTableWidgetItem();
        yItem->setData(Qt::DisplayRole, QVariant(coord[1]));
        ui.pointsTableWidget->setItem(p, 1, yItem);
        QTableWidgetItem* zItem = new QTableWidgetItem();
        zItem->setData(Qt::DisplayRole, QVariant(coord[2]));
        ui.pointsTableWidget->setItem(p,2, zItem);
        }
      }
    }
  if (pointDisplayNode)
    {


    // unselected color
    double *pointUnSelColor = pointDisplayNode->GetColor();
    QColor pointUnSelQColor;
    qDMMLUtils::colorToQColor(pointUnSelColor,pointUnSelQColor);
    ui.pointUnselectedColorPickerButton->setDisplayColorName(false);
    ui.pointUnselectedColorPickerButton->setColor(pointUnSelQColor);
    ui.pointUnselectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

    // selected color
    double *pointSelColor = pointDisplayNode->GetSelectedColor();
    QColor pointSelQColor;
    qDMMLUtils::colorToQColor(pointSelColor, pointSelQColor);
    ui.pointSelectedColorPickerButton->setDisplayColorName(false);
    ui.pointSelectedColorPickerButton->setColor(pointSelQColor);
    ui.pointSelectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

    // load the glyph type


    // fill in the combo box with glyph types if not already done
    if (ui.pointGlyphTypeComboBox->count() == 0)
      {
      vtkDMMLAnnotationPointDisplayNode *displayNode = vtkDMMLAnnotationPointDisplayNode::New();
      int min = displayNode->GetMinimumGlyphType();
      int max = displayNode->GetMaximumGlyphType();
      bool enabled =  ui.pointGlyphTypeComboBox->isEnabled();
      ui.pointGlyphTypeComboBox->setEnabled(false);
      for (int i = min; i <= max; i++)
        {
        displayNode->SetGlyphType(i);
        ui.pointGlyphTypeComboBox->addItem(displayNode->GetGlyphTypeAsString());
        //std::cout << "Adding glyphs to the combo box i = " << i << ", glyph type = " << displayNode->GetGlyphType() << ", as string = " << displayNode->GetGlyphTypeAsString() << ", combo box current index = " << ui.pointGlyphTypeComboBox->currentIndex() << ", count = " << ui.pointGlyphTypeComboBox->count() << std::endl;
        }
      ui.pointGlyphTypeComboBox->setEnabled(enabled);
      displayNode->Delete();
      }
    int index =  ui.pointGlyphTypeComboBox->findData(QString(pointDisplayNode->GetGlyphTypeAsString()));
//    std::cout << "glyph type = " << pointDisplayNode->GetGlyphType() << ", as string = " << pointDisplayNode->GetGlyphTypeAsString() << ", index = " << index << ", combo box size = " << ui.pointGlyphTypeComboBox->count() << std::endl;
    if (index != -1)
      {
      ui.pointGlyphTypeComboBox->setCurrentIndex(index);
      }
    else
      {
      // glyph types start at 1, combo box is 0 indexed
      ui.pointGlyphTypeComboBox->setCurrentIndex(pointDisplayNode->GetGlyphType() - 1);
//      std::cout << "\tset current index via the glyph type " << pointDisplayNode->GetGlyphType() << ", current index = " << ui.pointGlyphTypeComboBox->currentIndex() << std::endl;
      }
    // glyph size
    ui.pointSizeSliderSpinBoxWidget->setValue(pointDisplayNode->GetGlyphScale());

    // glyph material properties
    ui.pointOpacitySliderSpinBoxWidget->setValue(pointDisplayNode->GetOpacity());
    ui.pointAmbientSliderSpinBoxWidget->setValue(pointDisplayNode->GetAmbient());
    ui.pointDiffuseSliderSpinBoxWidget->setValue(pointDisplayNode->GetDiffuse());
    ui.pointSpecularSliderSpinBoxWidget->setValue(pointDisplayNode->GetSpecular());

    // Expand fiducial projection panel if projection turned on
    bool collapseGroupBox =
      pointDisplayNode->GetSliceProjection() & pointDisplayNode->ProjectionOn ? false : true;
    ui.pointFiducialProjectionPropertiesGroupBox->setCollapsed(collapseGroupBox);
    }

  /// Lines
  // get the line version of the node
  vtkDMMLAnnotationLinesNode *linesNode = vtkDMMLAnnotationLinesNode::SafeDownCast(dmmlNode);
  vtkDMMLAnnotationLineDisplayNode *lineDisplayNode = nullptr;
  if (linesNode)
    {
    lineDisplayNode = linesNode->GetAnnotationLineDisplayNode();
    }

  if (!lineDisplayNode)
    {
    // disable the lines tab
    ui.tabWidget->setTabEnabled(2, false);
    }
  else
    {
    // enable the lines tab
    ui.tabWidget->setTabEnabled(2, true);
    // colors

    // unselected
    double *lineUnSelColor = lineDisplayNode->GetColor();
    QColor lineUnSelQColor;
    qDMMLUtils::colorToQColor(lineUnSelColor,lineUnSelQColor);
    ui.lineUnselectedColorPickerButton->setDisplayColorName(false);
    ui.lineUnselectedColorPickerButton->setColor(lineUnSelQColor);
    ui.lineUnselectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

    // selected
    double *lineSelColor = lineDisplayNode->GetSelectedColor();
    QColor lineSelQColor;
    qDMMLUtils::colorToQColor(lineSelColor, lineSelQColor);
    ui.lineSelectedColorPickerButton->setDisplayColorName(false);
    ui.lineSelectedColorPickerButton->setColor(lineSelQColor);
    ui.lineSelectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

    // width
    ui.lineWidthSliderSpinBoxWidget_2->setValue(lineDisplayNode->GetLineThickness());

    // label position
    ui.lineLabelPositionSliderSpinBoxWidget->setValue(lineDisplayNode->GetLabelPosition());
    // label visibility
    ui.lineLabelVisibilityCheckBox->setChecked(lineDisplayNode->GetLabelVisibility());
    // tick spacing
    ui.lineTickSpacingSlider->setDMMLScene(this->m_logic->GetDMMLScene());
    ui.lineTickSpacingSlider->setValue(lineDisplayNode->GetTickSpacing());

    // max ticks
    ui.lineMaxTicksSliderSpinBoxWidget->setValue(lineDisplayNode->GetMaxTicks());

    // line material properties
    ui.lineOpacitySliderSpinBoxWidget_2->setValue(lineDisplayNode->GetOpacity());
    ui.lineAmbientSliderSpinBoxWidget_2->setValue(lineDisplayNode->GetAmbient());
    ui.lineDiffuseSliderSpinBoxWidget_2->setValue(lineDisplayNode->GetDiffuse());
    ui.lineSpecularSliderSpinBoxWidget_2->setValue(lineDisplayNode->GetSpecular());

    // Expand ruler projection panel if projection turned on
    bool collapseGroupBox =
      lineDisplayNode->GetSliceProjection() & lineDisplayNode->ProjectionOn ? false : true;
    ui.lineRulerProjectionPropertiesGroupBox->setCollapsed(collapseGroupBox);
    }

  /// ROI
  // get the line version of the node
  vtkDMMLAnnotationROINode *roiNode = vtkDMMLAnnotationROINode::SafeDownCast(dmmlNode);
  if (roiNode)
    {
    ui.tabWidget->setTabEnabled(3, true);
    ui.ROIWidget->setDMMLAnnotationROINode(roiNode);

    // activate the roi tab
    ui.tabWidget->setCurrentIndex(3);

    if (roiNode->GetLocked())
      {
      ui.ROIWidget->setEnabled(false);
      }
    else
      {
      ui.ROIWidget->setEnabled(true);
      }
    }
  else
    {
    ui.tabWidget->setTabEnabled(3, false);
    }

  // Ruler
  vtkDMMLAnnotationRulerNode* rulerNode = vtkDMMLAnnotationRulerNode::SafeDownCast(dmmlNode);
  if (rulerNode)
    {
    ui.lineRulerProjectionWidget->setDMMLRulerNode(rulerNode);
    }

  // Fiducial
  vtkDMMLAnnotationFiducialNode* fiducialNode = vtkDMMLAnnotationFiducialNode::SafeDownCast(dmmlNode);
  if (fiducialNode)
    {
    ui.pointFiducialProjectionWidget->setDMMLFiducialNode(fiducialNode);
    }

  if (dmmlNode->IsA("vtkDMMLAnnotationFiducialNode") || dmmlNode->IsA("vtkDMMLAnnotationRulerNode"))
    {
    // activate the points tab
    ui.tabWidget->setCurrentIndex(1);
    }
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::createConnection()
{
  this->connect(this, SIGNAL(rejected()), this, SLOT(onDialogRejected()));
  this->connect(this, SIGNAL(accepted()), this, SLOT(onDialogAccepted()));

  // global
  this->connect(ui.nameLineEdit, SIGNAL(textChanged(QString)), this,
                SLOT(onNameLineEditChanged()));
  this->connect(ui.allColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onAllColorChanged(QColor)));

  this->connect(ui.hierarchyTextScaleSlider, SIGNAL(valueChanged(double)),
      this, SLOT(onHierarchyTextScaleChanged(double)));
  this->connect(ui.hierarchyTextScaleDefaultButton, SIGNAL(clicked()),
                this, SLOT(onHierarchyTextScaleDefaultButtonClicked()));
  this->connect(ui.hierarchyPointSizeSlider, SIGNAL(valueChanged(double)),
      this, SLOT(onHierarchyPointSizeChanged(double)));
  this->connect(ui.hierarchyPointSizeDefaultButton, SIGNAL(clicked()),
                this, SLOT(onHierarchyPointSizeDefaultButtonClicked()));
  this->connect(ui.hierarchyPointGlyphTypeComboBox, SIGNAL(currentIndexChanged(QString)),
                this, SLOT(onHierarchyPointGlyphChanged(QString)));
  this->connect(ui.hierarchyPointGlyphTypeDefaultButton, SIGNAL(clicked()),
                this, SLOT(onHierarchyPointGlyphTypeDefaultButtonClicked()));


  this->connect(ui.sizeSmallPushButton, SIGNAL(clicked()), this, SLOT(onSizeSmallPushButtonClicked()));
  this->connect(ui.sizeMediumPushButton, SIGNAL(clicked()), this, SLOT(onSizeMediumPushButtonClicked()));
  this->connect(ui.sizeLargePushButton, SIGNAL(clicked()), this, SLOT(onSizeLargePushButtonClicked()));
  this->connect(ui.DescriptionTextEdit, SIGNAL(textChanged()), this,
      SLOT(onDescriptionTextChanged()));
  this->connect(ui.RASCoordinatesWidget, SIGNAL(coordinatesChanged(double*)), this, SLOT(onRASCoordinatesChanged(double*)));
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

  this->connect(ui.textVisibilityCheckBox, SIGNAL(toggled(bool)),
                this, SLOT(onTextVisibilityChanged(bool)));

  // point
  this->connect(ui.pointsTableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
                this, SLOT(onPointsTableWidgetChanged(QTableWidgetItem*)));
  this->connect(ui.pointUnselectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onPointColorChanged(QColor)));
  this->connect(ui.pointSelectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onPointSelectedColorChanged(QColor)));

  this->connect(ui.pointSizeSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onPointSizeChanged(double)));

  this->connect(ui.pointGlyphTypeComboBox, SIGNAL(currentIndexChanged(QString)),
                this, SLOT(onPointGlyphChanged(QString)));

  this->connect(ui.pointOpacitySliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onPointOpacityChanged(double)));
  this->connect(ui.pointAmbientSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onPointAmbientChanged(double)));
  this->connect(ui.pointDiffuseSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onPointDiffuseChanged(double)));
  this->connect(ui.pointSpecularSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onPointSpecularChanged(double)));

  // line
  this->connect(ui.lineUnselectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onLineColorChanged(QColor)));
  this->connect(ui.lineSelectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onLineSelectedColorChanged(QColor)));

  this->connect(ui.lineWidthSliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
                this, SLOT(onLineWidthChanged(double)));
  this->connect(ui.lineLabelPositionSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onLineLabelPositionChanged(double)));
  this->connect(ui.lineLabelVisibilityCheckBox, SIGNAL(stateChanged(int)),
                this, SLOT(onLineLabelVisibilityStateChanged(int)));
  this->connect(ui.lineTickSpacingSlider, SIGNAL(valueChanged(double)),
                this, SLOT(onLineTickSpacingChanged()));

  this->connect(ui.lineMaxTicksSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onLineMaxTicksChanged(double)));

  // line material properties
  this->connect(ui.lineOpacitySliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
                this, SLOT(onLineOpacityChanged(double)));
  this->connect(ui.lineAmbientSliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
                this, SLOT(onLineAmbientChanged(double)));
  this->connect(ui.lineDiffuseSliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
                this, SLOT(onLineDiffuseChanged(double)));
  this->connect(ui.lineSpecularSliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
                this, SLOT(onLineSpecularChanged(double)));

  this->connect(ui.lockUnlockButton, SIGNAL(clicked()), this, SLOT(onLockUnlockButtonClicked()));
  this->connect(ui.visibleInvisibleButton, SIGNAL(clicked()), this, SLOT(onVisibleInvisibleButtonClicked()));
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
void qCjyxAnnotationModulePropertyDialog::onDescriptionTextChanged()
{
  QString text = ui.DescriptionTextEdit->toPlainText();
  this->m_logic->SetAnnotationText(this->m_id.c_str(), text.toUtf8());
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onTextChanged()
{
  QString text = ui.annotationTextEdit->toPlainText();
  this->m_logic->SetAnnotationText(this->m_id.c_str(), text.toUtf8());
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onRASCoordinatesChanged(double *coords)
{
  if (!coords)
    {
    return;
    }
  vtkDMMLNode *node = this->m_logic->GetDMMLScene()->GetNodeByID(this->m_id.c_str());
  // only update if it's a fiducial
  if (!node || !node->IsA("vtkDMMLAnnotationFiducialNode"))
    {
    return;
    }
  vtkDMMLAnnotationFiducialNode* fiducialNode = vtkDMMLAnnotationFiducialNode::SafeDownCast(node);
  if (!fiducialNode)
    {
    return;
    }
  fiducialNode->SetFiducialCoordinates(coords[0], coords[1], coords[2]);
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
void qCjyxAnnotationModulePropertyDialog::saveLinesNode(vtkDMMLAnnotationLinesNode* node)
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
void qCjyxAnnotationModulePropertyDialog::saveControlPoints(vtkDMMLAnnotationControlPointsNode* node)
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
void qCjyxAnnotationModulePropertyDialog::saveAnnotationNode(vtkDMMLAnnotationNode* node)
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
void qCjyxAnnotationModulePropertyDialog::saveStateForUndo(vtkDMMLNode* node)
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
   fiducialNode->GetScene()->SaveStateForUndo(fiducialNode);
   fiducialNode->GetAnnotationTextDisplayNode()->GetScene()->SaveStateForUndo(
   fiducialNode->GetAnnotationTextDisplayNode());
   fiducialNode->GetAnnotationPointDisplayNode()->GetScene()->SaveStateForUndo(
   fiducialNode->GetAnnotationPointDisplayNode());
   }
   */
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::undoLinesNode(vtkDMMLAnnotationLinesNode* node)
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
void qCjyxAnnotationModulePropertyDialog::undoControlPoints(vtkDMMLAnnotationControlPointsNode* node)
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
void qCjyxAnnotationModulePropertyDialog::undoAnnotationNode(vtkDMMLAnnotationNode* node)
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
void qCjyxAnnotationModulePropertyDialog::undo(vtkDMMLNode* node)
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
  qDMMLUtils::qColorToColor(qcolor, color);

  this->m_logic->SetAnnotationTextUnselectedColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onTextSelectedColorChanged(QColor qcolor)
{
  double color[3];
  qDMMLUtils::qColorToColor(qcolor, color);

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
  if (textDisplayNode->GetScene())
    {
    textDisplayNode->GetScene()->SaveStateForUndo();
    }
  textDisplayNode->SetOpacity(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onTextVisibilityChanged(bool value)
{
  // get the text display node
  vtkDMMLAnnotationTextDisplayNode *textDisplayNode = this->m_logic->GetTextDisplayNode(this->m_id.c_str());
  if (!textDisplayNode)
    {
    return;
    }
  if (textDisplayNode->GetScene())
    {
    textDisplayNode->GetScene()->SaveStateForUndo();
    }
  textDisplayNode->SetVisibility(value);
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

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointsTableWidgetChanged(QTableWidgetItem *tableItem)
{
  if (tableItem == nullptr)
    {
    return;
    }
  int row = tableItem->row();
  int col = tableItem->column();
  double newValue = tableItem->data(Qt::DisplayRole).toDouble();

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
//  std::cout << "onPointsTableWidgetChanged: row = " << row << ", col = " <<  col << ", newValue = " << newValue << std::endl;

  // otherwise it's the coordinate that changed
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
  if (col == 0)
    {
    // x
    if (newCoords[0] != newValue)
      {
      newCoords[0] = newValue;
      }
    }
  else if (col == 1)
    {
    // y
    if (newCoords[1] != newValue)
      {
      newCoords[1] = newValue;
      }
    }
  else if (col == 2)
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
    if (pointsNode->GetScene())
      {
      pointsNode->GetScene()->SaveStateForUndo();
      }
    pointsNode->SetControlPoint(row, newCoords);
    }
}
//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointColorChanged(QColor qcolor)
{
  double color[3];
  qDMMLUtils::qColorToColor(qcolor, color);

  this->m_logic->SetAnnotationPointUnselectedColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointSelectedColorChanged(QColor qcolor)
{
  double color[3];
  qDMMLUtils::qColorToColor(qcolor, color);

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
  if (pointDisplayNode->GetScene())
    {
    pointDisplayNode->GetScene()->SaveStateForUndo();
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
  if (pointDisplayNode->GetScene())
    {
    pointDisplayNode->GetScene()->SaveStateForUndo();
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
  if (pointDisplayNode->GetScene())
    {
    pointDisplayNode->GetScene()->SaveStateForUndo();
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
  if (pointDisplayNode->GetScene())
    {
    pointDisplayNode->GetScene()->SaveStateForUndo();
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
  if (pointDisplayNode->GetScene())
    {
    pointDisplayNode->GetScene()->SaveStateForUndo();
    }
  pointDisplayNode->SetSpecular(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onPointGlyphChanged(QString value)
{
//  std::cout << "OnPointGlyphChanged: " << qPrintable(value) << std::endl;
  this->m_logic->SetAnnotationPointGlyphTypeFromString(this->m_id.c_str(), value.toUtf8());
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineColorChanged(QColor qcolor)
{
  double color[3];
  qDMMLUtils::qColorToColor(qcolor, color);

  this->m_logic->SetAnnotationLineUnselectedColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onLineSelectedColorChanged(QColor qcolor)
{
  double color[3];
  qDMMLUtils::qColorToColor(qcolor, color);

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
  if (lineDisplayNode->GetScene())
    {
    lineDisplayNode->GetScene()->SaveStateForUndo();
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
  if (lineDisplayNode->GetScene())
    {
    lineDisplayNode->GetScene()->SaveStateForUndo();
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
  if (lineDisplayNode->GetScene())
    {
    lineDisplayNode->GetScene()->SaveStateForUndo();
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
  double value = ui.lineTickSpacingSlider->value();
  if (lineDisplayNode->GetScene())
    {
    lineDisplayNode->GetScene()->SaveStateForUndo();
    }
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
  if (lineDisplayNode->GetScene())
    {
    lineDisplayNode->GetScene()->SaveStateForUndo();
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
  if (lineDisplayNode->GetScene())
    {
    lineDisplayNode->GetScene()->SaveStateForUndo();
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
  if (lineDisplayNode->GetScene())
    {
    lineDisplayNode->GetScene()->SaveStateForUndo();
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
  if (lineDisplayNode->GetScene())
    {
    lineDisplayNode->GetScene()->SaveStateForUndo();
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
  if (lineDisplayNode->GetScene())
    {
    lineDisplayNode->GetScene()->SaveStateForUndo();
    }
  lineDisplayNode->SetSpecular(value);
}

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::updateLockUnlockStatus(bool isLock)
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

//------------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::formatValueToChar(const char* format, std::vector<
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

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::getAllColor(QColor &qcolor)
{
  // use black as a default
  qcolor.setRgbF(0.0, 0.0, 0.0, 1.0);

  vtkDMMLNode *dmmlNode = this->m_logic->GetDMMLScene()->GetNodeByID(this->m_id.c_str());
  if (!dmmlNode)
    {
    return;
    }

  if (dmmlNode->IsA("vtkDMMLAnnotationHierarchyNode"))
    {
    vtkDMMLAnnotationHierarchyNode *hierarchyNode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(dmmlNode);
    if (hierarchyNode && hierarchyNode->GetDisplayNode())
      {
      double *hierarchyColor = hierarchyNode->GetDisplayNode()->GetColor();
      if (hierarchyColor)
        {
        qDMMLUtils::colorToQColor(hierarchyColor, qcolor);
        }
      }
    return;
    }

  vtkDMMLDisplayableNode *displayableNode = vtkDMMLDisplayableNode::SafeDownCast(dmmlNode);
  if (!displayableNode)
    {
    return;
    }
  int numDisplayNodes = displayableNode->GetNumberOfDisplayNodes();
  double *firstColor = nullptr;
  bool allTheSame = true;
  for (int i = 0; i < numDisplayNodes; ++i)
    {
    vtkDMMLDisplayNode* displayNode = displayableNode->GetNthDisplayNode(i);
    if (displayNode)
      {
      if (i == 0)
        {
        firstColor = displayNode->GetColor();
        }
      else
        {
        double *thisColor = displayNode->GetColor();
        if (thisColor && firstColor &&
            (thisColor[0] != firstColor[0] ||
             thisColor[1] != firstColor[1] ||
             thisColor[2] != firstColor[2]))
          {
          allTheSame = false;
          break;
          }
        }
      }
    }

  if (allTheSame)
    {
    qDMMLUtils::colorToQColor(firstColor, qcolor);
    }
}
//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::updateAllColorButton()
{
  QColor allColor;
  this->getAllColor(allColor);
  ui.allColorPickerButton->setColor(allColor);
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

  if (!this->m_logic)
    {
    return;
    }

  // delete all backups
  this->m_logic->DeleteBackupNodes(this->m_id.c_str());

  emit dialogAccepted();
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::lockUnlockInterface(bool lock)
{
  lock = !lock;

  // top level
  // name and visibility are editable via the data panel, so allow editing
  // them here
  //  ui.nameLineEdit->setEnabled(lock);
  //  ui.visibleInvisibleButton->setEnabled(lock);

  ui.allColorPickerButton->setEnabled(lock);
  ui.sizeSmallPushButton->setEnabled(lock);
  ui.sizeMediumPushButton->setEnabled(lock);
  ui.sizeLargePushButton->setEnabled(lock);
  ui.DescriptionTextEdit->setEnabled(lock);
  ui.RASCoordinatesWidget->setEnabled(lock);

  // Text
  ui.annotationTextEdit->setEnabled(lock);
  ui.measurementLineEdit->setEnabled(lock);
  ui.textSelectedColorPickerButton->setEnabled(lock);
  ui.textUnselectedColorPickerButton->setEnabled(lock);
  ui.textScaleSliderSpinBoxWidget->setEnabled(lock);
  ui.textOpacitySliderSpinBoxWidget->setEnabled(lock);
  ui.textVisibilityCheckBox->setEnabled(lock);


  // Point
  ui.pointsTableWidget->setEnabled(lock);
  ui.pointSelectedColorPickerButton->setEnabled(lock);
  ui.pointUnselectedColorPickerButton->setEnabled(lock);
  ui.pointGlyphTypeComboBox->setEnabled(lock);
  ui.pointAmbientSliderSpinBoxWidget->setEnabled(lock);
  ui.pointDiffuseSliderSpinBoxWidget->setEnabled(lock);
  ui.pointOpacitySliderSpinBoxWidget->setEnabled(lock);
  ui.pointSizeSliderSpinBoxWidget->setEnabled(lock);
  ui.pointSpecularSliderSpinBoxWidget->setEnabled(lock);

  // Line
  ui.lineSelectedColorPickerButton->setEnabled(lock);
  ui.lineUnselectedColorPickerButton->setEnabled(lock);
  ui.lineAmbientSliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineDiffuseSliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineOpacitySliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineSpecularSliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineWidthSliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineLabelPositionSliderSpinBoxWidget->setEnabled(lock);
  ui.lineLabelVisibilityCheckBox->setEnabled(lock);
  ui.lineTickSpacingSlider->setEnabled(lock);
  ui.lineMaxTicksSliderSpinBoxWidget->setEnabled(lock);

  // ROI
  ui.ROIWidget->setEnabled(lock);
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::updateTypeLabelText()
{
  vtkDMMLNode *dmmlNode = this->m_logic->GetDMMLScene()->GetNodeByID(this->m_id.c_str());
  ui.typeNameLabel->setText(dmmlNode ? dmmlNode->GetNodeTagName() : "");
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::updateIDLabelText()
{
//  ui.idNameLabel->setText(QString::fromStdString(this->m_id));
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::updateNameText()
{
  ui.nameLineEdit->setText(this->m_logic->GetAnnotationName(this->m_id));
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onNameLineEditChanged()
{
  vtkDMMLNode* node = this->m_logic->GetDMMLScene()->GetNodeByID(this->m_id.c_str());
  if (!node)
    {
    return;
    }
  // change the name
  QString name = ui.nameLineEdit->text();
  node->SetName(name.toUtf8());
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onAllColorChanged(QColor qcolor)
{
  double color[3];
  qDMMLUtils::qColorToColor(qcolor, color);

  // is it a hierarchy node?
  if (this->m_logic->IsAnnotationHierarchyNode(this->m_id))
    {
    vtkDMMLNode *dmmlNode = this->m_logic->GetDMMLScene()->GetNodeByID(this->m_id.c_str());
    if (dmmlNode)
      {
      vtkDMMLAnnotationHierarchyNode *hnode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(dmmlNode);
      if (hnode && hnode->GetDisplayNode())
        {
        hnode->GetDisplayNode()->SetColor(color);
        }
      // push down to all the nodes in the hierarchy?
      if (ui.hierarchyPushCheckBox->isChecked())
        {
        std::vector< vtkDMMLHierarchyNode *> allChildren;
        hnode->GetAllChildrenNodes(allChildren);
        for (unsigned int i = 0; i < allChildren.size(); ++i)
          {
          // set hierarchy display node color
          vtkDMMLDisplayableHierarchyNode *dispHierarchyNode = vtkDMMLDisplayableHierarchyNode::SafeDownCast(allChildren[i]);
          if (dispHierarchyNode && dispHierarchyNode->GetDisplayNode())
            {
            dispHierarchyNode->GetDisplayNode()->SetColor(color);
            }
          // and associated node color
          if (allChildren[i]->GetAssociatedNode())
            {
            this->setColorOnAnnotationDisplayNodes(allChildren[i]->GetAssociatedNode()->GetID(), qcolor);
            }
          }
        }
      }
    return;
    }

  // look for the display nodes on regular annotation nodes
  // and update the unselected color buttons to reflect the new color
  this->setColorOnAnnotationDisplayNodes(this->m_id.c_str(), qcolor);
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::setColorOnAnnotationDisplayNodes(const char *id, QColor qcolor)
{
  if (!id)
    {
    return;
    }
  double color[3];
  qDMMLUtils::qColorToColor(qcolor, color);
  // get the text display node
  vtkDMMLAnnotationTextDisplayNode *textDisplayNode = this->m_logic->GetTextDisplayNode(id);
  if (textDisplayNode)
    {
    textDisplayNode->SetColor(color);
    // if it's the currently displayed node, update the button
    if (this->m_id.compare(id) == 0)
      {
      ui.textUnselectedColorPickerButton->setColor(qcolor);
      }
    }

  // get the point display node
  vtkDMMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(id);
  if (pointDisplayNode)
    {
    pointDisplayNode->SetColor(color);
    if (this->m_id.compare(id) == 0)
      {
      ui.pointUnselectedColorPickerButton->setColor(qcolor);
      }
    }
  // get the line display node
  vtkDMMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(id);
  if (lineDisplayNode)
    {
    lineDisplayNode->SetColor(color);
    if (this->m_id.compare(id) == 0)
      {
      ui.lineUnselectedColorPickerButton->setColor(qcolor);
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onHierarchyPointSizeChanged(double value)
{
  // is it a hierarchy node?
  if (!this->m_logic->IsAnnotationHierarchyNode(this->m_id))
    {
    return;
    }
  vtkDMMLNode *dmmlNode = this->m_logic->GetDMMLScene()->GetNodeByID(this->m_id.c_str());
  if (!dmmlNode)
    {
    return;
    }
  vtkDMMLAnnotationHierarchyNode *hnode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(dmmlNode);
  if (!hnode)
    {
    return;
    }
  std::vector< vtkDMMLHierarchyNode *> allChildren;
  hnode->GetAllChildrenNodes(allChildren);
  for (unsigned int i = 0; i < allChildren.size(); ++i)
    {
    // set on the associated node
    if (allChildren[i]->GetAssociatedNode())
      {
      vtkDMMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(allChildren[i]->GetAssociatedNode()->GetID());
      if (pointDisplayNode)
        {
        pointDisplayNode->SetGlyphScale(value);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onHierarchyPointSizeDefaultButtonClicked()
{
  vtkDMMLAnnotationPointDisplayNode *pdNode = vtkDMMLAnnotationPointDisplayNode::New();
  double glyphScale = pdNode->GetGlyphScale();
  pdNode->Delete();
  ui.hierarchyPointSizeSlider->setValue(glyphScale);
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onHierarchyTextScaleChanged(double value)
{
  // is it a hierarchy node?
  if (!this->m_logic->IsAnnotationHierarchyNode(this->m_id))
    {
    return;
    }
  vtkDMMLNode *dmmlNode = this->m_logic->GetDMMLScene()->GetNodeByID(this->m_id.c_str());
  if (!dmmlNode)
    {
    return;
    }
  vtkDMMLAnnotationHierarchyNode *hnode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(dmmlNode);
  if (!hnode)
    {
    return;
    }
  std::vector< vtkDMMLHierarchyNode *> allChildren;
  hnode->GetAllChildrenNodes(allChildren);
  for (unsigned int i = 0; i < allChildren.size(); ++i)
    {
    // set on the associated node
    if (allChildren[i]->GetAssociatedNode() &&
        allChildren[i]->GetAssociatedNode()->GetID())
      {
      this->m_logic->SetAnnotationTextScale(allChildren[i]->GetAssociatedNode()->GetID(), value);
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onHierarchyTextScaleDefaultButtonClicked()
{
  vtkDMMLAnnotationTextDisplayNode *tdNode = vtkDMMLAnnotationTextDisplayNode::New();
  double textScale = tdNode->GetTextScale();
  tdNode->Delete();

  ui.hierarchyTextScaleSlider->setValue(textScale);
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onHierarchyPointGlyphChanged(QString value)
{
  // is it a hierarchy node?
  if (!this->m_logic->IsAnnotationHierarchyNode(this->m_id))
    {
    return;
    }
  vtkDMMLNode *dmmlNode = this->m_logic->GetDMMLScene()->GetNodeByID(this->m_id.c_str());
  if (!dmmlNode)
    {
    return;
    }
  vtkDMMLAnnotationHierarchyNode *hnode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(dmmlNode);
  if (!hnode)
    {
    return;
    }

  std::vector< vtkDMMLHierarchyNode *> allChildren;
  hnode->GetAllChildrenNodes(allChildren);
  for (unsigned int i = 0; i < allChildren.size(); ++i)
    {
    // set on the associated node
    if (allChildren[i]->GetAssociatedNode() &&
        allChildren[i]->GetAssociatedNode()->GetID())
      {
      this->m_logic->SetAnnotationPointGlyphTypeFromString(allChildren[i]->GetAssociatedNode()->GetID(), value.toUtf8());
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onHierarchyPointGlyphTypeDefaultButtonClicked()
{
  vtkDMMLAnnotationPointDisplayNode *pdNode = vtkDMMLAnnotationPointDisplayNode::New();
  QString glyphType = QString(pdNode->GetGlyphTypeAsString());
  int index =  ui.hierarchyPointGlyphTypeComboBox->findData(glyphType);
  if (index != -1)
    {
    ui.hierarchyPointGlyphTypeComboBox->setCurrentIndex(index);
    }
  else
    {
    // glyph types start at 1, combo box is 0 indexed
    int newIndex = pdNode->GetGlyphType() - 1;
    ui.hierarchyPointGlyphTypeComboBox->setCurrentIndex(newIndex);
    if (newIndex == -1)
      {
      qWarning() << "Unable to find default glyph type index for glyph type " << qPrintable(glyphType) << ", combo box count = " << ui.hierarchyPointGlyphTypeComboBox->count();
      }
    }
  pdNode->Delete();
}
//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog::onSizeSmallPushButtonClicked()
{
  // get and use the default sizes from the display nodes and scale them down
  vtkNew<vtkDMMLAnnotationTextDisplayNode> defaultTextDisplayNode;
  double defaultTextSize = defaultTextDisplayNode->GetTextScale();
  vtkDMMLAnnotationTextDisplayNode *textDisplayNode = this->m_logic->GetTextDisplayNode(this->m_id.c_str());
  if (textDisplayNode)
    {
    textDisplayNode->SetTextScale(defaultTextSize / 2.0);
    }

  vtkNew<vtkDMMLAnnotationPointDisplayNode> defaultPointDisplayNode;
  double defaultPointSize = defaultPointDisplayNode->GetGlyphScale();
  vtkDMMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(this->m_id.c_str());
  if (pointDisplayNode)
    {
    pointDisplayNode->SetGlyphScale(defaultPointSize / 2.0);
    }
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog:: onSizeMediumPushButtonClicked()
{
  // get and use the default sizes from the display nodes and scale them down
  vtkNew<vtkDMMLAnnotationTextDisplayNode> defaultTextDisplayNode;
  double defaultTextSize = defaultTextDisplayNode->GetTextScale();
  vtkDMMLAnnotationTextDisplayNode *textDisplayNode = this->m_logic->GetTextDisplayNode(this->m_id.c_str());
  if (textDisplayNode)
    {
    textDisplayNode->SetTextScale(defaultTextSize * 0.75);
    }

  vtkNew<vtkDMMLAnnotationPointDisplayNode> defaultPointDisplayNode;
  double defaultPointSize = defaultPointDisplayNode->GetGlyphScale();
  vtkDMMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(this->m_id.c_str());
  if (pointDisplayNode)
    {
    pointDisplayNode->SetGlyphScale(defaultPointSize * 0.75);
    }
}

//-----------------------------------------------------------------------------
void qCjyxAnnotationModulePropertyDialog:: onSizeLargePushButtonClicked()
{
  // get and use the default sizes from the display nodes
  vtkNew<vtkDMMLAnnotationTextDisplayNode> defaultTextDisplayNode;
  double defaultTextSize = defaultTextDisplayNode->GetTextScale();
  vtkDMMLAnnotationTextDisplayNode *textDisplayNode = this->m_logic->GetTextDisplayNode(this->m_id.c_str());
  if (textDisplayNode)
    {
    textDisplayNode->SetTextScale(defaultTextSize);
    }

  vtkNew<vtkDMMLAnnotationPointDisplayNode> defaultPointDisplayNode;
  double defaultPointSize = defaultPointDisplayNode->GetGlyphScale();
  vtkDMMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(this->m_id.c_str());
  if (pointDisplayNode)
    {
    pointDisplayNode->SetGlyphScale(defaultPointSize);
    }
}
