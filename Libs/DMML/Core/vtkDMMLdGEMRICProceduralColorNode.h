/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLdGEMRICProceduralColorNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.0 $

=========================================================================auto=*/

#ifndef __vtkDMMLdGEMRICProceduralColorNode_h
#define __vtkDMMLdGEMRICProceduralColorNode_h

#include "vtkDMMLProceduralColorNode.h"

/// \brief DMML node to represent procedurally defined color information.
///
/// Procedural nodes define methods that are used to map colors to scalar
/// values. Usually they will incorporate a custom subclass of a
/// vtkLookupTable, or a vtkColorTransferFunction.
class VTK_DMML_EXPORT vtkDMMLdGEMRICProceduralColorNode : public vtkDMMLProceduralColorNode
{
public:
  static vtkDMMLdGEMRICProceduralColorNode *New();
  vtkTypeMacro(vtkDMMLdGEMRICProceduralColorNode,vtkDMMLProceduralColorNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// DMMLNode methods
  //--------------------------------------------------------------------------

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "dGEMRICProceduralColor";}

  ///
  ///
  void UpdateScene(vtkDMMLScene *scene) override;

  ///
  /// Get/Set for Type. In SetType, set up the custom color options for this
  /// set of colors
  void SetType(int type) override;

  void ProcessDMMLEvents ( vtkObject *caller, unsigned long event, void *callData ) override;

  /// The list of valid procedural types
  //enum
  //{
  ///
  //};

  /// DisplayModifiedEvent is generated when display node parameters is changed
  enum
    {
      DisplayModifiedEvent = 20000
    };

  ///
  /// Create default storage node or nullptr if does not have one
  vtkDMMLStorageNode* CreateDefaultStorageNode() override
    {
    return Superclass::CreateDefaultStorageNode();
    }

  /// The list of valid types
  /// dGEMRIC-15T to display 1.5T dGEMRIC scans
  /// dGEMRIC-3T to display 3T dGEMRIC scans
  enum
  {
    dGEMRIC15T = 0,
    dGEMRIC3T = 1
  };

  /// Return the lowest and the highest integers, for use in looping
  int GetFirstType() override { return this->dGEMRIC15T; }
  int GetLastType() override { return this->dGEMRIC3T; }

  const char *GetTypeAsString() override;
  void SetTypeTo15T();
  void SetTypeTo3T();

protected:
  vtkDMMLdGEMRICProceduralColorNode();
  ~vtkDMMLdGEMRICProceduralColorNode() override;
  vtkDMMLdGEMRICProceduralColorNode(const vtkDMMLdGEMRICProceduralColorNode&);
  void operator=(const vtkDMMLdGEMRICProceduralColorNode&);
};

#endif
