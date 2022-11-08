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

#ifndef __qDMMLSortFilterHierarchyProxyModel_h
#define __qDMMLSortFilterHierarchyProxyModel_h

// qDMML includes
#include "qDMMLWidgetsExport.h"
#include "qDMMLSortFilterProxyModel.h"

class qDMMLSortFilterHierarchyProxyModelPrivate;

class QDMML_WIDGETS_EXPORT qDMMLSortFilterHierarchyProxyModel
  : public qDMMLSortFilterProxyModel
{
  Q_OBJECT
public:
  typedef qDMMLSortFilterProxyModel Superclass;
  qDMMLSortFilterHierarchyProxyModel(QObject *parent=nullptr);
  ~qDMMLSortFilterHierarchyProxyModel() override;

protected:
  // Don't show vtkDMMLHierarchyNode if they are tied to a vtkDMMLModelNode
  // The only vtkDMMLHierarchyNode to display are the ones who reference other
  // vtkDMMLHierarchyNode (tree parent) or empty (tree parent to be)
  AcceptType filterAcceptsNode(vtkDMMLNode* node)const override;

protected:
  QScopedPointer<qDMMLSortFilterHierarchyProxyModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSortFilterHierarchyProxyModel);
  Q_DISABLE_COPY(qDMMLSortFilterHierarchyProxyModel);
};

#endif
