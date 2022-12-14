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

#ifndef __qDMMLThreeDView_p_h
#define __qDMMLThreeDView_p_h

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
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLThreeDView.h"

class vtkDMMLDisplayableManagerGroup;
class vtkDMMLViewNode;
class vtkDMMLCameraNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qDMMLThreeDViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLThreeDView);
protected:
  qDMMLThreeDView* const q_ptr;
public:
  qDMMLThreeDViewPrivate(qDMMLThreeDView& object);
  ~qDMMLThreeDViewPrivate() override;

  virtual void init();

  void setDMMLScene(vtkDMMLScene* scene);

public slots:
  /// Handle DMML scene event
  void onSceneStartProcessing();
  void onSceneEndProcessing();

  void updateWidgetFromDMML();

protected:
  void initDisplayableManagers();

  vtkDMMLDisplayableManagerGroup*    DisplayableManagerGroup;
  vtkDMMLScene*                      DMMLScene;
  vtkDMMLViewNode*                   DMMLViewNode;
};

#endif
