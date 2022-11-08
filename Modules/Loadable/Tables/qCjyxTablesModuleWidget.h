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

#ifndef __qCjyxTransformsModuleWidget_h
#define __qCjyxTransformsModuleWidget_h

// Cjyx includes
#include "qCjyxAbstractModuleWidget.h"

// Transforms includes
#include "qCjyxTablesModuleExport.h"

class vtkMatrix4x4;
class vtkDMMLNode;
class qCjyxTablesModuleWidgetPrivate;

class Q_CJYX_QTMODULES_TABLES_EXPORT qCjyxTablesModuleWidget :
  public qCjyxAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qCjyxAbstractModuleWidget Superclass;
  qCjyxTablesModuleWidget(QWidget *parent=nullptr);
  ~qCjyxTablesModuleWidget() override;

  bool setEditedNode(vtkDMMLNode* node, QString role = QString(), QString context = QString()) override;

public slots:
  /// Select the specified node as the current node in the user interface
  void setCurrentTableNode(vtkDMMLNode*);

protected:

  void setup() override;

protected slots:

  void onDMMLTableNodeModified(vtkObject* caller);
  void onNodeSelected(vtkDMMLNode* node);
  void onLockTableButtonClicked();

protected:
  ///
  /// Convenient method to return the coordinate system currently selected
  //int coordinateReference()const;

protected:
  QScopedPointer<qCjyxTablesModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxTablesModuleWidget);
  Q_DISABLE_COPY(qCjyxTablesModuleWidget);
};

#endif
