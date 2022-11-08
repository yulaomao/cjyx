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
 * @class   vtkDMMLSliceIntersectionInteractionRepresentation
 * @brief   represent intersections of other slice views in the current slice view
 *
 * @sa
 * vtkSliceIntersectionWidget vtkWidgetRepresentation vtkAbstractWidget
*/

#ifndef vtkDMMLSliceIntersectionInteractionRepresentation_h
#define vtkDMMLSliceIntersectionInteractionRepresentation_h

#include "vtkDMMLDisplayableManagerExport.h" // For export macro
#include "vtkDMMLAbstractWidgetRepresentation.h"
#include "vtkDMMLSliceIntersectionInteractionRepresentationHelper.h"

#include "vtkDMMLSliceDisplayNode.h"
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

class SliceIntersectionInteractionDisplayPipeline;
class vtkDMMLInteractionEventData;

class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLSliceIntersectionInteractionRepresentation : public vtkDMMLAbstractWidgetRepresentation
{
  public:
    /**
     * Instantiate this class.
     */
    static vtkDMMLSliceIntersectionInteractionRepresentation* New();

    //@{
    /**
     * Standard methods for instances of this class.
     */
    vtkTypeMacro(vtkDMMLSliceIntersectionInteractionRepresentation, vtkDMMLAbstractWidgetRepresentation);
    void PrintSelf(ostream& os, vtkIndent indent) override;
    //@}

    void SetSliceNode(vtkDMMLSliceNode* sliceNode);
    vtkDMMLSliceNode* GetSliceNode();

    vtkDMMLSliceDisplayNode* GetSliceDisplayNode();

    bool IsDisplayable();
    void UpdateFromDMML(vtkDMMLNode* caller, unsigned long event, void* callData = nullptr) override;

    void AddIntersectingSliceLogic(vtkDMMLSliceLogic* sliceLogic);
    void RemoveIntersectingSliceNode(vtkDMMLSliceNode* sliceNode);
    void UpdateIntersectingSliceNodes();
    void RemoveAllIntersectingSliceNodes();

    //@{
    /**
     * Methods to make this class behave as a vtkProp.
     */
    void GetActors2D(vtkPropCollection*) override;
    void ReleaseGraphicsResources(vtkWindow*) override;
    int RenderOverlay(vtkViewport* viewport) override;
    //@}

    void SetDMMLApplicationLogic(vtkDMMLApplicationLogic*);
    vtkGetObjectMacro(DMMLApplicationLogic, vtkDMMLApplicationLogic);

    /// Compute slice intersection point between red, green and yellow slice nodes
    void ComputeSliceIntersectionPoint();

    /// Get slice intersection point between red, green and yellow slice nodes
    double* GetSliceIntersectionPoint();

    /// Check whether the mouse cursor is within the slice view or not
    bool IsMouseCursorInSliceView(double cursorPosition[2]);

    void SetPipelinesHandlesVisibility(bool visible);
    void SetPipelinesHandlesOpacity(double opacity);

    void TransformIntersectingSlices(vtkMatrix4x4* rotatedSliceToSliceTransformMatrix);

    /// Return found component type (as vtkDMMLInteractionDisplayNode::ComponentType).
    /// closestDistance2 is the squared distance in display coordinates from the closest position where interaction is possible.
    /// componentIndex returns index of the found component (e.g., if control point is found then control point index is returned).
    virtual std::string CanInteract(vtkDMMLInteractionEventData* interactionEventData,
        int& foundComponentType, int& foundComponentIndex, double& closestDistance2, double& handleOpacity);

    virtual double GetMaximumHandlePickingDistance2();

    class HandleInfo
      {
      public:
        HandleInfo(int index, int componentType, const std::string& intersectingSliceNodeID, double positionWorld[3], double positionLocal[3])
          : Index(index)
          , ComponentType(componentType)
          , IntersectingSliceNodeID(intersectingSliceNodeID)
          {
          for (int i = 0; i < 3; ++i)
            {
            this->PositionWorld[i] = positionWorld[i];
            }
          this->PositionWorld[3] = 1.0;
          for (int i = 0; i < 3; ++i)
            {
            this->PositionLocal[i] = positionLocal[i];
            }
          this->PositionLocal[3] = 1.0;
          }
        int Index;
        int ComponentType;
        std::string IntersectingSliceNodeID;
        double PositionLocal[4];
        double PositionWorld[4];
      };

    /// Get the list of info for all interaction handles
    typedef std::vector<HandleInfo> HandleInfoList;
    virtual HandleInfoList GetHandleInfoList(SliceIntersectionInteractionDisplayPipeline* pipeline);

    virtual int GetTranslateArrowCursor(const std::string& intersectingSliceNodeID);

  protected:
    vtkDMMLSliceIntersectionInteractionRepresentation();
    ~vtkDMMLSliceIntersectionInteractionRepresentation() override;

    SliceIntersectionInteractionDisplayPipeline* GetDisplayPipelineFromSliceLogic(vtkDMMLSliceLogic* sliceLogic);

    static void SliceNodeModifiedCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);
    void SliceNodeModified(vtkDMMLSliceNode* sliceNode);
    void SliceModelDisplayNodeModified(vtkDMMLModelDisplayNode* sliceNode);

    void UpdateSliceIntersectionDisplay(SliceIntersectionInteractionDisplayPipeline* pipeline);

    vtkDMMLSliceDisplayNode* GetSliceDisplayNode(vtkDMMLSliceNode* sliceNode);

    void SetSliceDisplayNode(vtkDMMLSliceDisplayNode* sliceDisplayNode);

    double GetSliceRotationAngleRad(int eventPos[2]);

    double GetLineThicknessFromMode(int lineThicknessMode);

    /// Support picking
    double LastEventPosition[2];

    /// Slice intersection point in XY
    double SliceIntersectionPoint[4];

    /// Indicate whether a valid slice intersection point was found or not
    bool SliceIntersectionPointFound;

    /// Handle size, specified in renderer world coordinate system.
    /// For slice views, renderer world coordinate system is the display coordinate system, so it is measured in pixels.
    /// For 3D views, renderer world coordinate system is the Cjyx world coordinate system, so it is measured in the
    /// scene length unit (typically millimeters).
    double InteractionSize{ 3.0 };

    double GetViewScaleFactorAtPosition(double positionWorld[3]);

    vtkDMMLApplicationLogic* DMMLApplicationLogic;

    class vtkInternal;
    vtkInternal* Internal;

    vtkDMMLSliceIntersectionInteractionRepresentationHelper* Helper;

  private:
    vtkDMMLSliceIntersectionInteractionRepresentation(const vtkDMMLSliceIntersectionInteractionRepresentation&) = delete;
    void operator=(const vtkDMMLSliceIntersectionInteractionRepresentation&) = delete;
};

#endif
