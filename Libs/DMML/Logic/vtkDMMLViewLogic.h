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

#ifndef __vtkDMMLViewLogic_h
#define __vtkDMMLViewLogic_h

// DMMLLogic includes
#include "vtkDMMLAbstractLogic.h"

// STD includes
#include <vector>
#include <deque>

class vtkDMMLDisplayNode;
class vtkDMMLLinearTransformNode;
class vtkDMMLModelDisplayNode;
class vtkDMMLModelNode;
class vtkDMMLViewNode;
class vtkDMMLCameraNode;
class vtkDMMLVolumeNode;

class vtkAlgorithmOutput;
class vtkCollection;
class vtkImageBlend;
class vtkTransform;
class vtkImageData;
class vtkImageReslice;
class vtkTransform;

struct SliceLayerInfo;
struct BlendPipeline;

/// \brief Cjyx logic class for view manipulation.
///
/// This class manages the logic associated with display of view windows
/// (but not the GUI).  Features of the class include:
///  -- manage the linking of the 3D View linking.
///
class VTK_DMML_LOGIC_EXPORT vtkDMMLViewLogic : public vtkDMMLAbstractLogic
{
public:
  /// The Usual VTK class functions
  static vtkDMMLViewLogic *New();
  vtkTypeMacro(vtkDMMLViewLogic,vtkDMMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set/Get layout name. This is used for finding the camera and view node in the scene.
  virtual void SetName(const char* name);
  virtual const char* GetName() const;

  /// The DMML View node for this View logic
  vtkGetObjectMacro (ViewNode, vtkDMMLViewNode);

  /// The DMML camera node for this View logic
  vtkGetObjectMacro (CameraNode, vtkDMMLCameraNode);

  /// Indicate an interaction with the camera node is beginning. The
  /// parameters of the camera node being manipulated are passed as a
  /// bitmask. See vtkDMMLViewNode::InteractionFlagType.
  void StartCameraNodeInteraction(unsigned int parameters);

  /// Indicate an interaction with the slice node has been completed
  void EndCameraNodeInteraction();

  /// Indicate an interaction with the view node is
  /// beginning. The parameters of the view node being manipulated
  /// are passed as a bitmask. See vtkDMMLViewNode::InteractionFlagType.
  void StartViewNodeInteraction(unsigned int parameters);

  /// Indicate an interaction with the view node has been completed
  void EndViewNodeInteraction();

  /// Convenience function for adding a view node and setting it in this logic
  vtkDMMLViewNode* AddViewNode(const char* layoutName);

  /// Convenient method to get the view node from scene and name of the Logic.
  /// The name of the Logic is the same of the widget one to which it is associated
  static vtkDMMLViewNode* GetViewNode(vtkDMMLScene* scene,
                                      const char* layoutName);

  /// Convenient method to get the camera node from scene and name of the Logic.
  /// The name of the Logic is the same of the widget one to which it is associated
  static vtkDMMLCameraNode* GetCameraNode(vtkDMMLScene* scene,
                                          const char* layoutName);

protected:
  vtkDMMLViewLogic();
  ~vtkDMMLViewLogic() override;

  void SetDMMLSceneInternal(vtkDMMLScene* newScene) override;
  void SetViewNode(vtkDMMLViewNode* newViewNode);
  void SetCameraNode(vtkDMMLCameraNode* newCameraNode);

  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
  void UpdateFromDMMLScene() override;

  void UpdateDMMLNodes();

  // View and camera nodes are looked up from the scene based on the layout name.
  std::string Name;

  vtkDMMLViewNode* ViewNode;
  vtkDMMLCameraNode* CameraNode;
  bool UpdatingDMMLNodes;

private:
  vtkDMMLViewLogic(const vtkDMMLViewLogic&) = delete;
  void operator=(const vtkDMMLViewLogic&) = delete;

};

#endif
