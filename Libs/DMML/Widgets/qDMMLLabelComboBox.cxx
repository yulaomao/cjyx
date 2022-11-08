
// Qt includes
#include <QVBoxLayout>
#include <QDebug>

// CTK includes
#include <ctkComboBox.h>
#include <ctkUtils.h>

// qDMML includes
#include "qDMMLLabelComboBox.h"
#include "qDMMLUtils.h"

// DMML includes
#include <vtkDMMLColorNode.h>

// VTK includes
#include <vtkLookupTable.h>

//-----------------------------------------------------------------------------
class qDMMLLabelComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qDMMLLabelComboBox);
protected:
  qDMMLLabelComboBox* const q_ptr;
public:
  qDMMLLabelComboBoxPrivate(qDMMLLabelComboBox& object);

  void setDMMLColorNode(vtkDMMLColorNode *newDMMLColorNode);

  QColor colorFromIndex(int index) const;

  ctkComboBox *       ComboBox;
  bool                NoneEnabled;
  vtkDMMLColorNode *  ColorNode;
  int                 CurrentColor;
  int                 MaximumColorCount;
  bool                ColorNameVisible;
  bool                LabelValueVisible;
};

// --------------------------------------------------------------------------
// qDMMLLabelComboBoxPrivate methods

// --------------------------------------------------------------------------
qDMMLLabelComboBoxPrivate::qDMMLLabelComboBoxPrivate(qDMMLLabelComboBox& object)
  : q_ptr(&object)
{
  this->ComboBox = nullptr;
  this->NoneEnabled = false;
  this->ColorNode = nullptr;
  this->CurrentColor = -1;
  this->MaximumColorCount = 0;
  this->ColorNameVisible = true;
  this->LabelValueVisible = false;
}

// ------------------------------------------------------------------------------
void qDMMLLabelComboBoxPrivate::setDMMLColorNode(vtkDMMLColorNode * newDMMLColorNode)
{
  Q_Q(qDMMLLabelComboBox);

  if (this->ColorNode == newDMMLColorNode)
    {
    return;
    }
  q->qvtkReconnect(this->ColorNode, newDMMLColorNode, vtkCommand::ModifiedEvent,
                      q, SLOT(updateWidgetFromDMML()));
  q->setEnabled(newDMMLColorNode != nullptr);
  this->ColorNode = newDMMLColorNode;

  if (this->ColorNode)
    {
    q->updateWidgetFromDMML();
    }
  else
    {
    this->ComboBox->clear();
    }
}

// ------------------------------------------------------------------------------
QColor qDMMLLabelComboBoxPrivate::colorFromIndex(int index) const
{
  //qDebug() << "qDMMLLabelComboBox::colorFromIndex - index:" << index;
  if (index < 0 || this->ColorNode == nullptr)
    {
    return QColor::Invalid;
    }

  double colorTable[4];
  vtkLookupTable *table = this->ColorNode->GetLookupTable();

  table->GetTableValue(index, colorTable);

  // HACK - The alpha associated with Black was 0
  if (colorTable[0] == 0 && colorTable[1] == 0 && colorTable[2] == 0)
    {
    colorTable[3] = 1;
    }

  return QColor::fromRgbF(colorTable[0], colorTable[1], colorTable[2], colorTable[3]);
}

// --------------------------------------------------------------------------
// qDMMLLabelComboBox methods

// --------------------------------------------------------------------------
qDMMLLabelComboBox::qDMMLLabelComboBox(QWidget* newParent)
  : Superclass(newParent)
  , d_ptr(new qDMMLLabelComboBoxPrivate(*this))
{
  Q_D(qDMMLLabelComboBox);

  d->ComboBox = new ctkComboBox(this);
  d->ComboBox->setDefaultText("None");
  this->setLayout(new QVBoxLayout);
  this->layout()->addWidget(d->ComboBox);
  this->layout()->setContentsMargins(0, 0, 0, 0);

  this->connect(d->ComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onCurrentIndexChanged(int)));

  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qDMMLLabelComboBox::~qDMMLLabelComboBox() = default;

// ----------------------------------------------------------------
void qDMMLLabelComboBox::printAdditionalInfo()
{
  Q_D(qDMMLLabelComboBox);
  qDebug().nospace() << "qDMMLLabelComboBox:" << this << ctk::endl
      << " DMMLColorNode:" << d->ColorNode << ctk::endl
      << "  ClassName:" << (d->ColorNode ? d->ColorNode->GetClassName() : "null") << ctk::endl
      << "  ID:" << (d->ColorNode ? d->ColorNode->GetID() : "null") << ctk::endl
      << "  Type:" << (d->ColorNode ? d->ColorNode->GetTypeAsString() : "null") << ctk::endl
      << " CurrentColor:" << d->CurrentColor << ctk::endl
      << " NoneEnabled:" << d->NoneEnabled << ctk::endl
      << " ColorNameVisible:" << d->ColorNameVisible << ctk::endl;
}

// ---------------------------------------------------------------------------------
CTK_GET_CPP(qDMMLLabelComboBox, vtkDMMLColorNode*, dmmlColorNode, ColorNode);

// ------------------------------------------------------------------------------
CTK_GET_CPP(qDMMLLabelComboBox, int, currentColor, CurrentColor);

// ------------------------------------------------------------------------------
void qDMMLLabelComboBox::setCurrentColor(int index)
{
  Q_D(qDMMLLabelComboBox);
  //qDebug() << "qDMMLLabelComboBox::setCurrentColor - index:" << index;

  if (index == d->CurrentColor)
    {
    return;
    }

  if (d->NoneEnabled)
    {
    if (index < -1 || index >= (d->ComboBox->count() - 1) )
      {
      return;
      }
    index++;
    }
  else
    {
    if (index < 0 || index >= d->ComboBox->count())
      {
      return;
      }
    }

  // Will trigger onCurrentIndexChanged
  d->ComboBox->setCurrentIndex(index);
}

// ------------------------------------------------------------------------------
void qDMMLLabelComboBox::setCurrentColor(const QString& color)
{
  Q_D(qDMMLLabelComboBox);
  int index = d->ComboBox->findText(color);
  // Will trigger onCurrentIndexChanged
  d->ComboBox->setCurrentIndex(index);
}

// ------------------------------------------------------------------------------
QString qDMMLLabelComboBox::currentColorName()const
{
  Q_D(const qDMMLLabelComboBox);
  return d->ComboBox->currentText();
}

// ------------------------------------------------------------------------------
CTK_GET_CPP(qDMMLLabelComboBox, bool, noneEnabled, NoneEnabled);

// -------------------------------------------------------------------------------
void qDMMLLabelComboBox::setNoneEnabled(bool enabled)
{
  Q_D(qDMMLLabelComboBox);

  if (d->NoneEnabled == enabled)
    {
    return;
    }
  d->NoneEnabled = enabled;

  if (enabled)
    {
    d->ComboBox->insertItem(0, "None");
    }
  else
    {
    d->ComboBox->removeItem(0);
    }
}
// ------------------------------------------------------------------------------
CTK_GET_CPP(qDMMLLabelComboBox, bool, colorNameVisible, ColorNameVisible);

// -------------------------------------------------------------------------------
void qDMMLLabelComboBox::setColorNameVisible(bool visible)
{
  Q_D(qDMMLLabelComboBox);

  if ( visible != d->ColorNameVisible )
    {
    d->ColorNameVisible = visible;
    this->updateWidgetFromDMML();
    }
}

// ------------------------------------------------------------------------------
CTK_GET_CPP(qDMMLLabelComboBox, bool, labelValueVisible, LabelValueVisible);

// -------------------------------------------------------------------------------
void qDMMLLabelComboBox::setLabelValueVisible(bool visible)
{
  Q_D(qDMMLLabelComboBox);

  if ( visible != d->LabelValueVisible )
    {
    d->LabelValueVisible = visible;
    this->updateWidgetFromDMML();
    }
}

// ---------------------------------------------------------------------------------
void qDMMLLabelComboBox::setMaximumColorCount(int maximum)
{
  Q_D(qDMMLLabelComboBox);
  d->MaximumColorCount = maximum <= 0 ? 0 : maximum;
}

// ---------------------------------------------------------------------------------
CTK_GET_CPP(qDMMLLabelComboBox, int, maximumColorCount, MaximumColorCount);

// ---------------------------------------------------------------------------------
// qDMMLLabelComboBox Slots

// ---------------------------------------------------------------------------------
void qDMMLLabelComboBox::setDMMLColorNode(vtkDMMLNode *newDMMLColorNode)
{
  Q_D(qDMMLLabelComboBox);
  d->setDMMLColorNode(vtkDMMLColorNode::SafeDownCast(newDMMLColorNode));
}

// ------------------------------------------------------------------------------
void qDMMLLabelComboBox::onCurrentIndexChanged(int index)
{
  Q_D(qDMMLLabelComboBox);
  //qDebug() << "qDMMLLabelComboBox::onCurrentIndexChanged - index:" << index;
  if (d->NoneEnabled)
    {
    index--;
    }

  d->CurrentColor = index;

  emit currentColorChanged(d->colorFromIndex(index));
  emit currentColorChanged(d->ComboBox->itemText(index));
  emit currentColorChanged(index);
}

// ---------------------------------------------------------------------------------
void qDMMLLabelComboBox::updateWidgetFromDMML()
{
  Q_D(qDMMLLabelComboBox);
  Q_ASSERT(d->ColorNode);

  //qDebug() << "qDMMLLabelComboBox::updateWidgetFromDMML";

  d->ComboBox->clear();

  if (!d->ColorNode->GetNamesInitialised())
    {
    qCritical() << "qDMMLLabelComboBox::updateWidgetFromDMML - ColorNode names are NOT initialized !";
    return;
    }

  if(d->NoneEnabled)
    {
    d->ComboBox->insertItem(0, "None");
    }

  //LookUpTabletime.start();
  vtkLookupTable * lookupTable = d->ColorNode->GetLookupTable();
  Q_ASSERT(lookupTable);

  const int numberOfColors = lookupTable->GetNumberOfColors();
  //qDebug() << "updateWidgetFromDMML - NumberOfColors:" << numberOfColors;

  int actualMax = d->MaximumColorCount > 0 ? d->MaximumColorCount : numberOfColors;
  for (int i = 0 ; i < actualMax ; ++i)
    {
    QString colorName = QString::fromUtf8(d->ColorNode->GetColorName(i));
    //qDebug() << QString("updateWidgetFromDMML - Color(index:%1, name: %2)").arg(i).arg(colorName);

    QIcon colorIcon(qDMMLUtils::createColorPixmap(this->style(), d->colorFromIndex(i)));

    QString text = "";

    if (d->LabelValueVisible)
      {
      text.append(QString("%1").arg(i));

      if (d->ColorNameVisible)
        {
        // add delimiter if the colorName is visible as well
        text.append(", ");
        }

      }

    if ( d->ColorNameVisible )
      {
      text += colorName;
      }

    d->ComboBox->addItem(colorIcon, text);

    }
  d->CurrentColor = -1;
}
