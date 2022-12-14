#
# ScriptedExampleDisplayableManager
#

#
# Since it's not possible to derive a VTK class in python, all scripted DisplayableManager
# should expose the following methods:
#    - Create
#    - GetDMMLSceneEventsToObserve
#    - ProcessDMMLSceneEvents
#    - ProcessDMMLNodesEvents
#    - RemoveDMMLObservers
#    - UpdateFromDMML
#    - OnInteractorStyleEvent
#    - OnDMMLDisplayableNodeModifiedEvent
#
# The constructor has one parameter named 'parent' corresponding to the associated instance of
# vtkScriptedDisplayableManager in the C++ world.
#
# The python methods listed above corresponds to the implementation of the virtual method
# available in vtkScriptedDisplayableManager.
#
# The only exception is the virtual method SetDMMLSceneInternal, the python class only needs to
# implement the method GetDMMLSceneEventsToObserve. This later one just return a list of integer
# representing the eventid to observe.
#
# It's also possible to access the API of the associated C++ instance using the self.Parent
# For example:
#   self.Parent.RemoveInteractorStyleObservableEvent(26) # vtkCommand::MouseWheelForwardEvent
#
# Make also sure NOT to call the corresponding C++ method from it's python equivalent, it will
# result in an infinite loop.
# The following statement will likely lead to an unstable state:
#    def Create(self): self.Parent.Create()
#
# If a a method isn't implemented, the following syntax should be used:
#   def Create(self): pass
#
# NOTE
#   Ideally, a DisplayableManager should deal only with DMMLNodes. Incriminated code should
# be moved either in the DisplayableManager itself, in the associated DMML Node or
# in a DMMLNode helper class.
#
# TODO
#   While porting existing code, to overcome this problem, the following need to be done:
#     - DisplayableManager abstract base class should have a reference to the current DMMLApplicationLogic
#     - The DMMLApplicationLogic should contain a map of logics
#     - The list of logic internally used by the qCjyxLayoutManager should be removed and
#     the list from the DMMLApplicationLogic used instead.

class vtkScriptedExampleDisplayableManager:

    def __init__(self, parent):
        self.Parent = parent
        print("vtkScriptedExampleDisplayableManager - __init__")

    def Create(self):
        print("vtkScriptedExampleDisplayableManager - Create")
        pass

    def GetDMMLSceneEventsToObserve(self):
        print("vtkScriptedExampleDisplayableManager - GetDMMLSceneEventsToObserve")
        sceneEvents = vtkIntArray()
        sceneEvents.InsertNextValue(cjyx.vtkDMMLScene.NodeAddedEvent)
        sceneEvents.InsertNextValue(cjyx.vtkDMMLScene.NodeRemovedEvent)
        return sceneEvents

    def ProcessDMMLSceneEvents(self, scene, eventid, node):
        print("vtkScriptedExampleDisplayableManager - ProcessDMMLSceneEvents(eventid,", eventid, ")")
        pass

    def ProcessDMMLNodesEvents(self, scene, eventid, callData):
        print("vtkScriptedExampleDisplayableManager - ProcessDMMLNodesEvents(eventid,", eventid, ")")
        pass

    def RemoveDMMLObservers(self):
        print("vtkScriptedExampleDisplayableManager - RemoveDMMLObservers")
        pass

    def UpdateFromDMML(self):
        print("vtkScriptedExampleDisplayableManager - UpdateFromDMML")
        pass

    def OnInteractorStyleEvent(self, eventid):
        print("vtkScriptedExampleDisplayableManager - OnInteractorStyleEvent(eventid,", eventid, ")")

    def OnDMMLDisplayableNodeModifiedEvent(self, viewNode):
        print("vtkScriptedExampleDisplayableManager - onDMMLDisplayableNodeModifiedEvent")
