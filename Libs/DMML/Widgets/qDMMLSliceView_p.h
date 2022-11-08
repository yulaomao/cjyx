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

#ifndef __qDMMLSliceView_p_h
#define __qDMMLSliceView_p_h

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
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLSliceView.h"

// DMML includes
#include "vtkLightBoxRendererManager.h"
#include "vtkDMMLLightBoxRendererManagerProxy.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkDMMLDisplayableManagerGroup;
class vtkDMMLSliceNode;
class vtkDMMLCameraNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qDMMLSliceViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLSliceView);
protected:
  qDMMLSliceView* const q_ptr;
public:
  qDMMLSliceViewPrivate(qDMMLSliceView& object);
  ~qDMMLSliceViewPrivate() override;

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
  vtkDMMLSliceNode*                  DMMLSliceNode;
  QColor                             InactiveBoxColor;

  class vtkInternalLightBoxRendererManagerProxy;
  vtkSmartPointer<vtkInternalLightBoxRendererManagerProxy> LightBoxRendererManagerProxy;
};

//--------------------------------------------------------------------------
// qDMMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy class

//---------------------------------------------------------------------------
class qDMMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy
  : public vtkDMMLLightBoxRendererManagerProxy
{
public:
  static vtkInternalLightBoxRendererManagerProxy* New();
  vtkTypeMacro(vtkInternalLightBoxRendererManagerProxy,
                       vtkDMMLLightBoxRendererManagerProxy);


  /// Method to query the mapping from an id of a LightBox frame to
  /// the Renderer for that frame
  vtkRenderer *GetRenderer(int id) override;

  /// Method to set the real LightBoxManager
  virtual void SetLightBoxRendererManager(vtkLightBoxRendererManager *mgr);

protected:
  vtkInternalLightBoxRendererManagerProxy();
  ~vtkInternalLightBoxRendererManagerProxy() override;

private:
  vtkInternalLightBoxRendererManagerProxy(const vtkInternalLightBoxRendererManagerProxy&) = delete;
  void operator=(const vtkInternalLightBoxRendererManagerProxy&) = delete;

  vtkWeakPointer<vtkLightBoxRendererManager> LightBoxRendererManager;

};


#endif
