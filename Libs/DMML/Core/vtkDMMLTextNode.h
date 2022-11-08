/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLTextNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkDMMLTextNode_h
#define __vtkDMMLTextNode_h

// DMML includes
#include <vtkDMMLStorableNode.h>

class  VTK_DMML_EXPORT vtkDMMLTextNode : public vtkDMMLStorableNode
{
public:

  static vtkDMMLTextNode* New();
  vtkTypeMacro(vtkDMMLTextNode, vtkDMMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  enum
  {
    CreateStorageNodeAuto,
    CreateStorageNodeAlways,
    CreateStorageNodeNever
  };

  vtkDMMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes(const char** atts) override;

  ///
  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentMacro(vtkDMMLTextNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "Text"; };

  /// Set text node contents and encoding.
  /// If the encoding is not specified, then it will not be changed from the current value.
  /// \sa SetEncoding()
  void SetText(const std::string &text, int encoding=-1);
  vtkGetMacro(Text, std::string);

  ///
  /// Set encoding of the text
  /// For character encoding, please refer IANA Character Sets
  /// (https://www.iana.org/assignments/character-sets/character-sets.xhtml)
  /// Default is VTK_ENCODING_US_ASCII
  void SetEncoding(int encoding);
  vtkGetMacro(Encoding, int);
  std::string GetEncodingAsString();

  /// Force the use of a storage node, regardless of text length.
  /// By default, a storage node will only be used for nodes that have been read from file (drag and drop),
  /// or for nodes that have text longer than 250 characters.
  /// This option should be also be enabled for nodes with highly structured text (such as XML) that would
  /// not be good to have in the DMML.
  vtkSetClampMacro(ForceCreateStorageNode, int, CreateStorageNodeAuto, CreateStorageNodeNever);
  vtkGetMacro(ForceCreateStorageNode, int);

  /// Create a storage node for this node type.
  /// If it returns nullptr then it means the node can be stored
  /// in the scene (in XML), without using a storage node.
  vtkDMMLStorageNode* CreateDefaultStorageNode() override;

  /// Determines the most appropriate storage node class for the
  /// provided file name and node content.
  std::string GetDefaultStorageNodeClassName(const char* filename=nullptr) override;

  enum
  {
    TextModifiedEvent = 51000, // Invoked if the text OR encoding is changed with SetText() and SetEncoding() methods and is not invoked
                               // if the text and encoding specified in the set methods are the same as the current text and encoding
  };

protected:
  vtkDMMLTextNode();
  ~vtkDMMLTextNode() override;
  vtkDMMLTextNode(const vtkDMMLTextNode&);
  void operator=(const vtkDMMLTextNode&);

  std::string Text;
  int Encoding{VTK_ENCODING_US_ASCII};
  int ForceCreateStorageNode{CreateStorageNodeAuto};

};

#endif
