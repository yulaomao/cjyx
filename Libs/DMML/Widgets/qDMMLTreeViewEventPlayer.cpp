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
#include <QDebug>

// qDMML includes
#include "qDMMLSceneModel.h"
#include "qDMMLTreeView.h"
#include "qDMMLTreeViewEventPlayer.h"

// DMML includes
#include <vtkDMMLNode.h>
#include <vtkDMMLScene.h>

//-----------------------------------------------------------------------------
QModelIndex qDMMLTreeViewEventPlayerGetIndex(const QString& str_index,
  QTreeView* treeView, bool &error)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  QStringList indices = str_index.split(".", Qt::SkipEmptyParts);
#else
  QStringList indices = str_index.split(".",QString::SkipEmptyParts);
#endif
  QModelIndex index;
  for (int cc=0; (cc+1) < indices.size(); cc+=2)
    {
    index = treeView->model()->index(indices[cc].toInt(), indices[cc+1].toInt(),
      index);
    if (!index.isValid())
      {
      error=true;
      qCritical() << "ERROR: Tree view must have changed. "
        << "Indices recorded in the test are no longer valid. Cannot playback.";
      break;
      }
    }
  return index;
}

// ----------------------------------------------------------------------------
qDMMLTreeViewEventPlayer::qDMMLTreeViewEventPlayer(QObject *parent)
  : Superclass(parent)
{
}

// ----------------------------------------------------------------------------
bool qDMMLTreeViewEventPlayer::playEvent(QObject *Object,
                                    const QString &Command,
                                    const QString &Arguments,
                                    int EventType,
                                    bool &Error)
{
  if(Command != "currentNodeRenamed" && Command != "currentNodeDeleted" &&
     Command != "editNodeRequested" && Command != "decorationClicked" &&
     Command != "reParentByDragnDrop")
    {
    return this->Superclass::playEvent(Object, Command, Arguments, EventType, Error);
    }

  if(qDMMLTreeView* const treeView =
     qobject_cast<qDMMLTreeView*>(Object))
    {
    if(Command == "currentNodeRenamed")
      {
      treeView->currentNode()->SetName(Arguments.toUtf8());
      // for improvement, see the method qDMMLTreeView::renameCurrentNode()
      // and set the name in the line edit, then simulate a OK
      return true;
      }
    if(Command == "currentNodeDeleted")
      {
      QModelIndex index = ::qDMMLTreeViewEventPlayerGetIndex(Arguments, treeView, Error);
      if (index.isValid())
        {
        treeView->setCurrentIndex(index);
        treeView->deleteCurrentNode();
        return true;
        }
//      return false;
      }
    if(Command == "editNodeRequested")
      {
//      vtkDMMLNode* node = treeView->dmmlScene()->GetNodeByID(Arguments.toUtf8());
//      emit treeView->editNodeRequested(node);
      treeView->editCurrentNode();
      return true;
      }
    if(Command == "reParentByDragnDrop")
      {
      QStringList nodes = Arguments.split(".");
      if(nodes.count() != 2)
        {
        return false;
        }
      vtkDMMLNode* node = treeView->dmmlScene()->GetNodeByID(nodes[0].toUtf8());
      vtkDMMLNode* nodeParent = treeView->dmmlScene()->GetNodeByID(nodes[1].toUtf8());
      treeView->sceneModel()->reparent(node, nodeParent);
      return true;
      }
    if(Command == "decorationClicked")
      {
      QString str_index = Arguments;
      QModelIndex index = ::qDMMLTreeViewEventPlayerGetIndex(str_index, treeView, Error);
      treeView->clickDecoration(index);
      return true;
      }
    }

  qCritical() << "calling currentNodeRenamed/currentNodeDeleted/editNodeRequested on unhandled type " << Object;
  Error = true;
  return true;
}

