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

==============================================================================*/

/**
 * @class   vtkDMMLRubberBandWidgetRepresentation
 * @brief   represent intersections of other slice views in the current slice view
 *
 * @sa
 * vtkSliceIntersectionWidget vtkWidgetRepresentation vtkAbstractWidget
*/

#ifndef vtkDMMLRubberBandWidgetRepresentation_h
#define vtkDMMLRubberBandWidgetRepresentation_h

#include "vtkDMMLDisplayableManagerExport.h" // For export macro
#include "vtkDMMLAbstractWidgetRepresentation.h"

class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLRubberBandWidgetRepresentation : public vtkDMMLAbstractWidgetRepresentation
{
public:
  /**
   * Instantiate this class.
   */
  static vtkDMMLRubberBandWidgetRepresentation *New();

  //@{
  /**
   * Standard methods for instances of this class.
   */
  vtkTypeMacro(vtkDMMLRubberBandWidgetRepresentation, vtkDMMLAbstractWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /// Top-left point position in display coordinates
  vtkGetVector2Macro(CornerPoint1, int);
  vtkSetVector2Macro(CornerPoint1, int);
  vtkGetVector2Macro(CornerPoint2, int);
  vtkSetVector2Macro(CornerPoint2, int);

  //@{
  /**
   * Methods to make this class behave as a vtkProp.
   */
  void GetActors2D(vtkPropCollection *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;
  int RenderOverlay(vtkViewport *viewport) override;
  //@}

protected:
  vtkDMMLRubberBandWidgetRepresentation();
  ~vtkDMMLRubberBandWidgetRepresentation() override;

  class vtkInternal;
  vtkInternal * Internal;

  int CornerPoint1[2];
  int CornerPoint2[2];

private:
  vtkDMMLRubberBandWidgetRepresentation(const vtkDMMLRubberBandWidgetRepresentation&) = delete;
  void operator=(const vtkDMMLRubberBandWidgetRepresentation&) = delete;
};

#endif
