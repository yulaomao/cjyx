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
 * @class   vtkDMMLAbstractWidget
 * @brief   Process interaction events to update state of DMML widget nodes
 *
 * vtkDMMLAbstractWidget is the abstract class that handles interaction events
 * received from the displayable manager. To decide which widget gets the chance
 * to process an interaction event, this class does not use VTK picking manager,
 * but interactor style class queries displayable managers about what events they can
 * process, displayable manager queries its widgets, and based on the returned
 * information, interactor style selects a displayable manager and the displayable
 * manager selects a widget.
 *
 * vtkAbstractWidget uses vtkCjyxWidgetEventTranslator to translate VTK
 * interaction events (defined in vtkCommand.h) into widget events (defined in
 * vtkDMMLAbstractWidget.h and subclasses). This class allows modification
 * of event bindings.
 *
 * @sa
 * vtkCjyxWidgetRepresentation vtkCjyxWidgetEventTranslator
 *
*/

#ifndef vtkDMMLAbstractWidget_h
#define vtkDMMLAbstractWidget_h

#include "vtkDMMLDisplayableManagerExport.h"
#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "vtkWeakPointer.h"
#include <vector>

class vtkDMMLAbstractViewNode;
class vtkDMMLApplicationLogic;
class vtkDMMLInteractionEventData;
class vtkDMMLInteractionNode;
class vtkDMMLNode;
class vtkDMMLAbstractWidgetRepresentation;
class vtkRenderer;
class vtkWidgetEventTranslator;

class VTK_DMML_DISPLAYABLEMANAGER_EXPORT vtkDMMLAbstractWidget : public vtkObject
{
public:
  /// Standard methods for a VTK class.
  vtkTypeMacro(vtkDMMLAbstractWidget, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  virtual void SetDMMLApplicationLogic(vtkDMMLApplicationLogic* applicationLogic);
  vtkDMMLApplicationLogic* GetDMMLApplicationLogic();

  /// Set the representation.
  /// The widget takes over the ownership of this actor.
  virtual void SetRepresentation(vtkDMMLAbstractWidgetRepresentation *r);

  /// Get the representation
  virtual vtkDMMLAbstractWidgetRepresentation *GetRepresentation();

  /// Build the actors of the representation with the info stored in the DMML scene
  virtual void UpdateFromDMML(vtkDMMLNode* caller, unsigned long event, void *callData = nullptr);

  /// Convenient method to change what state the widget is in.
  vtkSetMacro(WidgetState,int);

  /// Convenient method to determine the state of the method
  vtkGetMacro(WidgetState,int);

  /// The state of the widget
  enum
  {
    WidgetStateAny, ///< this state is used for referring to any widget state (for defining event translations)
    WidgetStateIdle, ///< mouse pointer is outside the widget, click does not do anything
    WidgetStateOnWidget, ///< mouse pointer is over the widget, clicking will add a point or manipulate the line
    WidgetStateTranslate, ///< mouse move transforms the entire widget
    WidgetStateRotate, ///< mouse move transforms the entire widget
    WidgetStateScale, ///< mouse move transforms the entire widget
    WidgetStateUser ///< this is a starting index that can be used for widget-specific states
  };

  /// Widget events
  enum WidgetEvents
  {
    WidgetEventNone,
    WidgetEventMouseMove,
    WidgetEventTranslateStart,
    WidgetEventTranslateEnd,
    WidgetEventRotateStart,
    WidgetEventRotateEnd,
    WidgetEventScaleStart,
    WidgetEventScaleEnd,
    // DMML events
    WidgetEventPick, ///< generates a DMML Pick event (e.g., on left click)
    WidgetEventJumpCursor, ///< jumps cursor to the selected position
    WidgetEventAction, // generates a DMML Action event (e.g., left double-click)
    WidgetEventCustomAction1, ///< allows modules to define custom widget actions and get notification via DMML node event
    WidgetEventCustomAction2, ///< allows modules to define custom widget actions and get notification via DMML node event
    WidgetEventCustomAction3, ///< allows modules to define custom widget actions and get notification via DMML node event
    WidgetEventCustomAction4, ///< allows modules to define custom widget actions and get notification via DMML node event
    WidgetEventCustomAction5, ///< allows modules to define custom widget actions and get notification via DMML node event
    WidgetEventCustomAction6, ///< allows modules to define custom widget actions and get notification via DMML node event
    WidgetEventSelect, ///< change DMML node Selected attribute
    WidgetEventUnselect, ///< change DMML node Selected attribute
    WidgetEventToggleSelect, ///< change DMML node Selected attribute
    // Other actions
    WidgetEventMenu, ///< show context menu
    WidgetEventReset, ///< reset widget to initial state (clear all points)
    WidgetEventUser ///< this is a starting index that can be used for widget-specific events
  };

  /// Return true if the widget can process the event.
  /// Distance2 is the squared distance in display coordinates from the closest interaction position.
  /// The displayable manager with the closest distance will get the chance to process the interaction event.
  virtual bool CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &distance2);

  /// Allows injecting interaction events for processing, without directly observing window interactor events.
  /// Return true if the widget processed the event.
  virtual bool ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData);

  /// Define interaction event to widget event translation for mouse and other controller events
  /// Used in the specified widget state only.
  void SetEventTranslation(int widgetState, unsigned long interactionEvent, int modifiers, unsigned long widgetEvent);

  void SetEventTranslationClickAndDrag(int widgetState, unsigned long startInteractionEvent, int modifiers,
    int widgetStateDragging, unsigned long widgetStartEvent, unsigned long widgetEndEvent);

  /// Define interaction event to widget event translation for mouse and other controller events.
  /// Used in any widget state.
  void SetEventTranslation(unsigned long interactionEvent, int modifiers, unsigned long widgetEvent);

  /// Define interaction event to widget event translation for keyboard events.
  /// Used in any widget state.
  void SetKeyboardEventTranslation(int modifier, char keyCode, int repeatCount, const char* keySym, unsigned long widgetEvent);

  /// Define interaction event to widget event translation for keyboard events.
  /// Used in the specified widget state only.
  void SetKeyboardEventTranslation(int widgetState, int modifier, char keyCode, int repeatCount, const char* keySym, unsigned long widgetEvent);

  /// Get widget event from translation event
  virtual unsigned long TranslateInteractionEventToWidgetEvent(vtkDMMLInteractionEventData* eventData);

  /// Called when the the widget loses the focus.
  virtual void Leave(vtkDMMLInteractionEventData* eventData);

  void SetRenderer(vtkRenderer* renderer);
  vtkRenderer* GetRenderer();

  vtkDMMLInteractionNode* GetInteractionNode();

  // Allows the widget to request a cursor shape
  virtual int GetMouseCursor();

  // Allows the widget to request grabbing of all events (even when the mouse pointer moves out of view)
  virtual bool GetGrabFocus();

  // Allows the widget to request interactive mode (faster updates)
  virtual bool GetInteractive();

  // Allows the widget to request render
  virtual bool GetNeedToRender();

  // Acknowledge rendering request (rendering completed)
  virtual void NeedToRenderOff();

  virtual vtkWidgetEventTranslator* GetEventTranslator(int widgetState);

  virtual int GetNumberOfEventTranslators();

protected:
  vtkDMMLAbstractWidget();
  ~vtkDMMLAbstractWidget() override;

  /// Get ID of the node at the specified event position.
  /// Returns nullptr if nothing can be found.
  const char* GetAssociatedNodeID(vtkDMMLInteractionEventData* eventData);

  /// Helper function that attempts to translate an event with a specific translator only.
  unsigned long TranslateInteractionEventToWidgetEvent(
    vtkWidgetEventTranslator* translator, vtkDMMLInteractionEventData* eventData);

  /// Generate a button click event and checks if it can be processed with CanProcessInteractionEvent.
  bool CanProcessButtonClickEvent(vtkDMMLInteractionEventData* eventData, double& distance2);

  /// Generate a button click event and get it processed with ProcessInteractionEvent.
  /// Returns true if the event was processed.
  virtual int ProcessButtonClickEvent(vtkDMMLInteractionEventData* eventData);

  vtkWeakPointer<vtkRenderer> Renderer;

  vtkWeakPointer<vtkDMMLApplicationLogic> ApplicationLogic;

  // Translates interaction event to widget event.
  // In the future, a vector of event translators could be added
  // (one for each state) to be able to define events
  // that are only allowed in a specific state.
  std::vector< vtkSmartPointer<vtkWidgetEventTranslator> > EventTranslators;

  int WidgetState;

  vtkSmartPointer<vtkDMMLAbstractWidgetRepresentation> WidgetRep;

//  bool NeedToRender;

private:
  vtkDMMLAbstractWidget(const vtkDMMLAbstractWidget&) = delete;
  void operator=(const vtkDMMLAbstractWidget&) = delete;
};

#endif
