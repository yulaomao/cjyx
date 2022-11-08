/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkDMMLColorLogic_h
#define __vtkDMMLColorLogic_h

// DMMLLogic includes
#include "vtkDMMLAbstractLogic.h"
#include "vtkDMMLLogicExport.h"

// DMML includes
class vtkDMMLColorNode;
class vtkDMMLColorTableNode;
class vtkDMMLProceduralColorNode;
class vtkDMMLPETProceduralColorNode;
class vtkDMMLdGEMRICProceduralColorNode;
class vtkDMMLColorTableNode;

// STD includes
#include <cstdlib>
#include <vector>

/// \brief DMML logic class for color manipulation.
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the colors.
class VTK_DMML_LOGIC_EXPORT vtkDMMLColorLogic : public vtkDMMLAbstractLogic
{
public:

  /// The Usual vtk class functions
  static vtkDMMLColorLogic *New();
  vtkTypeMacro(vtkDMMLColorLogic,vtkDMMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// \brief Add default color nodes.
  ///
  /// The default color nodes are singleton and are not included in the
  /// the saved scene.
  ///
  /// This function enables the vtkDMMLScene::BatchProcessState.
  ///
  /// The type of default nodes along with their properties are listed
  /// in the table below:
  ///
  /// | Family                    | Category                 | Type                           | Node name                      | Singleton Tag                         | Node ID                                                     |
  /// | ------------------------- | ------------------------ | ------------------------------ | ------------------------------ | ------------------------------------- | ----------------------------------------------------------- |
  /// | ColorTable                | Discrete                 | Labels                         | Labels                         | Labels                                | vtkDMMLColorTableNodeLabels                                 |
  /// | ColorTable                | Discrete                 | FullRainbow                    | FullRainbow                    | FullRainbow                           | vtkDMMLColorTableNodeFullRainbow                            |
  /// | ColorTable                | Discrete                 | Grey                           | Grey                           | Grey                                  | vtkDMMLColorTableNodeGrey                                   |
  /// | ColorTable                | Discrete                 | Iron                           | Iron                           | Iron                                  | vtkDMMLColorTableNodeIron                                   |
  /// | ColorTable                | Discrete                 | Rainbow                        | Rainbow                        | Rainbow                               | vtkDMMLColorTableNodeRainbow                                |
  /// | ColorTable                | Discrete                 | Ocean                          | Ocean                          | Ocean                                 | vtkDMMLColorTableNodeOcean                                  |
  /// | ColorTable                | Discrete                 | Desert                         | Desert                         | Desert                                | vtkDMMLColorTableNodeDesert                                 |
  /// | ColorTable                | Discrete                 | InvertedGrey                   | InvertedGrey                   | InvertedGrey                          | vtkDMMLColorTableNodeInvertedGrey                           |
  /// | ColorTable                | Discrete                 | ReverseRainbow                 | ReverseRainbow                 | ReverseRainbow                        | vtkDMMLColorTableNodeReverseRainbow                         |
  /// | ColorTable                | Discrete                 | fMRI                           | fMRI                           | fMRI                                  | vtkDMMLColorTableNodefMRI                                   |
  /// | ColorTable                | Discrete                 | fMRIPA                         | fMRIPA                         | fMRIPA                                | vtkDMMLColorTableNodefMRIPA                                 |
  /// | ColorTable                | Discrete                 | Random                         | Random                         | Random                                | vtkDMMLColorTableNodeRandom                                 |
  /// | ColorTable                | Discrete                 | Red                            | Red                            | Red                                   | vtkDMMLColorTableNodeRed                                    |
  /// | ColorTable                | Discrete                 | Green                          | Green                          | Green                                 | vtkDMMLColorTableNodeGreen                                  |
  /// | ColorTable                | Discrete                 | Blue                           | Blue                           | Blue                                  | vtkDMMLColorTableNodeBlue                                   |
  /// | ColorTable                | Discrete                 | Yellow                         | Yellow                         | Yellow                                | vtkDMMLColorTableNodeYellow                                 |
  /// | ColorTable                | Discrete                 | Cyan                           | Cyan                           | Cyan                                  | vtkDMMLColorTableNodeCyan                                   |
  /// | ColorTable                | Discrete                 | Magenta                        | Magenta                        | Magenta                               | vtkDMMLColorTableNodeMagenta                                |
  /// | ColorTable                | Discrete                 | Warm1                          | Warm1                          | Warm1                                 | vtkDMMLColorTableNodeWarm1                                  |
  /// | ColorTable                | Discrete                 | Warm2                          | Warm2                          | Warm2                                 | vtkDMMLColorTableNodeWarm2                                  |
  /// | ColorTable                | Discrete                 | Warm3                          | Warm3                          | Warm3                                 | vtkDMMLColorTableNodeWarm3                                  |
  /// | ColorTable                | Discrete                 | Cool1                          | Cool1                          | Cool1                                 | vtkDMMLColorTableNodeCool1                                  |
  /// | ColorTable                | Discrete                 | Cool2                          | Cool2                          | Cool2                                 | vtkDMMLColorTableNodeCool2                                  |
  /// | ColorTable                | Discrete                 | Cool3                          | Cool3                          | Cool3                                 | vtkDMMLColorTableNodeCool3                                  |
  /// | ColorTable                | Shade                    | WarmShade1                     | WarmShade1                     | WarmShade1                            | vtkDMMLColorTableNodeWarmShade1                             |
  /// | ColorTable                | Shade                    | WarmShade2                     | WarmShade2                     | WarmShade2                            | vtkDMMLColorTableNodeWarmShade2                             |
  /// | ColorTable                | Shade                    | WarmShade3                     | WarmShade3                     | WarmShade3                            | vtkDMMLColorTableNodeWarmShade3                             |
  /// | ColorTable                | Shade                    | CoolShade1                     | CoolShade1                     | CoolShade1                            | vtkDMMLColorTableNodeCoolShade1                             |
  /// | ColorTable                | Shade                    | CoolShade2                     | CoolShade2                     | CoolShade2                            | vtkDMMLColorTableNodeCoolShade2                             |
  /// | ColorTable                | Shade                    | CoolShade3                     | CoolShade3                     | CoolShade3                            | vtkDMMLColorTableNodeCoolShade3                             |
  /// | ColorTable                | Tint                     | WarmTint1                      | WarmTint1                      | WarmTint1                             | vtkDMMLColorTableNodeWarmTint1                              |
  /// | ColorTable                | Tint                     | WarmTint2                      | WarmTint2                      | WarmTint2                             | vtkDMMLColorTableNodeWarmTint2                              |
  /// | ColorTable                | Tint                     | WarmTint3                      | WarmTint3                      | WarmTint3                             | vtkDMMLColorTableNodeWarmTint3                              |
  /// | ColorTable                | Tint                     | CoolTint1                      | CoolTint1                      | CoolTint1                             | vtkDMMLColorTableNodeCoolTint1                              |
  /// | ColorTable                | Tint                     | CoolTint2                      | CoolTint2                      | CoolTint2                             | vtkDMMLColorTableNodeCoolTint2                              |
  /// | ColorTable                | Tint                     | CoolTint3                      | CoolTint3                      | CoolTint3                             | vtkDMMLColorTableNodeCoolTint3                              |
  /// | ProceduralColor           | Discrete                 | RandomIntegers                 | RandomIntegers                 | RandomIntegers                        | vtkDMMLProceduralColorNodeRandomIntegers                    |
  /// | ProceduralColor           | Continuous               | RedGreenBlue                   | RedGreenBlue                   | RedGreenBlue                          | vtkDMMLProceduralColorNodeRedGreenBlue                      |
  /// | PETProceduralColor        | PET                      | PET-Heat                       | PET-Heat                       | PET-Heat                              | vtkDMMLPETProceduralColorNodePET-Heat                       |
  /// | PETProceduralColor        | PET                      | PET-Rainbow                    | PET-Rainbow                    | PET-Rainbow                           | vtkDMMLPETProceduralColorNodePET-Rainbow                    |
  /// | PETProceduralColor        | PET                      | PET-MaximumIntensityProjection | PET-MaximumIntensityProjection | PET-MaximumIntensityProjection        | vtkDMMLPETProceduralColorNodePET-MaximumIntensityProjection |
  /// | dGEMRICProceduralColor    | Cartilage MRI            | dGEMRIC-1.5T                   | dGEMRIC-1.5T                   | dGEMRIC-1.5T                          | vtkDMMLdGEMRICProceduralColorNodedGEMRIC-1.5T               |
  /// | dGEMRICProceduralColor    | Cartilage MRI            | dGEMRIC-3T                     | dGEMRIC-3T                     | dGEMRIC-3T                            | vtkDMMLdGEMRICProceduralColorNodedGEMRIC-3T                 |
  /// | ColorTable                | Default Labels from File | File                           | LightPaleChartColors           | FileLightPaleChartColors.txt          | vtkDMMLColorTableNodeFileLightPaleChartColors.txt           |
  /// | ColorTable                | Default Labels from File | File                           | ColdToHotRainbow               | FileColdToHotRainbow.txt              | vtkDMMLColorTableNodeFileColdToHotRainbow.txt               |
  /// | ColorTable                | Default Labels from File | File                           | Viridis                        | FileViridis.txt                       | vtkDMMLColorTableNodeFileViridis.txt                        |
  /// | ColorTable                | Default Labels from File | File                           | Magma                          | FileMagma.txt                         | vtkDMMLColorTableNodeFileMagma.txt                          |
  /// | ColorTable                | Default Labels from File | File                           | DivergingBlueRed               | FileDivergingBlueRed.txt              | vtkDMMLColorTableNodeFileDivergingBlueRed.txt               |
  /// | ColorTable                | Default Labels from File | File                           | HotToColdRainbow               | FileHotToColdRainbow.txt              | vtkDMMLColorTableNodeFileHotToColdRainbow.txt               |
  /// | ColorTable                | Default Labels from File | File                           | DarkBrightChartColors          | FileDarkBrightChartColors.txt         | vtkDMMLColorTableNodeFileDarkBrightChartColors.txt          |
  /// | ColorTable                | Default Labels from File | File                           | MediumChartColors              | FileMediumChartColors.txt             | vtkDMMLColorTableNodeFileMediumChartColors.txt              |
  /// | ColorTable                | Default Labels from File | File                           | Cjyx3_2010_Label_Colors      | FileCjyx3_2010_Label_Colors.txt     | vtkDMMLColorTableNodeFileCjyx3_2010_Label_Colors.txt      |
  /// | ColorTable                | Default Labels from File | File                           | Cjyx3_2010_Brain_Labels      | FileCjyx3_2010_Brain_Labels.txt     | vtkDMMLColorTableNodeFileCjyx3_2010_Brain_Labels.txt      |
  /// | ColorTable                | Default Labels from File | File                           | Inferno                        | FileInferno.txt                       | vtkDMMLColorTableNodeFileInferno.txt                        |
  /// | ColorTable                | Default Labels from File | File                           | PelvisColor                    | FilePelvisColor.txt                   | vtkDMMLColorTableNodeFilePelvisColor.txt                    |
  /// | ColorTable                | Default Labels from File | File                           | SPL-BrainAtlas-ColorFile       | FileSPL-BrainAtlas-ColorFile.txt      | vtkDMMLColorTableNodeFileSPL-BrainAtlas-ColorFile.txt       |
  /// | ColorTable                | Default Labels from File | File                           | AbdomenColors                  | FileAbdomenColors.txt                 | vtkDMMLColorTableNodeFileAbdomenColors.txt                  |
  /// | ColorTable                | None                     | File                           | GenericColors                  | FileGenericColors.txt                 | vtkDMMLColorTableNodeFileGenericColors.txt                  |
  /// | ColorTable                | Default Labels from File | File                           | 64Color-Nonsemantic            | File64Color-Nonsemantic.txt           | vtkDMMLColorTableNodeFile64Color-Nonsemantic.txt            |
  /// | ColorTable                | Default Labels from File | File                           | Plasma                         | FilePlasma.txt                        | vtkDMMLColorTableNodeFilePlasma.txt                         |
  /// | ColorTable                | Default Labels from File | File                           | Cividis                        | FileCividis.txt                       | vtkDMMLColorTableNodeFileCividis.txt                        |
  /// | ColorTable                | Default Labels from File | File                           | SPL-BrainAtlas-2009-ColorFile  | FileSPL-BrainAtlas-2009-ColorFile.txt | vtkDMMLColorTableNodeFileSPL-BrainAtlas-2009-ColorFile.txt  |
  /// | ColorTable                | Default Labels from File | File                           | SPL-BrainAtlas-2012-ColorFile  | FileSPL-BrainAtlas-2012-ColorFile.txt | vtkDMMLColorTableNodeFileSPL-BrainAtlas-2012-ColorFile.txt  |
  /// | ColorTable                | None                     | File                           | GenericAnatomyColors           | FileGenericAnatomyColors.txt          | vtkDMMLColorTableNodeFileGenericAnatomyColors.txt           |
  ///
  /// \note The table has been generated using Libs/DMML/Core/Documentation/generate_default_color_node_property_table.py
  ///
  /// \sa vtkDMMLNode::GetSingletonTag(), vtkDMMLScene::Commit()
  /// \sa RemoveDefaultColorNodes()
  ///
  /// \sa AddLabelsNode()
  /// \sa AddDefaultTableNodes()
  /// \sa AddDefaultProceduralNodes()
  /// \sa AddPETNodes()
  /// \sa AddDGEMRICNodes()
  /// \sa AddDefaultFileNodes()
  /// \sa AddUserFileNodes()
  virtual void AddDefaultColorNodes();

  /// \brief Remove default color nodes.
  ///
  /// \sa AddDefaultColorNodes()
  virtual void RemoveDefaultColorNodes();

  /// Return the default color table node id for a given type
  static const char * GetColorTableNodeID(int type);

  /// Return the default dGEMRIC color node id for a given type
  static const char * GetdGEMRICColorNodeID(int type);

  /// Return the default PET color node id for a given type
  static const char * GetPETColorNodeID(int type);

  /// \brief Return a default color node id for a procedural color node.
  ///
  /// \warning You are responsible to delete the returned string.
  static const char * GetProceduralColorNodeID(const char *name);

  /// \brief Return a default color node id for a file based node,
  /// based on the file name.
  ///
  /// \warning You are responsible to delete the returned string.
  static const char * GetFileColorNodeID(const char *fileName);
  static std::string  GetFileColorNodeSingletonTag(const char * fileName);

  /// Return a default color node id for a volume
  virtual const char * GetDefaultVolumeColorNodeID();

  /// Return a default color node id for a label map
  virtual const char * GetDefaultLabelMapColorNodeID();

  /// Return a default color node id for the editor
  virtual const char * GetDefaultEditorColorNodeID();

  /// Return a default color node id for a model
  virtual const char * GetDefaultModelColorNodeID();

  /// Return a default color node id for a chart
  virtual const char * GetDefaultChartColorNodeID();

  /// Return a default color node id for a plot
  virtual const char * GetDefaultPlotColorNodeID();

  /// Add a file to the input list Files, checking first for null, duplicates
  void AddColorFile(const char *fileName, std::vector<std::string> *Files);

  /// Load in a color file, creating a storage node. Returns a pointer to the
  /// created node on success, 0 on failure (no file, invalid color file). The
  /// name of the created color node is \a nodeName if specified or
  /// the fileName otherwise. Try first to load it as a color table
  /// node, then if that fails, as a procedural color node. It calls
  /// CreateFileNode or CreateProceduralFileNode which are also used
  /// for the built in color nodes, so it has to unset some flags: set
  /// the category to File, turn save with scene on on the node and
  /// it's storage node, turn off hide from editors, remove the
  /// singleton tag.
  /// \sa CreateFileNode, CreateProceduralFileNode
  vtkDMMLColorNode* LoadColorFile(const char *fileName, const char *nodeName = nullptr);

  /// Get/Set the user defined paths where to look for extra color files
  vtkGetStringMacro(UserColorFilePaths);
  vtkSetStringMacro(UserColorFilePaths);

  /// Returns a vtkDMMLColorTableNode copy (type = vtkDMMLColorTableNode::User)
  /// of the \a color node. The node is not added to the scene and you are
  /// responsible for deleting it.
  static vtkDMMLColorTableNode* CopyNode(vtkDMMLColorNode* colorNode, const char* copyName);

  /// Returns a vtkDMMLProceduralColorNode copy (type = vtkDMMLColorTableNode::User)
  /// of the \a color node. The node is not added to the scene and you are
  /// responsible for deleting it. If there is no color transfer function on the
  /// input node, for example if it's a color table node, it will return a
  /// procedural node with a blank color transfer function.
  static vtkDMMLProceduralColorNode* CopyProceduralNode(vtkDMMLColorNode* colorNode, const char* copyName);

protected:
  vtkDMMLColorLogic();
  ~vtkDMMLColorLogic() override;
  // disable copy constructor and operator
  vtkDMMLColorLogic(const vtkDMMLColorLogic&);
  void operator=(const vtkDMMLColorLogic&);

  /// Reimplemented to listen to specific scene events
  void SetDMMLSceneInternal(vtkDMMLScene* newScene) override;

  /// Called when the scene fires vtkDMMLScene::NewSceneEvent.
  /// We add the default LUTs.
  virtual void OnDMMLSceneNewEvent();

  vtkDMMLColorTableNode* CreateLabelsNode();
  vtkDMMLColorTableNode* CreateDefaultTableNode(int type);
  vtkDMMLProceduralColorNode* CreateRandomNode();
  vtkDMMLProceduralColorNode* CreateRedGreenBlueNode();
  vtkDMMLPETProceduralColorNode* CreatePETColorNode(int type);
  vtkDMMLdGEMRICProceduralColorNode* CreatedGEMRICColorNode(int type);
  vtkDMMLColorTableNode* CreateDefaultFileNode(const std::string& colorname);
  vtkDMMLColorTableNode* CreateUserFileNode(const std::string& colorname);
  vtkDMMLColorTableNode* CreateFileNode(const char* fileName);
  vtkDMMLProceduralColorNode* CreateProceduralFileNode(const char* fileName);

  void AddLabelsNode();
  void AddDefaultTableNode(int i);
  void AddDefaultProceduralNodes();
  void AddPETNode(int type);
  void AddDGEMRICNode(int type);
  void AddDefaultFileNode(int i);
  void AddUserFileNode(int i);

  void AddDefaultTableNodes();
  void AddPETNodes();
  void AddDGEMRICNodes();
  void AddDefaultFileNodes();
  void AddUserFileNodes();

  virtual std::vector<std::string> FindDefaultColorFiles();
  virtual std::vector<std::string> FindUserColorFiles();

  /// Return the ID of a node that doesn't belong to a scene.
  /// It is the concatenation of the node class name and its type.
  static const char * GetColorNodeID(vtkDMMLColorNode* colorNode);

  /// a vector holding discovered default color files, found in the
  /// Resources/ColorFiles directory, white space separated with:
  /// int name r g b a
  /// with rgba in the range 0-255
  std::vector<std::string> ColorFiles;

  /// a vector holding discovered user defined color files, found in the
  /// UserColorFilesPath directories.
  std::vector<std::string> UserColorFiles;
  /// a string holding delimiter separated (; on win32, : else) paths where to
  /// look for extra color files, set from the return value of
  /// vtkDMMLApplication::GetColorFilePaths
  char *UserColorFilePaths;

  static std::string TempColorNodeID;

  std::string RemoveLeadAndTrailSpaces(std::string);
};

#endif
