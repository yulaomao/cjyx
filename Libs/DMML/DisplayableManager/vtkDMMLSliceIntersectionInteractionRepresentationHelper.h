/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Ebatinca S.L., Las Palmas de Gran Canaria, Spain

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

/**
 * @class   vtkDMMLSliceIntersectionInteractionRepresentationHelper
 * @brief   represent intersections of other slice views in the current slice view
 *
 * @sa
 * vtkSliceIntersectionWidget vtkWidgetRepresentation vtkAbstractWidget
*/

#ifndef vtkDMMLSliceIntersectionInteractionRepresentationHelper_h
#define vtkDMMLSliceIntersectionInteractionRepresentationHelper_h

#include "vtkDMMLDisplayableManagerExport.h" // For export macro
#include "vtkDMMLAbstractWidgetRepresentation.h"

#include "vtkDMMLSliceNode.h"

class vtkDMMLApplicationLogic;
class vtkDMMLModelDisplayNode;
class vtkDMMLSliceLogic;

class vtkProperty2D;
class vtkActor2D;
class vtkPolyDataMapper2D;
class vtkPolyData;
class vtkPoints;
class vtkCellArray;
class vtkTextProperty;
class vtkLeaderActor2D;
class vtkTextMapper;
class vtkTransform;
class vtkActor2D;
class vtkDMMLInteractionEventData;

class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLSliceIntersectionInteractionRepresentationHelper : public vtkDMMLAbstractWidgetRepresentation
{
  public:
    /**
     * Instantiate this class.
     */
    static vtkDMMLSliceIntersectionInteractionRepresentationHelper* New();

    //@{
    /**
     * Standard methods for instances of this class.
     */
    vtkTypeMacro(vtkDMMLSliceIntersectionInteractionRepresentationHelper, vtkDMMLAbstractWidgetRepresentation);
    void PrintSelf(ostream& os, vtkIndent indent) override;
    //@}

    int IntersectWithFinitePlane(double n[3], double o[3], double pOrigin[3], double px[3], double py[3], double x0[3], double x1[3]);

    /// Compute intersection between a 2D line and the slice view boundaries
    void GetIntersectionWithSliceViewBoundaries(double* pointA, double* pointB, double* sliceViewBounds, double* intersectionPoint);

    /// Get boundaries of the slice view associated with a given vtkDMMLSliceNode
    void GetSliceViewBoundariesXY(vtkDMMLSliceNode* sliceNode, double* sliceViewBounds);

    int GetLineTipsFromIntersectingSliceNode(vtkDMMLSliceNode* intersectingSliceNode, vtkMatrix4x4* intersectingXYToXY,
        double intersectionLineTip1[3], double intersectionLineTip2[3]);

    void ComputeHandleToWorldTransformMatrix(double handlePosition[2], double handleOrientation[2], vtkMatrix4x4* handleToWorldTransformMatrix);
    void RotationMatrixFromVectors(double vector1[2], double vector2[2], vtkMatrix4x4* rotationMatrixHom);

  protected:
    vtkDMMLSliceIntersectionInteractionRepresentationHelper();
    ~vtkDMMLSliceIntersectionInteractionRepresentationHelper() override;

  private:
    vtkDMMLSliceIntersectionInteractionRepresentationHelper(const vtkDMMLSliceIntersectionInteractionRepresentationHelper&) = delete;
    void operator=(const vtkDMMLSliceIntersectionInteractionRepresentationHelper&) = delete;
};

#endif
