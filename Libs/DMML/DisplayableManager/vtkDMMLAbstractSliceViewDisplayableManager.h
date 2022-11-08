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

#ifndef __vtkDMMLAbstractSliceViewDisplayableManager_h
#define __vtkDMMLAbstractSliceViewDisplayableManager_h

// DMMLDisplayableManager includes
#include "vtkDMMLAbstractDisplayableManager.h"

#include "vtkDMMLDisplayableManagerExport.h"

class vtkDMMLSliceNode;

/// \brief Superclass for displayable manager classes.
///
/// A displayable manager class is responsible to represent a
/// DMMLDisplayable node in a renderer.
class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLAbstractSliceViewDisplayableManager :
    public vtkDMMLAbstractDisplayableManager
{
public:

  typedef vtkDMMLAbstractSliceViewDisplayableManager Self;

  static vtkDMMLAbstractSliceViewDisplayableManager *New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkDMMLAbstractSliceViewDisplayableManager,
                       vtkDMMLAbstractDisplayableManager);

  /// Get DMML SliceNode
  vtkDMMLSliceNode * GetDMMLSliceNode();

  /// Convert device coordinates (display) to XYZ coordinates (viewport).
  /// Parameter \a xyz is double[3]
  /// \sa ConvertDeviceToXYZ(vtkRenderWindowInteractor *, vtkDMMLSliceNode *, double x, double y, double xyz[3])
  void ConvertDeviceToXYZ(double x, double y, double xyz[3]);

  /// Convenience function allowing to convert device coordinates (display) to XYZ coordinates (viewport).
  /// Parameter \a xyz is double[3]
  static void ConvertDeviceToXYZ(vtkRenderWindowInteractor * interactor,
                                 vtkDMMLSliceNode * sliceNode, double x, double y, double xyz[3]);

  /// Convenience function allowing to convert device coordinates (display) to XYZ coordinates (viewport).
  /// Parameter \a xyz is double[3]
  static void ConvertDeviceToXYZ(vtkRenderer * renderer,
    vtkDMMLSliceNode * sliceNode, double x, double y, double xyz[3]);


  /// Convert RAS to XYZ coordinates (viewport).
  /// Parameters \a ras and \a xyz are double[3]. \a xyz[2] is the lightbox id.
  /// \sa ConvertRASToXYZ(vtkDMMLSliceNode * sliceNode, double ras[3], double xyz[3])
  void ConvertRASToXYZ(double ras[3], double xyz[3]);

  /// Convenience function allowing to convert RAS to XYZ coordinates (viewport).
  /// Parameters \a ras and \a xyz are double[3]. \a xyz[2] is the lightbox id.
  static void ConvertRASToXYZ(vtkDMMLSliceNode * sliceNode, double ras[3], double xyz[3]);

  /// Convert XYZ (viewport) to RAS coordinates.
  /// Parameters \a ras and \a xyz are double[3]. \a xyz[2] is the lightbox id.
  /// \sa ConvertXYZToRAS(vtkDMMLSliceNode * sliceNode, double xyz[3], double ras[3])
  void ConvertXYZToRAS(double xyz[3], double ras[3]);

  /// Convenience function allowing to Convert XYZ (viewport) to RAS coordinates.
  /// Parameters \a ras and \a xyz are double[3]. \a xyz[2] is the lightbox id.
  static void ConvertXYZToRAS(vtkDMMLSliceNode * sliceNode, double xyz[3], double ras[3]);

protected:

  vtkDMMLAbstractSliceViewDisplayableManager();
  ~vtkDMMLAbstractSliceViewDisplayableManager() override;

  void OnDMMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

  /// Could be overloaded if DisplayableManager subclass
  virtual void OnDMMLSliceNodeModifiedEvent(){}

private:

  vtkDMMLAbstractSliceViewDisplayableManager(const vtkDMMLAbstractSliceViewDisplayableManager&) = delete;
  void operator=(const vtkDMMLAbstractSliceViewDisplayableManager&) = delete;
};

#endif
