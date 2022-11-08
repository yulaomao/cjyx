# DMML Overview

## Introduction

Medical Reality Modeling Language (DMML) is a data model developed to represent all data sets that may be used in medical software applications.

- DMML software library: An open-source software library implements DMML data in-memory representation, reading/writing files, visualization, processing framework, and GUI widgets for viewing and editing. The library is based on VTK toolkit, uses ITK for reading/writing some file format, and has a few additional optional dependencies, such as Qt for GUI widgets. The library kept fully independent from Cjyx and so it can be used in any other medical applications, but Cjyx is the only major application that uses it and therefore DMML library source code is maintained in Cjyx's source code repository.
- DMML file: When an DMML data is saved to file then an XML document is created (with .dmml file extension), which contains an index of all data sets and it may refer to other data files for bulk data storage. A variant of this file format is the DMML bundle file, which contains the .dmml file and all referenced data files in a single zip file (with .mrb extension).

## DMML Scene

- All data is stored in a *DMML scene*, which contains a list of *DMML nodes*.
- Each DMML node has a unique ID in the scene, has a name, custom attributes (key:value pairs), and a number of additional properties to store information specific to its data type. Node types include image volume, surface mesh, point set, transformation, etc.
- Nodes can keep *references* (links) to each other.
- Nodes can *invoke events* when their contents or internal state change. Most common event is "Modified" event, which is invoked whenever the node content is changed. Other nodes, [application logic objects](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Logics), or user interface widgets may add *observers*, which are callback functions that are executed whenever the corresponding event is invoked.

## DMML nodes

### Basic DMML node types

- **Data nodes** store basic properties of a data set. Since the same data set can be displayed in different ways (even within the same application, you may want to show the same data set differently in each view), display properties are not stored in the data node. Similarly, the same data set can be stored in various file formats, therefore file storage properties are not stored in a data node. Data nodes are typically thin wrappers over VTK objects, such as vtkPolyData, vtkImageData, vtkTable. Most important Cjyx core data nodes are the following:
  - **Volume** ([vtkDMMLVolume](https://apidocs.slicer.org/master/classvtkDMMLVolumeNode.html) and its subclasses): stores a 3D image. Each voxel of a volume may be a scalar (to store images with continuous grayscale values, such as a CT image), label (to store discrete labels, such as a segmentation result), vector (for storing displacement fields or RGB color images), or tensor (MRI diffusion images). 2D image volumes are represented as single-slice 3D volumes. 4D volumes are stored in sequence nodes (vtkDMMLSequenceNode).
  - **Model** ([vtkDMMLModelNode](https://apidocs.slicer.org/master/classvtkDMMLModelNode.html)): stores a surface mesh (polygonal elements, points, lines, etc.) or volumetric mesh (tetrahedral, wedge elements, unstructured grid, etc.)
  - **Segmentation** ([vtkDMMLSegmentationNode](https://apidocs.slicer.org/master/classvtkDMMLSegmentationNode.html)): complex data node that can store image segmentation (also known as contouring, labeling). It can store multiple representations internally, for example it can store both binary labelmap image and closed surface mesh.
  - **Markups** ([vtkDMMLMarkupsNode](https://apidocs.slicer.org/master/classvtkDMMLMarkupsNode.html) and subclasses): store simple geometrical objects, such as point lists (formerly called "fiducial lists"), lines, angles, curves, planes for annotation and measurements. Annotations module is the old generation of markups functionality and is being phased out.
  - **Transform** ([vtkDMMLTransformNode](https://apidocs.slicer.org/master/classvtkDMMLTransformNode.html)): stores geometrical transformation that can be applied to any [transformable nodes](https://apidocs.slicer.org/master/classvtkDMMLTransformableNode.html). A transformation can contain any number of linear or non-linear (warping) transforms chained together. In general, it is recommended to use vtkDMMLTransformNode. Child types (vtkDMMLLinearTransformNode, vtkDMMLBSplineTransformNode, vtkDMMLGridTransformNode) are kept for backward compatibility and to allow filtering for specific transformation types in user interface widgets.
  - **Text** ([vtkDMMLTextNode](https://apidocs.slicer.org/master/classvtkDMMLTextNode.html)): stores text data, such as configuration files, descriptive text, etc.
  - **Table** ([vtkDMMLTableNode](https://apidocs.slicer.org/master/classvtkDMMLTableNode.html)): stores tabular data (multiple scalar or vector arrays), used mainly for showing quantitative results in tables and plots
- **Display nodes** ([vtkDMMLDisplayNode](https://apidocs.slicer.org/master/classvtkDMMLDisplayNode.html) and its subclasses) specify properties how to display data nodes. For example, a model node's color is stored in a display node associated with a model node.
  - Multiple display nodes may be added for a single data, each specifying different display properties and view nodes. Built-in Cjyx modules typically only show and allow editing of the *first* display node associated with a data node.
  - If a display node specifies a list of view nodes then the associated data node is only displayed in those views.
  - Display nodes may refer to *color nodes* to specify a list of colors or color look-up-tables.
  - When a data node is created then default display node can be added by calling its `CreateDefaultDisplayNodes()` method. In some cases, Cjyx detects if the display and storage node is missing and tries to create a default nodes, but the developers should not rely on this error-recovery mechanism.
- **Storage nodes** ([vtkDMMLStorageNode](https://apidocs.slicer.org/master/classvtkDMMLStorageNode.html) and its subclasses) specify how to store a data node in file. It can store one or more file name, compression options, coordinate system information, etc.
  - Default storage node may be created for a data node by calling its `CreateDefaultStorageNode()` method.
- **View nodes** ([vtkDMMLAbstractViewNode](https://apidocs.slicer.org/v4.8/classvtkDMMLAbstractViewNode.html) and subclasses) specify view layout and appearance of views, such as background color. Additional nodes related to view nodes include:
  - vtkDMMLCameraNode stores properties of camera of a 3D view.
  - vtkDMMLClipModelsNode defines how to clip models with slice planes.
  - vtkDMMLCrosshairNode stores position and display properties of view crosshair (that is positioned by holding down `Shift` key while moving the mouse in slice or 3D views) and also provides the current mouse pointer position at any time.
  - vtkDMMLLayoutNode defines the current view layout: what views (slice, 3D, table, etc.) are display and where. In addition to switching between built-in view layouts, custom view layouts can be specified using an XML description.
  - vtkDMMLInteractionNode specifies interaction mode of viewers (view/transform, window/level, place markups), such as what happens the user clicks in a view
  - vtkDMMLSelectionNode stores global state information of the scene, such as active markup (that is being placed), units (length, time, etc.) used in the scene, etc
- **Plot nodes** specify how to display table node contents as plots. [Plot series node](https://apidocs.slicer.org/master/classvtkDMMLPlotSeriesNode.html) specifies a data series using one or two columns of a table node. [Plot chart node](https://apidocs.slicer.org/master/classvtkDMMLPlotChartNode.html) specifies which series to plot and how. [Plot view node](https://apidocs.slicer.org/master/classvtkDMMLPlotViewNode.html) specifies which plot chart to show in a view and how user can interact with it.
- **Subject hierarchy node** ([vtkDMMLSubjectHierarchyNode](https://apidocs.slicer.org/master/classvtkDMMLSubjectHierarchyNode.html)) allows organization of data nodes into folders. Subject hierarchy folders may be associated with display nodes, which can be used to override display properties of all children in that folder. It replaces all previous hierarchy management methods, such as model or annotation hierarchies.
- [**Sequence node**](https://github.com/Slicer/Slicer/blob/master/Libs/DMML/Core/vtkDMMLSequenceNode.h) stores a list of data nodes to represent time sequences or other multidimensional data sets in the scene. [Sequence browser node](https://github.com/Slicer/Slicer/blob/master/Modules/Loadable/Sequences/DMML/vtkDMMLSequenceBrowserNode.h) specifies which one of the internal data node should be copied to the scene so that it can be displayed or edited. The node that represents a node of the internal scene is called a *proxy node*. When a proxy node is modified in the scene, all changes can be saved into the internal scene.

Detailed documentation of DMML API can be found in [here](https://apidocs.slicer.org/master/classes.html).

### DMML node attributes

DMML nodes can store custom attributes as (attribute name and value) pairs, which allow storing additional application-specific information in nodes without the need to create new node types.

To avoid name clashes custom modules that adds attributes to nodes should prefix the attribute name with the module's name and the '.' character. Example: the DoseVolumeHistogram module can use attribute names such as `DoseVolumeHistogram.StructureSetName`, `DoseVolumeHistogram.Unit`, `DoseVolumeHistogram.LineStyle`.

DMML nodes attributes can be also used filter criteria in DMML node selector widgets in the user interface.

### DMML Node References

DMML nodes can reference and observe other DMML nodes using the node reference API. A node may reference multiple nodes, each performing a distinct role, and each addressed by a unique string. The same role name can be used to reference multiple nodes.

Node references are used for example for linking data nodes to display and storage nodes and modules can add more node references without changing the referring or referred node.

For more details, see [this page](https://www.slicer.org/wiki/Documentation/Nightly/Developers/DMML/NodeReferences).

### DMML Events and Observers

- Changes in DMML scene and individual nodes propagate to other observing nodes, GUI and Logic objects via VTK events and command-observer mechanism.
- `vtkSetMacro()` automatically invokes ModifiedEvent. Additional events can be invoked using `InvokeEvent()` method.
- Using `AddObserver()`/`RemoveObserver()` methods is tedious and error-prone, therefore it is recommended to use [EventBroker](https://www.slicer.org/wiki/Slicer3:EventBroker) and vtkObserverManager helper class, macros, and callback methods instead.
  - DMML observer macros are defined in Libs/DMML/vtkDMMLNode.h
  - vtkSetDMMLObjectMacro - registers DMML node with another vtk object (another DMML node, Logic or GUI). No observers added.
  - vtkSetAndObserveDMMLObjectMacro - registers DMML node and adds an observer for vtkCommand::ModifyEvent.
  - vtkSetAndObserveDMMLObjectEventsMacro - registers DMML node and adds an observer for a specified set of events.
  - `SetAndObserveDMMLScene()` and `SetAndObserveDMMLSceneEvents()` methods are used in GUI and Logic to observe Modify, NewScene, NodeAdded, etc. events.
  - `ProcessDMMLEvents()` method should be implemented in DMML nodes, Logic, and GUI classes in order to process events from the observed nodes.

## Advanced topics

### Scene undo/redo

DMML Scene provides Undo/Redo mechanism that restores a previous state of the scene and individual nodes. By default, undo/redo is disabled and not displayed on the user interface, as it increased memory usage and was not tested thoroughly.

Basic mechanism:
- Undo/redo is based on saving and restoring the state of DMML nodes in the Scene.
- DMML scene can save snapshot of all nodes into a special Undo and Redo stacks.
- The Undo and Redo stacks store copies of nodes that have changed from the previous snapshot. The node that have not changed are stored by a reference (pointer).
- When an Undo is called on the scene, the current state of Undo stack is copied into the current scene and also into Redo stack.
- All Undoable operations must store their data as DMML nodes

Developer controls at what point the snapshot is saved by calling `SaveStateForUndo()` method on the DMML scene. `SaveStateForUndo()` saves the state of all nodes in the scene. Iy should be called in GUI/Logic classes before changing the state of DMML nodes. This is usually done in the ProcessGUIEvents method that processes events from the user interactions with GUI widgets. `SaveStateForUndo()` should not be called while processing transient events such as continuous events sent by KW UI while dragging a slider (for example vtkKWScale::ScaleValueStartChangingEvent).

The following methods on the DMML scene are used to manage Undo/Redo stacks:

- `vtkDMMLScene::Undo()` restores the previously saved state of the DMML scene.
- `vtkDMMLScene::Redo()` restores the previously undone state of the DMML scene.
- `vtkDMMLScene::SetUndoOff()` ignores following SaveStateForUndo calls (useful when making multiple changes to the scene/nodes that does not need to be undone).
- `vtkDMMLScene::SetUndoOn()` enables following SaveStateForUndo calls.
- `vtkDMMLScene::ClearUndoStack()` clears the undo history.
- `vtkDMMLScene::ClearRedoStack()` clears the redo history.

### Creating Custom DMML Node Classes

If you are adding new functionality to 3D Cjyx either via extensions, or even updates to the core, most of the time the existing DMML nodes will be sufficient. Many powerful C++ and Python extensions simply use and combine existing the node types to create new functionality. Other extensions, instead of creating new DMML nodes from scratch, derive from existing nodes and add just a few methods to get the needed functionality. That said, if the existing DMML nodes don't offer enough (or almost enough) functionality to enable what needs to be done, it is possible to create custom DMML node classes with a little bit of effort.

There are a number of different DMML nodes and helper classes that can be implemented to enable new DMML data type functionality. Here is the not-so-short list. We will go over each of these in detail.

1. [Data node](#the-data-node)
2. [Display node](#the-display-node)
3. [Widget](#the-widget)
4. [Widget Representation](#the-widget-representation)
5. [Displayable Manager](#the-displayable-manager)
6. [Storage node](#the-storage-node)
7. [Reader](#the-reader)
8. [Writer](#the-writer)
9. [Subject Hierarchy Plugin](#the-subject-hierarchy-plugin)
10. [Module](#the-module-aka-putting-it-all-together)

While technically not all of these need to be implemented for a new DMML type to be used and useful, implementing all of them will yield the best results. The resulting DMML type will "be like the Model" and will streamline future maintenance work by providing relevant hints.

:::{note}

DMML nodes are implemented in C++.


DMML nodes can be implemented in a Cjyx extension.
:::

:::{note}

All links to API class and function documentation redirecting to `https://apidocs.slicer.org` correspond to documentation generated from the latest commit of the `master` branch of Slicer. This means that versions of this documentation associated with an older version of Slicer may be out of sync with the linked API.

:::

#### Convention

For the filenames and classes, replace `<MyCustomType>` with the name of your type.

#### The data node

The data node is where the essence of the new DMML type will live. It is where the actual data resides. Notably absent from the data node is any description of how the data should be displayed or stored on disk.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- DMML
                    |-- vtkDMML<MyCustomType>Node.h
                    |-- vtkDMML<MyCustomType>Node.cxx
```

Key points:

* Naming convention for class: `vtkDMML<MyCustomType>Node`
  * E.g. [`vtkDMMLModelNode`](https://apidocs.slicer.org/master/classvtkDMMLModelNode.html)
* Inherits from [`vtkDMMLDisplayableNode`](https://apidocs.slicer.org/master/classvtkDMMLDisplayableNode.html) if it is going to be displayed in the 3D or slice views.
* Constructing a new node:
  * Declare [`vtkDMMLNode* CreateNodeInstance() override`](https://apidocs.slicer.org/master/classvtkDMMLNode.html#a6acd6b70ed94ae7384372bd5cb569639) and `static vtkDMMLYourNodeType *New()`. The implementations will be generated by using the macro [`vtkDMMLNodeNewMacro(vtkDMMLYourNodeType);`](https://apidocs.slicer.org/master/vtkDMMLNode_8h.html#acfdcb323c511216ccf8279632a8f8e94) in your cxx file.
  * Create a protected default constructor.
    * It must be protected because VTK only allows its objects to be created through the `New()` factory function.
    * Because of the use of the `New()` factory function, constructors with parameters are not typically used.
  * Create a destructor if needed.
  * Delete copy/move constructors and copy/move assignment operators.
* To save to an MRB:
  * Override [`const char* GetNodeTagName()`](https://apidocs.slicer.org/master/classvtkDMMLNode.html#a65538ef0ba40565a13a6f42d49031592) to return a unique XML tag.
  * Override [`void ReadXMLAttributes(const char** atts)`](https://apidocs.slicer.org/master/classvtkDMMLNode.html#a7ab4e8e10cadd4486acb626763d0a891) and [`void WriteXML(ostream& of, int indent)`](https://apidocs.slicer.org/master/classvtkDMMLNode.html#aaff972a037da725aaa318bf4b8a6f9a8) to save to XML any attributes of the data node that will not be saved by the writer.
* To work with Transforms:
  * Override [`bool CanApplyNonLinearTransforms() const`](https://apidocs.slicer.org/master/classvtkDMMLTransformableNode.html#a7a7dda29c7ba59e55dcbaeb0f06c7f6b) to return true or false depending on if non linear transforms can be applied to your data type.
  * Override [`void OnTransformNodeReferenceChanged(vtkDMMLTransformNode* transformNode)`](https://apidocs.slicer.org/master/classvtkDMMLTransformableNode.html#aaf5e6afd30216db5aafeb7e1141e9eee). It will be called when a new transform is applied to your data node.
  * Override [`void ApplyTransform(vtkAbstractTransform* transform)`](https://apidocs.slicer.org/master/classvtkDMMLTransformableNode.html#add3fb3ed7b65c0f2de92f535f013869b), which will be called when a transform is hardened.
  * See bullet on `ProcessDMMLEvents`.
* Override [`void GetRASBounds(double bounds[6])`](https://apidocs.slicer.org/master/classvtkDMMLDisplayableNode.html#aa7fbcfe229f9249e1701bef2bbf7d64b) and [`void GetBounds(double bounds[6])`](https://apidocs.slicer.org/master/classvtkDMMLDisplayableNode.html#a0976f1f4f57c701f3301391e9b95e6f9) to allow the "Center the 3D view on scene" button in the 3D viewer to work. Note that the difference between these functions is that `GetRASBounds` returns the bounds after all transforms have been applied, while `GetBounds` returns the pre-transform bounds.
* Use macro `vtkDMMLCopyContentMacro(vtkDMMLYourNodeType)` in the class definition and implement [`void CopyContent(vtkDMMLNode* anode, bool deepCopy)`](https://apidocs.slicer.org/master/classvtkDMMLNode.html#a94940453f557fbc3a59f65bd37311cef) in the cxx file. This should copy the data content of your node via either a shallow or deep copy.
* Override [`vtkDMMLStorageNode* CreateDefaultStorageNode()`](https://apidocs.slicer.org/master/classvtkDMMLStorableNode.html#aa35937e99fbb721847c3aea7a818fa10) to return an owning pointer default storage node type for your class (see [The storage node](#the-storage-node)).
* Override [`void CreateDefaultDisplayNodes()`](https://apidocs.slicer.org/master/classvtkDMMLDisplayableNode.html#ac9f1e28f999d5ccc035317246b728b03) to create the default display nodes (for 3D and/or 2D viewing).
* Override [`void ProcessDMMLEvents(vtkObject * caller, unsigned long event, void* callData)`](https://apidocs.slicer.org/master/classvtkDMMLNode.html#afce3fc71df4f8d1e56901846f3c86a51):
  * This is used to process any events that happen regarding this object.
  * This method should handle the `vtkDMMLTransformableNode::TransformModifiedEvent` which is emitted any time the transform that is associated with the data object is changed.
* Convenience methods - while not necessarily needed, they are nice to have.
  * `Get<MyCustomType>DisplayNode()` function that returns the downcast version of [`GetDisplayNode()`](https://apidocs.slicer.org/master/classvtkDMMLDisplayableNode.html#ab2cac46d934a10817b75b37471fe65a8) saves users of your class a bit of downcasting.
* Other methods:
  * Add other methods as your heart desires to view/modify the actual content of the data being stored.

:::{tip}

Any methods with signatures that contain only primitives, raw pointers to VTK derived objects, or a few std library items like `std::vector` will be automatically wrapped for use in Python. Any functions signatures that contain other classes (custom classes, smart pointers from the std library, etc) will not be wrapped. For best results, try to use existing VTK data objects, or have your custom classes derive from [`vtkObject`](https://vtk.org/doc/nightly/html/classvtkObject.html) to get automatic wrapping.

:::

#### The display node

The display node, contrary to what one may think, is not actually how a DMML object is displayed on screen. Instead it is the list of options for displaying a DMML object. Things like colors, visibility, opacity; all of these can be found in the display node.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- DMML
                    |-- vtkDMML<MyCustomType>DisplayNode.h
                    |-- vtkDMML<MyCustomType>DisplayNode.cxx
```

Key Points:

* Naming convention for class: `vtkDMML<MyCustomType>DisplayNode`
  * E.g. [`vtkDMMLModelDisplayNode`](https://apidocs.slicer.org/master/classvtkDMMLModelDisplayNode.html)
* Inherits from [`vtkDMMLDisplayNode`](https://apidocs.slicer.org/master/classvtkDMMLDisplayNode.html).
* Constructing a new node is same as the [data node](#the-data-node).
* To save to an MRB is same as for the [data node](#the-data-node):
  * Some DMML types like Markups store display information when the actual data is being stored via the writer/storage node. If you do that, no action is needed in this class.
* Convenience methods:
  * `Get<YourDataType>Node()` function that returns a downcasted version of [`vtkDMMLDisplayableNode* GetDisplayableNode()`](https://apidocs.slicer.org/master/classvtkDMMLDisplayNode.html#a7100d9379c0b96b2a9ab1ad0a8514af7).
* Other methods:
  * Add any methods regarding coloring/sizing/displaying your data node.

#### The widget

The widget is interaction half of actually putting a usable object in the 2D or 3D viewer. It is in charge of making the widget representation and interacting with it.

:::{note}

If your DMML node is display only without any interaction from the viewers, the widget is not necessary, just the [widget representation](#the-widget-representation) for displaying.

:::

Files:

```
|-- <Extension>
       |-- <Module>
              |-- VTKWidgets
                    |-- vtkCjyx<MyCustomType>Widget.h
                    |-- vtkCjyx<MyCustomType>Widget.cxx
```

Key points:

* Naming convention for class: `vtkCjyx<MyCustomType>Widget`
* Inherits from [`vtkDMMLAbstractWidget`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidget.html).
* Constructing a new node is same as the [data node](#the-data-node).
* For viewing:
  * Add function(s) to create [widget representation](#the-widget-representation)(s). These will typically take a display node, a view node, and a vtkRenderer.
  * E.g. `void CreateDefaultRepresentation(vtkDMML<MyCustomType>DisplayNode* myCustomTypeDisplayNode, vtkDMMLAbstractViewNode* viewNode, vtkRenderer* renderer);`
* For interaction override some or all of the following methods from vtkDMMLAbstractWidget:
  * [`bool CanProcessInteractionEvent(vtkDMMLInteractionEventData* eventData, double &distance2)`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidget.html#a18d9e1148c3d53ea6244c69265e6176a)
  * [`bool ProcessInteractionEvent(vtkDMMLInteractionEventData* eventData)`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidget.html#ab8d51be6df9402eb96943e0e2d0b3123)
  * [`void Leave(vtkDMMLInteractionEventData* eventData)`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidget.html#a6428fe2d100bd1f38c306d3af74802a5)
  * [`bool GetInteractive()`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidget.html#aaca79315fd1e7dce0f09e82c0b1c5e6e)
  * [`int GetMouseCursor()`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidget.html#a8a6e0f8a3b81636e58f60cebbbd6f4d7)

#### The widget representation

The widget representation is the visualization half of displaying a node on screen. This is where any data structures describing your type are turned into [`vtkActor`](https://vtk.org/doc/nightly/html/classvtkActor.html)s that can be displayed in a VTK render window.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- VTKWidgets
                    |-- vtkCjyx<MyCustomType>WidgetRepresentation.h
                    |-- vtkCjyx<MyCustomType>WidgetRepresentation.cxx
```

Key Points:

* Naming convention for class: `vtkCjyx<MyCustomType>WidgetRepresentation`
* Inherits from [`vtkDMMLAbstractWidgetRepresentation`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidgetRepresentation.html).
* Constructing a new node is same as the [data node](#the-data-node).
* Override [`void UpdateFromDMML(vtkDMMLNode* caller, unsigned long event, void *callData = nullptr)`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidgetRepresentation.html#a404ce09ab17e92d203c82fee2a71b0ee) to update the widget representation when the underlying data or display nodes change.
* To make the class behave like a vtkProp override:
  * [`void GetActors(vtkPropCollection*)`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidgetRepresentation.html#ab0124c85fa2d5032a37c690d58c1ef2f)
  * [`void ReleaseGraphicsResources(vtkWindow*)`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidgetRepresentation.html#aa1949d0b26d58d83df1c0764db6d19bf)
  * [`int RenderOverlay(vtkViewport* viewport)`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidgetRepresentation.html#a51314976a613b4c7324a571ee7243cbc)
  * [`int RenderOpaqueGeometry(vtkViewport* viewport)`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidgetRepresentation.html#a14e33bf29e9272dd9fe29c7baadec730)
  * [`int RenderTranslucentPolygonalGeometry(vtkViewport* viewport)`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidgetRepresentation.html#a0d00da2fcb99df807c6884ac757e1735)
  * [`vtkTypeBool HasTranslucentPolygonalGeometry()`](https://apidocs.slicer.org/master/classvtkDMMLAbstractWidgetRepresentation.html#a16347752784cec5542751b9e6d81cf2b)

:::{important}

The points/lines/etc pulled from the data node should be post-transform, if there are any transforms applied.

:::

:::{tip}

Minimize the number of actors used for better rendering performance.

:::

#### The displayable manager

The data node, display node, widget, and widget representation are all needed pieces for data actually showing up on the screen. The displayable manager is the glue that brings all the pieces together. It monitors the DMML scene, and when data and display nodes are added or removed, it creates or destroys the corresponding widgets and widget representations.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- DMMLDM
                    |-- vtkDMML<MyCustomType>DisplayableManager.h
                    |-- vtkDMML<MyCustomType>DisplayableManager.cxx
```

Key Points:

* Naming convention for class: `vtkCjyx<MyCustomType>DisplayableManager`
* Inherits from [`vtkDMMLAbstractDisplayableManager`](https://apidocs.slicer.org/master/classvtkDMMLAbstractDisplayableManager.html).
* Constructing a new node is same as the [data node](#the-data-node).
* Override [`void OnDMMLSceneNodeAdded(vtkDMMLNode* node)`](https://apidocs.slicer.org/master/classvtkDMMLAbstractLogic.html#ac0102310f18a3880f00e93b8e37210c9) to watch for if a new node of your type is added to the scene. Add an appropriate widget(s) and widget representation(s) for any display nodes.
* Override [`void OnDMMLSceneNodeRemoved(vtkDMMLNode* node)`](https://apidocs.slicer.org/master/classvtkDMMLAbstractLogic.html#a92b1149d08ec5099a9ec61c0636f37c4) to watch for if a node of your type is removed from the scene. Remove corresponding widget(s) and widget representation(s).
* Override [`void OnDMMLSceneEndImport()`](https://apidocs.slicer.org/master/classvtkDMMLAbstractLogic.html#ae826dc18ec31156f0eae3456608eb1c4) to watch for an MRB file that has finished importing.
* Override [`void OnDMMLSceneEndClose()`](https://apidocs.slicer.org/master/classvtkDMMLAbstractLogic.html#a5f8a69a47eacec0cd95857beceb8eb34) to clean up when a scene closes.
* Override [`void ProcessDMMLNodesEvents(vtkObject *caller, unsigned long event, void *callData)`](https://apidocs.slicer.org/master/classvtkDMMLAbstractLogic.html#a43a759874ce32846c6dffc52e5941744) to watch for changes in the data node that would require the display to change.
* Override [`void UpdateFromDMML()`](https://apidocs.slicer.org/master/classvtkDMMLAbstractDisplayableManager.html#a0e691194d46e40d73e598ba318324388) and [`void UpdateFromDMMLScene()`](https://apidocs.slicer.org/master/classvtkDMMLAbstractLogic.html#a50caeb3a373ccd53635ae2c08bc2e63e) to bring the displayable manager in line with the DMML Scene.

#### The storage node

A storage node is responsible for reading and writing data nodes to files. A single data node type can have multiple storage node types associated with it for reading/writing different formats. A storage node will be created for both normal save/load operations for a single data node, as well as when you are saving a whole scene to an MRB.

It is common for a data node’s storage node to also write relevant values out of the display node (colors, opacity, etc) at the same time it writes the data.

:::{note}

The storage node is not sufficient in itself to allow the new data node to be saved/loaded from the normal 3D Cjyx save/load facilities; the [reader](#the-reader) and [writer](#the-writer) will help with that.

:::

Files:

```
|-- <Extension>
       |-- <Module>
              |-- DMML
                    |-- vtkDMML<MyCustomType>StorageNode.h
                    |-- vtkDMML<MyCustomType>StorageNode.cxx
```

Key Points:

* Naming convention for class: `vtkDMML<MyCustomType>StorageNode`
  * If you have multiple storage nodes you may have other information in the name, such as the format that is written. E.g. `vtkDMMLMarkupsJSONStorageNode`.
* Inherits from [`vtkDMMLStorageNode`](https://apidocs.slicer.org/master/classvtkDMMLStorageNode.html).
* Constructing a new node is same as the [data node](#the-data-node).
* Override [`bool CanReadInReferenceNode(vtkDMMLNode *refNode)`](https://apidocs.slicer.org/master/classvtkDMMLStorageNode.html#af76f30cec1ccdc03bc496e5d39c784da) to allow a user to inquire at runtime if a particular node can be read in by this storage node.
* Override protected [`void InitializeSupportedReadFileTypes()`](https://apidocs.slicer.org/master/classvtkDMMLStorageNode.html#af7c838cb215a8a5b1dd87a123b3fbc81) to show what file types and extensions this storage node can read (can be more than one).
* Override protected [`void InitializeSupportedWriteFileTypes()`](https://apidocs.slicer.org/master/classvtkDMMLStorageNode.html#ab161ed9e9869cb61441659fe7c358906) to show what types and extensions this storage node can read (can be more than one).
  * It is recommended to be able to read and write the same file types within a single storage node.
* Override protected [`int ReadDataInternal(vtkDMMLNode *refNode)`](https://apidocs.slicer.org/master/classvtkDMMLStorageNode.html#a56aa52786dad2724a0a6a706b7fcf014):
  * This is called by the public [`ReadData`](https://apidocs.slicer.org/master/classvtkDMMLStorageNode.html#afeeb4c95ee3164bd1d3374c053ffb04f) method.
  * This is where the actually reading data from a file happens.
* Override protected [`int WriteDataInternal(vtkDMMLNode *refNode)`](https://apidocs.slicer.org/master/classvtkDMMLStorageNode.html#a739831f001a7cebbeb72484944e842bf):
  * This is called by the public [`WriteData`](https://apidocs.slicer.org/master/classvtkDMMLStorageNode.html#a1eb7a2b35d28175e2c490ac51cbf5baf) method.
  * This is where the actually writing data to a file happens.
* If your data node uses any coordinates (most nodes that get displayed should) it is recommended to be specific in your storage format whether the saved coordinates are RAS or LPS coordinates.
  * Having a way to allow the user to specify this is even better.
* Other methods
  * Adding a `vtkDMML<MyCustomType>Node* Create<MyCustomType>Node(const char* nodeName)` function will be convenient for implementing the writer and is also convenient for users of the storage node.

:::{tip}

If your storage node reads/writes JSON, [RapidJSON](https://rapidjson.org/index.html) is already in the superbuild and is the recommended JSON parser.

It is recommended to have your extension be `.<something>.json` where the `<something>` is related to your node type (e.g. `.mrk.json` for Markups).

:::

#### The reader

The recommended way to read a file into a DMML node is through the storage node. The reader, on the other hand, exists to interface with the loading facilities of 3D Cjyx (drag and drop, as well as the button to load data into the scene). As such, the reader uses the storage node in its implementation.


Files:

```
|-- <Extension>
       |-- <Module>
              |-- qCjyx<MyCustomType>Reader.h
              |-- qCjyx<MyCustomType>Reader.cxx
```

Key Points:

* Naming convention for class: `qCjyx<MyCustomType>Reader`
* Inherits from [`qCjyxFileReader`](https://apidocs.slicer.org/master/classqSlicerFileReader.html).
* In the class definition, the following macros should be used:
  * `Q_OBJECT`
  * `Q_DECLARE_PRIVATE`
  * `Q_DISABLE_COPY`
* Constructing a new node:
  * Create constructor `qCjyx<MyCustomType>Reader(QObject* parent = nullptr)`.
    * This constructor, even if it is not explicitly used, allows this file to be wrapped in Python.
* Override [`QString description() const`](https://apidocs.slicer.org/master/classqSlicerIO.html#af44106dbf852df0e6cc836b377630f5e) to provide a short description on the types of files read.
* Override [`IOFileType fileType() const`](https://apidocs.slicer.org/master/classqSlicerIO.html#aac38b570ec5c0692caefa7d87657e58b) to give a string to associate with the types of files read.
  * This string can be used in conjunction with the python method [`cjyx.util.loadNodeFromFile`](https://slicer.readthedocs.io/en/latest/developer_guide/slicer.html#slicer.util.loadNodeFromFile)
* Override [`QStringList extensions() const`](https://apidocs.slicer.org/master/classqSlicerFileReader.html#afb3187915977b2253d86634cc465b23d) to provide the extensions that can be read.
  * Should be the same as the storage node since the reader uses the storage node.
* Override [`bool load(const IOProperties& properties)`](https://apidocs.slicer.org/master/classqSlicerFileReader.html#ac9acb878cd8adcc426e9a7a9edac15df). This is the function that actually loads the node from the file into the scene.

:::{important}

The reader is not a VTK object, like the previous objects discussed. It is actually a QObject, so we follow Qt guidelines. One such guideline is the [D-Pointer pattern](https://wiki.qt.io/D-Pointer), which is recommended for use.

:::

#### The writer

The writer is the companion to the reader, so, similar to the reader, it does not implement the actual writing of files, but rather it uses the storage node. Its existence is necessary to use 3D Cjyx’s built in saving facilities, such as the save button.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- qCjyx<MyCustomType>Writer.h
              |-- qCjyx<MyCustomType>Writer.cxx
```

Key points:

* Naming convention for class: `qCjyx<MyCustomType>Writer`
* Inherits from [`qCjyxNodeWriter`](https://apidocs.slicer.org/master/classqSlicerNodeWriter.html).
* See the [reader](#the-reader) for information on defining and constructing Qt style classes.
* Override [`QStringList extensions(vtkObject* object) const`](https://apidocs.slicer.org/master/classqSlicerNodeWriter.html#aa2d7322c22d3d5fa7b9ef2843948b31a) to provide file extensions that can be written to.
  * File extensions may be different, but don’t have to be, for different data nodes that in the same hierarchy (e.g. Markups Curve and Plane could reasonably require different file extensions, but they don’t).
* Override [`bool write(const qCjyxIO::IOProperties& properties)`](https://apidocs.slicer.org/master/classqSlicerNodeWriter.html#aa7e3af9bf485b46735131e6363223ad8) to do the actual writing (by way of a storage node, of course).

#### The subject hierarchy plugin

A convenient module in 3D Cjyx is the Data module. It brings all the different data types together under one roof and offers operations such as cloning, deleting, and renaming nodes that work regardless of the node type. The Data module uses the Subject Hierarchy, which is what we need to plug into so our new node type can be seen in and modified by the Data module.

Files:

```
|-- <Extension>
       |-- <Module>
              |-- SubjectHierarchyPlugins
                    |-- qCjyxSubjectHierarchy<MyCustomType>Plugin.h
                    |-- qCjyxSubjectHierarchy<MyCustomType>Plugin.cxx
```

Key Points:

* Naming convention for class: `qCjyxSubjectHierarchy<MyCustomType>Plugin`
* Inherits from [`qCjyxSubjectHierarchyAbstractPlugin`](https://apidocs.slicer.org/master/classqSlicerSubjectHierarchyAbstractPlugin.html).
* See the [reader](#the-reader) for information on defining and constructing Qt style classes.
* Override [`double canAddNodeToSubjectHierarchy(vtkDMMLNode* node, vtkIdType parentItemID=vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID) const`](https://apidocs.slicer.org/master/classqSlicerSubjectHierarchyAbstractPlugin.html#a91df5054dd11126a05e730922b5e9e43).
  * This method is used to determine if a data node can be placed in the hierarchy using this plugin.
* Override [`double canOwnSubjectHierarchyItem(vtkIdType itemID) const`](https://apidocs.slicer.org/master/classqSlicerSubjectHierarchyAbstractPlugin.html#afe009201b32cc0a115aff0022cc1dd9f) to say if this plugin can own a particular subject hierarchy item.
* Override [`const QString roleForPlugin() const`](https://apidocs.slicer.org/master/classqSlicerSubjectHierarchyAbstractPlugin.html#a55f0d686fe8e38576bc890de95622ad5) to give the plugin’s role (most often meaning the data type the plugin can handle, e.g. Markup).
* Override [`QIcon icon(vtkIdType itemID)`](https://apidocs.slicer.org/master/classqSlicerSubjectHierarchyAbstractPlugin.html#a7a1b0b5a55a2a13d7b88f3746ea573bc) and [`QIcon visibilityIcon(int visible)`](https://apidocs.slicer.org/master/classqSlicerSubjectHierarchyAbstractPlugin.html#a5bbd27154c71e174804bd833d50ce070) to set icons for your node type.
* Override [`QString tooltip(vtkIdType itemID) const`](https://apidocs.slicer.org/master/classqSlicerSubjectHierarchyAbstractPlugin.html#a7c748f1c4437fb4f63f88324b68157ef) to set a tool tip for your node type.
* Override the following to determine what happens when a user gets/sets the node color through the subject hierarchy:
  * [`void setDisplayColor(vtkIdType itemID, QColor color, QMap<int, QVariant> terminologyMetaData)`](https://apidocs.slicer.org/master/classqSlicerSubjectHierarchyAbstractPlugin.html#ad8b6d1065e78bfc73e6dcc71fce61c42)
  * [`QColor getDisplayColor(vtkIdType itemID, QMap<int, QVariant> &terminologyMetaData) const`](https://apidocs.slicer.org/master/classqSlicerSubjectHierarchyAbstractPlugin.html#a7be968d5ad3840512686b7c8501cf847)

#### The module (aka putting it all together)

If you have used 3D Cjyx for any length of time, you have probably noticed that for each type of node (or set of types as in something like markups) there is a dedicated module that is used solely for interacting with the single node type (or set of types). Examples would be the Models, Volumes, and Markups modules. These modules are useful from a user perspective and also necessary to get your new node registered everywhere it needs to be.

As these are normal 3D Cjyx modules, they come in three main parts, the module, the logic, and the module widget. The recommended way to create a new module is through the [Extension Wizard](https://www.slicer.org/wiki/Documentation/Nightly/Developers/ExtensionWizard).

Files:

```
|-- <Extension>
       |-- <Module>
              |-- qCjyx<MyCustomType>Module.h
              |-- qCjyx<MyCustomType>Module.cxx
              |-- qCjyx<MyCustomType>ModuleWidget.h
              |-- qCjyx<MyCustomType>ModuleWidget.cxx
              |-- Logic
                    |-- vtkCjyx<MyCustomType>Logic.h
                    |-- vtkCjyx<MyCustomType>Logic.cxx
```

In `qCjyx<MyCustomType>Module.cxx`:

* Override the [`void setup()`](https://apidocs.slicer.org/master/classqSlicerAbstractCoreModule.html#a9ad37e756338e7226f157b4eb54b9bcd) function:
  * Register your displayable manager with the [`vtkDMMLThreeDViewDisplayableManagerFactory`](https://apidocs.slicer.org/master/classvtkDMMLThreeDViewDisplayableManagerFactory.html) and/or the [`vtkDMMLSliceViewDisplayableManagerFactory`](https://apidocs.slicer.org/master/classvtkDMMLSliceViewDisplayableManagerFactory.html).
  * Register your subject hierarchy plugin with the [`qCjyxSubjectHierarchyPluginHandler`](https://apidocs.slicer.org/master/classqSlicerSubjectHierarchyPluginHandler.html).
  * Register your reader and writer with the [`qCjyxIOManager`](https://apidocs.slicer.org/master/classqSlicerIOManager.html).
* Override the [`QStringList associatedNodeTypes() const`](https://apidocs.slicer.org/master/classqSlicerAbstractCoreModule.html#a51d7c31d4901b3a313cb53b4891961f9) function and return all the new DMML classes created (data, display, and storage nodes).

In `vtkCjyx<MyCustomType>Logic.cxx`:

* Override the protected [`void RegisterNodes()`](https://apidocs.slicer.org/master/classvtkDMMLAbstractLogic.html#acfa7f65f53d5fbe6d056e7cf32a23058) function and register all the new DMML classes created (data, display, and storage nodes) with the DMML scene.

In `qCjyx<MyCustomType>ModuleWidget.cxx`:

* Override [`bool setEditedNode(vtkDMMLNode* node, QString role = QString(), QString context = QString())`](https://apidocs.slicer.org/master/classqSlicerAbstractModuleWidget.html#a52fe94bcc034b4d841d7fc05a4899d3c) and [`double nodeEditable(vtkDMMLNode* node)`](https://apidocs.slicer.org/master/classqSlicerAbstractModuleWidget.html#adebae2ce9686043d7f9dee09620b28bb) if you want this module to be connected to the Data module’s "Edit properties..." option in the right click menu.

### Slice view pipeline

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/slice_view_pipeline.png)

Another view of [VTK/DMML pipeline for the 2D slice views](https://www.slicer.org/wiki/File:SliceView.pptx).

Notes: the MapToWindowLevelColors has no lookup table set, so it maps the scalar volume data to 0,255 with no "color" operation.  This is controlled by the Window/Level settings of the volume display node.  The MapToColors applies the current lookup table to go from 0-255 to full RGBA.

Management of slice views is distributed between several objects:
- Slice node ([vtkDMMLSliceNode](https://apidocs.slicer.org/master/classvtkDMMLSliceNode.html)): Stores the position, orientation, and size of the slice. This is a [view node](https://apidocs.slicer.org/master/classvtkDMMLAbstractViewNode.html) and as such it stores common view properties, such as the view name, layout color, background color.
- Slice display node ([vtkDMMLSliceDisplayNode](https://apidocs.slicer.org/master/classvtkDMMLSliceDisplayNode.html)): Specifies how the slice should be displayed, such as visibility and style of display of intersecting slices. The class is based on [classvtkDMMLModelDisplayNode](https://apidocs.slicer.org/master/classvtkDMMLModelDisplayNode.html), therefore it also specifies which 3D views the slice is displayed in.
- Slice composite node ([vtkDMMLSliceCompositeNode](https://apidocs.slicer.org/master/classvtkDMMLSliceCompositeNode.html)): Specifies what volumes are displayed in the slice and how to blend them. It is ended up being a separate node probably because it is somewhat a mix between a data node (that tells what data to display) and a display node (that specifies how the data is displayed).
- Slice model node ([vtkDMMLModelNode](http://apidocs.slicer.org/master/classvtkDMMLModelNode.html)): It is a model node that displays the slice in 3D views. It stores a simple rectangle mesh on that the image cross-section is renderd as a texture.
- Slice model transform node ([vtkDMMLTransformNode](http://apidocs.slicer.org/master/classvtkDMMLTransformNode.html)). The transform node is used for positioning the slice model node in 3D views. It is faster to use a transform node to position the plane than modifying the plane points each time the slice is moved.
- Slice logic ([vtkDMMLSliceLogic](https://apidocs.slicer.org/master/classvtkDMMLSliceLogic.html)): This is not a DMML node but a logic class, which implements operationson DMML nodes. There is one logic for each slice view. The object keeps reference to all DMML nodes, so it is a good starting point for accessing any data related to slices and performing operations on slices. Slice logics are stored in the application logic object and can be retrieved like this: `sliceLogic = slicer.app.applicationLogic().GetSliceLogicByLayoutName('Red')`. There are a few other `GetSlicerLogic...` methods to get slice logic based on slice node, slice model display node, and to get all the slice logics.
- Slice layer logic([vtkDMMLSliceLayerLogic](http://apidocs.slicer.org/master/classvtkDMMLSliceLayerLogic.html)): Implements reslicing and interpolation for a volume. There is one slice layer logic for each volume layer (foreground, background, label) for each slice view.
- Slice link logic ([vtkDMMLSliceLinkLogic](https://apidocs.slicer.org/master/classvtkDMMLSliceLinkLogic.html)): There is only a singla instance of this object in the entire application. This object synchronizes slice view property changes between all slice views in the same view group.

### Layout

A layout manager ([qCjyxLayoutManager][qCjyxLayoutManager-apidoc]) shows or hides layouts:

- It instantiates, shows or hides relevant view widgets.
- It is associated with a [vtkDMMLLayoutNode][vtkDMMLLayoutNode-apidoc] describing the current layout configuration and ensuring it can be saved and restored.
- It owns an instance of [vtkDMMLLayoutLogic][vtkDMMLLayoutLogic-apidoc] that controls the layout node and the view nodes in a DMML scene.
- Pre-defined layouts are described using XML and are registered in `vtkDMMLLayoutLogic::AddDefaultLayouts()`.
- Developer may register additional layout.

[qCjyxLayoutManager-apidoc]: https://apidocs.slicer.org/master/classqSlicerLayoutManager.html
[vtkDMMLLayoutNode-apidoc]: https://apidocs.slicer.org/master/classvtkDMMLLayoutNode.html
[vtkDMMLLayoutLogic-apidoc]: https://apidocs.slicer.org/master/classvtkDMMLLayoutLogic.html

#### Registering a custom layout

See [example in the script repository](script_repository.md#customize-view-layout).

#### Layout XML Format

Layout description may be validated using the following DTD:

```
<!DOCTYPE layout SYSTEM "https://slicer.org/layout.dtd"
[
<!ELEMENT layout (item+)>
<!ELEMENT item (layout*, view)>
<!ELEMENT view (property*)>
<!ELEMENT property (#PCDATA)>

<!ATTLIST layout
type (horizontal|grid|tab|vertical) #IMPLIED "horizontal"
split (true|false) #IMPLIED "true" >

<!ATTLIST item
multiple (true|false) #IMPLIED "false"
splitSize CDATA #IMPLIED "0"
row CDATA #IMPLIED "0"
column CDATA #IMPLIED "0"
rowspan CDATA #IMPLIED "1"
colspan CDATA #IMPLIED "1"
>

<!ATTLIST view
class CDATA #REQUIRED
singletontag CDATA #IMPLIED
horizontalStretch CDATA #IMPLIED "-1"
verticalStretch CDATA #IMPLIED "-1" >

<!ATTLIST property
name CDATA #REQUIRED
action (default|relayout) #REQUIRED >

]>
```

Notes:

- `layout` element:
  - `split` attribute applies only to layout of type `horizontal` and `vertical`
- `item` element:
  - `row`, `column`, `rowspan` and `colspan` attributes applies only to layout of type `grid`
  - `splitSize` must be specified only for `layout` element with `split` attribute set to `true`
- `view` element:
  - `class` must correspond to a DMML view node class name (e.g `vtkDMMLViewNode`, `vtkDMMLSliceNode` or `vtkDMMLPlotViewNode`)
  - `singletontag` must always be specified when `multiple` attribute of `item` element is specified.
- `property` element:
  - `name` attribute may be set to the following values:
    - `viewlabel`
    - `viewcolor`
    - `viewgroup`
    - `orientation` applies only if parent `view` element is associated with `class` (or subclass) of type `vtkDMMLSliceNode`
