/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLSnapshotClipNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkDMMLSnapshotClipNode_h
#define __vtkDMMLSnapshotClipNode_h

// DMML includes
#include "vtkDMMLNode.h"
class vtkDMMLSceneViewNode;

// VTK includes
class vtkCollection;

// STD includes
#include <vector>

/// \brief Abstract class representing a hierarchy member.
class VTK_DMML_EXPORT vtkDMMLSnapshotClipNode : public vtkDMMLNode
{
  public:
  static vtkDMMLSnapshotClipNode *New();
  vtkTypeMacro(vtkDMMLSnapshotClipNode,vtkDMMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "SnapshotClip";}

   ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateScene(vtkDMMLScene *scene) override;

  ///
  /// Add SceneSnapshot node
  void AddSceneSnapshotNode(vtkDMMLSceneViewNode * node);

  ///
  /// Get Numbre of SceneSnapshot nodes
  int GetNumberOfSceneSnapshotNodes();
  ///
  /// Get SceneSnapshot node
  vtkDMMLSceneViewNode* GetSceneSnapshotNode(int index);

protected:
  vtkDMMLSnapshotClipNode();
  ~vtkDMMLSnapshotClipNode() override;
  vtkDMMLSnapshotClipNode(const vtkDMMLSnapshotClipNode&);
  void operator=(const vtkDMMLSnapshotClipNode&);

  std::vector< std::string > SceneSnapshotNodeIDs;
  vtkCollection* SceneSnapshotNodes;

};

#endif
