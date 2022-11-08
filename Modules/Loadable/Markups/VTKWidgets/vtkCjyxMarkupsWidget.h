/*=========================================================================

 Copyright (c) ProxSim ltd., Kwun Tong, Hong Kong. All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
 and development was supported by ProxSim ltd.

=========================================================================*/

/**
 * @class   vtkCjyxMarkupsWidget
 * @brief   Process interaction events to update state of markup widget nodes
 *
 * @sa
 * vtkDMMLAbstractWidget vtkCjyxWidgetRepresentation vtkCjyxWidgetEventTranslator
 *
*/

#ifndef vtkCjyxMarkupsWidget_h
#define vtkCjyxMarkupsWidget_h

#include "vtkCjyxMarkupsModuleVTKWidgetsExport.h"
#include "vtkDMMLAbstractWidget.h"
#include "vtkWidgetCallbackMapper.h"

#include "vtkDMMLMarkupsNode.h"

class vtkDMMLAbstractViewNode;
class vtkDMMLApplicationLogic;
class vtkDMMLInteractionEventData;
class vtkDMMLInteractionNode;
class vtkIdList;
class vtkPolyData;
class vtkCjyxMarkupsWidgetRepresentation;

class VTK_CJYX_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkCjyxMarkupsWidget : public vtkDMMLAbstractWidget
{
public:
  /// Standard methods for a VTK class.
  vtkTypeMacro(vtkCjyxMarkupsWidget, vtkDMMLAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create the default widget representation and initializes the widget and representation.
  virtual void CreateDefaultRepresentation(vtkDMMLMarkupsDisplayNode* markupsDisplayNode, vtkDMMLAbstractViewNode* viewNode, vtkRenderer* renderer) = 0;

  /// Create instance of the markups widget
  virtual vtkCjyxMarkupsWidget* CreateInstance() const = 0;

  /// Widget states
  enum
  {
    WidgetStateDefine = WidgetStateUser, // click in empty area will place a new point
    WidgetStateTranslateControlPoint, // translating the active point by mouse move
    WidgetStateOnTranslationHandle, // hovering over a translation interaction handle
    WidgetStateOnRotationHandle, // hovering over a rotation interaction handle
    WidgetStateOnScaleHandle, // hovering over a scale interaction handle
    WidgetStateMarkups_Last
  };

  /// Widget events
  enum
  {
    WidgetEventControlPointPlace = WidgetEventUser,
    WidgetEventClickAndDragStart,
    WidgetEventClickAndDragEnd,
    WidgetEventStopPlace,
    WidgetEventControlPointMoveStart,
    WidgetEventControlPointMoveEnd,
    WidgetEventControlPointDelete,
    WidgetEventControlPointInsert,
    WidgetEventControlPointSnapToSlice,
    WidgetEventReserved,  // this events is only to prevent other widgets from processing an event
    WidgetEventMarkups_Last
  };

  // Returns true if one of the markup points are just being previewed and not placed yet.
  bool IsPointPreviewed();

  /// Update a the current index of the point preview being previewed.
  void UpdatePreviewPointIndex(vtkDMMLInteractionEventData* eventData);

  /// Add/update a point preview to the current active Markup at the specified position.
  void UpdatePreviewPoint(vtkDMMLInteractionEventData* eventData, const char* associatedNodeID, int positionStatus);

  /// Remove the point preview to the current active Markup.
  /// Returns true is preview point existed and now it is removed.
  bool RemovePreviewPoint();

  // Places a new markup point.
  // Reuses current preview point, if possible.
  // Returns true if the event is processed.
  virtual bool PlacePoint(vtkDMMLInteractionEventData* eventData);

  /// Add a point to the current active Markup at input World coordinates.
  virtual int AddPointFromWorldCoordinate(const double worldCoordinates[3]);

  /// Given a specific X, Y pixel location, add a new node
  /// on the widget at this location.
  virtual int AddNodeOnWidget(const int displayPos[2]);

  /// Return true if the widget can process the event.
  bool CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &distance2) override;

  /// Process interaction event.
  bool ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData) override;

  /// Called when the the widget loses the focus.
  void Leave(vtkDMMLInteractionEventData* eventData) override;

  // Allows the widget to request interactive mode (faster updates)
  bool GetInteractive() override;
  // Allows the widget to request a cursor shape
  int GetMouseCursor() override;

  vtkDMMLMarkupsNode* GetMarkupsNode();
  vtkDMMLMarkupsDisplayNode* GetMarkupsDisplayNode();
  int GetActiveControlPoint();

  vtkCjyxMarkupsWidgetRepresentation* GetMarkupsRepresentation();

  int GetActiveComponentType();
  int GetActiveComponentIndex();

  vtkDMMLSelectionNode* selectionNode();

protected:
  vtkCjyxMarkupsWidget();
  ~vtkCjyxMarkupsWidget() override;

  void StartWidgetInteraction(vtkDMMLInteractionEventData* eventData);
  void EndWidgetInteraction();

  virtual void TranslatePoint(double eventPos[2], bool snapToSlice = false);
  virtual void TranslateWidget(double eventPos[2]);
  virtual void ScaleWidget(double eventPos[2]);
  virtual void RotateWidget(double eventPos[2]);

  bool IsAnyControlPointLocked();

  // Get accurate world position.
  // World position that comes in the event data may be inaccurate, this method computes a more reliable position.
  // Returns true on success.
  // refWorldPos is an optional reference position: if point distance from camera cannot be determined then
  // depth of this reference position is used.
  bool ConvertDisplayPositionToWorld(const int displayPos[2], double worldPos[3], double worldOrientationMatrix[9],
    double* refWorldPos = nullptr);

  /// Index of the control point that is currently being previewed (follows the mouse pointer).
  /// If <0 it means that there is currently no point being previewed.
  int PreviewPointIndex;

  // Callback interface to capture events when
  // placing the widget.
  // Return true if the event is processed.
  virtual bool ProcessMouseMove(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessWidgetMenu(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessWidgetAction(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessWidgetStopPlace(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessControlPointSnapToSlice(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessControlPointDelete(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessControlPointInsert(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessControlPointMoveStart(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessWidgetTranslateStart(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessWidgetRotateStart(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessWidgetScaleStart(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessEndMouseDrag(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessWidgetReset(vtkDMMLInteractionEventData* eventData);
  virtual bool ProcessWidgetJumpCursor(vtkDMMLInteractionEventData* eventData);

  // Get the closest point on the line defined by the interaction handle axis.
  // Input coordinates are in display coordinates, while output are in world coordinates.
  virtual bool GetClosestPointOnInteractionAxis(int type, int index, const double inputDisplay[2], double outputIntersectionWorld[3]);

  // Get the closest point on the plane defined using the interaction handle axis as the plane normal.
  // Input coordinates are in display coordinates, while output are in world coordinates
  virtual bool GetIntersectionOnAxisPlane(int type, int index, const double inputDisplay[2], double outputIntersectionWorld[3]);

  // Variables for translate/rotate/scale
  double LastEventPosition[2];
  double StartEventOffsetPosition[2];

private:
  vtkCjyxMarkupsWidget(const vtkCjyxMarkupsWidget&) = delete;
  void operator=(const vtkCjyxMarkupsWidget&) = delete;
};

//----------------------------------------------------------------------
// CREATE INSTANCE MACRO

#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
#define vtkCjyxMarkupsWidgetCreateInstanceMacro(type) \
vtkCjyxMarkupsWidget* CreateInstance() const override\
{ \
  vtkObject* ret = vtkObjectFactory::CreateInstance(#type); \
  if(ret) \
    { \
    return static_cast<type *>(ret); \
    } \
  type* result = new type; \
  result->InitializeObjectBase(); \
  return result; \
}
#else
#define vtkCjyxMarkupsWidgetCreateInstanceMacro(type) \
vtkCjyxMarkupsWidget* CreateInstance() const override\
{ \
  vtkObject* ret = vtkObjectFactory::CreateInstance(#type); \
  if(ret) \
    { \
    return static_cast<type *>(ret); \
    } \
  return new type; \
}
#endif

#endif
