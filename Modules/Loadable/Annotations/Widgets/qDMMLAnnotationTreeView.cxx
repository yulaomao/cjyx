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
#include <QHeaderView>
#include <QMessageBox>
#include <QMouseEvent>

// CTK includes
//#include "ctkModelTester.h"

// qDMML includes
//#include "qDMMLSortFilterProxyModel.h"
//#include "qDMMLSceneTransformModel.h"
//#include "qDMMLTreeView.h"

// Annotations includes
#include "qDMMLAnnotationTreeView.h"
#include "qDMMLSceneAnnotationModel.h"
#include "vtkDMMLAnnotationHierarchyNode.h"
#include "vtkDMMLAnnotationNode.h"
#include "vtkCjyxAnnotationModuleLogic.h"

// DMML includes
#include <vtkDMMLScene.h>

//------------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Annotation
class qDMMLAnnotationTreeViewPrivate
{
  Q_DECLARE_PUBLIC(qDMMLAnnotationTreeView);
protected:
  qDMMLAnnotationTreeView* const q_ptr;
public:
  qDMMLAnnotationTreeViewPrivate(qDMMLAnnotationTreeView& object);
  void init();

  qDMMLSceneAnnotationModel* SceneModel;
  qDMMLSortFilterProxyModel* SortFilterModel;
};

//------------------------------------------------------------------------------
qDMMLAnnotationTreeViewPrivate::qDMMLAnnotationTreeViewPrivate(qDMMLAnnotationTreeView& object)
  : q_ptr(&object)
{
  this->SceneModel = nullptr;
  this->SortFilterModel = nullptr;
}

//------------------------------------------------------------------------------
void qDMMLAnnotationTreeViewPrivate::init()
{
  Q_Q(qDMMLAnnotationTreeView);
  //p->qDMMLTreeView::setModel(new qDMMLItemModel(p));
  //this->SceneModel = new qDMMLSceneAnnotationModel(q);
  //this->SceneModel->setColumnCount(6);

  this->SceneModel = new qDMMLSceneAnnotationModel(q);
  q->setSceneModel(this->SceneModel, "Annotation");
  //this->SortFilterModel = new qDMMLSortFilterProxyModel(q);
  // we only want to show vtkDMMLAnnotationNodes and vtkDMMLAnnotationHierarchyNodes
  QStringList nodeTypes = QStringList();
  nodeTypes.append("vtkDMMLAnnotationNode");
  nodeTypes.append("vtkDMMLAnnotationHierarchyNode");

  //this->SortFilterModel->setNodeTypes(nodeTypes);
  q->setNodeTypes(nodeTypes);
  this->SortFilterModel = q->sortFilterProxyModel();

  q->setShowScene(false);
  //this->SortFilterModel->setSourceModel(this->SceneModel);
  //q->qDMMLTreeView::setModel(this->SortFilterModel);

  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);

  QObject::connect(q, SIGNAL(clicked(QModelIndex)),
                   q, SLOT(onClicked(QModelIndex)));

}

//------------------------------------------------------------------------------
qDMMLAnnotationTreeView::qDMMLAnnotationTreeView(QWidget *_parent)
  :qDMMLTreeView(_parent)
  , d_ptr(new qDMMLAnnotationTreeViewPrivate(*this))
{
  Q_D(qDMMLAnnotationTreeView);
  d->init();

  // we need to enable mouse tracking to set the appropriate cursor while mouseMove occurs
  this->setMouseTracking(true);
}

//------------------------------------------------------------------------------
qDMMLAnnotationTreeView::~qDMMLAnnotationTreeView() = default;

//------------------------------------------------------------------------------
//
// Click and selected event handling
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qDMMLAnnotationTreeView::onSelectionChanged(const QItemSelection & selected,
                                                 const QItemSelection & deselected)
{
  Q_D(qDMMLAnnotationTreeView);

  // if the user clicked on a hierarchy, set this as the active one
  // this means, new annotations or new user-created hierarchies will be created
  // as children of this one
  vtkDMMLNode* newCurrentNode = nullptr;
  if (selected.indexes().count() > 0)
    {
    newCurrentNode = d->SortFilterModel->dmmlNodeFromIndex(selected.indexes()[0]);
    }
  vtkDMMLNode* newActiveNode =
    this->annotationModel()->activeHierarchyNode(newCurrentNode);
  this->m_Logic->SetActiveHierarchyNodeID(
    newActiveNode ? newActiveNode->GetID() : nullptr);

  this->Superclass::onSelectionChanged(selected, deselected);
}

//------------------------------------------------------------------------------
void qDMMLAnnotationTreeView::onClicked(const QModelIndex& index)
{

  Q_D(qDMMLAnnotationTreeView);

  vtkDMMLNode *dmmlNode = d->SortFilterModel->dmmlNodeFromIndex(index);
  if (!dmmlNode)
    {
    return;
    }

  // check if user clicked on icon, this can happen even after we marked a
  // hierarchy as active
  if (index.column() == d->SceneModel->checkableColumn())
    {
    // Let the superclass view to handle the event, it will update the item
    // which will update the node.
    }
  else if (index.column() == d->SceneModel->visibilityColumn())
    {
    // Let the superclass view to handle the event.
    // user wants to toggle the visibility of the annotation
    //this->onVisibilityColumnClicked(dmmlNode);
    }
  else if (index.column() == d->SceneModel->lockColumn())
    {
    // user wants to toggle the un-/lock of the annotation
    this->onLockColumnClicked(dmmlNode);
    }
  else if (index.column() == d->SceneModel->editColumn())
    {
    // user wants to edit the properties of this annotation
    // signal the widget
    this->onPropertyEditButtonClicked(QString(dmmlNode->GetID()));
//    this->m_Widget->propertyEditButtonClicked(QString(dmmlNode->GetID()));
    }

}

//------------------------------------------------------------------------------
const char* qDMMLAnnotationTreeView::firstSelectedNode()
{
  Q_D(qDMMLAnnotationTreeView);
  QModelIndexList selected = this->selectedIndexes();

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return nullptr;
    }

  // now get the first selected item
  QModelIndex index = selected.first();

  // check if it is a valid node
  if (!d->SortFilterModel->dmmlNodeFromIndex(index))
    {
    return nullptr;
    }

  return d->SortFilterModel->dmmlNodeFromIndex(index)->GetID();
}

//------------------------------------------------------------------------------
void qDMMLAnnotationTreeView::toggleLockForSelected()
{
  Q_D(qDMMLAnnotationTreeView);
  QModelIndexList selected = this->selectedIndexes();

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return;
    }

  for (int i = 0; i < selected.size(); ++i)
    {

    // we need to prevent looping through all columns
    // there we only update once a row
    if (selected.at(i).column() == d->SceneModel->lockColumn())
      {

      vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(
        d->SortFilterModel->dmmlNodeFromIndex(selected.at(i)));

      if (annotationNode)
        {
        this->onLockColumnClicked(annotationNode);
        }

      }

  }

}

//------------------------------------------------------------------------------
void qDMMLAnnotationTreeView::toggleVisibilityForSelected()
{
  Q_D(qDMMLAnnotationTreeView);
  QModelIndexList selected = this->selectedIndexes();

  foreach(const QModelIndex& index, selected )
    {
    // we need to prevent looping through all columns
    // there we only update once a row
    if (index.column() == d->SceneModel->visibilityColumn())
      {
      this->toggleVisibility(index);
      }

    } // for loop

}

//------------------------------------------------------------------------------
void qDMMLAnnotationTreeView::deleteSelected()
{
  Q_D(qDMMLAnnotationTreeView);
  QModelIndexList selected = this->selectedIndexes();

  QStringList markedForDeletion;

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return;
    }

  // case: delete a hierarchy only, if it is the only selection
  // warning: all directly under this hierarchy laying annotation nodes will be lost
  // if there are other hierarchies underneath the one which gets deleted,
  // they will get reparented

  // the selected count will be number of rows that are highlighted * number
  // of columns (each item in a row is selected when the row is highlighted),
  // so to check for one row being selected, the count has to be 1 * the
  // columnCount
  if (selected.count() == d->SceneModel->columnCount())
    {
    // only one item was selected, is this a hierarchy?
    vtkDMMLAnnotationHierarchyNode* hierarchyNode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(d->SortFilterModel->dmmlNodeFromIndex(selected.first()));

    if (hierarchyNode)
      {
      // this is exciting!!

      // get confirmation to delete
      QMessageBox msgBox;
      msgBox.setText("Do you really want to delete the selected hierarchy?");
      msgBox.setInformativeText("This includes all directly associated annotations.");
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::No);
      int ret = msgBox.exec();

      if (ret == QMessageBox::Yes)
        {
        this->dmmlScene()->StartState(vtkDMMLScene::BatchProcessState);
        hierarchyNode->DeleteDirectChildren();
        this->dmmlScene()->EndState(vtkDMMLScene::BatchProcessState);

        this->dmmlScene()->RemoveNode(hierarchyNode);

        }
      // all done, bail out
      return;
      }
    // if this is not a hierarchyNode, treat this single selection as a normal case

    }
  // end hierarchy case


  // get confirmation to delete
  QMessageBox msgBox;
  msgBox.setText("Do you really want to delete the selected annotations?");
  msgBox.setInformativeText("This does not include hierarchies.");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  int ret = msgBox.exec();

  if (ret == QMessageBox::No)
    {
    //bail out
    return;
    }

  // case:: delete all selected annotationNodes but no hierarchies
  for (int i = 0; i < selected.count(); ++i)
    {

    // we need to prevent looping through all columns
    // there we only update once a row
    if (selected.at(i).column() == d->SceneModel->visibilityColumn())
      {

      vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(d->SortFilterModel->dmmlNodeFromIndex(selected.at(i)));

      if (annotationNode)
        {

        // we mark this one for deletion
        markedForDeletion.append(QString(annotationNode->GetID()));

        }

      }
    } // for

  // we parsed the complete selection and saved all dmmlIds to delete
  // now, it is safe to delete
  this->dmmlScene()->StartState(vtkDMMLScene::BatchProcessState);
  for (int j=0; j < markedForDeletion.size(); ++j)
    {

    vtkDMMLAnnotationNode* annotationNodeToDelete = vtkDMMLAnnotationNode::SafeDownCast(this->dmmlScene()->GetNodeByID(markedForDeletion.at(j).toUtf8()));
    this->m_Logic->RemoveAnnotationNode(annotationNodeToDelete);

    }
  this->dmmlScene()->EndState(vtkDMMLScene::BatchProcessState);

  this->m_Logic->SetActiveHierarchyNodeID(nullptr);

}

//------------------------------------------------------------------------------
// Return the selected annotations as a collection of dmmlNodes
//------------------------------------------------------------------------------
void qDMMLAnnotationTreeView::selectedAsCollection(vtkCollection* collection)
{

  if (!collection)
    {
    return;
    }

  Q_D(qDMMLAnnotationTreeView);
  QModelIndexList selected = this->selectedIndexes();

  // first, check if we selected anything
  if (selected.isEmpty())
    {
    return;
    }

  std::sort(selected.begin(), selected.end());

  for (int i = 0; i < selected.size(); ++i)
    {

      // we need to prevent looping through all columns
      // there we only update once a row
    if (selected.at(i).column() == d->SceneModel->visibilityColumn())
      {
      vtkDMMLNode* node = vtkDMMLNode::SafeDownCast(d->SortFilterModel->dmmlNodeFromIndex(selected.at(i)));
      //if (node->IsA("vtkDMMLAnnotationNode"))
      // {
      collection->AddItem(node);
      // }
      }

    } // for

}

//------------------------------------------------------------------------------
//
// MouseMove event handling
//
//------------------------------------------------------------------------------

#ifndef QT_NO_CURSOR
//------------------------------------------------------------------------------
bool qDMMLAnnotationTreeView::viewportEvent(QEvent* e)
{

  // reset the cursor if we leave the viewport
  if(e->type() == QEvent::Leave)
    {
    this->setCursor(QCursor());
    }

  return this->Superclass::viewportEvent(e);
}

//------------------------------------------------------------------------------
void qDMMLAnnotationTreeView::mouseMoveEvent(QMouseEvent* e)
{
  Q_D(qDMMLAnnotationTreeView);
  this->Superclass::mouseMoveEvent(e);

  // get the index of the current column
  QModelIndex index = indexAt(e->pos());

  if (index.column() == d->SceneModel->visibilityColumn() ||
      index.column() == d->SceneModel->lockColumn() ||
      index.column() == d->SceneModel->editColumn())
    {
    // we are over a column with a clickable icon
    // let's change the cursor
    QCursor handCursor(Qt::PointingHandCursor);
    this->setCursor(handCursor);
    // .. and bail out
    return;
    }
  else if(this->cursor().shape() == Qt::PointingHandCursor)
    {
    // if we are NOT over such a column and we already have a changed cursor,
    // reset it!
    this->setCursor(QCursor());
    }

}
#endif

//------------------------------------------------------------------------------
//
// Layout and behavior customization
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qDMMLAnnotationTreeView::hideScene()
{
  // set the column widths
  for (int i = 0; i < this->header()->count(); ++i)
    {
    this->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
}

//------------------------------------------------------------------------------
//
// In-Place Editing of Annotations
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qDMMLAnnotationTreeView::toggleVisibility(const QModelIndex& index)
{
  Q_D(qDMMLAnnotationTreeView);
  vtkDMMLNode* node = d->SortFilterModel->dmmlNodeFromIndex(index);
  if (!node)
    {
    return;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(node);

  if (annotationNode)
    {
    // this is a valid annotationNode
    annotationNode->SetDisplayVisibility(!annotationNode->GetDisplayVisibility());
    }
  else
    {
    this->Superclass::toggleVisibility(index);
    }
  // taking out the switch for hierarchy nodes, do it via the buttons above
/*
  vtkDMMLAnnotationHierarchyNode* hierarchyNode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(node);

  if (hierarchyNode)
    {
    this->m_Logic->SetHierarchyAnnotationsVisibleFlag(hierarchyNode, true);
    } // if hierarchyNode
*/
}

//------------------------------------------------------------------------------
void qDMMLAnnotationTreeView::onLockColumnClicked(vtkDMMLNode* node)
{

  if (!node)
    {
    // no node found!
    return;
    }

  vtkDMMLAnnotationNode* annotationNode = vtkDMMLAnnotationNode::SafeDownCast(node);

  if (annotationNode)
    {
    // this is a valid annotationNode
    annotationNode->SetLocked(!annotationNode->GetLocked());

    }


  // taking out the switch for hierarchy nodes, do it via the buttons above
/*
  vtkDMMLAnnotationHierarchyNode* hierarchyNode = vtkDMMLAnnotationHierarchyNode::SafeDownCast(node);

  if (hierarchyNode)
    {
    this->m_Logic->SetHierarchyAnnotationsLockFlag(hierarchyNode, true);


    } // if hierarchyNode
*/

}

//------------------------------------------------------------------------------
void qDMMLAnnotationTreeView::mousePressEvent(QMouseEvent* event)
{
  // skip qDMMLTreeView
  this->Superclass::mousePressEvent(event);
}

//------------------------------------------------------------------------------
//
// Connections to other classes
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Set and observe the logic
//-----------------------------------------------------------------------------
void qDMMLAnnotationTreeView::setLogic(vtkCjyxAnnotationModuleLogic* logic)
{
  if (!logic)
    {
    return;
    }

  Q_D(qDMMLAnnotationTreeView);

  this->m_Logic = logic;

  // propagate down to model
  d->SceneModel->setLogic(this->m_Logic);

}

//-----------------------------------------------------------------------------
/// Annotation model
//-----------------------------------------------------------------------------
qDMMLSceneAnnotationModel* qDMMLAnnotationTreeView::annotationModel()const
{
  Q_D(const qDMMLAnnotationTreeView);
  return d->SceneModel;
}
