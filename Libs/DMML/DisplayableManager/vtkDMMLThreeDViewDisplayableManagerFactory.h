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

#ifndef __vtkDMMLThreeDViewDisplayableManagerFactory_h
#define __vtkDMMLThreeDViewDisplayableManagerFactory_h

// DMMLDisplayableManager includes
#include "vtkDMMLDisplayableManagerFactory.h"

// VTK includes
#include <vtkSingleton.h>

#include "vtkDMMLDisplayableManagerExport.h"

class vtkRenderer;

/// \brief Factory where displayable manager classes are registered.
///
/// A displayable manager class is responsible to represent a
/// DMMLDisplayable node in a renderer.
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLThreeDViewDisplayableManagerFactory
  : public vtkDMMLDisplayableManagerFactory
{
public:

  vtkTypeMacro(vtkDMMLThreeDViewDisplayableManagerFactory,
                       vtkDMMLDisplayableManagerFactory);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// This is a singleton pattern New.  There will only be ONE
  /// reference to a vtkDMMLThreeDViewDisplayableManagerFactory object per process. Clients that
  /// call this must call Delete on the object so that the reference counting will work.
  /// The single instance will be unreferenced when the program exits.
  static vtkDMMLThreeDViewDisplayableManagerFactory *New();

  /// Return the singleton instance with no reference counting.
  static vtkDMMLThreeDViewDisplayableManagerFactory* GetInstance();

protected:

  vtkDMMLThreeDViewDisplayableManagerFactory();
  ~vtkDMMLThreeDViewDisplayableManagerFactory() override;

  VTK_SINGLETON_DECLARE(vtkDMMLThreeDViewDisplayableManagerFactory);

private:

  vtkDMMLThreeDViewDisplayableManagerFactory(const vtkDMMLThreeDViewDisplayableManagerFactory&) = delete;
  void operator=(const vtkDMMLThreeDViewDisplayableManagerFactory&) = delete;

};

#ifndef __VTK_WRAP__
//BTX
VTK_SINGLETON_DECLARE_INITIALIZER(VTK_DMML_DISPLAYABLEMANAGER_EXPORT,
                                  vtkDMMLThreeDViewDisplayableManagerFactory);
//ETX
#endif // __VTK_WRAP__

#endif
