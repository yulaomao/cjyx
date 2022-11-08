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

#ifndef __vtkDMMLLightBoxRendererManagerProxy_h
#define __vtkDMMLLightBoxRendererManagerProxy_h

// DMMLDisplayableManager include
#include "vtkDMMLDisplayableManagerExport.h"
#include "vtkDMMLAbstractLogic.h"

class vtkRenderer;

/// \brief Proxy class to provide mechanisms for a displayable manager to
/// communicate with 3rd party renderer managers (like CTK).
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLLightBoxRendererManagerProxy
  : public vtkDMMLAbstractLogic
{
public:
  static vtkDMMLLightBoxRendererManagerProxy* New();
  vtkTypeMacro(vtkDMMLLightBoxRendererManagerProxy,
                       vtkDMMLAbstractLogic);

  /// Method to query the mapping from an id of a LightBox frame to
  /// the Renderer for that frame
  virtual vtkRenderer *GetRenderer(int vtkNotUsed(id)) { return nullptr; };

protected:
  vtkDMMLLightBoxRendererManagerProxy() ;
  ~vtkDMMLLightBoxRendererManagerProxy() override ;

private:
  vtkDMMLLightBoxRendererManagerProxy(const vtkDMMLLightBoxRendererManagerProxy&) = delete;
  void operator=(const vtkDMMLLightBoxRendererManagerProxy&) = delete;


};

#endif
