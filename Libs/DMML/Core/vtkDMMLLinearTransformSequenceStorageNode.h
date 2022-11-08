/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
///  vtkDMMLLinearTransformSequenceStorageNode - DMML node that can read/write
///  a Sequence node containing linear transforms in a single nrrd or mha file
///

#ifndef __vtkDMMLLinearTransformSequenceStorageNode_h
#define __vtkDMMLLinearTransformSequenceStorageNode_h

#include "vtkDMML.h"
#include "vtkDMMLNRRDStorageNode.h"

#include <deque>

class vtkDMMLSequenceNode;

enum SequenceFileType
  {
  INVALID_SEQUENCE_FILE,
  METAIMAGE_SEQUENCE_FILE,
  NRRD_SEQUENCE_FILE
  };

/// \ingroup Cjyx_QtModules_Sequences
class VTK_DMML_EXPORT vtkDMMLLinearTransformSequenceStorageNode : public vtkDMMLNRRDStorageNode
{
  public:

  static vtkDMMLLinearTransformSequenceStorageNode *New();
  vtkTypeMacro(vtkDMMLLinearTransformSequenceStorageNode,vtkDMMLNRRDStorageNode);

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "LinearTransformSequenceStorage";};

  /// Return true if the node can be read in.
  bool CanReadInReferenceNode(vtkDMMLNode *refNode) override;

  /// Return true if the node can be written by using the writer.
  bool CanWriteFromReferenceNode(vtkDMMLNode* refNode) override;
  int WriteDataInternal(vtkDMMLNode *refNode) override;

  ///
  /// Return a default file extension for writing
  const char* GetDefaultWriteFileExtension() override;

  /// Read all the fields in the metaimage file header.
  /// If sequence nodes are passed in createdNodes then they will be reused. New sequence nodes will be created if there are more transforms
  /// in the sequence metafile than pointers in createdNodes. The caller is responsible for deleting all nodes in createdNodes.
  /// Return number of created transform nodes.
  static int ReadSequenceFileTransforms(const std::string& fileName, vtkDMMLScene *scene,
    std::deque< vtkSmartPointer<vtkDMMLSequenceNode> > &createdNodes, std::map< int, std::string >& frameNumberToIndexValueMap,
    std::map< std::string, std::string > &imageMetaData, SequenceFileType fileType = METAIMAGE_SEQUENCE_FILE);

  /// Write the transform fields to the metaimage header
  static bool WriteSequenceMetafileTransforms(const std::string& fileName, std::deque< vtkDMMLSequenceNode* > &transformNodes,
    std::deque< std::string > &transformNames, vtkDMMLSequenceNode* masterNode, vtkDMMLSequenceNode* imageNode);

protected:
  vtkDMMLLinearTransformSequenceStorageNode();
  ~vtkDMMLLinearTransformSequenceStorageNode() override;
  vtkDMMLLinearTransformSequenceStorageNode(const vtkDMMLLinearTransformSequenceStorageNode&);
  void operator=(const vtkDMMLLinearTransformSequenceStorageNode&);

  /// Does the actual reading. Returns 1 on success, 0 otherwise.
  /// Returns 0 by default (read not supported).
  /// This implementation delegates most everything to the superclass
  /// but it has an early exit if the file to be read is incompatible.
  int ReadDataInternal(vtkDMMLNode* refNode) override;

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;
};

#endif
