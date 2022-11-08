/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Segmentations DMML includes
#include "vtkDMMLSegmentEditorNode.h"

#include "vtkOrientedImageDataResample.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLScalarVolumeNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
static const char* SEGMENTATION_REFERENCE_ROLE = "segmentationRef";
static const char* MASTER_VOLUME_REFERENCE_ROLE = "masterVolumeRef";

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLSegmentEditorNode);

//----------------------------------------------------------------------------
vtkDMMLSegmentEditorNode::vtkDMMLSegmentEditorNode()
{
  this->SetHideFromEditors(true);
  this->MasterVolumeIntensityMaskRange[0] = 0.0;
  this->MasterVolumeIntensityMaskRange[1] = 0.0;
}

//----------------------------------------------------------------------------
vtkDMMLSegmentEditorNode::~vtkDMMLSegmentEditorNode()
{
  this->SetSelectedSegmentID(nullptr);
  this->SetActiveEffectName(nullptr);
  this->SetMaskSegmentID(nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentEditorNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all DMML node attributes into output stream
  of << " selectedSegmentID=\"" << (this->SelectedSegmentID?this->SelectedSegmentID:"") << "\"";
  of << " activeEffectName=\"" << (this->ActiveEffectName?this->ActiveEffectName:"") << "\"";
  of << " maskMode=\"" << vtkDMMLSegmentationNode::ConvertMaskModeToString(this->MaskMode) << "\"";
  of << " maskSegmentID=\"" << (this->MaskSegmentID?this->MaskSegmentID:"") << "\"";
  of << " masterVolumeIntensityMask=\"" << (this->MasterVolumeIntensityMask ? "true" : "false") << "\"";
  of << " masterVolumeIntensityMaskRange=\"" << this->MasterVolumeIntensityMaskRange[0] << " " << this->MasterVolumeIntensityMaskRange[1] << "\"";
  of << " overwriteMode=\"" << vtkDMMLSegmentEditorNode::ConvertOverwriteModeToString(this->OverwriteMode) << "\"";
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentEditorNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  // Read all DMML node attributes from two arrays of names and values
  const char* attName = nullptr;
  const char* attValue = nullptr;

  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "selectedSegmentID"))
      {
      this->SetSelectedSegmentID(attValue);
      }
    else if (!strcmp(attName, "activeEffectName"))
      {
      this->SetActiveEffectName(attValue);
      }
    else if (!strcmp(attName, "maskMode"))
      {
      this->SetMaskMode(vtkDMMLSegmentationNode::ConvertMaskModeFromString(attValue));
      }
    else if (!strcmp(attName, "maskSegmentID"))
      {
      this->SetMaskSegmentID(attValue);
      }
    else if (!strcmp(attName, "masterVolumeIntensityMask"))
      {
      this->SetMasterVolumeIntensityMask(!strcmp(attValue,"true"));
      }
    else if (!strcmp(attName, "masterVolumeIntensityMaskRange"))
      {
      std::stringstream ss;
      ss << attValue;
      double range[2]={0};
      ss >> range[0];
      ss >> range[1];
      this->SetMasterVolumeIntensityMaskRange(range);
      }
    else if (!strcmp(attName, "overwriteMode"))
      {
      this->SetOverwriteMode(vtkDMMLSegmentEditorNode::ConvertOverwriteModeFromString(attValue));
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkDMMLSegmentEditorNode::Copy(vtkDMMLNode *anode)
{
  Superclass::Copy(anode);
  this->DisableModifiedEventOn();

  vtkDMMLSegmentEditorNode* otherNode = vtkDMMLSegmentEditorNode::SafeDownCast(anode);

  this->SetSelectedSegmentID(otherNode->SelectedSegmentID);
  this->SetActiveEffectName(otherNode->ActiveEffectName);
  this->SetMaskMode(otherNode->GetMaskMode());
  this->SetMaskSegmentID(otherNode->GetMaskSegmentID());
  this->SetMasterVolumeIntensityMask(otherNode->GetMasterVolumeIntensityMask());
  this->SetMasterVolumeIntensityMaskRange(otherNode->GetMasterVolumeIntensityMaskRange());
  this->SetOverwriteMode(otherNode->GetOverwriteMode());

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentEditorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "SelectedSegmentID: " << (this->SelectedSegmentID ? this->SelectedSegmentID : "") << "\n";
  os << indent << "ActiveEffectName: " << (this->ActiveEffectName ? this->ActiveEffectName : "") << "\n";
  os << indent << "MaskMode: " << vtkDMMLSegmentationNode::ConvertMaskModeToString(this->MaskMode) << "\n";
  os << indent << "MaskSegmentID: " << (this->MaskSegmentID?this->MaskSegmentID:"") << "\n";
  os << indent << "OverwriteMode: " << vtkDMMLSegmentEditorNode::ConvertOverwriteModeToString(this->OverwriteMode) << "\n";
  os << indent << "MasterVolumeIntensityMask: " << (this->MasterVolumeIntensityMask ? "true" : "false") << "\n";
  os << indent << "MasterVolumeIntensityMaskRange: " << this->MasterVolumeIntensityMaskRange[0] << " " << this->MasterVolumeIntensityMaskRange[1] << "\n";
}

//----------------------------------------------------------------------------
vtkDMMLScalarVolumeNode* vtkDMMLSegmentEditorNode::GetMasterVolumeNode()
{
  return vtkDMMLScalarVolumeNode::SafeDownCast( this->GetNodeReference(MASTER_VOLUME_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentEditorNode::SetAndObserveMasterVolumeNode(vtkDMMLScalarVolumeNode* node)
{
  this->SetNodeReferenceID(MASTER_VOLUME_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
vtkDMMLSegmentationNode* vtkDMMLSegmentEditorNode::GetSegmentationNode()
{
  return vtkDMMLSegmentationNode::SafeDownCast( this->GetNodeReference(SEGMENTATION_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkDMMLSegmentEditorNode::SetAndObserveSegmentationNode(vtkDMMLSegmentationNode* node)
{
  this->SetNodeReferenceID(SEGMENTATION_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
const char* vtkDMMLSegmentEditorNode::ConvertOverwriteModeToString(int mode)
{
  switch (mode)
  {
    case OverwriteAllSegments: return "OverwriteAllSegments";
    case OverwriteVisibleSegments: return "OverwriteVisibleSegments";
    case OverwriteNone: return "OverwriteNone";
    default: return "";
  }
}

//----------------------------------------------------------------------------
int vtkDMMLSegmentEditorNode::ConvertOverwriteModeFromString(const char* modeStr)
{
  if (!modeStr)
  {
    return -1;
  }
  for (int i=0; i<Overwrite_Last; i++)
  {
    if (strcmp(modeStr, vtkDMMLSegmentEditorNode::ConvertOverwriteModeToString(i))==0)
    {
      return i;
    }
  }
  return -1;
}
