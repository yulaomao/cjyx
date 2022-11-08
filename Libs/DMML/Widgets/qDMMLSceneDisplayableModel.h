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

#ifndef __qDMMLSceneDisplayableModel_h
#define __qDMMLSceneDisplayableModel_h

// DMMLWidgets includes
#include "qDMMLSceneHierarchyModel.h"
class qDMMLSceneDisplayableModelPrivate;

/// The Visibility icon is in the same column than the name by default.
class QDMML_WIDGETS_EXPORT qDMMLSceneDisplayableModel : public qDMMLSceneHierarchyModel
{
  Q_OBJECT
  /// Control in which column vtkDMMLModelDisplayNode::Color are displayed
  /// (Qt::DecorationRole). Even if a vtkDMMLModelNode doesn't have a color
  /// proper, the color of its display node is used. If the model node has
  /// more than one display node and their colors are different, it uses
  /// an invalid color.
  /// A value of -1 (default) hides the column
  Q_PROPERTY (int colorColumn READ colorColumn WRITE setColorColumn)

  /// This property holds the column ID where the node opacity is shown.
  /// A value of -1 (default) hides the column.
  Q_PROPERTY (int opacityColumn READ opacityColumn WRITE setOpacityColumn)

public:
  typedef qDMMLSceneHierarchyModel Superclass;
  qDMMLSceneDisplayableModel(QObject *parent=nullptr);
  ~qDMMLSceneDisplayableModel() override;

  int colorColumn()const;
  void setColorColumn(int column);

  int opacityColumn()const;
  void setOpacityColumn(int column);

  ///
  vtkDMMLNode* parentNode(vtkDMMLNode* node)const override;
  //virtual int          nodeIndex(vtkDMMLNode* node)const;
  /// fast function that only check the type of the node to know if it can be a child.
  bool         canBeAChild(vtkDMMLNode* node)const override;
  /// fast function that only check the type of the node to know if it can be a parent.
  bool         canBeAParent(vtkDMMLNode* node)const override;

protected:
  qDMMLSceneDisplayableModel(qDMMLSceneDisplayableModelPrivate* pimpl,
                             QObject *parent=nullptr);

  /// Reimplemented to listen to the displayable DisplayModifiedEvent event for
  /// visibility check state changes.
  void observeNode(vtkDMMLNode* node) override;
  QFlags<Qt::ItemFlag> nodeFlags(vtkDMMLNode* node, int column)const override;
  void updateItemDataFromNode(QStandardItem* item, vtkDMMLNode* node, int column) override;
  void updateNodeFromItemData(vtkDMMLNode* node, QStandardItem* item) override;

  int maxColumnId()const override;
private:
  Q_DECLARE_PRIVATE(qDMMLSceneDisplayableModel);
  Q_DISABLE_COPY(qDMMLSceneDisplayableModel);
};

#endif
