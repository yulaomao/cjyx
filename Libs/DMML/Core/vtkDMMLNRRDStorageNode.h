/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLNRRDStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/

#ifndef __vtkDMMLNRRDStorageNode_h
#define __vtkDMMLNRRDStorageNode_h

#include "vtkDMMLStorageNode.h"
class vtkDoubleArray;
class vtkTeemNRRDReader;

/// \brief DMML node for representing a volume storage.
///
/// vtkDMMLNRRDStorageNode nodes describe the archetybe based volume storage
/// node that allows to read/write volume data from/to file using generic ITK mechanism.
class VTK_DMML_EXPORT vtkDMMLNRRDStorageNode : public vtkDMMLStorageNode
{
  public:
  static vtkDMMLNRRDStorageNode *New();
  vtkTypeMacro(vtkDMMLNRRDStorageNode,vtkDMMLStorageNode);
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
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "NRRDStorage";}

  ///
  /// Center image on read
  vtkGetMacro(CenterImage, int);
  vtkSetMacro(CenterImage, int);

  ///
  /// Access the nrrd header fields to create a diffusion gradient table
  int ParseDiffusionInformation(vtkTeemNRRDReader *reader,vtkDoubleArray *grad,vtkDoubleArray *bvalues);

  /// Return true if the node can be read in.
  bool CanReadInReferenceNode(vtkDMMLNode *refNode) override;

  ///
  /// Configure the storage node for data exchange. This is an
  /// opportunity to optimize the storage node's settings, for
  /// instance to turn off compression.
  void ConfigureForDataExchange() override;

  /// Compression parameter corresponding to minimum compression (fast)
  std::string GetCompressionParameterFastest() { return "gzip_fastest"; };
  /// Compression parameter corresponding to normal compression
  std::string GetCompressionParameterNormal() { return "gzip_normal"; };
  /// Compression parameter corresponding to maximum compression (slow)
  std::string GetCompressionParameterMinimumSize() { return "gzip_minimum_size"; };

protected:
  vtkDMMLNRRDStorageNode();
  ~vtkDMMLNRRDStorageNode() override;
  vtkDMMLNRRDStorageNode(const vtkDMMLNRRDStorageNode&);
  void operator=(const vtkDMMLNRRDStorageNode&);

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkDMMLNode *refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkDMMLNode *refNode) override;

  /// Convert compression parameter string to gzip compression level
  int GetGzipCompressionLevelFromCompressionParameter(std::string parameter);

  int CenterImage;
};

#endif
