/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLSceneViewNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkDMMLSceneViewNode_h
#define __vtkDMMLSceneViewNode_h

#include "vtkDMMLStorableNode.h"

// VTK includes
#include <vtkStdString.h>
class vtkCollection;
class vtkImageData;

class vtkDMMLStorageNode;
class VTK_DMML_EXPORT vtkDMMLSceneViewNode : public vtkDMMLStorableNode
{
  public:
  static vtkDMMLSceneViewNode *New();
  vtkTypeMacro(vtkDMMLSceneViewNode,vtkDMMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Write this node's body to a DMML file in XML format.
  void WriteNodeBodyXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "SceneView";}

  ///
  /// Updates scene nodes
  void UpdateScene(vtkDMMLScene *scene) override;

  ///
  /// Updates internal nodes
  virtual void UpdateStoredScene();

  ///
  /// Set dependencies between this node and a child node
  /// when parsing XML file
  void ProcessChildNode(vtkDMMLNode *node) override;

  /// \sa StoreScene() RestoreScene()
  vtkDMMLScene* GetStoredScene();

  ///
  /// Store content of the scene
  /// \sa GetStoredScene() RestoreScene()
  void StoreScene();

  /// Add missing nodes from the Cjyx scene to the stored scene
  /// \sa RestoreScene()
  void AddMissingNodes();

  ///
  /// Restore content of the scene from the node.
  /// If removeNodes is true (default), remove nodes from the main Cjyx scene that
  /// do no appear in the scene view. If it is false, and nodes are found that will be
  /// deleted, don't remove them, then the method returns with false.
  /// This can be used for asking confirmation from the user to delete nodes
  /// (if the user decides that nodes can be removed then this method is called again
  /// with removeNodes=true).
  /// \sa GetStoredScene() StoreScene() AddMissingNodes()
  bool RestoreScene(bool removeNodes = true);

  void SetAbsentStorageFileNames();

  /// A description of this sceneView
  void SetSceneViewDescription(const vtkStdString& newDescription);
  vtkGetMacro(SceneViewDescription, vtkStdString);

  /// The attached screenshot of this sceneView
  virtual void SetScreenShot(vtkImageData* newScreenShot);
  vtkGetObjectMacro(ScreenShot, vtkImageData);

  /// The screenshot type of this sceneView
  /// 0: 3D View
  /// 1: Red Slice View
  /// 2: Yellow Slice View
  /// 3: Green Slice View
  /// 4: Full layout
  // TODO use an enum for the types
  virtual void SetScreenShotType(int type);
  vtkGetMacro(ScreenShotType, int);


  ///
  /// Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

  /// Get vector of nodes of a specified class in the scene.
  /// Returns 0 on failure, number of nodes on success.
  /// \sa vtkDMMLScene;:GetNodesByClass
  int GetNodesByClass(const char *className, std::vector<vtkDMMLNode *> &nodes);
  /// Get a collection of nodes of a specified class (for python access)
  /// You are responsible for deleting the returned collection.
  /// Returns nullptr on failure.
  /// \sa vtkDMMLScene::GetNodesByClass
  vtkCollection* GetNodesByClass(const char *className);

  /// check if a node should be included in the save/restore cycle. Returns
  /// false if it's a scene view node, scene view storage node, scene view
  /// hierarchy node, snapshot clip node, true otherwise
  bool IncludeNodeInSceneView(vtkDMMLNode *node);

  void SetSceneViewRootDir( const char* name);

protected:
  vtkDMMLSceneViewNode();
  ~vtkDMMLSceneViewNode() override;
  vtkDMMLSceneViewNode(const vtkDMMLSceneViewNode&);
  void operator=(const vtkDMMLSceneViewNode&);


  vtkDMMLScene* SnapshotScene;

  /// The associated Description
  vtkStdString SceneViewDescription;

  /// The vtkImageData of the screenshot
  vtkImageData* ScreenShot;

  /// The type of the screenshot
  int ScreenShotType;

};

#endif
