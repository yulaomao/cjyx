/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkCjyxVolumeRenderingLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

#ifndef __vtkCjyxVolumeRenderingLogic_h
#define __vtkCjyxVolumeRenderingLogic_h

// VolumeRendering includes
#include "vtkCjyxVolumeRenderingModuleLogicExport.h"
class vtkDMMLVolumeRenderingDisplayNode;

// Cjyx includes
#include "vtkCjyxModuleLogic.h"

// DMML includes
class vtkDMMLDisplayableNode;
class vtkDMMLLabelMapVolumeDisplayNode;
class vtkDMMLNode;
class vtkDMMLScalarVolumeDisplayNode;
class vtkDMMLScalarVolumeNode;
class vtkDMMLShaderPropertyNode;
class vtkDMMLViewNode;
class vtkDMMLVolumeDisplayNode;
class vtkDMMLVolumeNode;
class vtkDMMLVolumePropertyNode;

// VTK includes
class vtkColorTransferFunction;
class vtkLookupTable;
class vtkPiecewiseFunction;
class vtkScalarsToColors;
class vtkVolumeProperty;

// STD includes
#include <map>
#include <vector>

/// \ingroup Cjyx_QtModules_VolumeRendering
/// Collection of utility methods to control the Volume Rendering nodes.
/// The fastest to volume render of vtkDMMLVolumeNode is to use
/// \a CreateVolumeRenderingDisplayNode() and
/// \a UpdateDisplayNodeFromVolumeNode():
/// \code
/// vtkDMMLVolumeRenderingDisplayNode* displayNode =
///   logic->CreateVolumeRenderingDisplayNode();
/// logic->UpdateDisplayNodeFromVolumeNode(displayNode, volumeNode);
/// \endcode
class VTK_CJYX_VOLUMERENDERING_MODULE_LOGIC_EXPORT vtkCjyxVolumeRenderingLogic
  : public vtkCjyxModuleLogic
{
public:

  static vtkCjyxVolumeRenderingLogic *New();
  vtkTypeMacro(vtkCjyxVolumeRenderingLogic,vtkCjyxModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Inform the logic and observers that a rendering method (class deriving
  /// from vtkDMMLVolumeRenderingDisplayNode) is available.
  /// The event ModifiedEvent gets fired.
  /// \sa GetRenderingMethods
  void RegisterRenderingMethod(const char* methodName,
                               const char* displayNodeClassName);
  /// \sa RegisterRenderingMethod
  std::map<std::string, std::string> GetRenderingMethods();
  /// The default rendering method is set to display nodes created in
  /// CreateVolumeRenderingDisplayNode(). If no rendering method is given
  /// the VTKCPURayCast is instantiated.
  /// \sa CreateVolumeRenderingDisplayNode()
  vtkSetStringMacro(DefaultRenderingMethod);
  vtkGetStringMacro(DefaultRenderingMethod);

  /// Use a linear ramp (true) or a sharp ramp (false) when copying the volume
  /// display node threshold values into the volume rendering display node.
  /// True by default.
  /// \sa CopyScalarDisplayToVolumeRenderingDisplayNode()
  vtkSetMacro(UseLinearRamp, bool);
  vtkGetMacro(UseLinearRamp, bool);

  /// Create and set up all nodes needed for volume rendering for a given volume node:
  /// - Display node according to the selected rendering method
  /// - Volume property node for display options
  /// - ROI node for cropping
  /// When dealing with a volume node that may not have been shown in volume rendering before,
  /// it's enough to call this function to prepare them for that. After this, only SetVisibility
  /// needs to be called on its display node for showing it.
  /// Does not create new nodes if they exist already.
  /// \return Volume rendering display node for the given volume
  vtkDMMLVolumeRenderingDisplayNode* CreateDefaultVolumeRenderingNodes(vtkDMMLVolumeNode* volumeNode);

  /// Create a volume rendering display node.
  /// The node to instantiate will be of type \a renderingType if not null,
  /// \a DefaultRenderingMethod if not null or
  /// vtkDMMLCPURayCastVolumeRenderingDisplayNode in that order.
  /// Return the created node or 0 if there is no scene or the class name
  /// doesn't exist.
  /// If \a renderingClassName is 0, the returned node has a name generated
  /// using "VolumeRendering" as base name.
  /// You are responsible for deleting the node (read more about it in the
  /// section \ref GetNewCreate).
  /// \sa DefaultRenderingMethod
  vtkDMMLVolumeRenderingDisplayNode* CreateVolumeRenderingDisplayNode(const char* renderingClassName = nullptr);

  /// Observe the volume rendering display node to copy the volume display
  /// node if needed.
  /// This function is called automatically when a display node is added into
  /// the scene. You shouldn't have to call it.
  /// \sa vtkDMMLVolumeRenderingDisplayNode::FollowVolumeDisplayNode
  /// \sa RemoveVolumeRenderingDisplayNode
  void AddVolumeRenderingDisplayNode(vtkDMMLVolumeRenderingDisplayNode* node);

  /// Unobserve the volume rendering display node.
  /// \sa AddVolumeRenderingDisplayNode
  void RemoveVolumeRenderingDisplayNode(vtkDMMLVolumeRenderingDisplayNode* node);

  /// Observe all the volume rendering display nodes of the scene.
  /// \sa AddVolumeRenderingDisplayNode
  void AddAllVolumeRenderingDisplayNodes();

  /// Unobserve all the volume rendering display nodes of the scene.
  /// \sa AddVolumeRenderingDisplayNode, AddAllVolumeRenderingDisplayNodes
  void RemoveAllVolumeRenderingDisplayNodes();

  /// Re-create all volume rendering display nodes of the requested type.
  /// Common properties of the display nodes are propagated.
  void ChangeVolumeRenderingMethod(const char* displayNodeClassName = nullptr);

  /// Set volume rendering properties that seems well suited for the volume.
  /// The function uses heuristics to detect what kind of volume it is (CT, MRI, other),
  /// based on its intensity range and choses preset accordingly.
  /// Returns false is volume type could not be detected and so properties are not changed.
  bool SetRecommendedVolumeRenderingProperties(vtkDMMLVolumeRenderingDisplayNode* vrDisplayNode);

  /// Applies the properties (window level, threshold and color function) of
  /// a volume display node to the volume rendering display node.
  /// If displayNode is 0, it uses the first display node.
  /// It's a utility method that internally calls
  /// CopyScalarDisplayToVolumeRenderingDisplayNode() or
  /// CopyLabelMapDisplayToVolumeRenderingDisplayNode() based on the type of
  /// displayNode.
  /// \sa CopyScalarDisplayToVolumeRenderingDisplayNode()
  /// \sa CopyLabelMapDisplayToVolumeRenderingDisplayNode()
  void CopyDisplayToVolumeRenderingDisplayNode(
    vtkDMMLVolumeRenderingDisplayNode* node,
    vtkDMMLVolumeDisplayNode* displayNode = nullptr);

  /// Applies the properties (window level, threshold and color function) of
  /// the scalar display node to the volume rendering displaynode.
  /// If scalarDisplayNode is 0, it uses the first display node.
  /// \sa CopyDisplayToVolumeRenderingDisplayNode()
  /// \sa CopyLabelMapDisplayToVolumeRenderingDisplayNode()
  void CopyScalarDisplayToVolumeRenderingDisplayNode(
    vtkDMMLVolumeRenderingDisplayNode* volumeRenderingDisplayNode,
    vtkDMMLScalarVolumeDisplayNode* scalarDisplayNode = nullptr);

  /// Applies the properties (threshold ) of
  /// the labelmap display node to the volume rendering displaynode.
  /// If labelMapDisplayNode is 0, it uses the first displaynode.
  /// \sa CopyDisplayToVolumeRenderingDisplayNode()
  /// \sa CopyScalarDisplayToVolumeRenderingDisplayNode()
  void CopyLabelMapDisplayToVolumeRenderingDisplayNode(
    vtkDMMLVolumeRenderingDisplayNode* volumeRenderingDisplayNode,
    vtkDMMLLabelMapVolumeDisplayNode* labelMapDisplayNode = nullptr);

  /// Applies a threshold to a volume property
  /// \arg \b scalarRange is the entire range of the transfer function
  /// \arg \b threshold is the range where the threshold is applied
  /// \arg \b node is the container of the transfer function to save
  /// \arg \b linearRamp controls the shape of the threshold:
  /// \verbatim
  ///  true:            false:    _
  ///        __/|__            __| |__
  /// \endverbatim
  /// \arg \b stayUpAtUpperLimit controls whether the threshold not maxed out:
  /// \verbatim
  ///  true:    ______  false:
  ///        __/               __/|_____
  /// \endverbatim
  /// \sa SetWindowLevelToVolumeProp()
  void SetThresholdToVolumeProp(
    double scalarRange[2], double threshold[2],
    vtkVolumeProperty* node,
    bool linearRamp = false, bool stayUpAtUpperLimit = false);

  /// Create a color transfer function that ranges from \a scalarRange[0] to
  /// \a scalarRange[1] and containing a \a windowLevel[0] wide ramp centered
  /// on \a level:
  /// \verbatim
  ///                         max = level + window/2      scalarRange[1]
  ///                         .___________________________.
  ///  .____________________./
  ///  scalarRange[0]       min = level - window/2
  /// \endverbatim
  /// The generated transfer function contains at least 4 points located in:
  ///  * scalarRange[0]
  ///  * min = level - window/2
  ///  * max = level + window/2
  ///  * scalarRange[1]
  ///
  /// If \a lut is 0, the colors go from black (0, 0, 0) to white (1, 1, 1)
  /// If \a lut contains only 1 value, that color is used for all the
  /// generated points.
  /// If \a lut contains more than 1 value, each color is used in the ramp.
  /// The generated transfer function will be made of lut->size() + 2 points.
  /// The function is then applied to the volume property \a node.
  /// \sa SetThresholdToVolumeProp
  void SetWindowLevelToVolumeProp(
    double scalarRange[2], double windowLevel[2],
    vtkLookupTable* lut, vtkVolumeProperty* node);

  /// Create an opacity transfer function for gradient opacity.
  /// It ranges from 0 to scalarRange[1] - scalarRange[0].
  /// \sa SetThresholdToVolumeProp, SetWindowLevelToVolumeProp
  void SetGradientOpacityToVolumeProp(
    double scalarRange[2], vtkVolumeProperty* node);

  /// Generate and set to the volume property \a node an opacity and color
  /// transfer function from the labelmap LUT \a colors.
  /// \sa SetWindowLevelToVolumeProp, SetThresholdToVolumeProp
  void SetLabelMapToVolumeProp(
    vtkScalarsToColors* lut, vtkVolumeProperty* node);

  /// Update DisplayNode from VolumeNode,
  /// Can pass a VolumePropertyNode and an ROI node to be the display node.
  /// If they are nullptr and the display node does not already have any, new ones
  /// will be created then set and observed to the display node.
  void UpdateDisplayNodeFromVolumeNode(vtkDMMLVolumeRenderingDisplayNode *displayNode,
                                       vtkDMMLVolumeNode *volumeNode,
                                       vtkDMMLVolumePropertyNode *propNode = nullptr,
                                       vtkDMMLNode *roiNode = nullptr,
                                       bool createROI=true);

  ///  Create cropping ROI node, if does not exist yet
  vtkDMMLDisplayableNode* CreateROINode(vtkDMMLVolumeRenderingDisplayNode* displayNode);

  /// Remove ViewNode from VolumeRenderingDisplayNode for a VolumeNode,
  void RemoveViewFromVolumeDisplayNodes(vtkDMMLVolumeNode *volumeNode,
                                        vtkDMMLViewNode *viewNode);

  /// Find volume rendering display node reference in the volume
  vtkDMMLVolumeRenderingDisplayNode* GetVolumeRenderingDisplayNodeByID(vtkDMMLVolumeNode *volumeNode,
                                                                    char *displayNodeID);

  /// Find volume rendering display node referencing the view node and volume node
  vtkDMMLVolumeRenderingDisplayNode* GetVolumeRenderingDisplayNodeForViewNode(
                                                        vtkDMMLVolumeNode *volumeNode,
                                                        vtkDMMLViewNode *viewNode);

  /// Find volume rendering display node referencing the view node in the scene
  vtkDMMLVolumeRenderingDisplayNode* GetVolumeRenderingDisplayNodeForViewNode(
                                                        vtkDMMLViewNode *viewNode);

  /// Find first volume rendering display node
  vtkDMMLVolumeRenderingDisplayNode* GetFirstVolumeRenderingDisplayNode(vtkDMMLVolumeNode *volumeNode);

  /// Find the first volume rendering display node that uses the ROI
  vtkDMMLVolumeRenderingDisplayNode* GetFirstVolumeRenderingDisplayNodeByROINode(vtkDMMLNode* roiNode);

  void UpdateTranferFunctionRangeFromImage(vtkDMMLVolumeRenderingDisplayNode* vspNode);

  /// Utility function that modifies the ROI node of the display node
  /// to fit the boundaries of the volume node
  /// \sa vtkDMMLVolumeRenderingDisplayNode::GetROINode
  void FitROIToVolume(vtkDMMLVolumeRenderingDisplayNode* vspNode);

  /// Load from file and add into the scene a transfer function.
  /// \sa vtkDMMLVolumePropertyStorageNode
  vtkDMMLVolumePropertyNode* AddVolumePropertyFromFile (const char* filename);

  /// Load from file and add into the scene a shader property.
  /// \sa vtkDMMLShaderPropertyStorageNode
  vtkDMMLShaderPropertyNode* AddShaderPropertyFromFile (const char* filename);

  /// Return the scene containing the volume rendering presets.
  /// If there is no presets scene, a scene is created and presets are loaded
  /// into.
  /// The presets scene is loaded from a file (presets.xml) located in the
  /// module share directory
  /// \sa vtkDMMLVolumePropertyNode, GetModuleShareDirectory()
  vtkDMMLScene* GetPresetsScene();

  /// Add a preset to the preset scene.
  /// If the optional icon image is specified then that will be used to
  /// in preset selector widgets. The icon is stored as a volume node
  /// in the preset scene.
  /// \param appendToEnd controls if the preset is added before or after existing presets.
  /// \sa GetPresetsScene(), GetIconVolumeReferenceRole()
  void AddPreset(vtkDMMLVolumePropertyNode* preset, vtkImageData* icon = nullptr, bool appendToEnd=false);

  /// Removes a preset and its associated icon (if specified) from the preset scene.
  /// \sa GetPresetsScene(), GetIconVolumeReferenceRole()
  void RemovePreset(vtkDMMLVolumePropertyNode* preset);

  /// Use custom presets scene
  /// \return Nonzero if successfully loaded
  int LoadCustomPresetsScene(const char* sceneFilePath);

  /// This node reference role name allows linking from a preset node to a volume
  /// node that contains an icon for the preset node.
  /// For example, the icon is used for representing the node in qCjyxPresetComboBox.
  static const char* GetIconVolumeReferenceRole() { return "IconVolume"; };

  /// Return the preset \a presetName contained in the presets scene
  /// loaded using \a GetPresetsScene().
  /// If no presets are found, return 0.
  /// If multiple presets are found, the first one is returned.
  /// \sa GetPresetsScene(), vtkDMMLVolumePropertyNode
  vtkDMMLVolumePropertyNode* GetPresetByName(const char *presetName);

  /// Utility function that checks if the piecewise functions are equal
  /// Returns true if different
  bool IsDifferentFunction(vtkPiecewiseFunction* function1,
                           vtkPiecewiseFunction* function2) const;

  /// Utility function that checks if the color transfer functions are equal
  /// Returns true if different
  bool IsDifferentFunction(vtkColorTransferFunction* function1,
                           vtkColorTransferFunction* function2) const;

  vtkSetMacro(DefaultROIClassName, std::string);
  vtkGetMacro(DefaultROIClassName, std::string);

protected:
  vtkCjyxVolumeRenderingLogic();
  ~vtkCjyxVolumeRenderingLogic() override;

  void SetDMMLSceneInternal(vtkDMMLScene* scene) override;
  // Register local DMML nodes
  void RegisterNodes() override;

  /// Reimplemented to initialize display nodes in the scene.
  void ObserveDMMLScene() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
  void OnDMMLNodeModified(vtkDMMLNode* node) override;

  // Update from
  void UpdateVolumeRenderingDisplayNode(vtkDMMLVolumeRenderingDisplayNode* node);

  std::map<std::string, std::string> RenderingMethods;
  /// This property holds the default rendering method to instantiate in
  /// \a CreateVolumeRenderingDisplayNode().
  /// If no rendering method is given, the VTKCPURayCast technique is
  /// instantiated.
  /// \sa SetDefaultRenderingMethod(), GetDefaultRenderingMethod(),
  /// CreateVolumeRenderingDisplayNode()
  char* DefaultRenderingMethod;

  bool UseLinearRamp;

  typedef std::vector<vtkDMMLNode*> DisplayNodesType;
  DisplayNodesType DisplayNodes;

  bool LoadPresets(vtkDMMLScene* scene);
  vtkDMMLScene* PresetsScene;

  std::string DefaultROIClassName;
private:
  vtkCjyxVolumeRenderingLogic(const vtkCjyxVolumeRenderingLogic&) = delete;
  void operator=(const vtkCjyxVolumeRenderingLogic&) = delete;
};

#endif
