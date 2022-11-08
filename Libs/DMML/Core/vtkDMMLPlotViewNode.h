/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

#ifndef __vtkDMMLPlotViewNode_h
#define __vtkDMMLPlotViewNode_h

#include "vtkDMMLAbstractViewNode.h"

class vtkDMMLPlotChartNode;

/// \brief DMML node to represent Plot view parameters.
///
/// PlotViewNodes are associated one to one with a PlotWidget.
class VTK_DMML_EXPORT vtkDMMLPlotViewNode : public vtkDMMLAbstractViewNode
{
public:
  static vtkDMMLPlotViewNode *New();
  vtkTypeMacro(vtkDMMLPlotViewNode, vtkDMMLAbstractViewNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Enum of the available plot types
  enum {
    InteractionModePanView,
    InteractionModeSelectPoints,
    InteractionModeFreehandSelectPoints,
    InteractionModeMovePoints,
    InteractionMode_Last // must be last
  };

  //--------------------------------------------------------------------------
  /// DMMLNode methods
  //--------------------------------------------------------------------------

   vtkDMMLNode* CreateNodeInstance()  override;

  ///
  /// Read node attributes from XML file.
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentMacro(vtkDMMLPlotViewNode);

  ///
  /// Get node XML tag name (like Volume, Model).
  const char* GetNodeTagName() override { return "PlotView"; };

  ///
  /// Set and Update the PlotChart node id displayed in this PlotView.
  virtual void SetPlotChartNodeID(const char *PlotChartNodeID);

  ///
  /// Get the PlotChart node id displayed in this PlotView.
  const char* GetPlotChartNodeID();

  ///
  /// Get the PlotChart node displayed in this PlotView.
  vtkDMMLPlotChartNode* GetPlotChartNode();

  ///
  /// Configures the behavior of PropagatePlotChartSelection().
  /// If DoPropagatePlotChartSelection set to false then this
  /// view will not be affected by PropagatePlotChartSelection.
  /// Default value is true.
  vtkSetMacro (DoPropagatePlotChartSelection, bool );
  vtkGetMacro (DoPropagatePlotChartSelection, bool );

  /// Get/Set interaction mode for mouse left-click and drag.
  /// InteractionModePanView: pans chart view
  /// InteractionModeSelectPoints: select points with rectangular shape
  /// InteractionModeFreehandSelectPoints: select points with free-hand drawn shape
  /// InteractionModeMovePoints: move plot data points
  vtkGetMacro(InteractionMode, int);
  vtkSetMacro(InteractionMode, int);

  /// Enable moving of data points along X axis by click-and-drag in plot views.
  /// Points may only be moved in the view if moving is enabled in the plot chart node as well.
  vtkBooleanMacro(EnablePointMoveAlongX, bool);
  vtkGetMacro(EnablePointMoveAlongX, bool);
  vtkSetMacro(EnablePointMoveAlongX, bool);

  /// Enable moving of data points along Y axis by click-and-drag in plot views.
  /// Points may only be moved in the view if moving is enabled in the plot chart node as well.
  vtkBooleanMacro(EnablePointMoveAlongY, bool);
  vtkGetMacro(EnablePointMoveAlongY, bool);
  vtkSetMacro(EnablePointMoveAlongY, bool);

  ///
  /// Convert between interaction mode ID and name
  static const char *GetInteractionModeAsString(int id);
  static int GetInteractionModeFromString(const char *name);

  ///
  /// Method to propagate events generated in dmml.
  void ProcessDMMLEvents(vtkObject *caller,
                                 unsigned long event,
                                 void *callData) override;

  /// PlotModifiedEvent is fired when:
  ///  - a new PlotChart node is observed
  ///  - a PlotChart node is not longer observed
  ///  - an associated PlotChart node is modified
  /// Note that when SetAndObserve(Nth)NodeID() is called with an ID that
  /// has not yet any associated plot node in the scene, then
  /// plotModifiedEvent is not fired until found for the first time in
  /// the scene, e.g. UpdateScene()...
  enum
  {
    PlotChartNodeChangedEvent = 18000
  };

  virtual const char* GetPlotChartNodeReferenceRole();

protected:
  vtkDMMLPlotViewNode();
  ~vtkDMMLPlotViewNode() override;
  vtkDMMLPlotViewNode(const vtkDMMLPlotViewNode&);
  void operator=(const vtkDMMLPlotViewNode&);

  virtual const char* GetPlotChartNodeReferenceDMMLAttributeName();

  static const char* PlotChartNodeReferenceRole;
  static const char* PlotChartNodeReferenceDMMLAttributeName;

  ///
  /// Called when a node reference ID is added (list size increased).
  void OnNodeReferenceAdded(vtkDMMLNodeReference *reference) override;

  ///
  /// Called when a node reference ID is modified.
  void OnNodeReferenceModified(vtkDMMLNodeReference *reference) override;

  ///
  /// Called after a node reference ID is removed (list size decreased).
  void OnNodeReferenceRemoved(vtkDMMLNodeReference *reference) override;

  bool DoPropagatePlotChartSelection{true};

  int InteractionMode{InteractionModePanView};
  bool EnablePointMoveAlongX{true};
  bool EnablePointMoveAlongY{true};
};

#endif
