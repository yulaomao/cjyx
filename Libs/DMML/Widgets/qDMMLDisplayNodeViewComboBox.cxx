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

// DMMLWidgets includes
#include "qDMMLDisplayNodeViewComboBox.h"

// DMML includes
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLAbstractViewNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// -----------------------------------------------------------------------------
class qDMMLDisplayNodeViewComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qDMMLDisplayNodeViewComboBox);
protected:
  qDMMLDisplayNodeViewComboBox* const q_ptr;

public:
  qDMMLDisplayNodeViewComboBoxPrivate(qDMMLDisplayNodeViewComboBox& object);
  void init();
  vtkWeakPointer<vtkDMMLDisplayNode> DMMLDisplayNode;
  bool IsUpdatingWidgetFromDMML;
};

// -----------------------------------------------------------------------------
qDMMLDisplayNodeViewComboBoxPrivate
::qDMMLDisplayNodeViewComboBoxPrivate(qDMMLDisplayNodeViewComboBox& object)
  : q_ptr(&object)
  , IsUpdatingWidgetFromDMML(false)
{
}

// -----------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBoxPrivate::init()
{
  Q_Q(qDMMLDisplayNodeViewComboBox);
  q->setNodeTypes(QStringList(QString("vtkDMMLAbstractViewNode")));
  q->setBaseName("View");
  QObject::connect(q, SIGNAL(checkedNodesChanged()),
                   q, SLOT(updateDMMLFromWidget()));
  QObject::connect(q, SIGNAL(nodeAdded(vtkDMMLNode*)),
                   q, SLOT(updateWidgetFromDMML()));
  QObject::connect(q, SIGNAL(nodeAboutToBeRemoved(vtkDMMLNode*)),
                   q, SLOT(updateWidgetFromDMML()));
}

// --------------------------------------------------------------------------
// qDMMLDisplayNodeViewComboBox

// --------------------------------------------------------------------------
qDMMLDisplayNodeViewComboBox::qDMMLDisplayNodeViewComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qDMMLDisplayNodeViewComboBoxPrivate(*this))
{
  Q_D(qDMMLDisplayNodeViewComboBox);
  d->init();
}

// --------------------------------------------------------------------------
qDMMLDisplayNodeViewComboBox::~qDMMLDisplayNodeViewComboBox() = default;

// --------------------------------------------------------------------------
vtkDMMLDisplayNode* qDMMLDisplayNodeViewComboBox::dmmlDisplayNode()const
{
  Q_D(const qDMMLDisplayNodeViewComboBox);
  return d->DMMLDisplayNode;
}

// --------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBox
::setDMMLDisplayNode(vtkDMMLDisplayNode* displayNode)
{
  Q_D(qDMMLDisplayNodeViewComboBox);
  this->qvtkReconnect(d->DMMLDisplayNode, displayNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromDMML()));
  d->DMMLDisplayNode = displayNode;
  if (d->DMMLDisplayNode)
    {
    // Only overwrite the scene if the node has a valid scene
    // (otherwise scene may be swapped out to an invalid scene during scene close
    // causing a crash if it happens during a scene model update).
    if (d->DMMLDisplayNode->GetScene())
      {
      this->setDMMLScene(d->DMMLDisplayNode->GetScene());
      }
    }
  else
    {
    this->setDMMLScene(nullptr);
    }
  this->updateWidgetFromDMML();
}

// --------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBox
::setDMMLDisplayNode(vtkDMMLNode* displayNode)
{
  this->setDMMLDisplayNode(
    vtkDMMLDisplayNode::SafeDownCast(displayNode));
}

// --------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBox::updateWidgetFromDMML()
{
  Q_D(qDMMLDisplayNodeViewComboBox);
  this->setEnabled(this->dmmlScene() != nullptr && d->DMMLDisplayNode != nullptr);
  if (!d->DMMLDisplayNode)
    {
    return;
    }
  bool oldUpdating = d->IsUpdatingWidgetFromDMML;
  d->IsUpdatingWidgetFromDMML = true;

  bool wasBlocking = this->blockSignals(true);
  bool modified = false;
  for (int i = 0; i < this->nodeCount(); ++i)
    {
    vtkDMMLNode* view = this->nodeFromIndex(i);
    if (!view)
      {
      // we get here if there is an orphan view node and the scene is closing
      this->setCheckState(view, Qt::Unchecked);
      continue;
      }
    bool check = d->DMMLDisplayNode->IsDisplayableInView(view->GetID());
    Qt::CheckState viewCheckState = check ? Qt::Checked : Qt::Unchecked;
    if (this->checkState(view) != viewCheckState)
      {
      modified = true;
      this->setCheckState(view, viewCheckState);
      }
    }
  this->blockSignals(wasBlocking);
  if (modified)
    {
    emit checkedNodesChanged();
    }
  d->IsUpdatingWidgetFromDMML = oldUpdating;
}

// --------------------------------------------------------------------------
void qDMMLDisplayNodeViewComboBox::updateDMMLFromWidget()
{
  Q_D(qDMMLDisplayNodeViewComboBox);
  if (!d->DMMLDisplayNode)
    {
    return;
    }
  if (d->IsUpdatingWidgetFromDMML)
    {
    return;
    }
  int wasModifying = d->DMMLDisplayNode->StartModify();

  if (this->allChecked() || this->noneChecked())
    {
    d->DMMLDisplayNode->RemoveAllViewNodeIDs();
    }
  else
    {
    foreach (vtkDMMLAbstractViewNode* viewNode, this->checkedViewNodes())
      {
      d->DMMLDisplayNode->AddViewNodeID(viewNode ? viewNode->GetID() : nullptr);
      }
    foreach (vtkDMMLAbstractViewNode* viewNode, this->uncheckedViewNodes())
      {
      d->DMMLDisplayNode->RemoveViewNodeID(viewNode ? viewNode->GetID() : nullptr);
      }
    }

  d->DMMLDisplayNode->EndModify(wasModifying);
}

// --------------------------------------------------------------------------
QList<vtkDMMLAbstractViewNode*> qDMMLDisplayNodeViewComboBox::checkedViewNodes()const
{
  QList<vtkDMMLAbstractViewNode*> res;
  foreach(vtkDMMLNode* checkedNode, this->checkedNodes())
    {
    res << vtkDMMLAbstractViewNode::SafeDownCast(checkedNode);
    }
  return res;
}

// --------------------------------------------------------------------------
QList<vtkDMMLAbstractViewNode*> qDMMLDisplayNodeViewComboBox::uncheckedViewNodes()const
{
  QList<vtkDMMLAbstractViewNode*> res;
  foreach(vtkDMMLNode* uncheckedNode, this->uncheckedNodes())
    {
    res << vtkDMMLAbstractViewNode::SafeDownCast(uncheckedNode);
    }
  return res;
}
