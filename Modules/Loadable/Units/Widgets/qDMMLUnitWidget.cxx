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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>


// Cjyx includes
#include "qDMMLUnitWidget.h"
#include "ui_qDMMLUnitWidget.h"

// DMML includes
#include "vtkDMMLUnitNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_ExtensionTemplate
class qDMMLUnitWidgetPrivate: public Ui_qDMMLUnitWidget
{
  Q_DECLARE_PUBLIC(qDMMLUnitWidget);
protected:
  qDMMLUnitWidget* const q_ptr;

public:
  qDMMLUnitWidgetPrivate(qDMMLUnitWidget& obj);

  void setupUi(qDMMLUnitWidget*);
  void clear();
  void updatePropertyWidgets();

  vtkDMMLUnitNode* CurrentUnitNode;
  qDMMLUnitWidget::UnitProperties DisplayFlags;
  qDMMLUnitWidget::UnitProperties EditableProperties;
};

//-----------------------------------------------------------------------------
// qDMMLUnitWidgetPrivate methods

//-----------------------------------------------------------------------------
qDMMLUnitWidgetPrivate::qDMMLUnitWidgetPrivate(
  qDMMLUnitWidget& object)
  : q_ptr(&object)
{
  this->CurrentUnitNode = nullptr;
  this->DisplayFlags = qDMMLUnitWidget::All;
  this->EditableProperties = qDMMLUnitWidget::All;
  this->EditableProperties &= ~qDMMLUnitWidget::Quantity;
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidgetPrivate::setupUi(qDMMLUnitWidget* q)
{
  this->Ui_qDMMLUnitWidget::setupUi(q);

  QObject::connect(this->NameLineEdit, SIGNAL(textChanged(QString)),
    q, SLOT(setName(QString)));
  QObject::connect(this->NameLineEdit, SIGNAL(textChanged(QString)),
    q, SIGNAL(nameChanged(QString)));
  QObject::connect(this->QuantityLineEdit, SIGNAL(textChanged(QString)),
    q, SLOT(setQuantity(QString)));
  QObject::connect(this->QuantityLineEdit, SIGNAL(textChanged(QString)),
    q, SIGNAL(quantityChanged(QString)));

  QObject::connect(this->PrefixLineEdit, SIGNAL(textChanged(QString)),
    q, SLOT(setPrefix(QString)));
  QObject::connect(this->PrefixLineEdit, SIGNAL(textChanged(QString)),
    q, SIGNAL(prefixChanged(QString)));
  QObject::connect(this->SuffixLineEdit, SIGNAL(textChanged(QString)),
    q, SLOT(setSuffix(QString)));
  QObject::connect(this->SuffixLineEdit, SIGNAL(textChanged(QString)),
    q, SIGNAL(suffixChanged(QString)));

  QObject::connect(this->PrecisionSpinBox, SIGNAL(valueChanged(int)),
    q, SLOT(setPrecision(int)));
  QObject::connect(this->PrecisionSpinBox, SIGNAL(valueChanged(int)),
    q, SIGNAL(precisionChanged(int)));

  QObject::connect(this->MinimumSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setMinimum(double)));
  QObject::connect(this->MinimumSpinBox, SIGNAL(valueChanged(double)),
    q, SIGNAL(minimumChanged(double)));
  QObject::connect(this->MaximumSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setMaximum(double)));
  QObject::connect(this->MaximumSpinBox, SIGNAL(valueChanged(double)),
    q, SIGNAL(maximumChanged(double)));

  QObject::connect(this->CoefficientSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setCoefficient(double)));
  QObject::connect(this->CoefficientSpinBox, SIGNAL(valueChanged(double)),
    q, SIGNAL(coefficientChanged(double)));
  QObject::connect(this->OffsetSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setOffset(double)));
  QObject::connect(this->OffsetSpinBox, SIGNAL(valueChanged(double)),
    q, SIGNAL(offsetChanged(double)));

  QObject::connect(this->PresetNodeComboBox,
    SIGNAL(currentNodeChanged(vtkDMMLNode*)),
    q, SLOT(setUnitFromPreset(vtkDMMLNode*)));
  this->updatePropertyWidgets();
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidgetPrivate::clear()
{
  this->NameLineEdit->clear();
  this->QuantityLineEdit->clear();
  this->PrefixLineEdit->clear();
  this->SuffixLineEdit->clear();
  this->PrecisionSpinBox->setValue(3);
  this->MinimumSpinBox->setValue(-1000);
  this->MaximumSpinBox->setValue(1000);
  this->CoefficientSpinBox->setValue(1.0);
  this->OffsetSpinBox->setValue(0.0);
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidgetPrivate::updatePropertyWidgets()
{
  this->PresetNodeComboBox->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Preset));
  this->PresetNodeComboBox->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Preset));
  this->PresetLabel->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Preset));
  this->PresetLabel->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Preset));

  this->SeparationLine->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Preset)
    && this->DisplayFlags > qDMMLUnitWidget::Preset);

  this->NameLineEdit->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Name));
  this->NameLineEdit->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Name));
  this->NameLabel->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Name));
  this->NameLabel->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Name));

  this->QuantityLineEdit->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Quantity));
  this->QuantityLineEdit->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Quantity));
  this->QuantityLabel->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Quantity));
  this->QuantityLabel->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Quantity));

  this->PrefixLineEdit->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Prefix));
  this->PrefixLineEdit->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Prefix));
  this->PrefixLabel->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Prefix));
  this->PrefixLabel->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Prefix));

  this->SuffixLineEdit->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Suffix));
  this->SuffixLineEdit->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Suffix));
  this->SuffixLabel->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Suffix));
  this->SuffixLabel->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Suffix));

  this->PrecisionSpinBox->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Precision));
  this->PrecisionSpinBox->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Precision));
  this->PrecisionLabel->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Precision));
  this->PrecisionLabel->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Precision));

  this->MinimumSpinBox->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Minimum));
  this->MinimumSpinBox->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Minimum));
  this->MinimumValueLabel->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Minimum));
  this->MinimumValueLabel->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Minimum));

  this->MaximumSpinBox->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Maximum));
  this->MaximumSpinBox->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Maximum));
  this->MaximumValueLabel->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Maximum));
  this->MaximumValueLabel->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Maximum));

  this->CoefficientSpinBox->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Coefficient));
  this->CoefficientSpinBox->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Coefficient));
  this->CoefficientLabel->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Coefficient));
  this->CoefficientLabel->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Coefficient));

   this->OffsetSpinBox->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Offset));
  this->OffsetSpinBox->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Offset));
  this->OffsetLabel->setVisible(
    this->DisplayFlags.testFlag(qDMMLUnitWidget::Offset));
  this->OffsetLabel->setEnabled(
    this->EditableProperties.testFlag(qDMMLUnitWidget::Offset));
}

//-----------------------------------------------------------------------------
// qDMMLUnitWidget methods

//-----------------------------------------------------------------------------
qDMMLUnitWidget::qDMMLUnitWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qDMMLUnitWidgetPrivate(*this) )
{
  Q_D(qDMMLUnitWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qDMMLUnitWidget::~qDMMLUnitWidget() = default;

//-----------------------------------------------------------------------------
qDMMLUnitWidget::UnitProperties qDMMLUnitWidget::displayedProperties() const
{
  Q_D(const qDMMLUnitWidget);
  return d->DisplayFlags;
}

//-----------------------------------------------------------------------------
qDMMLUnitWidget::UnitProperties qDMMLUnitWidget::editableProperties() const
{
  Q_D(const qDMMLUnitWidget);
  return d->EditableProperties;
}
//-----------------------------------------------------------------------------
void qDMMLUnitWidget::setDMMLScene(vtkDMMLScene* scene)
{
  this->Superclass::setDMMLScene(scene);
  this->updateWidgetFromNode();
}

//-----------------------------------------------------------------------------
vtkDMMLNode* qDMMLUnitWidget::currentNode() const
{
  Q_D(const qDMMLUnitWidget);
  return d->CurrentUnitNode;
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget::setCurrentNode(vtkDMMLNode* node)
{
  Q_D(qDMMLUnitWidget);

  vtkDMMLUnitNode* unitNode = vtkDMMLUnitNode::SafeDownCast(node);
  this->qvtkReconnect(d->CurrentUnitNode, unitNode,
    vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromNode()));
  d->CurrentUnitNode = unitNode;

  this->updateWidgetFromNode();
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget::updateWidgetFromNode()
{
  Q_D(qDMMLUnitWidget);

  d->PresetNodeComboBox->setEnabled(d->CurrentUnitNode != nullptr);
  d->NameLineEdit->setEnabled(d->CurrentUnitNode != nullptr);
  d->PrefixLineEdit->setEnabled(d->CurrentUnitNode != nullptr);
  d->SuffixLineEdit->setEnabled(d->CurrentUnitNode != nullptr);
  d->PrecisionSpinBox->setEnabled(d->CurrentUnitNode != nullptr);
  d->MinimumSpinBox->setEnabled(d->CurrentUnitNode != nullptr);
  d->MaximumSpinBox->setEnabled(d->CurrentUnitNode != nullptr);
  d->CoefficientSpinBox->setEnabled(d->CurrentUnitNode != nullptr);
  d->OffsetSpinBox->setEnabled(d->CurrentUnitNode != nullptr);

  if (!d->CurrentUnitNode)
    {
    d->clear();
    return;
    }

  // Preset
  bool modifying = d->PresetNodeComboBox->blockSignals(true);
  d->PresetNodeComboBox->addAttribute(
    "vtkDMMLUnitNode", "Quantity", d->CurrentUnitNode->GetQuantity());
  d->PresetNodeComboBox->setDMMLScene(this->dmmlScene());
  d->PresetNodeComboBox->setCurrentNode(nullptr);
  d->PresetNodeComboBox->blockSignals(modifying);

  d->NameLineEdit->setText(d->CurrentUnitNode->GetName());
  d->QuantityLineEdit->setText(d->CurrentUnitNode->GetQuantity());
  d->SuffixLineEdit->setText(d->CurrentUnitNode->GetSuffix());
  d->PrefixLineEdit->setText(QString(d->CurrentUnitNode->GetPrefix()));
  d->PrecisionSpinBox->setValue(d->CurrentUnitNode->GetPrecision());
  d->MinimumSpinBox->setValue(d->CurrentUnitNode->GetMinimumValue());
  d->MaximumSpinBox->setValue(d->CurrentUnitNode->GetMaximumValue());
  d->CoefficientSpinBox->setValue(d->CurrentUnitNode->GetDisplayCoefficient());
  d->OffsetSpinBox->setValue(d->CurrentUnitNode->GetDisplayOffset());
}

//-----------------------------------------------------------------------------
QString qDMMLUnitWidget::name() const
{
  Q_D(const qDMMLUnitWidget);
  return d->NameLineEdit->text();
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget::setName(const QString& newName)
{
  Q_D(qDMMLUnitWidget);

  if (d->CurrentUnitNode)
    {
    d->CurrentUnitNode->SetName(newName.toUtf8());
    }
}

//-----------------------------------------------------------------------------
QString qDMMLUnitWidget::quantity() const
{
  Q_D(const qDMMLUnitWidget);
  return d->QuantityLineEdit->text();
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget::setQuantity(const QString& newQuantity)
{
  Q_D(qDMMLUnitWidget);

  if (d->CurrentUnitNode)
    {
    d->CurrentUnitNode->SetQuantity(newQuantity.toUtf8());
    }
}

//-----------------------------------------------------------------------------
QString qDMMLUnitWidget::prefix() const
{
  Q_D(const qDMMLUnitWidget);
  return d->PrefixLineEdit->text();
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget::setPrefix(const QString& newPrefix)
{
  Q_D(qDMMLUnitWidget);

  if (d->CurrentUnitNode)
    {
    d->CurrentUnitNode->SetPrefix(newPrefix.toUtf8());
    }
  d->MaximumSpinBox->setPrefix(newPrefix);
  d->MinimumSpinBox->setPrefix(newPrefix);
}

//-----------------------------------------------------------------------------
QString qDMMLUnitWidget::suffix() const
{
  Q_D(const qDMMLUnitWidget);
  return d->SuffixLineEdit->text();
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget::setSuffix(const QString& newSuffix)
{
  Q_D(qDMMLUnitWidget);

  if (d->CurrentUnitNode)
    {
    d->CurrentUnitNode->SetSuffix(newSuffix.toUtf8());
    }
  d->MaximumSpinBox->setSuffix(newSuffix);
  d->MinimumSpinBox->setSuffix(newSuffix);
}

//-----------------------------------------------------------------------------
int qDMMLUnitWidget::precision() const
{
  Q_D(const qDMMLUnitWidget);
  return d->PrecisionSpinBox->value();
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget::setPrecision(int newPrecision)
{
  Q_D(qDMMLUnitWidget);

  if (d->CurrentUnitNode)
    {
    d->CurrentUnitNode->SetPrecision(newPrecision);
    }
  d->MaximumSpinBox->setDecimals(newPrecision);
  d->MinimumSpinBox->setDecimals(newPrecision);

  d->CoefficientSpinBox->setDecimals(newPrecision);
  d->OffsetSpinBox->setDecimals(newPrecision);
}

//-----------------------------------------------------------------------------
double qDMMLUnitWidget::minimum() const
{
  Q_D(const qDMMLUnitWidget);
  return d->MinimumSpinBox->value();
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget::setMinimum(double newMin)
{
  Q_D(qDMMLUnitWidget);

  if (!d->CurrentUnitNode)
    {
    return;
    }

  d->CurrentUnitNode->SetMinimumValue(newMin);
}

//-----------------------------------------------------------------------------
double qDMMLUnitWidget::maximum() const
{
  Q_D(const qDMMLUnitWidget);
  return d->MaximumSpinBox->value();
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget::setMaximum(double newMax)
{
  Q_D(qDMMLUnitWidget);

  if (!d->CurrentUnitNode)
    {
    return;
    }

  d->CurrentUnitNode->SetMaximumValue(newMax);
}

//-----------------------------------------------------------------------------
double qDMMLUnitWidget::coefficient() const
{
  Q_D(const qDMMLUnitWidget);
  return d->CoefficientSpinBox->value();
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget::setCoefficient(double newCoeff)
{
  Q_D(qDMMLUnitWidget);

  if (!d->CurrentUnitNode)
    {
    return;
    }

  d->CurrentUnitNode->SetDisplayCoefficient(newCoeff);
}

//-----------------------------------------------------------------------------
double qDMMLUnitWidget::offset() const
{
  Q_D(const qDMMLUnitWidget);
  return d->OffsetSpinBox->value();
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget::setOffset(double newOffset)
{
  Q_D(qDMMLUnitWidget);

  if (!d->CurrentUnitNode)
    {
    return;
    }

  d->CurrentUnitNode->SetDisplayOffset(newOffset);
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget::setUnitFromPreset(vtkDMMLNode* presetNode)
{
  Q_D(qDMMLUnitWidget);

  vtkDMMLUnitNode *presetUnitNode = vtkDMMLUnitNode::SafeDownCast(presetNode);
  if (!presetUnitNode || !d->CurrentUnitNode)
    {
    return;
    }

  int disabledModify = d->CurrentUnitNode->StartModify();
  d->CurrentUnitNode->SetQuantity(presetUnitNode->GetQuantity());
  d->CurrentUnitNode->SetPrefix(presetUnitNode->GetPrefix());
  d->CurrentUnitNode->SetSuffix(presetUnitNode->GetSuffix());
  d->CurrentUnitNode->SetPrecision(presetUnitNode->GetPrecision());
  d->CurrentUnitNode->SetMinimumValue(presetUnitNode->GetMinimumValue());
  d->CurrentUnitNode->SetMaximumValue(presetUnitNode->GetMaximumValue());
  d->CurrentUnitNode->SetDisplayCoefficient(
    presetUnitNode->GetDisplayCoefficient());
  d->CurrentUnitNode->SetDisplayOffset(presetUnitNode->GetDisplayOffset());
  d->CurrentUnitNode->EndModify(disabledModify);
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget
::setDisplayedProperties(qDMMLUnitWidget::UnitProperties flag)
{
  Q_D(qDMMLUnitWidget);

  if (d->DisplayFlags == flag)
    {
    return;
    }

  d->DisplayFlags = flag;
  d->updatePropertyWidgets();
}

//-----------------------------------------------------------------------------
void qDMMLUnitWidget
::setEditableProperties(qDMMLUnitWidget::UnitProperties properties)
{
  Q_D(qDMMLUnitWidget);

  if (d->EditableProperties == properties)
    {
    return;
    }

  d->EditableProperties = properties;
  d->updatePropertyWidgets();
}

