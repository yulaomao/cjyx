/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkCjyxVolumesLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkCjyxVolumesLogic - cjyx logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing properties of volumes


#ifndef __vtkCjyxVolumesLogic_h
#define __vtkCjyxVolumesLogic_h

// Cjyx includes
#include "vtkCjyxModuleLogic.h"

// DMML includes
#include "vtkDMML.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLStorageNode.h"
#include "vtkDMMLVolumeDisplayNode.h"
#include "vtkDMMLVolumeNode.h"

// STD includes
#include <cstdlib>
#include <list>

#include "vtkCjyxVolumesModuleLogicExport.h"

class vtkDMMLLabelMapVolumeNode;
class vtkDMMLScalarVolumeNode;
class vtkDMMLScalarVolumeDisplayNode;
class vtkDMMLVolumeHeaderlessStorageNode;
class vtkStringArray;

struct ArchetypeVolumeNodeSet
{
  ArchetypeVolumeNodeSet(vtkDMMLScene * scene):Scene(scene), LabelMap(false){}
  ArchetypeVolumeNodeSet(const ArchetypeVolumeNodeSet& set) {
    Node = set.Node;
    DisplayNode = set.DisplayNode;
    StorageNode = set.StorageNode;
    Scene = set.Scene;
    LabelMap = set.LabelMap;
  }
  vtkSmartPointer<vtkDMMLVolumeNode> Node;
  vtkSmartPointer<vtkDMMLVolumeDisplayNode> DisplayNode;
  vtkSmartPointer<vtkDMMLStorageNode> StorageNode;
  vtkSmartPointer<vtkDMMLScene> Scene;
  bool LabelMap;  // is this node set for labelmaps?
};

class VTK_CJYX_VOLUMES_MODULE_LOGIC_EXPORT vtkCjyxVolumesLogic :
  public vtkCjyxModuleLogic
{
public:

  static vtkCjyxVolumesLogic *New();
  vtkTypeMacro(vtkCjyxVolumesLogic,vtkCjyxModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  typedef vtkCjyxVolumesLogic Self;

  struct VolumeDisplayPreset
    {
    std::string id;
    std::string name;
    std::string description;
    std::string icon;
    double window{0.0};
    double level{0.0};
    std::string colorNodeID;
    bool valid{false};
    };
  std::vector<VolumeDisplayPreset> VolumeDisplayPresets;

  /// Loading options, bitfield
  enum LoadingOptions {
    LabelMap = 1,
    CenterImage = 2,
    SingleFile = 4,
    AutoWindowLevel = 8,
    DiscardOrientation = 16
  };

  /// Factory function to create a volume node, display node, and
  /// storage node, configure the in the specified scene, and
  /// initialize the storage node with the "options".
  typedef ArchetypeVolumeNodeSet (*ArchetypeVolumeNodeSetFactory)(std::string& volumeName, vtkDMMLScene* scene, int options);

  /// Examine the file name to see if the extension is one of the supported
  /// freesurfer volume formats. Used to assign the proper color node to label maps.
  int IsFreeSurferVolume(const char* filename);

  /// Register a factory method that can create and configure a node
  /// set (ArchetypeVolumeNodeSet) containing a volume node, display
  /// node, and storage node. The nodes are configured within the
  /// factory method with default settings and are added to the scene
  /// and cross-referenced appropriately. Node types must be
  /// registered with the scene beforehand the factory is
  /// called. Factories are tested in the order they are registered.
  void RegisterArchetypeVolumeNodeSetFactory(ArchetypeVolumeNodeSetFactory factory);

  /// Register a factory method that can create and configure a node
  /// set (ArchetypeVolumeNodeSet) containing a volume node, display
  /// node, and storage node. The nodes are configured within the
  /// factory method with default settings and are added to the scene
  /// and cross-referenced appropriately. Node types must be
  /// registered with the scene beforehand the factory is called.
  /// This version inserts the factory at the head of the list, and
  /// hence the factory will be tested first, rather than pushing onto
  /// the back of the list of factories.
  void PreRegisterArchetypeVolumeNodeSetFactory(ArchetypeVolumeNodeSetFactory factory);

  /// Overloaded function of AddArchetypeVolume to provide more
  /// loading options, where variable loadingOptions is bit-coded as following:
  /// bit 0: label map
  /// bit 1: centered
  /// bit 2: loading single file
  /// bit 3: calculate window level automatically
  /// bit 4: discard image orientation
  /// higher bits are reserved for future use
  vtkDMMLVolumeNode* AddArchetypeVolume (const char* filename, const char* volname, int loadingOptions)
    {
    return (this->AddArchetypeVolume( filename, volname, loadingOptions, nullptr));
    }
  vtkDMMLVolumeNode* AddArchetypeVolume (const char* filename, const char* volname, int loadingOptions, vtkStringArray *fileList);
  vtkDMMLVolumeNode* AddArchetypeVolume (const char *filename, const char* volname)
    {
    return this->AddArchetypeVolume( filename, volname, 0, nullptr);
    }

  /// Load a scalar volume function directly, bypassing checks of all factories done in AddArchetypeVolume.
  /// \sa AddArchetypeVolume(const NodeSetFactoryRegistry& volumeRegistry, const char* filename, const char* volname, int loadingOptions, vtkStringArray *fileList)
  vtkDMMLScalarVolumeNode* AddArchetypeScalarVolume(const char* filename, const char* volname, int loadingOptions, vtkStringArray *fileList);

  /// Write volume's image data to a specified file
  int SaveArchetypeVolume (const char* filename, vtkDMMLVolumeNode *volumeNode);

  /// Create a label map volume to match the given \a volumeNode and add it to the current scene
  /// \sa GetDMMLScene()
  vtkDMMLLabelMapVolumeNode *CreateAndAddLabelVolume(vtkDMMLVolumeNode *volumeNode,
                                                   const char *name);

  /// Create a label map volume to match the given \a volumeNode and add it to the \a scene
  vtkDMMLLabelMapVolumeNode *CreateAndAddLabelVolume(vtkDMMLScene *scene,
                                                   vtkDMMLVolumeNode *volumeNode,
                                                   const char *name);
  /// \deprecated
  /// Create a label map volume to match the given \a volumeNode and add it to
  /// the current scene.
  /// \sa CreateAndAddLabelVolume
  vtkDMMLLabelMapVolumeNode *CreateLabelVolume(vtkDMMLVolumeNode *volumeNode,
                                             const char *name);
  /// \deprecated
  /// Create a label map volume to match the given \a volumeNode and add it to the \a scene
  /// \sa CreateAndAddLabelVolume
  vtkDMMLLabelMapVolumeNode *CreateLabelVolume(vtkDMMLScene *scene,
                                             vtkDMMLVolumeNode *volumeNode,
                                             const char *name);

  /// \deprecated
  /// Fill in a label map volume to match the given template volume node.
  /// \sa FillLabelVolumeFromTemplate(vtkDMMLScene*, vtkDMMLScalarVolumeNode*, vtkDMMLVolumeNode*)
  /// \sa GetDMMLScene()
  vtkDMMLLabelMapVolumeNode *FillLabelVolumeFromTemplate(vtkDMMLLabelMapVolumeNode *labelNode,
                                                       vtkDMMLVolumeNode *templateNode);

  /// \deprecated
  /// Fill in a label map volume to match the given template volume node, under
  /// the assumption that the given label map node is already added to the scene.
  /// A display node will be added to it if the label node doesn't already have
  /// one, and the image data associated with the label node will be allocated
  /// according to the template volumeNode.
  vtkDMMLLabelMapVolumeNode *FillLabelVolumeFromTemplate(vtkDMMLScene *scene,
                                                       vtkDMMLLabelMapVolumeNode *labelNode,
                                                       vtkDMMLVolumeNode *templateNode);

  /// Set a label map volume to match the given input volume node, under
  /// the assumption that the given label map node is already added to the scene.
  /// A display node will be added to it if the label node doesn't already have
  /// one, and the image data associated with the label node will be allocated
  /// according to the template volumeNode.
  vtkDMMLLabelMapVolumeNode *CreateLabelVolumeFromVolume(vtkDMMLScene *scene,
                                                       vtkDMMLLabelMapVolumeNode *outputVolume,
                                                       vtkDMMLVolumeNode *inputVolume);

  /// Set a scalar volume to match the given input volume node, under
  /// the assumption that the given label map node is already added to the scene.
  /// A display node will be added to it if the label node doesn't already have
  /// one, and the image data associated with the label node will be allocated
  /// according to the template volumeNode.
  vtkDMMLScalarVolumeNode *CreateScalarVolumeFromVolume(vtkDMMLScene *scene,
    vtkDMMLScalarVolumeNode *outputVolume,
    vtkDMMLVolumeNode *inputVolume);

  /// Clear the image data of a volume node to contain all zeros
  static void ClearVolumeImageData(vtkDMMLVolumeNode *volumeNode);

  /// Return a string listing any warnings about the spatial validity of
  /// the labelmap with respect to the volume.  An empty string indicates
  /// that the two volumes are identical samplings of the same spatial
  /// region and that the second volume input is a label map.
  /// \sa CompareVolumeGeometry
  std::string CheckForLabelVolumeValidity(vtkDMMLScalarVolumeNode *volumeNode,
                                          vtkDMMLLabelMapVolumeNode *labelNode);

  /// Generate a string listing any warnings about the spatial validity of
  /// the second volume with respect to the first volume.  An empty string
  /// indicates that the two volumes are identical samplings of the same
  /// spatial region.
  /// Checks include:
  ///  Valid image data.
  ///  Same dimensions.
  ///  Same spacing.
  ///  Same origin.
  ///  Same IJKtoRAS.
  /// \sa CheckForLabelVolumeValidity, ResampleVolumeToReferenceVolume
  std::string CompareVolumeGeometry(vtkDMMLScalarVolumeNode *volumeNode1,
                                    vtkDMMLScalarVolumeNode *volumeNode2);


  /// Create a deep copy of a \a volumeNode and add it to the current scene.
  /// If cloneImageData is false then the volume node is created without image data.
  /// \sa GetDMMLScene()
  vtkDMMLScalarVolumeNode *CloneVolume(vtkDMMLVolumeNode *volumeNode, const char *name);

  /// Create a empty copy of a \a volumeNode without imageData and add it to the current scene
  /// \sa GetDMMLScene()
  static vtkDMMLScalarVolumeNode *CloneVolumeWithoutImageData(vtkDMMLScene *scene,
                                                              vtkDMMLVolumeNode *volumeNode,
                                                              const char *name);

  /// Create a deep copy of a \a volumeNode and add it to the \a scene
  /// Only works for vtkDMMLScalarVolumeNode.
  /// The method is kept as is for background compatibility only, internally it calls CloneVolumeGeneric.
  /// \sa CloneVolumeGeneric
  static vtkDMMLScalarVolumeNode *CloneVolume(vtkDMMLScene *scene,
                                              vtkDMMLVolumeNode *volumeNode,
                                              const char *name,
                                              bool cloneImageData=true);
  /// Create a deep copy of a \a volumeNode and add it to the \a scene
  static vtkDMMLVolumeNode *CloneVolumeGeneric(vtkDMMLScene *scene,
    vtkDMMLVolumeNode *volumeNode,
    const char *name,
    bool cloneImageData = true);

  /// Computes matrix we need to register
  /// V1Node to V2Node given the "register.dat" matrix from tkregister2 (FreeSurfer)
  void TranslateFreeSurferRegistrationMatrixIntoCjyxRASToRASMatrix(vtkDMMLVolumeNode *V1Node,
                             vtkDMMLVolumeNode *V2Node,
                             vtkMatrix4x4 *FSRegistrationMatrix,
                             vtkMatrix4x4 *ResultsMatrix);

  /// Convenience method to compute a volume's Vox2RAS-tkreg Matrix
  void ComputeTkRegVox2RASMatrix ( vtkDMMLVolumeNode *VNode,
                                   vtkMatrix4x4 *M );

  /// Center the volume on the origin (0,0,0)
  /// \sa GetVolumeCenteredOrigin()
  void CenterVolume(vtkDMMLVolumeNode *volumeNode);

  /// Compute the origin of the volume in order for the volume to be centered.
  /// \sa CenterVolume()
  void GetVolumeCenteredOrigin(vtkDMMLVolumeNode *volumeNode, double* origin);

  ///  Convenience method to resample input volume using reference volume info
  /// \sa CompareVolumeGeometry
  static vtkDMMLScalarVolumeNode* ResampleVolumeToReferenceVolume(vtkDMMLVolumeNode *inputVolumeNode,
                                                           vtkDMMLVolumeNode *referenceVolumeNode);

  /// Getting the epsilon value to use when determining if the
  /// elements of the IJK to RAS matrices of two volumes match.
  /// Defaults to 10 to the minus 6.
  vtkGetMacro(CompareVolumeGeometryEpsilon, double);
  /// Setting the epsilon value and associated precision to use when determining
  /// if the elements of the IJK to RAS matrices of two volumes match and how to
  /// print out the mismatched elements.
  void SetCompareVolumeGeometryEpsilon(double epsilon);

  /// Get the precision with which to print out volume geometry mismatches,
  /// value is set when setting the compare volume geometry epsilon.
  /// \sa SetCompareVolumeGeometryEpsilon
  vtkGetMacro(CompareVolumeGeometryPrecision, int);

  /// Method to set volume window/level based on a volume display preset.
  /// Returns true on success.
  bool ApplyVolumeDisplayPreset(vtkDMMLVolumeDisplayNode* displayNode, std::string presetId);

  /// Get ID of the volume display preset that matches current display settings.
  /// Returns empty string if there is no match.
  std::string GetAppliedVolumeDisplayPresetId(vtkDMMLVolumeDisplayNode* displayNode);

  ///  Method to get a vector to currently defined window level preset IDs.
  std::vector<std::string> GetVolumeDisplayPresetIDs();

  /// Get volume display preset based on ID.
  /// If not found then the returned preset will have Valid member set to false.
  VolumeDisplayPreset GetVolumeDisplayPreset(const std::string& presetId);

protected:
  vtkCjyxVolumesLogic();
  ~vtkCjyxVolumesLogic() override;
  vtkCjyxVolumesLogic(const vtkCjyxVolumesLogic&);
  void operator=(const vtkCjyxVolumesLogic&);

  /// Read default volume display presets from configuration file
  void InitializeDefaultVolumeDisplayPresets();

  void ProcessDMMLNodesEvents(vtkObject * caller,
                                  unsigned long event,
                                  void * callData) override;


  void InitializeStorageNode(vtkDMMLStorageNode * storageNode,
                             const char * filename,
                             vtkStringArray *fileList,
                             vtkDMMLScene * dmmlScene = nullptr);

  void SetAndObserveColorToDisplayNode(vtkDMMLDisplayNode* displayNode,
                                       int labelmap, const char* filename);

  typedef std::list<ArchetypeVolumeNodeSetFactory> NodeSetFactoryRegistry;

  /// Convenience function allowing to try to load a volume using a given
  /// list of \a NodeSetFactoryRegistry
  vtkDMMLVolumeNode* AddArchetypeVolume(
      const NodeSetFactoryRegistry& volumeRegistry,
      const char* filename, const char* volname, int loadingOptions,
      vtkStringArray *fileList);

protected:

  NodeSetFactoryRegistry VolumeRegistry;

  /// Allowable difference in comparing volume geometry double values.
  /// Defaults to 1 to the power of 10 to the minus 6
  double CompareVolumeGeometryEpsilon;

  /// Error print out precision, paried with CompareVolumeGeometryEpsilon.
  /// defaults to 6
  int CompareVolumeGeometryPrecision;
};

#endif
