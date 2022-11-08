/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLScriptedModuleNode.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLScriptedModuleNode);

//----------------------------------------------------------------------------
vtkDMMLScriptedModuleNode::vtkDMMLScriptedModuleNode()
{
  this->HideFromEditors = 1;

  this->ModuleName = nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLScriptedModuleNode::~vtkDMMLScriptedModuleNode()
{
  if (this->ModuleName)
    {
    delete [] this->ModuleName;
    this->ModuleName = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkDMMLScriptedModuleNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all DMML node attributes into output stream

  if (this->ModuleName != nullptr)
    {
    of << " ModuleName =\"" << this->XMLAttributeEncodeString(this->ModuleName) << "\"";
    }

  ParameterMap::iterator iter;
  int i = 0;
  for (iter=this->Parameters.begin(); iter != this->Parameters.end(); iter++)
    {
    std::string paramName = iter->first;
    // space is used as separator, so space (and the escape character) have to be encoded
    vtksys::SystemTools::ReplaceString(paramName, "%", "%25");
    vtksys::SystemTools::ReplaceString(paramName, " ", "%20");
    std::string paramValue = iter->second;
    of << " parameter" << i << "= \"" << this->XMLAttributeEncodeString(paramName) << " " << this->XMLAttributeEncodeString(paramValue)<< "\"";
    i++;
    }
}

//----------------------------------------------------------------------------
void vtkDMMLScriptedModuleNode::ReadXMLAttributes(const char** atts)
{
  vtkDMMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if ( !strcmp(attName, "ModuleName") )
      {
      this->SetModuleName( attValue );
      }
    else if ( !strncmp(attName, "parameter", strlen("parameter") ) )
      {
      std::string satt(attValue);
      int space = (int)satt.find(" ", 0);
      std::string sname = satt.substr(0,space);
      std::string svalue = satt.substr(space+1,satt.length()-space-1);
      // decode separator character (space) and escape character
      vtksys::SystemTools::ReplaceString(sname, "%20", " "); \
      vtksys::SystemTools::ReplaceString(sname, "%25", "%"); \
      this->SetParameter(sname, svalue);
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLScriptedModuleNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLScriptedModuleNode* node = vtkDMMLScriptedModuleNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  this->SetModuleName(node->GetModuleName());
  this->Parameters = node->Parameters;
}

//----------------------------------------------------------------------------
const char* vtkDMMLScriptedModuleNode::GetNodeTagName()
{
  return "ScriptedModule";
}

//----------------------------------------------------------------------------
void vtkDMMLScriptedModuleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDMMLNode::PrintSelf(os,indent);

  std::map<std::string, std::string>::iterator iter;

  os << indent << "ModuleName: " << (this->GetModuleName() ? this->GetModuleName() : "(none)") << "\n";

  for (iter=this->Parameters.begin(); iter != this->Parameters.end(); iter++)
    {
    os << indent << iter->first << ": " << iter->second << "\n";
    }
}

//----------------------------------------------------------------------------
void vtkDMMLScriptedModuleNode
::SetParameter(const std::string& name, const std::string& value)
{
  const std::string currentValue = this->GetParameter(name);
  if (value != currentValue)
    {
    this->Parameters[name] = value;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLScriptedModuleNode::UnsetParameter(const std::string& name)
{
  int count = this->Parameters.erase(name);
  if (count > 0)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLScriptedModuleNode::UnsetAllParameters()
{
  std::string::size_type count = this->Parameters.size();
  this->Parameters.clear();
  if (count != this->Parameters.size())
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
std::string vtkDMMLScriptedModuleNode
::GetParameter(const std::string& name) const
{
  if ( this->Parameters.find(name) == this->Parameters.end() )
    {
    return std::string();
    }
  return this->Parameters.find(name)->second;
}

//----------------------------------------------------------------------------
int vtkDMMLScriptedModuleNode::GetParameterCount()
{
  return this->Parameters.size();
}

//----------------------------------------------------------------------------
std::string vtkDMMLScriptedModuleNode::GetParameterNamesAsCommaSeparatedList()
{
  std::vector<std::string> names = this->GetParameterNames();
  std::string namesAsStr;
  std::vector<std::string>::iterator it = names.begin();
  if (it != names.end())
    {
    namesAsStr = *it;
    ++it;
    }
  for(; it != names.end(); ++it)
    {
    namesAsStr.append(",").append(*it);
    }
  return namesAsStr;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkDMMLScriptedModuleNode::GetParameterNames()
{
  // If the number of parameter associated with a node become huge, it
  // could be interesting to cache the list of names instead of recomputing
  // each time the function is called.
  std::vector<std::string> names;

  ParameterMap::iterator it;
  for(it = this->Parameters.begin(); it != this->Parameters.end(); ++it)
    {
    names.push_back(it->first);
    }
  return names;
}
