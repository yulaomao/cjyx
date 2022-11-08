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

#ifndef __qDMMLThreeDViewControllerWidget_p_h
#define __qDMMLThreeDViewControllerWidget_p_h

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
#include "qDMMLThreeDViewControllerWidget.h"
#include "qDMMLViewControllerBar_p.h"
#include "ui_qDMMLThreeDViewControllerWidget.h"

// DMMLLogic includes
#include <vtkDMMLViewLogic.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class QAction;
class ctkButtonGroup;
class ctkSignalMapper;
class qDMMLSceneViewMenu;
class vtkDMMLCameraNode;
class vtkDMMLViewNode;
class QString;

//-----------------------------------------------------------------------------
class qDMMLThreeDViewControllerWidgetPrivate
  : public qDMMLViewControllerBarPrivate
  , public Ui_qDMMLThreeDViewControllerWidget
{
  Q_DECLARE_PUBLIC(qDMMLThreeDViewControllerWidget);
public:
  typedef qDMMLViewControllerBarPrivate Superclass;
  qDMMLThreeDViewControllerWidgetPrivate(qDMMLThreeDViewControllerWidget& object);
  ~qDMMLThreeDViewControllerWidgetPrivate() override;

  void init() override;

  vtkDMMLViewLogic* viewNodeLogic(vtkDMMLViewNode* node);

  vtkWeakPointer<vtkDMMLCameraNode>   CameraNode;
  qDMMLThreeDView*                    ThreeDView;

  vtkSmartPointer<vtkDMMLViewLogic>   ViewLogic;

  ctkSignalMapper*                    StereoTypesMapper;
  ctkButtonGroup*                     AnimateViewButtonGroup;
  ctkSignalMapper*                    OrientationMarkerTypesMapper;
  ctkSignalMapper*                    OrientationMarkerSizesMapper;
  ctkSignalMapper*                    RulerTypesMapper;
  ctkSignalMapper*                    RulerColorMapper;

  QToolButton*                        CenterToolButton;

protected:
  void setupPopupUi() override;
};

#endif
