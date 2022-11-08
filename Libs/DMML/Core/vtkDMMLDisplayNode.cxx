/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// DMML includes
#include "vtkEventBroker.h"
#include "vtkDMMLColorNode.h"
#include "vtkDMMLDisplayNode.h"
#include "vtkDMMLDisplayableNode.h"
#include "vtkDMMLFolderDisplayNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSliceNode.h"
#include "vtkDMMLViewNode.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkTextProperty.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <algorithm>
#include <sstream>

// Constants for UpdateTextPropertyFromString
static const int SHADOW_H_OFFSET_INDEX = 0;
static const int SHADOW_V_OFFSET_INDEX = 1;
static const int SHADOW_BLUR_RADIUS_INDEX = 2;
static const int SHADOW_COLOR_INDEX = 3;

//----------------------------------------------------------------------------
vtkCxxSetReferenceStringMacro(vtkDMMLDisplayNode, ColorNodeID);

//----------------------------------------------------------------------------
vtkDMMLDisplayNode::vtkDMMLDisplayNode()
{
  this->HideFromEditors = 1;

  this->Opacity = 1.0;
  this->SliceIntersectionOpacity = 1.0;
  this->Ambient = 0.0;
  this->Diffuse = 1.0;
  this->Specular = 0;
  this->Power = 1;
  this->Metallic = 0.0;
  this->Roughness = 0.5;
  this->SelectedAmbient = 0.4;
  this->SelectedSpecular = 0.5;

  this->PointSize = 1.0;
  this->LineWidth = 1.0;
  this->Representation = vtkDMMLDisplayNode::SurfaceRepresentation;
  this->Lighting = 1;
  this->Interpolation = vtkDMMLDisplayNode::GouraudInterpolation;
  this->Shading = 1;

  this->Visibility = 1;
  this->Visibility2D = 0;
  this->Visibility3D = 1;
  this->EdgeVisibility = 0;
  this->Clipping = 0;
  this->SliceIntersectionThickness = 1;
  this->FrontfaceCulling = 0;
  // Backface culling can make rendering faster but it can cause partial rendering
  // if the surface is cut, semi-transparent, or faces are not oriented consistently,
  // therefore it is disabled by default.
  this->BackfaceCulling = 0;
  this->ScalarVisibility = 0;
  this->VectorVisibility = 0;
  this->TensorVisibility = 0;
  this->InterpolateTexture = 0;
  this->ScalarRangeFlag = vtkDMMLDisplayNode::UseDataScalarRange;
  this->FolderDisplayOverrideAllowed = true;
  this->ShowMode = vtkDMMLDisplayNode::ShowDefault;

  // Arrays
  this->ScalarRange[0] = 0;
  this->ScalarRange[1] = 100;

  // Set default color to yellow to have some contrast compared to grayscale images
  this->Color[0] = 0.9;
  this->Color[1] = 0.9;
  this->Color[2] = 0.3;
  this->EdgeColor[0] = 0.0;
  this->EdgeColor[1] = 0.0;
  this->EdgeColor[2] = 0.0;
  this->SelectedColor[0] = 1.0;
  this->SelectedColor[1] = 0.0;
  this->SelectedColor[2] = 0.0;
  this->SelectedAmbient = 0.4;
  this->SelectedSpecular = 0.5;

  this->TextureImageDataConnection = nullptr;
  this->ColorNodeID = nullptr;
  this->ColorNode = nullptr;

  this->ActiveScalarName = nullptr;
  this->ActiveAttributeLocation = vtkAssignAttribute::POINT_DATA;

  // add observer to process visualization pipeline
  vtkEventBroker::GetInstance()->AddObservation(
    this, vtkCommand::ModifiedEvent, this, this->DMMLCallbackCommand );
}

//----------------------------------------------------------------------------
vtkDMMLDisplayNode::~vtkDMMLDisplayNode()
{
  this->SetTextureImageDataConnection(nullptr);
  this->SetActiveScalarName(nullptr);
  this->SetAndObserveColorNodeID( nullptr);
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLVectorMacro(color, Color, double, 3);
  vtkDMMLWriteXMLVectorMacro(edgeColor, EdgeColor, double, 3);
  vtkDMMLWriteXMLVectorMacro(selectedColor, SelectedColor, double, 3);
  vtkDMMLWriteXMLFloatMacro(selectedAmbient, SelectedAmbient);
  vtkDMMLWriteXMLFloatMacro(ambient, Ambient);
  vtkDMMLWriteXMLFloatMacro(diffuse, Diffuse);
  vtkDMMLWriteXMLFloatMacro(selectedSpecular, SelectedSpecular);
  vtkDMMLWriteXMLFloatMacro(specular, Specular);
  vtkDMMLWriteXMLFloatMacro(power, Power);
  vtkDMMLWriteXMLFloatMacro(metallic, Metallic);
  vtkDMMLWriteXMLFloatMacro(roughness, Roughness);
  vtkDMMLWriteXMLFloatMacro(opacity, Opacity);
  vtkDMMLWriteXMLFloatMacro(sliceIntersectionOpacity, SliceIntersectionOpacity);
  vtkDMMLWriteXMLFloatMacro(pointSize, PointSize);
  vtkDMMLWriteXMLFloatMacro(lineWidth, LineWidth);
  vtkDMMLWriteXMLIntMacro(representation, Representation);
  vtkDMMLWriteXMLBooleanMacro(lighting, Lighting);
  vtkDMMLWriteXMLIntMacro(interpolation, Interpolation);
  vtkDMMLWriteXMLBooleanMacro(shading, Shading);
  vtkDMMLWriteXMLBooleanMacro(visibility, Visibility);
  vtkDMMLWriteXMLBooleanMacro(visibility2D, Visibility2D);
  vtkDMMLWriteXMLBooleanMacro(visibility3D, Visibility3D);
  vtkDMMLWriteXMLBooleanMacro(edgeVisibility, EdgeVisibility);
  vtkDMMLWriteXMLBooleanMacro(clipping, Clipping);
  vtkDMMLWriteXMLIntMacro(sliceIntersectionThickness, SliceIntersectionThickness);
  vtkDMMLWriteXMLBooleanMacro(frontfaceCulling, FrontfaceCulling);
  vtkDMMLWriteXMLBooleanMacro(backfaceCulling, BackfaceCulling);
  vtkDMMLWriteXMLBooleanMacro(scalarVisibility, ScalarVisibility);
  vtkDMMLWriteXMLBooleanMacro(vectorVisibility, VectorVisibility);
  vtkDMMLWriteXMLBooleanMacro(tensorVisibility, TensorVisibility);
  vtkDMMLWriteXMLBooleanMacro(interpolateTexture, InterpolateTexture);
  vtkDMMLWriteXMLStringMacro(scalarRangeFlag, ScalarRangeFlagAsString);
  vtkDMMLWriteXMLVectorMacro(scalarRange, ScalarRange, double, 2);
  vtkDMMLWriteXMLStringMacro(colorNodeID, ColorNodeID);
  vtkDMMLWriteXMLStringMacro(activeScalarName, ActiveScalarName);
  vtkDMMLWriteXMLStringMacro(activeAttributeLocation, ActiveAttributeLocationAsString);
  vtkDMMLWriteXMLStdStringVectorMacro(viewNodeRef, ViewNodeIDs, std::vector);
  vtkDMMLWriteXMLBooleanMacro(folderDisplayOverrideAllowed, FolderDisplayOverrideAllowed);
  if (this->GetShowMode() != vtkDMMLDisplayNode::ShowDefault)
    {
    // only write out the show mode if using non-default mode
    vtkDMMLWriteXMLEnumMacro(showMode, ShowMode);
    }
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayNode::ReadXMLAttributes(const char** atts)
{
  DMMLNodeModifyBlocker blocker(this);

  Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLVectorMacro(color, Color, double, 3);
  vtkDMMLReadXMLVectorMacro(edgeColor, EdgeColor, double, 3);
  vtkDMMLReadXMLVectorMacro(selectedColor, SelectedColor, double, 3);
  vtkDMMLReadXMLFloatMacro(selectedAmbient, SelectedAmbient);
  vtkDMMLReadXMLFloatMacro(ambient, Ambient);
  vtkDMMLReadXMLFloatMacro(diffuse, Diffuse);
  vtkDMMLReadXMLFloatMacro(selectedSpecular, SelectedSpecular);
  vtkDMMLReadXMLFloatMacro(specular, Specular);
  vtkDMMLReadXMLFloatMacro(power, Power);
  vtkDMMLReadXMLFloatMacro(metallic, Metallic);
  vtkDMMLReadXMLFloatMacro(roughness, Roughness);
  vtkDMMLReadXMLFloatMacro(opacity, Opacity);
  vtkDMMLReadXMLFloatMacro(sliceIntersectionOpacity, SliceIntersectionOpacity);
  vtkDMMLReadXMLFloatMacro(pointSize, PointSize);
  vtkDMMLReadXMLFloatMacro(lineWidth, LineWidth);
  vtkDMMLReadXMLIntMacro(representation, Representation);
  vtkDMMLReadXMLBooleanMacro(lighting, Lighting);
  vtkDMMLReadXMLIntMacro(interpolation, Interpolation);
  vtkDMMLReadXMLBooleanMacro(shading, Shading);
  vtkDMMLReadXMLBooleanMacro(visibility, Visibility);
  vtkDMMLReadXMLBooleanMacro(visibility2D, Visibility2D);
  vtkDMMLReadXMLBooleanMacro(sliceIntersectionVisibility, Visibility2D); // For backwards compatibility
  vtkDMMLReadXMLBooleanMacro(visibility3D, Visibility3D);
  vtkDMMLReadXMLBooleanMacro(edgeVisibility, EdgeVisibility);
  vtkDMMLReadXMLBooleanMacro(clipping, Clipping);
  vtkDMMLReadXMLIntMacro(sliceIntersectionThickness, SliceIntersectionThickness);
  vtkDMMLReadXMLBooleanMacro(frontfaceCulling, FrontfaceCulling);
  vtkDMMLReadXMLBooleanMacro(backfaceCulling, BackfaceCulling);
  vtkDMMLReadXMLBooleanMacro(scalarVisibility, ScalarVisibility);
  vtkDMMLReadXMLBooleanMacro(vectorVisibility, VectorVisibility);
  vtkDMMLReadXMLBooleanMacro(tensorVisibility, TensorVisibility);
  vtkDMMLReadXMLBooleanMacro(interpolateTexture, InterpolateTexture);
  vtkDMMLReadXMLStringMacro(scalarRangeFlag, ScalarRangeFlagFromString);
  vtkDMMLReadXMLVectorMacro(scalarRange, ScalarRange, double, 2);
  vtkDMMLReadXMLStringMacro(colorNodeID, ColorNodeID);
  vtkDMMLReadXMLStringMacro(colorNodeRef, ColorNodeID);
  vtkDMMLReadXMLStringMacro(activeScalarName, ActiveScalarName);
  vtkDMMLReadXMLStringMacro(activeAttributeLocation, ActiveAttributeLocationFromString);
  vtkDMMLReadXMLBooleanMacro(folderDisplayOverrideAllowed, FolderDisplayOverrideAllowed);
  vtkDMMLReadXMLEnumMacro(showMode, ShowMode);
  if (!strcmp(xmlReadAttName, "autoScalarRange"))
    {
    if (!strcmp(xmlReadAttName,"true"))
      {
      this->SetScalarRangeFlag(vtkDMMLDisplayNode::UseDataScalarRange);
      }
    else
      {
      this->SetScalarRangeFlag(vtkDMMLDisplayNode::UseManualScalarRange);
      }
    }
  if (!strcmp(xmlReadAttName, "viewNodeRef"))
    {
    std::string nodeIds = xmlReadAttValue;
    // Legacy scenes used " " as separator, replace that by ";".
    vtksys::SystemTools::ReplaceString(nodeIds, " ", ";");
    std::stringstream ss(nodeIds);
    std::string nodeId;
    while (std::getline(ss, nodeId, ';'))
      {
      if (nodeId.empty())
        {
        continue;
        }
      this->AddViewNodeID(nodeId.c_str());
      }
    }
  vtkDMMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLDisplayNode *node = vtkDMMLDisplayNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  // Strings
  this->SetColor(node->Color);
  this->SetSelectedColor(node->SelectedColor);

  // Vectors
  this->SetScalarRange(node->ScalarRange);

  // Numbers
  this->SetSelectedAmbient(node->SelectedAmbient);
  this->SetSelectedSpecular(node->SelectedSpecular);
  this->SetOpacity(node->Opacity);
  this->SetSliceIntersectionOpacity(node->SliceIntersectionOpacity);
  this->SetInterpolation(node->Interpolation);
  this->SetAmbient(node->Ambient);
  this->SetDiffuse(node->Diffuse);
  this->SetSpecular(node->Specular);
  this->SetPower(node->Power);
  this->SetMetallic(node->Metallic);
  this->SetRoughness(node->Roughness);
  this->SetVisibility(node->Visibility);
  this->SetVisibility2D(node->Visibility2D);
  this->SetVisibility3D(node->Visibility3D);
  this->SetScalarVisibility(node->ScalarVisibility);
  this->SetVectorVisibility(node->VectorVisibility);
  this->SetTensorVisibility(node->TensorVisibility);
  this->SetInterpolateTexture(node->InterpolateTexture);
  this->SetScalarRangeFlag(node->ScalarRangeFlag);
  this->SetBackfaceCulling(node->BackfaceCulling);
  this->SetClipping(node->Clipping);
  this->SetVisibility2D(node->GetSliceIntersectionVisibility());
  this->SetSliceIntersectionThickness(node->SliceIntersectionThickness);
  this->SetTextureImageDataConnection(node->TextureImageDataConnection);
  this->SetAndObserveColorNodeID(node->ColorNodeID);
  this->SetActiveScalarName(node->ActiveScalarName);
  this->SetFolderDisplayOverrideAllowed(node->FolderDisplayOverrideAllowed);
  this->SetShowMode(node->ShowMode);
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintVectorMacro(Color, double, 3);
  vtkDMMLPrintVectorMacro(EdgeColor, double, 3);
  vtkDMMLPrintVectorMacro(SelectedColor, double, 3);
  vtkDMMLPrintFloatMacro(SelectedAmbient);
  vtkDMMLPrintFloatMacro(Ambient);
  vtkDMMLPrintFloatMacro(Diffuse);
  vtkDMMLPrintFloatMacro(SelectedSpecular);
  vtkDMMLPrintFloatMacro(Specular);
  vtkDMMLPrintFloatMacro(Power);
  vtkDMMLPrintFloatMacro(Metallic);
  vtkDMMLPrintFloatMacro(Roughness);
  vtkDMMLPrintFloatMacro(Opacity);
  vtkDMMLPrintFloatMacro(SliceIntersectionOpacity);
  vtkDMMLPrintFloatMacro(PointSize);
  vtkDMMLPrintFloatMacro(LineWidth);
  vtkDMMLPrintIntMacro(Representation);
  vtkDMMLPrintBooleanMacro(Lighting);
  vtkDMMLPrintIntMacro(Interpolation);
  vtkDMMLPrintBooleanMacro(Shading);
  vtkDMMLPrintBooleanMacro(Visibility);
  vtkDMMLPrintBooleanMacro(Visibility2D);
  vtkDMMLPrintBooleanMacro(Visibility3D);
  vtkDMMLPrintBooleanMacro(EdgeVisibility);
  vtkDMMLPrintBooleanMacro(Clipping);
  vtkDMMLPrintIntMacro(SliceIntersectionThickness);
  vtkDMMLPrintBooleanMacro(FrontfaceCulling);
  vtkDMMLPrintBooleanMacro(BackfaceCulling);
  vtkDMMLPrintBooleanMacro(ScalarVisibility);
  vtkDMMLPrintBooleanMacro(VectorVisibility);
  vtkDMMLPrintBooleanMacro(TensorVisibility);
  vtkDMMLPrintBooleanMacro(InterpolateTexture);
  vtkDMMLPrintStringMacro(ScalarRangeFlagAsString);
  vtkDMMLPrintVectorMacro(ScalarRange, double, 2);
  vtkDMMLPrintStringMacro(ColorNodeID);
  vtkDMMLPrintStringMacro(ActiveScalarName);
  vtkDMMLPrintStringMacro(ActiveAttributeLocationAsString);
  vtkDMMLPrintStdStringVectorMacro(ViewNodeIDs, std::vector);
  vtkDMMLPrintBooleanMacro(FolderDisplayOverrideAllowed);
  vtkDMMLPrintEnumMacro(ShowMode);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  if (this->Scene == nullptr)
    {
    return;
    }
  this->Scene->AddReferencedNodeID(this->ColorNodeID, this);
  for (unsigned int i=0; i<this->ViewNodeIDs.size(); i++)
    {
    this->Scene->AddReferencedNodeID(this->ViewNodeIDs[i].c_str(), this);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
  if (this->ColorNodeID && !strcmp(oldID, this->ColorNodeID))
    {
    this->SetAndObserveColorNodeID(newID);
    }
  else
    {
    bool modified = false;
    for (unsigned int i=0; i<this->ViewNodeIDs.size(); i++)
      {
      if ( std::string(oldID) == this->ViewNodeIDs[i])
        {
        this->ViewNodeIDs[i] =  std::string(newID);
        modified = true;
        }
      }
    if (modified)
      {
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
vtkDMMLDisplayableNode* vtkDMMLDisplayNode::GetDisplayableNode()
{
  if (this->Scene == nullptr)
    {
    return nullptr;
    }
  // It is an expensive operation to determine the displayable node
  // (need to iterate through the scene), so the last found value
  // is cached. If it is still valid then we use it.
  if (this->LastFoundDisplayableNode != nullptr)
    {
    if (this->LastFoundDisplayableNode->GetScene() == this->Scene
      && this->LastFoundDisplayableNode->HasDisplayNodeID(this->GetID()))
      {
      return this->LastFoundDisplayableNode;
      }
    }
  vtkDMMLNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* sceneNodes = this->Scene->GetNodes();
  for (sceneNodes->InitTraversal(it);
       (node = vtkDMMLNode::SafeDownCast(sceneNodes->GetNextItemAsObject(it))) ;)
    {
    vtkDMMLDisplayableNode* displayableNode =
      vtkDMMLDisplayableNode::SafeDownCast(node);
    if (displayableNode && displayableNode->HasDisplayNodeID(this->GetID()))
      {
      this->LastFoundDisplayableNode = displayableNode;
      return displayableNode;
      }
    }
  this->LastFoundDisplayableNode = nullptr;
  return nullptr;
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayNode
::SetTextureImageDataConnection(vtkAlgorithmOutput* newTextureImageDataConnection)
{
   if (newTextureImageDataConnection == this->TextureImageDataConnection)
    {
    return;
    }

  vtkAlgorithm* oldTextureImageDataAlgorithm = this->TextureImageDataConnection ?
    this->TextureImageDataConnection->GetProducer() : nullptr;

  this->TextureImageDataConnection = newTextureImageDataConnection;

  vtkAlgorithm* textureImageDataAlgorithm = this->TextureImageDataConnection ?
    this->TextureImageDataConnection->GetProducer() : nullptr;

  if (textureImageDataAlgorithm != nullptr)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      this->TextureImageDataConnection, vtkCommand::ModifiedEvent,
      this, this->DMMLCallbackCommand );
    textureImageDataAlgorithm->Register(this);
    }

  if (oldTextureImageDataAlgorithm != nullptr)
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      this->TextureImageDataConnection, vtkCommand::ModifiedEvent,
      this, this->DMMLCallbackCommand );
    oldTextureImageDataAlgorithm->UnRegister(this);
    }
  this->Modified();
}

//-----------------------------------------------------------
void vtkDMMLDisplayNode::UpdateScene(vtkDMMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveColorNodeID(this->GetColorNodeID());
}

//-----------------------------------------------------------
void vtkDMMLDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->Scene == nullptr)
    {
    return;
    }

  if (this->ColorNodeID != nullptr && this->Scene->GetNodeByID(this->ColorNodeID) == nullptr)
    {
    this->SetAndObserveColorNodeID(nullptr);
    }

  std::vector< std::string > viewNodeIDs;

  for (unsigned int i=0; i < this->ViewNodeIDs.size(); i++)
    {
    if (this->Scene->GetNodeByID(this->ViewNodeIDs[i]) != nullptr)
      {
      viewNodeIDs.push_back(this->ViewNodeIDs[i]);
      }
    }
  if (this->ViewNodeIDs.size() != viewNodeIDs.size())
    {
    // The following line is probably incorrect
    // (should be this->ViewNodeIDs = viewNodeIDs;)
    viewNodeIDs = this->ViewNodeIDs;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
vtkDMMLColorNode* vtkDMMLDisplayNode::GetColorNode()
{
  if (this->ColorNode || !this->ColorNodeID)
    {
    return this->ColorNode;
    }
  vtkDMMLColorNode* cnode = nullptr;
  if (this->GetScene())
    {
    cnode = vtkDMMLColorNode::SafeDownCast(
      this->GetScene()->GetNodeByID(this->ColorNodeID));
    }
  vtkSetAndObserveDMMLObjectMacro(this->ColorNode, cnode);
  return cnode;
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayNode::SetAndObserveColorNodeID(const char *colorNodeID)
{
  vtkDMMLColorNode* cnode = nullptr;
  if (this->GetScene() && colorNodeID)
    {
    cnode = vtkDMMLColorNode::SafeDownCast(
      this->GetScene()->GetNodeByID(colorNodeID));
    }
  if (this->ColorNode != cnode)
    {
    vtkSetAndObserveDMMLObjectMacro(this->ColorNode, cnode);
    }
  this->SetColorNodeInternal(cnode);
  this->SetColorNodeID(colorNodeID);
  if (this->Scene)
    {
    this->Scene->AddReferencedNodeID(this->ColorNodeID, this);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayNode::SetAndObserveColorNodeID(const std::string& colorNodeID)
{
  this->SetAndObserveColorNodeID( colorNodeID.c_str() );
}

//----------------------------------------------------------------------------
void vtkDMMLDisplayNode::SetColorNodeInternal(vtkDMMLColorNode* vtkNotUsed(newColorNode))
{
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayNode::ProcessDMMLEvents( vtkObject *caller, unsigned long event, void *callData)
{
  this->Superclass::ProcessDMMLEvents(caller, event, callData);

  if (this->TextureImageDataConnection != nullptr &&
      this->TextureImageDataConnection == vtkAlgorithmOutput::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }

  vtkDMMLColorNode* cnode = vtkDMMLColorNode::SafeDownCast(caller);
  if (cnode != nullptr &&
      this->ColorNodeID != nullptr && cnode->GetID() != nullptr &&
      strcmp(this->ColorNodeID, cnode->GetID()) == 0 &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }
}

//-------------------------------------------------------
void vtkDMMLDisplayNode::AddViewNodeID(const char* viewNodeID)
{
  if (viewNodeID == nullptr)
    {
    return;
    }

  if (this->IsViewNodeIDPresent(viewNodeID))
    {
    return; // already exists, do nothing
    }

  this->ViewNodeIDs.emplace_back(viewNodeID);
  if (this->Scene)
    {
    this->Scene->AddReferencedNodeID(viewNodeID, this);
    }

  this->Modified();
}

//-------------------------------------------------------
void vtkDMMLDisplayNode::RemoveViewNodeID(char* viewNodeID)
{
  if (viewNodeID == nullptr)
    {
    return;
    }
  std::vector< std::string > viewNodeIDs;
  for(unsigned int i=0; i<this->ViewNodeIDs.size(); i++)
    {
    if (std::string(viewNodeID) != this->ViewNodeIDs[i])
      {
      viewNodeIDs.push_back(this->ViewNodeIDs[i]);
      }
    }
  if (viewNodeIDs.size() != this->ViewNodeIDs.size() && this->Scene!=nullptr)
    {
    this->Scene->RemoveReferencedNodeID(viewNodeID, this);
    this->ViewNodeIDs = viewNodeIDs;
    this->Modified();
    }
}

//-------------------------------------------------------
void vtkDMMLDisplayNode::RemoveAllViewNodeIDs()
{
  if (this->Scene!=nullptr)
    {
    for(unsigned int i=0; i<this->ViewNodeIDs.size(); i++)
      {
      this->Scene->RemoveReferencedNodeID(ViewNodeIDs[i].c_str(), this);
      }
    }
  this->ViewNodeIDs.clear();
  this->Modified();
}

//-------------------------------------------------------
const char* vtkDMMLDisplayNode::GetNthViewNodeID(unsigned int index)
{
  if (index >= ViewNodeIDs.size())
    {
    vtkErrorMacro("vtkDMMLDisplayNode::GetNthViewNodeID() index " << index << " outside the range 0-" << ViewNodeIDs.size()-1 );
    return nullptr;
    }
  return ViewNodeIDs[index].c_str();
}

//-------------------------------------------------------
bool vtkDMMLDisplayNode::IsViewNodeIDPresent(const char* viewNodeID)const
{
  if (viewNodeID == nullptr)
    {
    return false;
    }
  std::string value(viewNodeID);
  std::vector< std::string >::const_iterator it =
    std::find(this->ViewNodeIDs.begin(), this->ViewNodeIDs.end(), value);
  return it != this->ViewNodeIDs.end();
}

//-------------------------------------------------------
bool vtkDMMLDisplayNode::IsDisplayableInView(const char* viewNodeID)const
{
  return this->GetNumberOfViewNodeIDs() == 0 || this->IsViewNodeIDPresent(viewNodeID);
}

//-------------------------------------------------------
void vtkDMMLDisplayNode::SetDisplayableOnlyInView(const char *viewNodeID)
{
  if (viewNodeID == nullptr)
    {
    return;
    }

  int disabledModify = this->StartModify();
  this->RemoveAllViewNodeIDs();
  this->AddViewNodeID(viewNodeID);
  this->EndModify(disabledModify);
}

//-------------------------------------------------------
void vtkDMMLDisplayNode
::SetViewNodeIDs(const std::vector<std::string>& viewNodeIDs)
{
  std::vector<std::string>::const_iterator sourceIt =
    viewNodeIDs.begin();
  std::vector<std::string>::iterator destIt =
    this->ViewNodeIDs.begin();
  bool different = false;
  for (; sourceIt != viewNodeIDs.end(); ++sourceIt, ++destIt)
    {
    if (destIt == this->ViewNodeIDs.end())
      {
      different = true;
      }
    else if (*sourceIt != *destIt)
      {
      different = true;
      destIt = this->ViewNodeIDs.erase(destIt);
      }
    if (different)
      {
      destIt = this->ViewNodeIDs.insert(destIt, *sourceIt);
      }
    }
  if (destIt != this->ViewNodeIDs.end())
    {
    different = true;
    this->ViewNodeIDs.erase(destIt, this->ViewNodeIDs.end());
    }
  if (different)
    {
    this->Modified();
    }
}

//-------------------------------------------------------
bool vtkDMMLDisplayNode::GetVisibility(const char* viewNodeID)
{
  if (!viewNodeID
    || !this->GetVisibility()
    || !this->IsDisplayableInView(viewNodeID))
    {
    return false;
    }

  // If parent folder visibility is set to false then the markups is not visible
  if (this->GetFolderDisplayOverrideAllowed())
    {
    vtkDMMLDisplayableNode* displayableNode = this->GetDisplayableNode();
    // Visibility is applied regardless the fact whether there is override or not.
    // Visibility of items defined by hierarchy is off if any of the ancestors is explicitly hidden.
    // However, this does not apply on display nodes that do not allow overrides (FolderDisplayOverrideAllowed)
    if (!vtkDMMLFolderDisplayNode::GetHierarchyVisibility(displayableNode))
      {
      return false;
      }
    }
  vtkDMMLNode* viewNode = this->GetScene() ? this->GetScene()->GetNodeByID(viewNodeID) : nullptr;
  if (vtkDMMLSliceNode::SafeDownCast(viewNode))
    {
    if (!this->GetVisibility2D())
      {
      return false;
      }
    }
  if (vtkDMMLViewNode::SafeDownCast(viewNode))
    {
    if (!this->GetVisibility3D())
      {
      return false;
      }
    }
  return true;
}

//-------------------------------------------------------
void vtkDMMLDisplayNode::SetAutoScalarRange(int b)
{
  if (b)
    {
    this->AutoScalarRangeOn();
    }
  else
    {
    this->AutoScalarRangeOff();
    }
}

//-------------------------------------------------------
int vtkDMMLDisplayNode::GetAutoScalarRange()
{
  return (this->GetScalarRangeFlag() == vtkDMMLDisplayNode::UseDataScalarRange);
}

//-------------------------------------------------------
void vtkDMMLDisplayNode::AutoScalarRangeOn()
{
  this->SetScalarRangeFlag(vtkDMMLDisplayNode::UseDataScalarRange);
}

//-------------------------------------------------------
void vtkDMMLDisplayNode::AutoScalarRangeOff()
{
  this->SetScalarRangeFlag(vtkDMMLDisplayNode::UseManualScalarRange);
}

//-------------------------------------------------------
const char* vtkDMMLDisplayNode::GetScalarRangeFlagTypeAsString(int flag)
{
  switch (flag)
    {
    case UseManualScalarRange: return "UseManual";
    case UseDataScalarRange: return "UseData";
    case UseColorNodeScalarRange: return "UseColorNode";
    case UseDataTypeScalarRange: return "UseDataType";
    case UseDirectMapping: return "UseDirectMapping";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLDisplayNode::GetScalarRangeFlagTypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < vtkDMMLDisplayNode::NUM_SCALAR_RANGE_FLAGS; i++)
    {
    if (strcmp(name, vtkDMMLDisplayNode::GetScalarRangeFlagTypeAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // name not found
  return -1;
}

//-----------------------------------------------------------
const char* vtkDMMLDisplayNode::GetScalarRangeFlagAsString()
{
  return this->GetScalarRangeFlagTypeAsString(this->ScalarRangeFlag);
}

//-----------------------------------------------------------
void vtkDMMLDisplayNode::SetScalarRangeFlagFromString(const char* str)
{
  int scalarRangeFlag = this->GetScalarRangeFlagTypeFromString(str);
  if (scalarRangeFlag < 0)
    {
    // invalid value, maybe legacy scene
    scalarRangeFlag = atoi(str);
    }
  this->SetScalarRangeFlag(scalarRangeFlag);
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayNode::SetScalarRangeFlag(int flag)
{
  if (flag == this->ScalarRangeFlag)
    {
    return;
    }
  DMMLNodeModifyBlocker blocker(this);
  this->ScalarRangeFlag = flag;
  this->Modified();
  this->UpdateScalarRange();
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayNode::UpdateScalarRange()
{
  if (!this->GetScalarDataSet())
    {
    return;
    }

  if (this->GetScalarRangeFlag() == vtkDMMLDisplayNode::UseManualScalarRange)
    {
    return;
    }

  double newScalarRange[2] = { 0.0, -1.0 };
  int flag = this->GetScalarRangeFlag();
  if (flag == vtkDMMLDisplayNode::UseDataScalarRange)
    {
    vtkDataArray *dataArray = this->GetActiveScalarArray();
    if (dataArray)
      {
      dataArray->GetRange(newScalarRange);
      }
    }
  else if (flag == vtkDMMLDisplayNode::UseColorNodeScalarRange)
    {
    if (this->GetColorNode())
      {
      vtkLookupTable* lut = this->GetColorNode()->GetLookupTable();
      if (lut)
        {
        double* lutRange = lut->GetRange();
        newScalarRange[0] = lutRange[0];
        newScalarRange[1] = lutRange[1];
        }
      else
        {
        vtkWarningMacro("Can not use color node scalar range since model "
                        << "display node color node does not have a lookup table.");
        }
      }
    else
      {
      vtkWarningMacro("Can not use color node scalar range since model "
                      << "display node does not have a color node.");
      }
    }
  else if (flag == vtkDMMLDisplayNode::UseDataTypeScalarRange)
    {
    vtkDataArray *dataArray = this->GetActiveScalarArray();
    if (dataArray)
      {
      newScalarRange[0] = dataArray->GetDataTypeMin();
      newScalarRange[1] = dataArray->GetDataTypeMax();
      }
    else
      {
      vtkWarningMacro("Can not use data type scalar range since the model display node's"
                      << "mesh does not have an active scalar array.");
      }
    }

  this->SetScalarRange(newScalarRange);
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayNode::SetActiveScalar(const char *scalarName, int location)
{
  if (location == this->ActiveAttributeLocation
    && ((scalarName && this->ActiveScalarName && !strcmp(scalarName, this->ActiveScalarName))
        || (scalarName == nullptr && this->ActiveScalarName == nullptr)))
    {
    // no change
    return;
    }
  DMMLNodeModifyBlocker blocker(this);
  this->SetActiveScalarName(scalarName);
  this->SetActiveAttributeLocation(location);
  this->UpdateAssignedAttribute();
}

//-----------------------------------------------------------
const char* vtkDMMLDisplayNode::GetAttributeLocationAsString(int id)
{
  switch (id)
    {
    case vtkAssignAttribute::POINT_DATA: return "point";
    case vtkAssignAttribute::CELL_DATA: return "cell";
    case vtkAssignAttribute::VERTEX_DATA: return "vertex";
    case vtkAssignAttribute::EDGE_DATA: return "edge";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLDisplayNode::GetAttributeLocationFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < vtkAssignAttribute::NUM_ATTRIBUTE_LOCS; i++)
    {
    if (strcmp(name, vtkDMMLDisplayNode::GetAttributeLocationAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // name not found
  return -1;
}

//-----------------------------------------------------------
const char* vtkDMMLDisplayNode::GetActiveAttributeLocationAsString()
{
  return vtkDMMLDisplayNode::GetAttributeLocationAsString(this->GetActiveAttributeLocation());
}

//-----------------------------------------------------------
void vtkDMMLDisplayNode::SetActiveAttributeLocationFromString(const char* str)
{
  int id = this->GetAttributeLocationFromString(str);
  if (id<0)
    {
    vtkWarningMacro("Invalid activeAttributeLocation: " << (str ? str : "(none)"));
    }
  else
    {
    this->SetActiveAttributeLocation(id);
    }
}

//-----------------------------------------------------------
void vtkDMMLDisplayNode::SetSliceIntersectionVisibility(int on)
{
  vtkWarningMacro("SetSliceIntersectionVisibility method is deprecated, please use SetVisibility2D instead");
  this->SetVisibility2D(on);
}

//-----------------------------------------------------------
int vtkDMMLDisplayNode::GetSliceIntersectionVisibility()
{
  vtkDebugMacro("GetSliceIntersectionVisibility method is deprecated, please use GetVisibility2D instead");
  return this->GetVisibility2D();
}

//-----------------------------------------------------------
void vtkDMMLDisplayNode::SliceIntersectionVisibilityOn()
{
  vtkWarningMacro("SliceIntersectionVisibilityOn method is deprecated, please use Visibility2DOn instead");
  this->Visibility2DOn();
}

//-----------------------------------------------------------
void vtkDMMLDisplayNode::SliceIntersectionVisibilityOff()
{
  vtkWarningMacro("SliceIntersectionVisibilityOff method is deprecated, please use Visibility2DOff instead");
  this->Visibility2DOff();
}

//---------------------------------------------------------------------------
std::string vtkDMMLDisplayNode::GetTextPropertyAsString(vtkTextProperty* textProperty)
{
  if (!textProperty)
    {
    return "";
    }

  std::stringstream ss;
  ss << "font-family:" << textProperty->GetFontFamilyAsString() << ";";
  if (textProperty->GetFontFile())
    {
    ss << "font-file:" << textProperty->GetFontFile() << ";";
    }
  ss << "font-size:" << textProperty->GetFontSize() << "px;";
  ss << "font-style:" << (textProperty->GetItalic() ? "italic" : "normal") << ";";
  ss << "font-weight:" << (textProperty->GetBold() ? "bold" : "normal") << ";";
  ss << "color:rgba(";
  for (int i = 0; i < 3; ++i)
    {
    ss << static_cast<int>(std::floor(textProperty->GetColor()[i] * 255)) << ",";
    }
  ss << textProperty->GetOpacity() << ");";
  ss << "background-color:rgba(";
  for (int i = 0; i < 3; ++i)
    {
    ss << static_cast<int>(std::floor(textProperty->GetBackgroundColor()[i] * 255))  << ",";
    }
  ss << textProperty->GetBackgroundOpacity() << ");";
  ss << "border-width:" << textProperty->GetFrameWidth() << "px;";
  ss << "border-color:rgba(";
  for (int i = 0; i < 3; ++i)
    {
    ss << static_cast<int>(std::floor(textProperty->GetFrameColor()[i] * 255)) << ",";
    }
  ss << (textProperty->GetFrame() ? "1.0" : "0.0") << ");";
  ss << "text-shadow:";
  ss << textProperty->GetShadowOffset()[0] << "px " << textProperty->GetShadowOffset()[1] << "px ";
  ss << "2px "; // blur radius (used in CSS but not supported in VTK yet)
  ss << "rgba(0,0,0," << (textProperty->GetShadow() ? "1.0" : "0.0") << ");";
  return ss.str();
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayNode::UpdateTextPropertyFromString(std::string inputString, vtkTextProperty* textProperty)
{
  if (!textProperty)
    {
    return ;
    }

  std::vector<std::string> textProperties = vtksys::SystemTools::SplitString(inputString, ';');
  for (std::string textPropertyString : textProperties)
    {
    std::vector<std::string> keyValue = vtksys::SystemTools::SplitString(textPropertyString, ':');
    if (keyValue.empty())
      {
      continue;
      }
    std::string key = keyValue[0];

    if (keyValue.size() < 2)
      {
      continue;
      }

    std::string value = keyValue[1];
    if (key == "font-family")
      {
      textProperty->SetFontFamilyAsString(value.c_str());
      }
    else if (key == "font-file")
      {
      textProperty->SetFontFile(value.c_str());
      }
    else if (key == "font-size")
      {
      size_t pos = value.find("px");
      std::stringstream ss;
      vtkVariant size = vtkVariant(value.substr(0, pos));
      textProperty->SetFontSize(size.ToInt());
      }
    else if (key == "font-style")
      {
      textProperty->SetItalic(value == "italic");
      }
    else if (key == "font-weight")
      {
      textProperty->SetBold(value == "bold");
      }
    else if (key == "color")
      {
      double colorF[4] = { 0.0, 0.0, 0.0, 0.0};
      vtkDMMLDisplayNode::GetColorFromString(value, colorF);
      textProperty->SetColor(colorF);
      textProperty->SetOpacity(colorF[3]);
      }
    else if (key == "background-color")
      {
      double colorF[4] = { 0.0, 0.0, 0.0, 0.0 };
      vtkDMMLDisplayNode::GetColorFromString(value, colorF);
      textProperty->SetBackgroundColor(colorF);
      textProperty->SetBackgroundOpacity(colorF[3]);
      }
    else if (key == "border-color")
      {
      double colorF[4] = { 0.0, 0.0, 0.0, 0.0 };
      vtkDMMLDisplayNode::GetColorFromString(value, colorF);
      textProperty->SetFrameColor(colorF);
      textProperty->SetFrame(colorF[3] > 0.0);
      }
    else if (key == "border-width")
      {
      size_t pos = value.find("px");
      std::stringstream ss;
      vtkVariant size = vtkVariant(value.substr(0, pos));
      textProperty->SetFrameWidth(size.ToInt());
      }
    else if (key == "text-shadow")
      {
      std::vector<std::string> shadowProperties = vtksys::SystemTools::SplitString(keyValue[1], ' ');
      int shadowOffset[2] = { 2, 2 };
      int shadowPropertyIndex = 0;
      for (std::string shadowProperty : shadowProperties)
        {
        if (shadowPropertyIndex == SHADOW_H_OFFSET_INDEX || shadowPropertyIndex == SHADOW_V_OFFSET_INDEX)
          {
          size_t pos = shadowProperty.find("px");
          std::stringstream ss;
          vtkVariant offset = vtkVariant(shadowProperty.substr(0, pos));
          if (shadowPropertyIndex == SHADOW_H_OFFSET_INDEX)
            {
            shadowOffset[0] = offset.ToInt();
            }
          else
            {
            shadowOffset[1] = offset.ToInt();
            }
          }
        else if (shadowPropertyIndex == SHADOW_COLOR_INDEX)
          {
          // The shadow color in vtkTextProperty is actually just calculated from the text color (either black or white)
          // and cannot be changed.
          // We use the opacity channel to determine if the shadow should be displayed.
          double shadowColorF[4] = { 0.0, 0.0, 0.0, 0.0 };
          vtkDMMLDisplayNode::GetColorFromString(shadowProperty, shadowColorF);
          textProperty->SetShadow(shadowColorF[3] > 0.0);
          }
        ++shadowPropertyIndex;
        }
        textProperty->SetShadowOffset(shadowOffset);
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLDisplayNode::GetColorFromString(const std::string& inputString, double colorF[4])
{
  std::string colorString = inputString;
  std::string prefixString = "rgba(";

  size_t pos = colorString.find(prefixString);
  if (pos != std::string::npos)
    {
    colorString = colorString.substr(pos + prefixString.size(), colorString.size());
    }

  pos = colorString.find(")");
  if (pos != std::string::npos)
    {
    colorString = colorString.substr(0, pos);
    }

  std::vector<std::string> componentStrings = vtksys::SystemTools::SplitString(colorString, ',');
  for (int i = 0; i < 4 && i <static_cast<int>(componentStrings.size()); ++i)
    {
    double componentF = vtkVariant(componentStrings[i]).ToDouble();
    if (i != 3)
      {
      componentF /= 255.0;
      }
    colorF[i] = componentF;
    }
}

//---------------------------------------------------------------------------
const char* vtkDMMLDisplayNode::GetShowModeAsString(int id)
{
  switch (id)
    {
    case ShowDefault: return "Default";
    case ShowIgnore: return "Ignore";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLDisplayNode::GetShowModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < ShowMode_Last; ii++)
    {
    if (strcmp(name, GetShowModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}
