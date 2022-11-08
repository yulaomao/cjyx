/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLTextNode.cxx,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

// CjyxOpenIGTLink DMML includes
#include "vtkDMMLTextNode.h"
#include "vtkDMMLTextStorageNode.h"

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include "vtkObjectFactory.h"
#include "vtkXMLUtilities.h"

const int MAX_STRING_LENGTH_FOR_SAVE_WITHOUT_STORAGE_NODE = 256;

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLTextNode);

//-----------------------------------------------------------------------------
vtkDMMLTextNode::vtkDMMLTextNode()
{
  this->ContentModifiedEvents->InsertNextValue(vtkDMMLTextNode::TextModifiedEvent);
}

//-----------------------------------------------------------------------------
vtkDMMLTextNode::~vtkDMMLTextNode()
{
}

//----------------------------------------------------------------------------
void vtkDMMLTextNode::SetText(const std::string &text, int encoding/*-1*/)
{
  vtkDebugMacro( << this->GetClassName() << " (" << this << "): setting Text to " << text);

  DMMLNodeModifyBlocker blocker(this);
  if (encoding >= 0)
    {
    this->SetEncoding(encoding);
    }
  if (this->Text == text)
    {
    return;
    }
  this->Text = text;
  // this indicates that the text (that is stored in a separate file) is modified
  // and therefore the object will be marked as changed for file saving
  this->StorableModifiedTime.Modified();
  this->InvokeCustomModifiedEvent(vtkDMMLTextNode::TextModifiedEvent);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLTextNode::SetEncoding(int encoding)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting encoding to " << encoding);
  int clampedEncoding = std::max(VTK_ENCODING_NONE, std::min(encoding, VTK_ENCODING_UNKNOWN));
  if (this->Encoding == clampedEncoding)
    {
    return;
    }
  DMMLNodeModifyBlocker blocker(this);
  this->Encoding = clampedEncoding;
  // this indicates that the text (that is stored in a separate file) is modified
  // and therefore the object will be marked as changed for file saving
  this->StorableModifiedTime.Modified();
  this->InvokeCustomModifiedEvent(vtkDMMLTextNode::TextModifiedEvent);
  this->Modified();
}

//----------------------------------------------------------------------------
std::string vtkDMMLTextNode::GetEncodingAsString()
{
  switch (this->Encoding)
    {
    case VTK_ENCODING_NONE:
      return "None";
    case VTK_ENCODING_US_ASCII:
      return "ASCII";
    case VTK_ENCODING_UNICODE:
      return "Unicode";
    case VTK_ENCODING_UTF_8:
      return "UTF-8";
    case VTK_ENCODING_ISO_8859_1:
    case VTK_ENCODING_ISO_8859_2:
    case VTK_ENCODING_ISO_8859_3:
    case VTK_ENCODING_ISO_8859_4:
    case VTK_ENCODING_ISO_8859_5:
    case VTK_ENCODING_ISO_8859_6:
    case VTK_ENCODING_ISO_8859_7:
    case VTK_ENCODING_ISO_8859_8:
    case VTK_ENCODING_ISO_8859_10:
    case VTK_ENCODING_ISO_8859_11:
    case VTK_ENCODING_ISO_8859_12:
    case VTK_ENCODING_ISO_8859_13:
    case VTK_ENCODING_ISO_8859_14:
    case VTK_ENCODING_ISO_8859_15:
    case VTK_ENCODING_ISO_8859_16:
      return "ISO-8859-" + vtkVariant(this->Encoding - VTK_ENCODING_ISO_8859_1 + 1).ToString();
    }
  return "Unknown";
}

//----------------------------------------------------------------------------
void vtkDMMLTextNode::ReadXMLAttributes(const char** atts)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);
  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLStdStringMacro(text, Text);
  vtkDMMLReadXMLIntMacro(encoding, Encoding);
  vtkDMMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLTextNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkDMMLWriteXMLBeginMacro(of);
  if (!this->GetStorageNode())
    {
    vtkDMMLWriteXMLStdStringMacro(text, Text);
    }
  vtkDMMLWriteXMLIntMacro(encoding, Encoding);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLTextNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyStringMacro(Text);
  vtkDMMLCopyIntMacro(Encoding);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLTextNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintStdStringMacro(Text);
  vtkDMMLPrintIntMacro(Encoding);
  vtkDMMLPrintEndMacro();
}

//---------------------------------------------------------------------------
std::string vtkDMMLTextNode::GetDefaultStorageNodeClassName(const char* vtkNotUsed(filename))
{
  if (!this->Scene)
    {
    return "";
    }

  if (this->ForceCreateStorageNode == CreateStorageNodeNever)
    {
    return "";
    }

  if (this->ForceCreateStorageNode == CreateStorageNodeAuto)
    {
    int length = this->Text.length();
    if (length < MAX_STRING_LENGTH_FOR_SAVE_WITHOUT_STORAGE_NODE)
      {
      return "";
      }
    }

  return "vtkDMMLTextStorageNode";
}

//---------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLTextNode::CreateDefaultStorageNode()
{
  if (!this->Scene)
    {
    return nullptr;
    }

  if (!this->ForceCreateStorageNode)
    {
    int length = this->Text.length();
    if (length < MAX_STRING_LENGTH_FOR_SAVE_WITHOUT_STORAGE_NODE)
      {
      return nullptr;
      }
    }
  return vtkDMMLTextStorageNode::SafeDownCast(
    this->Scene->CreateNodeByClass(this->GetDefaultStorageNodeClassName().c_str()));
}
