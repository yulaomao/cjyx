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

#ifndef __qDMMLNodeComboBox_p_h
#define __qDMMLNodeComboBox_p_h

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

// CTK includes
#include <ctkPimpl.h>

// qDMML includes
#include "qDMMLNodeComboBox.h"
class QComboBox;
class qDMMLNodeFactory;
class qDMMLSceneModel;

#include "vtkCallbackCommand.h"
#include "vtkSmartPointer.h"
#include "vtkWeakPointer.h"

// -----------------------------------------------------------------------------
class qDMMLNodeComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qDMMLNodeComboBox);
protected:
  qDMMLNodeComboBox* const q_ptr;
  virtual void setModel(QAbstractItemModel* model);
public:
  qDMMLNodeComboBoxPrivate(qDMMLNodeComboBox& object);
  virtual ~qDMMLNodeComboBoxPrivate();
  virtual void init(QAbstractItemModel* model);

  vtkDMMLNode* dmmlNode(int row)const;
  vtkDMMLNode* dmmlNodeFromIndex(const QModelIndex& index)const;
  QModelIndexList indexesFromDMMLNodeID(const QString& nodeID)const;

  void updateDefaultText();
  void updateNoneItem(bool resetRootIndex = true);
  void updateActionItems(bool resetRootIndex = true);
  void updateDelegate(bool force = false);

  bool hasPostItem(const QString& name)const;

  static void onDMMLSceneEvent(vtkObject* vtk_obj, unsigned long event, void* client_data, void* call_data);

  QComboBox*        ComboBox;
  qDMMLNodeFactory* DMMLNodeFactory;
  qDMMLSceneModel*  DMMLSceneModel;
  bool              NoneEnabled;
  bool              AddEnabled;
  bool              RemoveEnabled;
  bool              EditEnabled;
  bool              RenameEnabled;
  QString           InteractionNodeSingletonTag;

  vtkWeakPointer<vtkDMMLScene> DMMLScene;
  vtkSmartPointer<vtkCallbackCommand> CallBack;

  QHash<QString, QString> NodeTypeLabels;

  bool SelectNodeUponCreation;
  QString NoneDisplay;
  bool AutoDefaultText;

  // Store requested node or ID if setCurrentNode(ID) is called before a scene was set.
  QString RequestedNodeID;
  vtkWeakPointer<vtkDMMLNode> RequestedNode;

  QList<QAction*> UserMenuActions;
};

#endif
