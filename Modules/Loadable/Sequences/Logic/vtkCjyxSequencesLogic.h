/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkCjyxSequencesLogic - cjyx logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkCjyxSequencesLogic_h
#define __vtkCjyxSequencesLogic_h

// Cjyx includes
#include "vtkCjyxModuleLogic.h"

// DMML includes

// STD includes
#include <cstdlib>

#include "vtkCjyxSequencesModuleLogicExport.h"

class vtkDMMLMessageCollection;
class vtkDMMLNode;
class vtkDMMLSequenceNode;
class vtkDMMLSequenceBrowserNode;

/// \ingroup Cjyx_QtModules_ExtensionTemplate
class VTK_CJYX_SEQUENCES_MODULE_LOGIC_EXPORT vtkCjyxSequencesLogic :
  public vtkCjyxModuleLogic
{
public:

  static vtkCjyxSequencesLogic *New();
  vtkTypeMacro(vtkCjyxSequencesLogic, vtkCjyxModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Add into the scene a new dmml sequence node and
  /// read its data from a specified file.
  /// A storage node is also added into the scene.
  /// User-displayable warning or error messages can be received if userMessages object is
  /// specified.
  vtkDMMLSequenceNode* AddSequence(const char* filename, vtkDMMLMessageCollection* userMessages=nullptr);

  /// Refreshes the output of all the active browser nodes. Called regularly by a timer.
  void UpdateAllProxyNodes();

  /// Updates the contents of all the proxy nodes (all the nodes copied from the master and synchronized sequences to the scene)
  void UpdateProxyNodesFromSequences(vtkDMMLSequenceBrowserNode* browserNode);

  /// Updates the sequence from a changed proxy node (if saving of state changes is allowed)
  void UpdateSequencesFromProxyNodes(vtkDMMLSequenceBrowserNode* browserNode, vtkDMMLNode* proxyNode);

  /// Deprecated method!
  void UpdateVirtualOutputNodes(vtkDMMLSequenceBrowserNode* browserNode)
    {
    static bool warningLogged = false;
    if (!warningLogged)
      {
      vtkWarningMacro("vtkCjyxSequenceBrowserLogic::UpdateVirtualOutputNodes is deprecated,"
        << " use vtkCjyxSequenceBrowserLogic::UpdateProxyNodes method instead");
      warningLogged = true;
      }
    this->UpdateProxyNodesFromSequences(browserNode);
    }

  /// Add a synchronized sequence node and virtual output node pair to the browser node for playback/recording
  /// \param sequenceNode Sequence node to add. If nullptr, then a new node is created.
  /// \param proxyNode Proxy node to use to represent selected item in the scene. May be nullptr.
  /// Returns the added/created sequence node, nullptr on error.
  vtkDMMLSequenceNode* AddSynchronizedNode(vtkDMMLNode* sequenceNode, vtkDMMLNode* proxyNode, vtkDMMLNode* browserNode);

  void GetCompatibleNodesFromScene(vtkCollection* compatibleNodes, vtkDMMLSequenceNode* sequenceNode);

  static bool IsNodeCompatibleForBrowsing(vtkDMMLSequenceNode* masterNode, vtkDMMLSequenceNode* testedNode);

  /// Get collection of browser nodes that use a specific sequence node.
  void GetBrowserNodesForSequenceNode(vtkDMMLSequenceNode* sequenceNode, vtkCollection* foundBrowserNodes);

  /// Get first browser node that use a specific sequence node. This is a convenience method for
  /// cases when it is known that a sequence is only used in one browser node. In general case,
  /// use GetBrowserNodesForSequenceNode instead.
  vtkDMMLSequenceBrowserNode* GetFirstBrowserNodeForSequenceNode(vtkDMMLSequenceNode* sequenceNode);

  /// Get collection of browser nodes that use a specific proxy node.
  void GetBrowserNodesForProxyNode(vtkDMMLNode* proxyNode, vtkCollection* foundBrowserNodes);

  /// Get first browser node that use a specific proxy node. This is a convenience method for
  /// cases when it is known that a proxy node is only used in one browser node. In general case,
  /// use GetBrowserNodesForProxyNode instead.
  vtkDMMLSequenceBrowserNode* GetFirstBrowserNodeForProxyNode(vtkDMMLNode* proxyNode);

protected:
  vtkCjyxSequencesLogic();
  ~vtkCjyxSequencesLogic() override;

  void SetDMMLSceneInternal(vtkDMMLScene* newScene) override;
  /// Register DMML Node classes to Scene. Gets called automatically when the DMMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromDMMLScene() override;
  void OnDMMLSceneNodeAdded(vtkDMMLNode* node) override;
  void OnDMMLSceneNodeRemoved(vtkDMMLNode* node) override;
  void ProcessDMMLNodesEvents(vtkObject *caller, unsigned long event, void *callData) override;

  bool IsDataConnectorNode(vtkDMMLNode*);

  // Time of the last update of each browser node (in universal time)
  std::map< vtkDMMLSequenceBrowserNode*, double > LastSequenceBrowserUpdateTimeSec;

private:

  bool UpdateProxyNodesFromSequencesInProgress{false};
  bool UpdateSequencesFromProxyNodesInProgress{false};

  vtkCjyxSequencesLogic(const vtkCjyxSequencesLogic&); // Not implemented
  void operator=(const vtkCjyxSequencesLogic&);               // Not implemented
};

#endif
