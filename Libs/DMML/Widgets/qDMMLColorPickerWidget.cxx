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
#include <QDialog>
#include <QKeyEvent>
#include <QStringListModel>

// qDMML includes
#include "qDMMLColorPickerWidget.h"
#include "ui_qDMMLColorPickerWidget.h"

// DMMLLogic includes
#include <vtkDMMLColorLogic.h>

// DMML includes
#include <vtkDMMLColorNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qDMMLColorPickerWidgetPrivate: public Ui_qDMMLColorPickerWidget
{
  Q_DECLARE_PUBLIC(qDMMLColorPickerWidget);

protected:
  qDMMLColorPickerWidget* const q_ptr;

public:
  qDMMLColorPickerWidgetPrivate(qDMMLColorPickerWidget& object);
  void init();

  vtkSmartPointer<vtkDMMLColorLogic> ColorLogic;
};

//------------------------------------------------------------------------------
qDMMLColorPickerWidgetPrivate::qDMMLColorPickerWidgetPrivate(qDMMLColorPickerWidget& object)
  : q_ptr(&object)
{
  // Create a default color logic
  this->ColorLogic = vtkSmartPointer<vtkDMMLColorLogic>::New();
}

//------------------------------------------------------------------------------
void qDMMLColorPickerWidgetPrivate::init()
{
  Q_Q(qDMMLColorPickerWidget);
  this->setupUi(q);
  QObject::connect(this->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                   q, SLOT(onCurrentColorNodeChanged(vtkDMMLNode*)));
  QObject::connect(this->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
                   this->SearchBox, SLOT(clear()));
  QObject::connect(this->DMMLColorListView, SIGNAL(colorSelected(int)),
                   q, SIGNAL(colorEntrySelected(int)));
  QObject::connect(this->DMMLColorListView, SIGNAL(colorSelected(QColor)),
                   q, SIGNAL(colorSelected(QColor)));
  QObject::connect(this->DMMLColorListView, SIGNAL(colorSelected(QString)),
                   q, SIGNAL(colorNameSelected(QString)));

  // SearchBox
  this->SearchBox->setPlaceholderText("Search color...");
  this->SearchBox->setShowSearchIcon(true);
  this->SearchBox->installEventFilter(q);
  QObject::connect(this->SearchBox, SIGNAL(textChanged(QString)),
                   q, SLOT(onTextChanged(QString)));
}

//------------------------------------------------------------------------------
qDMMLColorPickerWidget::qDMMLColorPickerWidget(QWidget *_parent)
  : qDMMLWidget(_parent)
  , d_ptr(new qDMMLColorPickerWidgetPrivate(*this))
{
  Q_D(qDMMLColorPickerWidget);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLColorPickerWidget::~qDMMLColorPickerWidget() = default;

//------------------------------------------------------------------------------
void qDMMLColorPickerWidget::setDMMLColorLogic(vtkDMMLColorLogic* colorLogic)
{
  Q_D(qDMMLColorPickerWidget);
  d->ColorLogic = colorLogic;
}

//------------------------------------------------------------------------------
vtkDMMLColorLogic* qDMMLColorPickerWidget::dmmlColorLogic()const
{
  Q_D(const qDMMLColorPickerWidget);
  return d->ColorLogic.GetPointer();
}

//------------------------------------------------------------------------------
vtkDMMLColorNode* qDMMLColorPickerWidget::currentColorNode()const
{
  Q_D(const qDMMLColorPickerWidget);
  return vtkDMMLColorNode::SafeDownCast(d->ColorTableComboBox->currentNode());
}

//------------------------------------------------------------------------------
void qDMMLColorPickerWidget::setCurrentColorNode(vtkDMMLNode* node)
{
  Q_D(qDMMLColorPickerWidget);
  d->ColorTableComboBox->setCurrentNode(node);
  this->qvtkDisconnect(this->dmmlScene(), vtkDMMLScene::NodeAddedEvent,
                       this, SLOT(onNodeAdded(vtkObject*,vtkObject*)));
}

//------------------------------------------------------------------------------
void qDMMLColorPickerWidget::setCurrentColorNodeToDefault()
{
  Q_D(qDMMLColorPickerWidget);
  if (!this->dmmlScene())
    {
    return;
    }
  vtkDMMLNode* defaultColorNode =
    this->dmmlScene()->GetNodeByID( d->ColorLogic.GetPointer() != nullptr ?
                                    d->ColorLogic->GetDefaultEditorColorNodeID() :
                                    nullptr);
  if (defaultColorNode)
    {
    this->setCurrentColorNode(defaultColorNode);
    }
}

//------------------------------------------------------------------------------
void qDMMLColorPickerWidget::onNodeAdded(vtkObject* scene, vtkObject* nodeObject)
{
  Q_D(qDMMLColorPickerWidget);
  Q_UNUSED(scene);
  vtkDMMLNode* node = vtkDMMLNode::SafeDownCast(nodeObject);
  if (node != nullptr && d->ColorLogic.GetPointer() != nullptr &&
      QString(node->GetID()) == d->ColorLogic->GetDefaultEditorColorNodeID())
    {
    this->setCurrentColorNode(node);
    }
}

//------------------------------------------------------------------------------
void qDMMLColorPickerWidget::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLColorPickerWidget);
  this->setCurrentColorNode(nullptr); // eventually disconnect NodeAddedEvent
  this->qDMMLWidget::setDMMLScene(scene);
  if (scene && !d->ColorTableComboBox->currentNode())
    {
    this->qvtkConnect(scene, vtkDMMLScene::NodeAddedEvent,
                      this, SLOT(onNodeAdded(vtkObject*,vtkObject*)));
    this->setCurrentColorNodeToDefault();
   }
}

//------------------------------------------------------------------------------
void qDMMLColorPickerWidget::onCurrentColorNodeChanged(vtkDMMLNode* colorNode)
{
  Q_D(qDMMLColorPickerWidget);
  // Search for the largest item
  QSize maxSizeHint;
  QModelIndex rootIndex = d->DMMLColorListView->rootIndex();
  const int count = d->DMMLColorListView->model()->rowCount(rootIndex);
  for (int i = 0; i < count; ++i)
    {
    QSize sizeHint = d->DMMLColorListView->sizeHintForIndex(
      d->DMMLColorListView->model()->index(i, 0, rootIndex));
    maxSizeHint.setWidth(qMax(maxSizeHint.width(), sizeHint.width()));
    maxSizeHint.setHeight(qMax(maxSizeHint.height(), sizeHint.height()));
    }
  // Set the largest the default size for all the items, that way they will
  // be aligned horizontally and vertically.
  d->DMMLColorListView->setGridSize(maxSizeHint);
  // Inform that the color node has changed.
  emit currentColorNodeChanged(colorNode);
}

//------------------------------------------------------------------------------
void qDMMLColorPickerWidget::onTextChanged(const QString& colorText)
{
  Q_D(qDMMLColorPickerWidget);
  QRegExp regExp(colorText,Qt::CaseInsensitive, QRegExp::RegExp);
  d->DMMLColorListView->sortFilterProxyModel()->setFilterRegExp(regExp);

  QModelIndex newCurrentIndex;

  if (!d->SearchBox->text().isEmpty())
    {
    QModelIndex start = d->DMMLColorListView->sortFilterProxyModel()
                        ->index(0,0);
    QModelIndexList indexList = d->DMMLColorListView->sortFilterProxyModel()
                              ->match(start, 0,
                                      d->SearchBox->text(), 1,
                                      Qt::MatchStartsWith);

    if (indexList.isEmpty())
      {
      indexList = d->DMMLColorListView->sortFilterProxyModel()
                                ->match(start, 0,
                                        d->SearchBox->text(), 1,
                                        Qt::MatchContains);
      }
    if(indexList.count() > 0 )
      {
      newCurrentIndex = indexList[0];
      }
    }
  // Show to the user and set the current index
  d->DMMLColorListView->setCurrentIndex(newCurrentIndex);
  d->SearchBox->setFocus();
}

//------------------------------------------------------------------------------
bool qDMMLColorPickerWidget::eventFilter(QObject* target, QEvent* event)
{
  Q_D(qDMMLColorPickerWidget);
  if (target == d->SearchBox)
    {
    if (event->type() == QEvent::Show)
      {
      d->SearchBox->clear();
      d->DMMLColorListView->setFocus();
      }
    if (event->type() == QEvent::KeyPress)
      {
      QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);
      if (keyEvent->key() == Qt::Key_Up ||
          keyEvent->key() == Qt::Key_Down)
        {
        // give the Focus to DMMLColorListView
        d->DMMLColorListView->setFocus();
        }
      }
    }
  return this->qDMMLWidget::eventFilter(target, event);
}
