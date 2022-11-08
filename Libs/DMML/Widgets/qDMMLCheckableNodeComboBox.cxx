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
#include <QStandardItem>

// CTK includes
#include <ctkCheckableComboBox.h>

// DMMLWidgets includes
#include "qDMMLCheckableNodeComboBox.h"
#include "qDMMLNodeComboBox_p.h"
#include "qDMMLSceneModel.h"

// DMML includes
#include <vtkDMMLNode.h>

// -----------------------------------------------------------------------------
class qDMMLCheckableNodeComboBoxPrivate: public qDMMLNodeComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qDMMLCheckableNodeComboBox);
protected:
  qDMMLCheckableNodeComboBox* const q_ptr;
  void setModel(QAbstractItemModel* model) override;
public:
  qDMMLCheckableNodeComboBoxPrivate(qDMMLCheckableNodeComboBox& object);
  ~qDMMLCheckableNodeComboBoxPrivate() override;
  void init(QAbstractItemModel* model) override;
};

// -----------------------------------------------------------------------------
qDMMLCheckableNodeComboBoxPrivate
::qDMMLCheckableNodeComboBoxPrivate(qDMMLCheckableNodeComboBox& object)
  : qDMMLNodeComboBoxPrivate(object)
  , q_ptr(&object)
{
}

// -----------------------------------------------------------------------------
qDMMLCheckableNodeComboBoxPrivate::~qDMMLCheckableNodeComboBoxPrivate() = default;

// -----------------------------------------------------------------------------
void qDMMLCheckableNodeComboBoxPrivate::init(QAbstractItemModel* model)
{
  Q_Q(qDMMLCheckableNodeComboBox);

  this->ComboBox = new ctkCheckableComboBox;
  this->qDMMLNodeComboBoxPrivate::init(model);

  q->setAddEnabled(false);
  q->setRemoveEnabled(false);
  q->setEditEnabled(false);
  q->setRenameEnabled(false);

}

// --------------------------------------------------------------------------
void qDMMLCheckableNodeComboBoxPrivate::setModel(QAbstractItemModel* model)
{
  if (model)
    {
    qobject_cast<ctkCheckableComboBox*>(this->ComboBox)->setCheckableModel(model);
    }
  this->qDMMLNodeComboBoxPrivate::setModel(model);
}

// --------------------------------------------------------------------------
// qDMMLCheckableNodeComboBox

// --------------------------------------------------------------------------
qDMMLCheckableNodeComboBox::qDMMLCheckableNodeComboBox(QWidget* parentWidget)
  : Superclass(new qDMMLCheckableNodeComboBoxPrivate(*this), parentWidget)
{
  Q_D(qDMMLCheckableNodeComboBox);
  // Can't be done in XXXPrivate::init() because XXX is not constructed at that
  // time.
  this->connect(d->ComboBox, SIGNAL(checkedIndexesChanged()),
                this, SIGNAL(checkedNodesChanged()));

}

// --------------------------------------------------------------------------
qDMMLCheckableNodeComboBox::~qDMMLCheckableNodeComboBox() = default;

// --------------------------------------------------------------------------
QList<vtkDMMLNode*> qDMMLCheckableNodeComboBox::checkedNodes()const
{
  Q_D(const qDMMLCheckableNodeComboBox);
  QList<vtkDMMLNode*> res;
  const ctkCheckableComboBox* checkableComboBox =
    qobject_cast<const ctkCheckableComboBox*>(d->ComboBox);
  foreach(const QModelIndex& checkedIndex, checkableComboBox->checkedIndexes())
    {
    vtkDMMLNode* checkedNode = d->dmmlNodeFromIndex(checkedIndex);
    // DMMLScene or extra items could be checked, we don't want them
    if (checkedNode)
      {
      res << checkedNode;
      }
    }
  return res;
}

// --------------------------------------------------------------------------
QList<vtkDMMLNode*> qDMMLCheckableNodeComboBox::uncheckedNodes()const
{
  QList<vtkDMMLNode*> res = this->nodes();
  foreach(vtkDMMLNode* checkedNode, this->checkedNodes())
    {
    res.removeAll(checkedNode);
    }
  return res;
}

// --------------------------------------------------------------------------
bool qDMMLCheckableNodeComboBox::allChecked()const
{
  Q_D(const qDMMLCheckableNodeComboBox);
  const ctkCheckableComboBox* checkableComboBox =
    qobject_cast<const ctkCheckableComboBox*>(d->ComboBox);
  return checkableComboBox->allChecked();
}

// --------------------------------------------------------------------------
bool qDMMLCheckableNodeComboBox::noneChecked()const
{
  Q_D(const qDMMLCheckableNodeComboBox);
  const ctkCheckableComboBox* checkableComboBox =
    qobject_cast<const ctkCheckableComboBox*>(d->ComboBox);
  return checkableComboBox->noneChecked();
}

// --------------------------------------------------------------------------
Qt::CheckState qDMMLCheckableNodeComboBox::checkState(vtkDMMLNode* node)const
{
  Q_D(const qDMMLCheckableNodeComboBox);
  const ctkCheckableComboBox* checkableComboBox =
    qobject_cast<const ctkCheckableComboBox*>(d->ComboBox);
  QModelIndexList indexes =
    d->indexesFromDMMLNodeID(node ? node->GetID() : QString());
  if (indexes.size() == 0)
    {
    return Qt::Unchecked;
    }
  return checkableComboBox->checkState(indexes[0]);
}

// --------------------------------------------------------------------------
void qDMMLCheckableNodeComboBox::setCheckState(vtkDMMLNode* node, Qt::CheckState check)
{
  Q_D(qDMMLCheckableNodeComboBox);
  ctkCheckableComboBox* checkableComboBox =
    qobject_cast<ctkCheckableComboBox*>(d->ComboBox);
  QModelIndexList indexes =
    d->indexesFromDMMLNodeID(node ? node->GetID(): QString());
  if (indexes.count() < 1)
    {
    return;
    }
  checkableComboBox->setCheckState(indexes[0], check);
}

// --------------------------------------------------------------------------
void qDMMLCheckableNodeComboBox::setUserCheckable(vtkDMMLNode* node, bool userCheckable)
{
  QStandardItem* nodeItem = this->sceneModel()->itemFromNode(node);
  if (nodeItem)
    {
    if (userCheckable)
      {
      nodeItem->setFlags(nodeItem->flags() | Qt::ItemIsUserCheckable);
      }
    else
      {
      nodeItem->setFlags(nodeItem->flags() & ~Qt::ItemIsUserCheckable);
      }
    }
}
