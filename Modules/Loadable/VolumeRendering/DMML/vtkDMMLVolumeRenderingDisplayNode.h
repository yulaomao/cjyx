/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLVolumeRenderingDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkDMMLVolumeRenderingDisplayNode_h
#define __vtkDMMLVolumeRenderingDisplayNode_h

// Volume Rendering includes
#include "vtkCjyxVolumeRenderingModuleDMMLExport.h"

// DMML includes
#include "vtkDMMLDisplayNode.h"
class vtkIntArray;
class vtkDMMLAnnotationROINode;
class vtkDMMLMarkupsROINode;
class vtkDMMLShaderPropertyNode;
class vtkDMMLViewNode;
class vtkDMMLVolumeNode;
class vtkDMMLVolumePropertyNode;

/// \ingroup Cjyx_QtModules_VolumeRendering
/// \name vtkDMMLVolumeRenderingDisplayNode
/// \brief Abstract DMML node for storing information for Volume Rendering
class VTK_CJYX_VOLUMERENDERING_MODULE_DMML_EXPORT vtkDMMLVolumeRenderingDisplayNode
  : public vtkDMMLDisplayNode
{
public:
  vtkTypeMacro(vtkDMMLVolumeRenderingDisplayNode,vtkDMMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  const char* GetVolumeNodeID();
  vtkDMMLVolumeNode* GetVolumeNode();

  const char* GetVolumePropertyNodeID();
  void SetAndObserveVolumePropertyNodeID(const char *volumePropertyNodeID);
  vtkDMMLVolumePropertyNode* GetVolumePropertyNode();

  const char* GetShaderPropertyNodeID();
  void SetAndObserveShaderPropertyNodeID(const char *shaderPropertyNodeID);
  vtkDMMLShaderPropertyNode* GetShaderPropertyNode();
  vtkDMMLShaderPropertyNode* GetOrCreateShaderPropertyNode( vtkDMMLScene * dmmlScene );

  const char* GetROINodeID();
  void SetAndObserveROINodeID(const char *roiNodeID);
  vtkDMMLDisplayableNode* GetROINode();
  vtkDMMLAnnotationROINode* GetAnnotationROINode();
  vtkDMMLMarkupsROINode* GetMarkupsROINode();

  vtkDMMLViewNode* GetFirstViewNode();

  double GetSampleDistance();

  vtkSetMacro(CroppingEnabled,int);
  vtkGetMacro(CroppingEnabled,int);
  vtkBooleanMacro(CroppingEnabled,int);

  vtkSetVector2Macro(Threshold, double);
  vtkGetVectorMacro(Threshold, double, 2);

  vtkGetMacro(FollowVolumeDisplayNode, int);
  vtkSetMacro(FollowVolumeDisplayNode, int);

  vtkGetMacro(IgnoreVolumeDisplayNodeThreshold, int);
  vtkSetMacro(IgnoreVolumeDisplayNodeThreshold, int);

  vtkGetMacro(UseSingleVolumeProperty, int);
  vtkSetMacro(UseSingleVolumeProperty, int);

  vtkSetVector2Macro(WindowLevel, double);
  vtkGetVectorMacro(WindowLevel, double, 2);

protected:
  vtkDMMLVolumeRenderingDisplayNode();
  ~vtkDMMLVolumeRenderingDisplayNode() override;
  vtkDMMLVolumeRenderingDisplayNode(const vtkDMMLVolumeRenderingDisplayNode&);
  void operator=(const vtkDMMLVolumeRenderingDisplayNode&);

  void ProcessDMMLEvents(vtkObject *caller, unsigned long event, void *callData) override;

  static const char* VolumePropertyNodeReferenceRole;
  static const char* VolumePropertyNodeReferenceDMMLAttributeName;
  static const char* ROINodeReferenceRole;
  static const char* ROINodeReferenceDMMLAttributeName;
  static const char* ShaderPropertyNodeReferenceRole;
  static const char* ShaderPropertyNodeReferenceDMMLAttributeName;

protected:
  /// Flag indicating whether cropping is enabled
  int CroppingEnabled;

  double Threshold[2];

  /// Follow window/level and thresholding setting in volume display node
  int FollowVolumeDisplayNode;
  int IgnoreVolumeDisplayNodeThreshold;

  int UseSingleVolumeProperty;

  /// Volume window & level
  double WindowLevel[2];
};

#endif
