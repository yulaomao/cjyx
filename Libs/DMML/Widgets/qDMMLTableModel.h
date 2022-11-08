/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef __qDMMLTableModel_h
#define __qDMMLTableModel_h

// Qt includes
#include <QStandardItemModel>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLWidgetsExport.h"

class vtkDMMLNode;
class vtkDMMLTableNode;
class QAction;

class qDMMLTableModelPrivate;

//------------------------------------------------------------------------------
class QDMML_WIDGETS_EXPORT qDMMLTableModel : public QStandardItemModel
{
  Q_OBJECT
  QVTK_OBJECT
  Q_ENUMS(ItemDataRole)
  Q_PROPERTY(bool transposed READ transposed WRITE setTransposed)

public:
  typedef QAbstractItemModel Superclass;
  qDMMLTableModel(QObject *parent=nullptr);
  ~qDMMLTableModel() override;

  enum ItemDataRole{
    SortRole = Qt::UserRole + 1
  };

  void setDMMLTableNode(vtkDMMLTableNode* node);
  vtkDMMLTableNode* dmmlTableNode()const;

  /// Set/Get transposed flag
  /// If transposed is true then columns of the DMML table are added as rows in the model.
  void setTransposed(bool transposed);
  bool transposed()const;

  /// Return the VTK table cell associated to the node index.
  void updateDMMLFromModel(QStandardItem* item);

  /// Update the entire table from the DMML node
  void updateModelFromDMML();

  /// Get DMML table index from model index
  int dmmlTableRowIndex(QModelIndex modelIndex)const;

  /// Get DMML table index from model index
  int dmmlTableColumnIndex(QModelIndex modelIndex)const;

  /// Delete entire row or column from the DMML table that contains item in the selection.
  /// Returns the number of deleted rows or columns.
  /// If removeModelRow is true then entire model rows are deleted, otherwise entire
  /// model columns are deleted.
  int removeSelectionFromDMML(QModelIndexList selection, bool removeModelRow);

protected slots:
  void onDMMLTableNodeModified(vtkObject* node);
  void onItemChanged(QStandardItem * item);

protected:

  qDMMLTableModel(qDMMLTableModelPrivate* pimpl, QObject *parent=nullptr);

  static void onDMMLNodeEvent(vtkObject* vtk_obj, unsigned long event,
                              void* client_data, void* call_data);
protected:
  QScopedPointer<qDMMLTableModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLTableModel);
  Q_DISABLE_COPY(qDMMLTableModel);
};

#endif
