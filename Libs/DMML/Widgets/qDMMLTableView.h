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

#ifndef __qDMMLTableView_h
#define __qDMMLTableView_h

// Qt includes
#include <QTableView>

// qDMML includes
#include "qDMMLWidgetsExport.h"

class QSortFilterProxyModel;
class qDMMLTableViewPrivate;
class qDMMLTableModel;
class vtkDMMLNode;
class vtkDMMLScene;
class vtkDMMLTableNode;
class vtkDMMLTableViewNode;

/// \brief Spreadsheet view for table nodes.
/// Allow view/edit of a vtkDMMLTableNode.
class QDMML_WIDGETS_EXPORT qDMMLTableView : public QTableView
{
  Q_OBJECT
  Q_PROPERTY(bool transposed READ transposed WRITE setTransposed)
  Q_PROPERTY(bool firstRowLocked READ firstRowLocked WRITE setFirstRowLocked)
  Q_PROPERTY(bool firstColumnLocked READ firstColumnLocked WRITE setFirstColumnLocked)
public:
  qDMMLTableView(QWidget *parent=nullptr);
  ~qDMMLTableView() override;

  /// Return a pointer on the current DMML scene
  Q_INVOKABLE vtkDMMLScene* dmmlScene() const;

  /// Get the TableView node observed by view.
  Q_INVOKABLE vtkDMMLTableViewNode* dmmlTableViewNode()const;

  Q_INVOKABLE vtkDMMLTableNode* dmmlTableNode()const;

  Q_INVOKABLE qDMMLTableModel* tableModel()const;
  Q_INVOKABLE QSortFilterProxyModel* sortFilterProxyModel()const;

  bool transposed()const;
  bool firstRowLocked()const;
  bool firstColumnLocked()const;

  QList<int> selectedDMMLTableColumnIndices()const;

public slots:
  /// Set the DMML \a scene that should be listened for events.
  /// If scene is set then DMMLTableViewNode has to be set, too.
  /// If scene is set then scene has to be set before DMMLTableNode is set.
  void setDMMLScene(vtkDMMLScene* newScene);

  /// Set the current \a viewNode to observe. If nullptr then view properties are not stored in the scene.
  void setDMMLTableViewNode(vtkDMMLTableViewNode* newTableViewNode);

  void setDMMLTableNode(vtkDMMLTableNode* tableNode);
  /// Utility function to simply connect signals/slots with Qt Designer
  void setDMMLTableNode(vtkDMMLNode* tableNode);

  /// Set transposed flag.
  /// If transposed is true then columns of the DMML table are added as rows in the model.
  /// This affects only this particular view, the settings is not stored in DMML.
  void setTransposed(bool transposed);

  void setFirstRowLocked(bool locked);
  void setFirstColumnLocked(bool locked);

  void copySelection();
  void pasteSelection();
  void plotSelection();

  void insertRow();
  void insertColumn();

  void deleteRow();
  void deleteColumn();

signals:

  /// When designing custom qDMMLWidget in the designer, you can connect the
  /// dmmlSceneChanged signal directly to the aggregated DMML widgets that
  /// have a setDMMLScene slot.
  void dmmlSceneChanged(vtkDMMLScene*);

  /// Emitted when a different table node is selected or different cells are selected.
  void selectionChanged();

protected:
  void keyPressEvent(QKeyEvent* event) override;

  QScopedPointer<qDMMLTableViewPrivate> d_ptr;

  void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected) override;

private:
  Q_DECLARE_PRIVATE(qDMMLTableView);
  Q_DISABLE_COPY(qDMMLTableView);
};

#endif
