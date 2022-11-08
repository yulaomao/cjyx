/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// qDMML includes
#include "qDMMLSequenceBrowserSeekWidget.h"
#include "ui_qDMMLSequenceBrowserSeekWidget.h"

// DMML includes
#include <vtkDMMLSequenceBrowserNode.h>
#include <vtkDMMLSequenceNode.h>

// Qt includes
#include <QDebug>
#include <QFontDatabase>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Markups
class qDMMLSequenceBrowserSeekWidgetPrivate
  : public Ui_qDMMLSequenceBrowserSeekWidget
{
  Q_DECLARE_PUBLIC(qDMMLSequenceBrowserSeekWidget);
protected:
  qDMMLSequenceBrowserSeekWidget* const q_ptr;
public:
  qDMMLSequenceBrowserSeekWidgetPrivate(qDMMLSequenceBrowserSeekWidget& object);
  void init();

  vtkWeakPointer<vtkDMMLSequenceBrowserNode> SequenceBrowserNode;
};

//-----------------------------------------------------------------------------
// qDMMLSequenceBrowserSeekWidgetPrivate methods

//-----------------------------------------------------------------------------
qDMMLSequenceBrowserSeekWidgetPrivate::qDMMLSequenceBrowserSeekWidgetPrivate(qDMMLSequenceBrowserSeekWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserSeekWidgetPrivate::init()
{
  Q_Q(qDMMLSequenceBrowserSeekWidget);
  this->setupUi(q);
  this->label_IndexValue->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  QObject::connect(this->slider_IndexValue, SIGNAL(valueChanged(int)), q, SLOT(setSelectedItemNumber(int)));
  q->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
// qDMMLSequenceBrowserSeekWidget methods

//-----------------------------------------------------------------------------
qDMMLSequenceBrowserSeekWidget::qDMMLSequenceBrowserSeekWidget(QWidget *newParent)
  : Superclass(newParent)
  , d_ptr(new qDMMLSequenceBrowserSeekWidgetPrivate(*this))
{
  Q_D(qDMMLSequenceBrowserSeekWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qDMMLSequenceBrowserSeekWidget::~qDMMLSequenceBrowserSeekWidget() = default;

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserSeekWidget::setDMMLSequenceBrowserNode(vtkDMMLNode* browserNode)
{
  setDMMLSequenceBrowserNode(vtkDMMLSequenceBrowserNode::SafeDownCast(browserNode));
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserSeekWidget::setDMMLSequenceBrowserNode(vtkDMMLSequenceBrowserNode* browserNode)
{
  Q_D(qDMMLSequenceBrowserSeekWidget);

  qvtkReconnect(d->SequenceBrowserNode, browserNode, vtkDMMLSequenceBrowserNode::IndexDisplayFormatModifiedEvent,
    this, SLOT(onIndexDisplayFormatModified()));
  qvtkReconnect(d->SequenceBrowserNode, browserNode, vtkCommand::ModifiedEvent,
    this, SLOT(updateWidgetFromDMML()));

  d->SequenceBrowserNode = browserNode;
  this->onIndexDisplayFormatModified();
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserSeekWidget::setSelectedItemNumber(int itemNumber)
{
  Q_D(qDMMLSequenceBrowserSeekWidget);
  if (d->SequenceBrowserNode == nullptr)
    {
    qCritical("setSelectedItemNumber failed: browser node is invalid");
    this->updateWidgetFromDMML();
    return;
    }
  int selectedItemNumber = -1;
  vtkDMMLSequenceNode* sequenceNode = d->SequenceBrowserNode->GetMasterSequenceNode();
  if (sequenceNode != nullptr && itemNumber >= 0)
    {
    if (itemNumber < sequenceNode->GetNumberOfDataNodes())
      {
      selectedItemNumber = itemNumber;
      }
    }
  d->SequenceBrowserNode->SetSelectedItemNumber(selectedItemNumber);
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserSeekWidget::onIndexDisplayFormatModified()
{
  Q_D(qDMMLSequenceBrowserSeekWidget);
  // Reset the fixed width of the label
  QFontMetrics fontMetrics = QFontMetrics(d->label_IndexValue->font());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
  d->label_IndexValue->setFixedWidth(fontMetrics.horizontalAdvance(d->label_IndexValue->text()));
#else
  d->label_IndexValue->setFixedWidth(fontMetrics.width(d->label_IndexValue->text()));
#endif
}

//-----------------------------------------------------------------------------
void qDMMLSequenceBrowserSeekWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLSequenceBrowserSeekWidget);
  vtkDMMLSequenceNode* sequenceNode = d->SequenceBrowserNode.GetPointer() ? d->SequenceBrowserNode->GetMasterSequenceNode() : nullptr;
  this->setEnabled(sequenceNode != nullptr);
  if (!sequenceNode)
    {
    d->label_IndexName->setText("");
    d->label_IndexUnit->setText("");
    d->label_IndexValue->setText("");
    return;
    }

  d->label_IndexName->setText(sequenceNode->GetIndexName().c_str());

  // Setting the min/max could trigger an index change (if current index is out of the new range),
  // therefore we have to block signals.
  bool sliderBlockSignals = d->slider_IndexValue->blockSignals(true);
  int numberOfDataNodes = sequenceNode->GetNumberOfDataNodes();
  if (numberOfDataNodes > 0 && !d->SequenceBrowserNode->GetRecordingActive())
    {
    d->slider_IndexValue->setEnabled(true);
    d->slider_IndexValue->setMinimum(0);
    d->slider_IndexValue->setMaximum(numberOfDataNodes - 1);
    }
  else
    {
    d->slider_IndexValue->setEnabled(false);
    }
  d->slider_IndexValue->blockSignals(sliderBlockSignals);

  int selectedItemNumber = d->SequenceBrowserNode->GetSelectedItemNumber();
  if (selectedItemNumber >= 0)
    {
    QString indexValue;
    QString indexUnit;

    if (d->SequenceBrowserNode->GetIndexDisplayMode() == vtkDMMLSequenceBrowserNode::IndexDisplayAsIndexValue)
      {
      // display as formatted index value (12.34sec)
      indexValue = QString::fromStdString(d->SequenceBrowserNode->GetFormattedIndexValue(d->SequenceBrowserNode->GetSelectedItemNumber()));
      indexUnit = QString::fromStdString(sequenceNode->GetIndexUnit());
      if (indexValue.length() == 0)
        {
        qWarning() << "Item " << selectedItemNumber << " has no index value defined";
        }
      }
    else
      {
      // display index as item index number (23/37)
      indexValue = QString::number(selectedItemNumber + 1) + "/" + QString::number(sequenceNode->GetNumberOfDataNodes());
      indexUnit = "";
      }

    QFontMetrics fontMetrics = QFontMetrics(d->label_IndexValue->font());

    d->label_IndexValue->setText(indexValue);
    d->label_IndexUnit->setText(indexUnit);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    d->label_IndexValue->setFixedWidth(std::max(fontMetrics.horizontalAdvance(indexValue), d->label_IndexValue->width()));
#else
    d->label_IndexValue->setFixedWidth(std::max(fontMetrics.width(indexValue), d->label_IndexValue->width()));
#endif
    d->slider_IndexValue->setValue(selectedItemNumber);
    }
  else
    {
    d->label_IndexValue->setFixedWidth(0);
    d->label_IndexValue->setText("");
    d->label_IndexUnit->setText("");
    d->slider_IndexValue->setValue(0);
    }
}

//-----------------------------------------------------------------------------
QSlider* qDMMLSequenceBrowserSeekWidget::slider() const
{
  Q_D(const qDMMLSequenceBrowserSeekWidget);
  return d->slider_IndexValue;
}
