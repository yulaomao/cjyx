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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qDMMLSceneViewMenu_p_h
#define __qDMMLSceneViewMenu_p_h

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
#include<QSignalMapper>

// qDMML includes
#include "qDMMLSceneViewMenu.h"

// CTK includes
#include <ctkVTKObject.h>

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qDMMLSceneViewMenuPrivate : public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLSceneViewMenu);
protected:
  qDMMLSceneViewMenu* const q_ptr;
public:
  typedef QObject Superclass;
  qDMMLSceneViewMenuPrivate(qDMMLSceneViewMenu& object);

  /// \brief Clear and update menu given the list of existing vtkDMMLSceneViewNode
  /// associated with the current scene
  void resetMenu();

public slots:

  void onDMMLNodeAdded(vtkObject* dmmlScene, vtkObject * dmmlNode);

  /// Add menu entry corresponding to \a sceneViewNode
  void addMenuItem(vtkDMMLNode * sceneViewNode);

  void onDMMLNodeRemoved(vtkObject* dmmlScene, vtkObject * dmmlNode);

  /// Remove menu entry corresponding to \a sceneViewNode
  void removeMenuItem(vtkDMMLNode * sceneViewNode);

  void onDMMLSceneViewNodeModified(vtkObject * dmmlNode);

  bool hasNoSceneViewItem()const;

  void restoreSceneView(const QString& sceneViewNodeId);
  void deleteSceneView(const QString& sceneViewNodeId);

public:
  vtkSmartPointer<vtkDMMLScene>         DMMLScene;
  QSignalMapper                         RestoreActionMapper;
  QSignalMapper                         DeleteActionMapper;
  QString                               NoSceneViewText;

};

#endif

