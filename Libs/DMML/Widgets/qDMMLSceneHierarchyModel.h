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

#ifndef __qDMMLSceneHierarchyModel_h
#define __qDMMLSceneHierarchyModel_h

// DMMLWidgets includes
#include "qDMMLSceneModel.h"
class qDMMLSceneHierarchyModelPrivate;

class QDMML_WIDGETS_EXPORT qDMMLSceneHierarchyModel : public qDMMLSceneModel
{
  Q_OBJECT
  /// Control in which column vtkDMMLHierarchyNode is displayed (Qt::CheckStateRole)
  /// A value of -1 hides it. Hidden (-1) by default.
  /// Note that the expand column should be different than checkableColumn if
  /// not both set to -1.
  /// The Expanded property is on vtkDMMLDisplayableHierarchyNode.
  /// \todo Move the Expanded property to vtkDMMLHierarchyNode.
  /// \sa nameColumn, idColumn, checkableColumn, visibilityColumn...
  Q_PROPERTY (int expandColumn READ expandColumn WRITE setExpandColumn)

public:
  typedef qDMMLSceneModel Superclass;
  qDMMLSceneHierarchyModel(QObject *parent=nullptr);
  ~qDMMLSceneHierarchyModel() override;

  int expandColumn()const;
  void setExpandColumn(int column);

  Qt::DropActions supportedDropActions()const override;

  ///
  vtkDMMLNode* parentNode(vtkDMMLNode* node)const override;
  int          nodeIndex(vtkDMMLNode* node)const override;
  /// fast function that only check the type of the node to know if it can be a child.
  bool         canBeAChild(vtkDMMLNode* node)const override;
  /// fast function that only check the type of the node to know if it can be a parent.
  bool         canBeAParent(vtkDMMLNode* node)const override;
  /// if newParent == 0, set the node into the vtkDMMLScene
  bool         reparent(vtkDMMLNode* node, vtkDMMLNode* newParent) override;

protected:
  qDMMLSceneHierarchyModel(qDMMLSceneHierarchyModelPrivate* pimpl,
                           QObject *parent=nullptr);
  QFlags<Qt::ItemFlag> nodeFlags(vtkDMMLNode* node, int column)const override;

  void observeNode(vtkDMMLNode* node) override;

  /// Reimplemented to add expandColumn support
  void updateItemDataFromNode(QStandardItem* item, vtkDMMLNode* node, int column) override;

  /// Reimplemented to add expandColumn support
  void updateNodeFromItemData(vtkDMMLNode* node, QStandardItem* item) override;

  /// Must be reimplemented in subclasses that add new column types
  int maxColumnId()const override;

private:
  Q_DECLARE_PRIVATE(qDMMLSceneHierarchyModel);
  Q_DISABLE_COPY(qDMMLSceneHierarchyModel);
};

#endif
