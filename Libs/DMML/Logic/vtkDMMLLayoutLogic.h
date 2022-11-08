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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkDMMLLayoutLogic_h
#define __vtkDMMLLayoutLogic_h

// DMMLLogic includes
#include "vtkDMMLAbstractLogic.h"
#include "vtkDMMLLogicExport.h"

// DMML includes
class vtkDMMLAbstractViewNode;
class vtkDMMLLayoutNode;

// VTK includes
class vtkCollection;
class vtkXMLDataElement;

// STD includes
#include <cstdlib>
#include <vector>

/// \brief DMML logic class for layout manipulation
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the colors
///
/// vtkDMMLLayoutLogic is a logic that controls the layout node and the view
/// nodes in a DMML scene.
/// It ensures that at least one 3D view and three slice views are always in
/// the DMML scene (after a scene is closed or imported).
/// The logic keeps an up-to-date list of the different DMML view nodes
/// (3D, slice ...) that are mapped into the layout.
/// A typical use case would be:
/// <code>
/// vtkDMMLScene* scene = vtkDMMLScene::New();
/// vtkDMMLLayoutLogic* layoutLogic = vtkDMMLLayoutLogic::New();
/// layoutLogic->SetDMMLScene(scene);
/// layoutLogic->GetLayoutNode()->SetViewArrangement(
///   vtkDMMLLayoutNode::CjyxLayoutConventionalView);
/// vtkCollection* views = layoutLogic->GetViewNodes();
/// vtkDMMLViewNode* viewNode = vtkDMMLViewNode::SafeDownCast(
///   views->GetItemAsObject(0));
/// vtkDMMLSliceNode* redNode = vtkDMMLSliceNode::SafeDownCast(
///   views->GetItemAsObject(1));
/// vtkDMMLSliceNode* yellowNode = vtkDMMLSliceNode::SafeDownCast(
///   views->GetItemAsObject(2));
/// vtkDMMLSliceNode* greenNode = vtkDMMLSliceNode::SafeDownCast(
///   views->GetItemAsObject(3));
/// </code>
/// When vtkDMMLScene::Clear() is called, vtkDMMLLayoutNode::Copy() is called
/// with an empty layout node, it sets the view arrangement to None.
/// So when the scene is created/closed/imported, the view arrangement is
/// restored to its previous valid layout.
class VTK_DMML_LOGIC_EXPORT vtkDMMLLayoutLogic : public vtkDMMLAbstractLogic
{
public:
  /// The Usual vtk class functions
  static vtkDMMLLayoutLogic *New();
  vtkTypeMacro(vtkDMMLLayoutLogic,vtkDMMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  typedef std::map<std::string, std::string> ViewAttributes;
  typedef ViewAttributes ViewProperty;
  typedef std::vector<ViewProperty> ViewProperties;

  /// Add all the layout descriptions of the known layouts
  /// TBD: could be done in vtkDMMLLayoutNode directly... not sure what's best.
  virtual void   AddDefaultLayouts();

  /// Returns the best node that fits the description given in the XML
  /// attributes in the element. The attributes should be precise enough to
  /// match a unique node in the scene. Returns 0 if no node can be found.
  vtkDMMLNode*   GetViewFromElement(vtkXMLDataElement* element);
  /// This returns the best view node that matches the attributes
  vtkDMMLNode*   GetViewFromAttributes(const ViewAttributes& attributes);

  /// This returns the best view node that matches the attributes
  vtkCollection* GetViewsFromAttributes(const ViewAttributes& attributes);

  vtkDMMLNode*   CreateViewFromAttributes(const ViewAttributes& attributes);

  void ApplyProperties(const ViewProperties& properties, vtkDMMLNode* view, const std::string& action);
  void ApplyProperty(const ViewProperty& property, vtkDMMLNode* view);

  /// Returns the up-to-date list of all the nodes that are mapped in the current
  /// layout.
  vtkGetObjectMacro(ViewNodes, vtkCollection);

  /// Returns the unique layout node of the scene. The logic scan the scene at
  /// first and if it can't find a layout node, it creates one.
  vtkGetObjectMacro(LayoutNode, vtkDMMLLayoutNode);

  /// Convenient function that creates and set a layout made of only 1 view.
  /// \sa CreateMaximizedViewLayoutDescription(),
  /// vtkDMMLLayoutNode::SetLayoutDescription(),
  /// vtkDMMLLayoutNode::CjyxLayoutCustomView, vtkDMMLLayoutNode::SetViewArrangement
  void MaximizeView(vtkDMMLAbstractViewNode* viewToMaximize);

  /// Create a layout description that maximizes a view.
  /// Note that the view node must be a singleton.
  /// \sa MaximizeView()
  void CreateMaximizedViewLayoutDescription(int layout,
                                            vtkDMMLAbstractViewNode* viewToMaximize);

  /// Returns layout description that shows the specified view maximized.
  std::string GetMaximizedViewLayoutDescription(vtkDMMLAbstractViewNode* viewToMaximize);

protected:
  /// Logic constructor
  vtkDMMLLayoutLogic();
  /// Logic destructor
  ~vtkDMMLLayoutLogic() override;
  // disable copy constructor and operator
  vtkDMMLLayoutLogic(const vtkDMMLLayoutLogic&);
  void operator=(const vtkDMMLLayoutLogic&);

  /// Reimplemented to listen to specific scene events
  void SetDMMLSceneInternal(vtkDMMLScene* newScene) override;

  void OnDMMLNodeModified(vtkDMMLNode* node) override;
  void OnDMMLSceneStartRestore() override;
  void OnDMMLSceneEndRestore() override;

  void UnobserveDMMLScene() override;
  void UpdateFromDMMLScene() override;

  /// Makes sure there is at least one 3D view node and three slice nodes (red,
  /// yellow and green)
  void UpdateViewNodes();

  /// Call SetLayoutNode with the unique layout node in the dmml scene
  /// Create a vtkDMMLLayoutNode if there is no layout node in the scene
  void UpdateLayoutNode();

  /// Not public as we internally take care of choosing/updating the layout node
  void SetLayoutNode(vtkDMMLLayoutNode* layoutNode);

  /// Update the logic when the layout node is set or modified
  void UpdateFromLayoutNode();
  /// Make sure the view node list mapped in the current layout is up-to-date.
  void UpdateViewCollectionsFromLayout();
  void CreateMissingViews();
  void CreateMissingViews(vtkXMLDataElement* layoutRootElement);

  /// As we pass the root element of the entire layout, it returns a list of
  /// all the nodes that are found in the layout.
  vtkCollection*     GetViewsFromLayout(vtkXMLDataElement* root);

  /// Define the compare view layouts available based on settings
  /// in the layout node
  void UpdateCompareViewLayoutDefinitions();

  /// Utility functions to browse XML data elements
  vtkXMLDataElement* GetNextViewElement(vtkXMLDataElement* viewElement);
  vtkXMLDataElement* GetNextElement(vtkXMLDataElement* element);
  ViewAttributes     GetViewElementAttributes(vtkXMLDataElement* viewElement)const;
  ViewProperties     GetViewElementProperties(vtkXMLDataElement* viewElement)const;
  ViewProperty       GetViewElementProperty(vtkXMLDataElement* viewProperty)const;

  /// Pointer on the unique Layout node of the dmml node.
  vtkDMMLLayoutNode* LayoutNode;
  int                LastValidViewArrangement;
  /// Up-to-date list of the nodes that are mapped into the scene
  vtkCollection*     ViewNodes;
  vtkXMLDataElement* ConventionalLayoutRootElement;
};

#endif

