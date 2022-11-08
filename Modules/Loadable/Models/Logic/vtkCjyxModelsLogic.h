/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

///  vtkCjyxModelsLogic - cjyx logic class for models manipulation
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the models

#ifndef __vtkCjyxModelsLogic_h
#define __vtkCjyxModelsLogic_h

// Cjyx includes
#include "vtkCjyxModuleLogic.h"
#include "vtkCjyxModelsModuleLogicExport.h"

// DMML includes
#include "vtkDMMLStorageNode.h"

// VTK includes
#include <vtkVersion.h>

class vtkDMMLMessageCollection;
class vtkDMMLModelNode;
class vtkDMMLStorageNode;
class vtkDMMLTransformNode;
class vtkAlgorithmOutput;
class vtkPolyData;

class VTK_CJYX_MODELS_MODULE_LOGIC_EXPORT vtkCjyxModelsLogic
  : public vtkCjyxModuleLogic
{
  public:

  /// The Usual vtk class functions
  static vtkCjyxModelsLogic *New();
  vtkTypeMacro(vtkCjyxModelsLogic, vtkCjyxModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Add into the scene a new dmml model node with an existing polydata
  /// A display node is also added into the scene.
  ///param polyData surface mesh in RAS coordinate system.
  vtkDMMLModelNode* AddModel(vtkPolyData* polyData = nullptr);

  /// Add into the scene a new dmml model node with an existing polydata
  /// A display node is also added into the scene.
  ///param polyData surface mesh algorithm output in RAS coordinate system.
  vtkDMMLModelNode* AddModel(vtkAlgorithmOutput* polyData = nullptr);

  /// Add into the scene a new dmml model node and
  /// read it's polydata from a specified file
  /// A display node and a storage node are also added into the scene
  /// \param coordinateSystem If coordinate system is not specified
  ///   in the file then this coordinate system is used. Default is LPS.
  /// \param userMessages User-displayable warning or error messages can be received if userMessages object is
  ///   specified.
  vtkDMMLModelNode* AddModel(const char* filename, int coordinateSystem = vtkDMMLStorageNode::CoordinateSystemLPS,
    vtkDMMLMessageCollection* userMessages = nullptr);

  /// Create model nodes and
  /// read their polydata from a specified directory
  /// \param coordinateSystem If coordinate system is not specified
  ///   in the file then this coordinate system is used. Default is LPS.
  /// \param userMessages User-displayable warning or error messages can be received if userMessages object is
  ///   specified.
  int AddModels(const char* dirname, const char* suffix, int coordinateSystem = vtkDMMLStorageNode::CoordinateSystemLPS,
    vtkDMMLMessageCollection* userMessages = nullptr);

  /// Write model's polydata  to a specified file
  /// \param coordinateSystem If coordinate system is not specified
  ///   in the file then this coordinate system is used. Default is -1, which means that
  ///   the coordinate system specified in the storage node will be used.
  /// \param userMessages User-displayable warning or error messages can be received if userMessages object is
  ///   specified.
  int SaveModel(const char* filename, vtkDMMLModelNode *modelNode, int coordinateSystem = vtkDMMLStorageNode::CoordinateSystemLPS,
    vtkDMMLMessageCollection* userMessages = nullptr);

  /// Transform models's polydata
  static void TransformModel(vtkDMMLTransformNode *tnode,
                              vtkDMMLModelNode *modelNode,
                              int transformNormals,
                              vtkDMMLModelNode *modelOut);

  /// Iterate through all models in the scene, find all their display nodes
  /// and set their visibility flag to flag. Does not touch model hierarchy
  /// nodes with display nodes
  void SetAllModelsVisibility(int flag);

protected:
  vtkCjyxModelsLogic();
  ~vtkCjyxModelsLogic() override;
  vtkCjyxModelsLogic(const vtkCjyxModelsLogic&);
  void operator=(const vtkCjyxModelsLogic&);

  /// Reimplemented to observe the NodeRemovedEvent scene event.
  void SetDMMLSceneInternal(vtkDMMLScene* newScene) override;

  /// Reimplemented to make sure the singleton vtkDMMLClipModelsNode is
  /// instantiated.
  void ObserveDMMLScene() override;

  void OnDMMLSceneEndImport() override;
};

#endif

