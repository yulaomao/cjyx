/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLScriptedModuleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkDMMLScriptedModuleNode_h
#define __vtkDMMLScriptedModuleNode_h

// DMML includes
#include "vtkDMMLNode.h"

// STD includes
#include <string>
#include <vector>

/// The scripted module node is simply a DMMLNode container for
/// an arbitrary keyword value pair map
class VTK_DMML_EXPORT vtkDMMLScriptedModuleNode : public vtkDMMLNode
{
public:
  static vtkDMMLScriptedModuleNode *New();
  vtkTypeMacro(vtkDMMLScriptedModuleNode,vtkDMMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDMMLNode* CreateNodeInstance() override;

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkDMMLNode::CopyContent
  vtkDMMLCopyContentMacro(vtkDMMLScriptedModuleNode);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override;

  /// The name of the Module - this is used to
  /// customize the node selectors and other things
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  /// Set module parameter
  void SetParameter(const std::string& name, const std::string& value);

  /// Unset the parameter identified by \a name
  void UnsetParameter(const std::string& name);

  /// Unset all parameters
  /// \sa UnsetParameter
  void UnsetAllParameters();

  /// Get module parameter identified by \a name
  std::string GetParameter(const std::string& name) const;

  /// Get number of parameters
  int GetParameterCount();

  /// Get list of parameter names separated by a comma
  /// \sa GetParameterNames
  std::string GetParameterNamesAsCommaSeparatedList();

  /// Get list of parameter names
  std::vector<std::string> GetParameterNames();

protected:
  vtkDMMLScriptedModuleNode();
  ~vtkDMMLScriptedModuleNode() override;

  vtkDMMLScriptedModuleNode(const vtkDMMLScriptedModuleNode&);
  void operator=(const vtkDMMLScriptedModuleNode&);

  typedef std::map<std::string, std::string> ParameterMap;
  ParameterMap Parameters;
  char *ModuleName;
};

#endif
