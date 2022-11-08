/*=========================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Benjamin LONG, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

=========================================================================*/

// Qt includes
#include <QAbstractItemModel>
#include <QDebug>
#include <QEvent>
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>

// CTK includes
#include <ctkCheckBoxPixmaps.h>

// qDMML includes
#include "qDMMLItemDelegate.h"
#include "qDMMLSceneModel.h"
#include "qDMMLTreeView.h"
#include "qDMMLTreeViewEventTranslator.h"

// DMML includes
#include <vtkDMMLNode.h>
#include <vtkDMMLScene.h>

// ----------------------------------------------------------------------------
qDMMLTreeViewEventTranslator::qDMMLTreeViewEventTranslator(QObject *parent)
  : Superclass(parent)
{
  this->CurrentObject = nullptr;
}

// ----------------------------------------------------------------------------
bool qDMMLTreeViewEventTranslator::translateEvent(QObject *Object,
                                             QEvent *Event,
                                             int EventType,
                                             bool &Error)
{
  Q_UNUSED(Error);

  qDMMLTreeView* treeView = nullptr;
  for(QObject* test = Object; treeView == nullptr && test != nullptr; test = test->parent())
    {
    treeView = qobject_cast<qDMMLTreeView*>(test);
    }
//  qDMMLTreeView* treeView = qobject_cast<qDMMLTreeView*>(Object);
  if(!treeView)
    {
    return false;
    }

  // For the custom action when we have a right click
  QMenu* menu = nullptr;
  for(QObject* test = Object; menu == nullptr && test != nullptr ; test = test->parent())
    {
    menu = qobject_cast<QMenu*>(test);
    }
  if (menu)
    {
    if(Event->type() == QEvent::KeyPress)
      {
      QKeyEvent* e = static_cast<QKeyEvent*>(Event);
      if(e->key() == Qt::Key_Enter)
        {
        QAction* action = menu->activeAction();
        if(action)
          {
          QString which = action->objectName();
          if(which.isNull())
            {
            which = action->text();
            }
          if (which != "Rename" && which != "Delete" )
            {
            emit recordEvent(menu, "activate", which);
            }
          }
        }
      }
    if(Event->type() == QEvent::MouseButtonRelease)
      {
      QMouseEvent* e = static_cast<QMouseEvent*>(Event);
      if(e->button() == Qt::LeftButton)
        {
        QAction* action = menu->actionAt(e->pos());
        if (action && !action->menu())
          {
          QString which = action->objectName();
          if(which.isNull())
            {
            which = action->text();
            }
          if (which != "Rename" && which != "Delete" )
            {
            emit recordEvent(menu, "activate", which);
            }
          }
        }
      }
    return true;
    }

  // We want to stop the action on the QDialog when we are renaming
  // and let passed the action for the "set_current".
  QInputDialog* dialog = nullptr;
  for(QObject* test = Object; dialog == nullptr && test != nullptr; test = test->parent())
    {
    dialog = qobject_cast<QInputDialog*>(test);
    if(dialog)
      {
      // block actions on the QInputDialog
      return true;
      }
    }

  if(Event->type() == QEvent::Enter && Object == treeView)
    {
    if(this->CurrentObject != Object)
      {
      if(this->CurrentObject)
        {
        disconnect(this->CurrentObject, nullptr, this, nullptr);
        }
      this->CurrentObject = Object;

      connect(treeView, SIGNAL(destroyed(QObject*)),
              this, SLOT(onDestroyed(QObject*)));
      connect(treeView, SIGNAL(currentNodeRenamed(QString)),
              this, SLOT(onCurrentNodeRenamed(QString)));

      // Can be better to do it on the model to recover the QModelIndex
      connect(treeView, SIGNAL(currentNodeDeleted(const QModelIndex&)),
              this, SLOT(onCurrentNodeDeleted(const QModelIndex&)));
      connect(treeView, SIGNAL(decorationClicked(QModelIndex)),
              this, SLOT(onDecorationClicked(QModelIndex)));

      connect(treeView->sceneModel(), SIGNAL(aboutToReparentByDragAndDrop(vtkDMMLNode*,vtkDMMLNode*)),
              this, SLOT(onAboutToReparentByDnD(vtkDMMLNode*,vtkDMMLNode*)));
      }
    return this->Superclass::translateEvent(Object, Event, EventType, Error);
    }

  return this->Superclass::translateEvent(Object, Event, EventType, Error);
}

// ----------------------------------------------------------------------------
void qDMMLTreeViewEventTranslator::onDestroyed(QObject* /*Object*/)
{
  this->CurrentObject = nullptr;
}

// ----------------------------------------------------------------------------
void qDMMLTreeViewEventTranslator::onCurrentNodeRenamed(const QString & newName)
{
  emit recordEvent(this->CurrentObject, "currentNodeRenamed", newName);
}

// ----------------------------------------------------------------------------
void qDMMLTreeViewEventTranslator::onCurrentNodeDeleted(const QModelIndex& index)
{
  if (index.isValid())
    {
    emit recordEvent(this->CurrentObject, "currentNodeDeleted", this->getIndexAsString(index));
    }
}

// ----------------------------------------------------------------------------
void qDMMLTreeViewEventTranslator::onDecorationClicked(const QModelIndex& index)
{
  if(index.isValid())
    {
    emit recordEvent(this->CurrentObject, "decorationClicked", this->getIndexAsString(index));
    }
}

//-----------------------------------------------------------------------------
void qDMMLTreeViewEventTranslator::onAboutToReparentByDnD(vtkDMMLNode* node , vtkDMMLNode* newParent )
{
  if (node)
    {
    QString parentID = newParent ? QString::fromUtf8(newParent->GetID()) : nullptr;
    QString args = QString("%1.%2").arg(
        QString::fromUtf8(node->GetID()),
        parentID);
    emit recordEvent(this->CurrentObject, "reParentByDragnDrop", args);
    }
}

//-----------------------------------------------------------------------------
QString qDMMLTreeViewEventTranslator::getIndexAsString(const QModelIndex& index)
{
  QModelIndex curIndex = index;
  QString str_index;
  while (curIndex.isValid())
    {
    str_index.prepend(QString("%1.%2.").arg(curIndex.row()).arg(curIndex.column()));
    curIndex = curIndex.parent();
    }

  // remove the last ".".
  str_index.chop(1);
  return str_index;
}
