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

#ifndef __qDMMLTableViewControllerWidget_p_h
#define __qDMMLTableViewControllerWidget_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Cjyx API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// qDMML includes
#include "qDMMLTableViewControllerWidget.h"
#include "qDMMLViewControllerBar_p.h"
#include "ui_qDMMLTableViewControllerWidget.h"

// VTK includes
#include <vtkWeakPointer.h>

class QAction;
class qDMMLSceneViewMenu;
class vtkDMMLTableViewNode;
class vtkDMMLTableNode;
class QString;

//-----------------------------------------------------------------------------
class qDMMLTableViewControllerWidgetPrivate
  : public qDMMLViewControllerBarPrivate
  , public Ui_qDMMLTableViewControllerWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLTableViewControllerWidget);

public:
  typedef qDMMLViewControllerBarPrivate Superclass;
  qDMMLTableViewControllerWidgetPrivate(qDMMLTableViewControllerWidget& object);
  ~qDMMLTableViewControllerWidgetPrivate() override;

  void init() override;

  vtkWeakPointer<vtkDMMLTableNode> TableNode;
  qDMMLTableView* TableView;

  QAction* CopyAction;
  QAction* PasteAction;
  QAction* PlotAction;

public slots:
  void onTableNodeSelected(vtkDMMLNode* node);
  void onLockTableButtonClicked();
  void insertColumn();
  void deleteColumn();
  void insertRow();
  void deleteRow();
  void setFirstRowLocked(bool locked);
  void setFirstColumnLocked(bool locked);
  void copySelection();
  void pasteSelection();
  void plotSelection();

protected:
  void setupPopupUi() override;

public:

};

#endif
