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

#ifndef __qDMMLTableView_p_h
#define __qDMMLTableView_p_h

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

// Qt includes
class QToolButton;

// VTK includes
#include <vtkWeakPointer.h>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>
class ctkPopupWidget;

// qDMML includes
#include "qDMMLTableView.h"

class vtkDMMLTableViewNode;
class vtkDMMLTableNode;
class vtkDMMLColorLogic;
class vtkDMMLColorNode;
class vtkDMMLDoubleArrayNode;
class vtkObject;
class vtkStringArray;

//-----------------------------------------------------------------------------
class qDMMLTableViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLTableView);
protected:
  qDMMLTableView* const q_ptr;
public:
  qDMMLTableViewPrivate(qDMMLTableView& object);
  ~qDMMLTableViewPrivate() override;

  virtual void init();

  void setDMMLScene(vtkDMMLScene* scene);
  vtkDMMLScene *dmmlScene();

  bool verifyTableModelAndNode(const char* methodName) const;

public slots:
  /// Handle DMML scene event
  void startProcessing();
  void endProcessing();

  void updateWidgetFromViewNode();

  /// slot when the view is configured to look at a different table node
  //void onTableNodeChanged();

protected:

  // Generate a string of options for a bar table
  QString barOptions(vtkDMMLTableNode*);

  vtkDMMLScene*                      DMMLScene;
  vtkDMMLTableViewNode*              DMMLTableViewNode;

  QToolButton*                       PinButton;
  ctkPopupWidget*                    PopupWidget;
};

#endif
