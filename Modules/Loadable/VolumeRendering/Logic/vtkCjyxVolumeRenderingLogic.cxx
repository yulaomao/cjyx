/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkCjyxVolumeRenderingLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkCjyxConfigure.h" // Cjyx_VTK_RENDERING_USE_{OpenGL|OpenGL2}_BACKEND

// Volume Rendering includes
#include "vtkDMMLSliceLogic.h"
#include "vtkDMMLVolumeRenderingDisplayNode.h"
#include "vtkCjyxVolumeRenderingLogic.h"
#include "vtkDMMLCPURayCastVolumeRenderingDisplayNode.h"
#include "vtkDMMLGPURayCastVolumeRenderingDisplayNode.h"
#include "vtkDMMLMultiVolumeRenderingDisplayNode.h"

// Annotations includes
#include <vtkDMMLAnnotationROINode.h>

// Markups includes
#include <vtkDMMLMarkupsROINode.h>

// DMML includes
#include <vtkCacheManager.h>
#include <vtkDMMLColorNode.h>
#include <vtkDMMLLabelMapVolumeDisplayNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLViewNode.h>
#include <vtkDMMLVectorVolumeDisplayNode.h>
#include <vtkDMMLVectorVolumeNode.h>
#include <vtkDMMLVolumePropertyNode.h>
#include <vtkDMMLVolumePropertyStorageNode.h>
#include <vtkDMMLShaderPropertyNode.h>
#include <vtkDMMLShaderPropertyStorageNode.h>

// VTKSYS includes
#include <itksys/SystemTools.hxx>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkVolumeProperty.h>

#if defined(Cjyx_VTK_RENDERING_USE_OpenGL_BACKEND)
#include <vtkOpenGLExtensionManager.h>
#include <vtkgl.h>
#endif

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxVolumeRenderingLogic);

//----------------------------------------------------------------------------
vtkCjyxVolumeRenderingLogic::vtkCjyxVolumeRenderingLogic()
{
  this->DefaultRenderingMethod = nullptr;
  this->UseLinearRamp = true;
  this->PresetsScene = nullptr;
  this->DefaultROIClassName = "vtkDMMLMarkupsROINode";

  this->RegisterRenderingMethod("VTK CPU Ray Casting",
    "vtkDMMLCPURayCastVolumeRenderingDisplayNode");
  this->RegisterRenderingMethod("VTK GPU Ray Casting",
    "vtkDMMLGPURayCastVolumeRenderingDisplayNode");
  this->RegisterRenderingMethod("VTK Multi-Volume (experimental)",
    "vtkDMMLMultiVolumeRenderingDisplayNode");
}

//----------------------------------------------------------------------------
vtkCjyxVolumeRenderingLogic::~vtkCjyxVolumeRenderingLogic()
{
  if (this->DefaultRenderingMethod)
  {
    delete [] this->DefaultRenderingMethod;
  }
  if (this->PresetsScene)
  {
    this->PresetsScene->Delete();
  }
  this->RemoveAllVolumeRenderingDisplayNodes();
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Display nodes:" << std::endl;
  for (unsigned int i = 0; i < this->DisplayNodes.size(); ++i)
  {
    os << indent << this->DisplayNodes[i]->GetID() << std::endl;
  }
#if defined(Cjyx_VTK_RENDERING_USE_OpenGL_BACKEND)
  const char *gl_vendor=reinterpret_cast<const char *>(glGetString(GL_VENDOR));
  os << indent << "Vendor: " << gl_vendor << std::endl;
  const char *gl_version=reinterpret_cast<const char *>(glGetString(GL_VERSION));
  os << indent << "Version: " << gl_version << std::endl;
  const char *glsl_version=
    reinterpret_cast<const char *>(glGetString(vtkgl::SHADING_LANGUAGE_VERSION));
  os << indent << "Shading Language Version: " << glsl_version << std::endl;
#endif
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::RegisterNodes()
{
  if(!this->GetDMMLScene())
  {
    vtkWarningMacro("RegisterNodes: No DMML scene.");
    return;
  }

  vtkNew<vtkDMMLVolumePropertyNode> vpn;
  this->GetDMMLScene()->RegisterNodeClass( vpn.GetPointer() );

  vtkNew<vtkDMMLVolumePropertyStorageNode> vpsn;
  this->GetDMMLScene()->RegisterNodeClass( vpsn.GetPointer() );

  vtkNew<vtkDMMLShaderPropertyNode> spn;
  this->GetDMMLScene()->RegisterNodeClass( spn.GetPointer() );

  vtkNew<vtkDMMLShaderPropertyStorageNode> spsn;
  this->GetDMMLScene()->RegisterNodeClass( spsn.GetPointer() );

  vtkNew<vtkDMMLCPURayCastVolumeRenderingDisplayNode> cpuVRNode;
  this->GetDMMLScene()->RegisterNodeClass( cpuVRNode.GetPointer() );
  // Volume rendering nodes used to have the tag "VolumeRenderingParameters"
  // in scenes prior to Cjyx 4.2
#if DMML_APPLICATION_SUPPORT_VERSION < DMML_VERSION_CHECK(4, 2, 0)
  this->GetDMMLScene()->RegisterNodeClass( cpuVRNode.GetPointer(), "VolumeRenderingParameters");
#endif

  vtkNew<vtkDMMLGPURayCastVolumeRenderingDisplayNode> gpuNode;
  this->GetDMMLScene()->RegisterNodeClass( gpuNode.GetPointer() );

  vtkNew<vtkDMMLMultiVolumeRenderingDisplayNode> multiNode;
  this->GetDMMLScene()->RegisterNodeClass( multiNode.GetPointer() );
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::RegisterRenderingMethod(const char* methodName, const char* displayNodeClassName)
{
  this->RenderingMethods[methodName] = displayNodeClassName;
  this->Modified();
}

//----------------------------------------------------------------------------
std::map<std::string, std::string> vtkCjyxVolumeRenderingLogic::GetRenderingMethods()
{
  return this->RenderingMethods;
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::AddVolumeRenderingDisplayNode(vtkDMMLVolumeRenderingDisplayNode* node)
{
  DisplayNodesType::iterator it = std::find(this->DisplayNodes.begin(), this->DisplayNodes.end(), node);
  if (it != this->DisplayNodes.end())
  {
    // already added
    return;
  }
  // push empty...
  it = this->DisplayNodes.insert(this->DisplayNodes.end(), static_cast<vtkDMMLNode*>(nullptr));
  // .. then set and observe
  vtkSetAndObserveDMMLNodeMacro(*it, node);

  //Don't update volume rendering while a scene is being imported
  if (!this->GetDMMLScene()->IsImporting())
  {
    this->UpdateVolumeRenderingDisplayNode(node);
  }

}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::RemoveVolumeRenderingDisplayNode(vtkDMMLVolumeRenderingDisplayNode* node)
{
  DisplayNodesType::iterator it = std::find(this->DisplayNodes.begin(), this->DisplayNodes.end(), node);
  if (it == this->DisplayNodes.end())
  {
    return;
  }
  // unobserve
  vtkSetAndObserveDMMLNodeMacro(*it, 0);
  this->DisplayNodes.erase(it);
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::RemoveAllVolumeRenderingDisplayNodes()
{
  while (!this->DisplayNodes.empty())
  {
    this->RemoveVolumeRenderingDisplayNode(vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(this->DisplayNodes[0]));
  }
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::AddAllVolumeRenderingDisplayNodes()
{
  if (!this->GetDMMLScene())
  {
    return;
  }
  std::vector<vtkDMMLNode*> volumeRenderingDisplayNodes;
  this->GetDMMLScene()->GetNodesByClass("vtkDMMLVolumeRenderingDisplayNode", volumeRenderingDisplayNodes);
  std::vector<vtkDMMLNode*>::const_iterator it;
  for (it = volumeRenderingDisplayNodes.begin(); it != volumeRenderingDisplayNodes.end(); ++it)
  {
    this->AddVolumeRenderingDisplayNode(vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(*it));
  }
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::ChangeVolumeRenderingMethod(const char* displayNodeClassName/*=0*/)
{
  if (this->DisplayNodes.empty())
  {
    // There are no display nodes, nothing to do
    return;
  }
  if (!this->GetDMMLScene())
  {
    return;
  }
  if (displayNodeClassName == nullptr || strlen(displayNodeClassName) == 0)
  {
    displayNodeClassName = this->DefaultRenderingMethod;
  }
  else
  {
    // In case of a non-empty class name set the default rendering method
    this->SetDefaultRenderingMethod(displayNodeClassName);
  }
  if (displayNodeClassName == nullptr || strlen(displayNodeClassName) == 0)
  {
    displayNodeClassName = "vtkDMMLGPURayCastVolumeRenderingDisplayNode";
  }
  if (!strcmp(this->DisplayNodes[0]->GetClassName(), displayNodeClassName))
  {
    // Type of existing display nodes match the requested type, nothing to do
    return;
  }

  this->GetDMMLScene()->StartState(vtkDMMLScene::BatchProcessState);

  // Create a display node of the requested type for all existing display nodes
  DisplayNodesType displayNodesCopy(this->DisplayNodes);
  DisplayNodesType::iterator displayIt;
  for (displayIt = displayNodesCopy.begin(); displayIt != displayNodesCopy.end(); ++displayIt)
  {
    vtkDMMLVolumeRenderingDisplayNode* oldDisplayNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(*displayIt);
    if (!oldDisplayNode)
    {
      // node may have been deleted
      continue;
    }
    vtkSmartPointer<vtkDMMLVolumeRenderingDisplayNode> newDisplayNode =
      vtkSmartPointer<vtkDMMLVolumeRenderingDisplayNode>::Take(this->CreateVolumeRenderingDisplayNode(displayNodeClassName));
    if (!newDisplayNode)
    {
      vtkErrorMacro("ChangeVolumeRenderingMethod: Failed to create display node of type " << displayNodeClassName);
      continue;
    }
    this->GetDMMLScene()->AddNode(newDisplayNode);
    newDisplayNode->vtkDMMLVolumeRenderingDisplayNode::Copy(oldDisplayNode);
    vtkDMMLDisplayableNode* displayableNode = oldDisplayNode->GetDisplayableNode();
    this->GetDMMLScene()->RemoveNode(oldDisplayNode);
    // Assign updated display node to displayable node.
    // There may be orphan volume rendering display nodes in the scene (without being assigned to a displayable node),
    // but we leave them alone, as maybe they are just temporarily not used.
    if (displayableNode)
    {
      displayableNode->AddAndObserveDisplayNodeID(newDisplayNode->GetID());
    }
  }

  this->GetDMMLScene()->EndState(vtkDMMLScene::BatchProcessState);
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::UpdateVolumeRenderingDisplayNode(vtkDMMLVolumeRenderingDisplayNode* node)
{
  if (!node)
    {
    vtkWarningMacro("UpdateVolumeRenderingDisplayNode: Volume Rendering display node does not exist.");
    return;
    }
  vtkDMMLVolumeNode* volumeNode = node->GetVolumeNode();
  if (!volumeNode)
    {
    return;
    }
  vtkDMMLVolumeDisplayNode* volumeDisplayNode = volumeNode->GetVolumeDisplayNode();
  if (!volumeDisplayNode)
    {
    return;
    }
  if (node->GetFollowVolumeDisplayNode())
    {
    // observe display node if not already observing it
    if (!this->GetDMMLNodesObserverManager()->GetObservationsCount(volumeDisplayNode))
      {
      vtkObserveDMMLNodeMacro(volumeDisplayNode);
      }
    this->CopyDisplayToVolumeRenderingDisplayNode(node);
    }
  else
    {
    // unobserve display node if no volume rendering display nodes follow it anymore
    bool needToObserveVolumeDisplayNode = false;
    int ndnodes = volumeNode->GetNumberOfDisplayNodes();
    for (int i = 0; i < ndnodes; i++)
      {
      vtkDMMLVolumeRenderingDisplayNode* vrDisplayNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(volumeNode->GetNthDisplayNode(i));
      if (vrDisplayNode && node->GetFollowVolumeDisplayNode())
        {
        needToObserveVolumeDisplayNode = true;
        break;
        }
      }
    if (!needToObserveVolumeDisplayNode)
      {
      vtkUnObserveDMMLNodeMacro(volumeDisplayNode);
      }
    }
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::SetDMMLSceneInternal(vtkDMMLScene* scene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  this->SetAndObserveDMMLSceneEventsInternal(scene, sceneEvents.GetPointer());
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::ObserveDMMLScene()
{
  this->RemoveAllVolumeRenderingDisplayNodes();
  this->AddAllVolumeRenderingDisplayNodes();
  this->Superclass::ObserveDMMLScene();
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  vtkDMMLVolumeRenderingDisplayNode* vrDisplayNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(node);
  if (vrDisplayNode)
  {
    this->AddVolumeRenderingDisplayNode(vrDisplayNode);
  }
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  vtkDMMLVolumeRenderingDisplayNode* vrDisplayNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(node);
  if (vrDisplayNode)
  {
    this->RemoveVolumeRenderingDisplayNode(vrDisplayNode);
  }
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::OnDMMLNodeModified(vtkDMMLNode* node)
{

  // If volume rendering display node changes then we may need to
  // add/remove volume display node observer.
  vtkDMMLVolumeRenderingDisplayNode* vrDisplayNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(node);
  if (vrDisplayNode)
    {
    this->UpdateVolumeRenderingDisplayNode(vrDisplayNode);
    }


  // If volume display node is changed then update all volume rendering display nodes that follow it
  vtkDMMLVolumeDisplayNode* volumeDisplayNode = vtkDMMLVolumeDisplayNode::SafeDownCast(node);
  if (volumeDisplayNode)
    {
    for (unsigned int i = 0; i < this->DisplayNodes.size(); ++i)
      {
      vrDisplayNode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(this->DisplayNodes[i]);
      if (!vrDisplayNode || !vrDisplayNode->GetFollowVolumeDisplayNode())
        {
        continue;
        }
      this->CopyDisplayToVolumeRenderingDisplayNode(vrDisplayNode, volumeDisplayNode);
      }
    }
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::UpdateTranferFunctionRangeFromImage(vtkDMMLVolumeRenderingDisplayNode* vspNode)
{
  vtkDebugMacro("vtkCjyxVolumeRenderingLogic::UpdateTranferFunctionRangeFromImage()");
  if (vspNode == nullptr || vspNode->GetVolumeNode() == nullptr || vspNode->GetVolumePropertyNode() == nullptr)
  {
    return;
  }
  vtkImageData *input = vtkDMMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData();
  vtkVolumeProperty *prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();
  if (input == nullptr || prop == nullptr)
  {
    return;
  }

  //update scalar range
  vtkColorTransferFunction *functionColor = prop->GetRGBTransferFunction();

  vtkDataArray* scalars = input->GetPointData()->GetScalars();
  if (!scalars)
  {
    return;
  }

  double rangeNew[2];
  scalars->GetRange(rangeNew);
  functionColor->AdjustRange(rangeNew);
  vtkDebugMacro("Color range: "<< functionColor->GetRange()[0] << " " << functionColor->GetRange()[1]);

  vtkPiecewiseFunction *functionOpacity = prop->GetScalarOpacity();
  functionOpacity->AdjustRange(rangeNew);

  vtkDebugMacro("Opacity range: " << functionOpacity->GetRange()[0] << " " << functionOpacity->GetRange()[1]);

  rangeNew[1] = (rangeNew[1] - rangeNew[0])*0.25;
  rangeNew[0] = 0;

  functionOpacity = prop->GetGradientOpacity();
  functionOpacity->RemovePoint(255); //Remove the standard value
  functionOpacity->AdjustRange(rangeNew);
  vtkDebugMacro("Gradient Opacity range: " << functionOpacity->GetRange()[0] << " " << functionOpacity->GetRange()[1]);
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::SetThresholdToVolumeProp(
  double scalarRange[2], double threshold[2], vtkVolumeProperty* volumeProp, bool linearRamp, bool stayUpAtUpperLimit)
{

  if (!volumeProp || !scalarRange || !threshold)
  {
    vtkWarningMacro("SetThresholdToVolumeProp: Inputs do not exist.");
    return;
  }

  // Sanity check
  threshold[0] = std::max(std::min(threshold[0], scalarRange[1]), scalarRange[0]);
  threshold[1] = std::min(std::max(threshold[1], scalarRange[0]), scalarRange[1]);
  vtkDebugMacro("Threshold: " << threshold[0] << " " << threshold[1]);

  double previous = VTK_DOUBLE_MIN;

  vtkNew<vtkPiecewiseFunction> opacity;
  // opacity doesn't support duplicate points
  opacity->AddPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(scalarRange[0], previous), 0.0);
  opacity->AddPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(threshold[0], previous), 0.0);
  if (!linearRamp)
  {
    opacity->AddPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(threshold[0], previous), 1.0);
  }
  opacity->AddPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(threshold[1], previous), 1.0);
  double endValue = stayUpAtUpperLimit ? 1.0 : 0.0;
  if (!stayUpAtUpperLimit)
  {
    opacity->AddPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(threshold[1], previous), endValue);
  }
  opacity->AddPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(scalarRange[1], previous), endValue);

  vtkPiecewiseFunction *volumePropOpacity = volumeProp->GetScalarOpacity();
  if (this->IsDifferentFunction(opacity.GetPointer(), volumePropOpacity))
  {
    volumePropOpacity->DeepCopy(opacity.GetPointer());
  }
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::SetWindowLevelToVolumeProp(
  double scalarRange[2], double windowLevel[2], vtkLookupTable* lut, vtkVolumeProperty* volumeProp)
{
  if (!volumeProp || !scalarRange || !windowLevel)
  {
    vtkWarningMacro("SetWindowLevelToVolumeProp: Inputs do not exist.");
    return;
  }

  double windowLevelMinMax[2];
  windowLevelMinMax[0] = windowLevel[1] - 0.5 * windowLevel[0];
  windowLevelMinMax[1] = windowLevel[1] + 0.5 * windowLevel[0];

  double previous = VTK_DOUBLE_MIN;

  vtkNew<vtkColorTransferFunction> colorTransfer;

  const int size = lut ? lut->GetNumberOfTableValues() : 0;
  if (size == 0)
  {
    const double black[3] = {0., 0., 0.};
    const double white[3] = {1., 1., 1.};
    colorTransfer->AddRGBPoint(scalarRange[0], black[0], black[1], black[2]);
    colorTransfer->AddRGBPoint(windowLevelMinMax[0], black[0], black[1], black[2]);
    colorTransfer->AddRGBPoint(windowLevelMinMax[1], white[0], white[1], white[2]);
    colorTransfer->AddRGBPoint(scalarRange[1], white[0], white[1], white[2]);
  }
  else if (size == 1)
  {
    double color[4];
    lut->GetTableValue(0, color);

    colorTransfer->AddRGBPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(scalarRange[0], previous),
      color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(windowLevelMinMax[0], previous),
      color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(windowLevelMinMax[1], previous),
      color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(scalarRange[1], previous),
      color[0], color[1], color[2]);
  }
  else // if (size > 1)
  {
    previous = VTK_DOUBLE_MIN;

    double color[4];
    lut->GetTableValue(0, color);
    colorTransfer->AddRGBPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(scalarRange[0], previous),
      color[0], color[1], color[2]);

    double value = windowLevelMinMax[0];

    double step = windowLevel[0] / (size - 1);

    int downSamplingFactor = 64;
    for (int i = 0; i < size; i += downSamplingFactor,
                              value += downSamplingFactor*step)
    {
      lut->GetTableValue(i, color);
      colorTransfer->AddRGBPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(value, previous),
        color[0], color[1], color[2]);
    }

    lut->GetTableValue(size - 1, color);
    colorTransfer->AddRGBPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(windowLevelMinMax[1], previous),
      color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(scalarRange[1], previous),
      color[0], color[1], color[2]);
  }

  vtkColorTransferFunction *volumePropColorTransfer = volumeProp->GetRGBTransferFunction();
  if (this->IsDifferentFunction(colorTransfer.GetPointer(), volumePropColorTransfer))
  {
    volumePropColorTransfer->DeepCopy(colorTransfer.GetPointer());
  }

  volumeProp->SetInterpolationTypeToLinear();
  volumeProp->ShadeOn();
  volumeProp->SetAmbient(0.30);
  volumeProp->SetDiffuse(0.60);
  volumeProp->SetSpecular(0.50);
  volumeProp->SetSpecularPower(40);
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::SetGradientOpacityToVolumeProp(double scalarRange[2], vtkVolumeProperty* volumeProp)
{
  if (!scalarRange || !volumeProp)
  {
    vtkWarningMacro("SetGradientOpacityToVolumeProp: Inputs do not exist.");
    return;
  }

  double previous = VTK_DOUBLE_MIN;
  vtkNew<vtkPiecewiseFunction> opacity;
  // opacity doesn't support duplicate points
  opacity->AddPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(scalarRange[0], previous), 1.0);
  opacity->AddPoint(vtkDMMLVolumePropertyNode::HigherAndUnique(scalarRange[1], previous), 1.0);

  vtkPiecewiseFunction *volumePropGradientOpacity = volumeProp->GetGradientOpacity();
  if (this->IsDifferentFunction(opacity.GetPointer(), volumePropGradientOpacity))
  {
    volumePropGradientOpacity->DeepCopy(opacity.GetPointer());
  }
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::SetLabelMapToVolumeProp(vtkScalarsToColors* colors, vtkVolumeProperty* volumeProp)
{
  if (!colors || !volumeProp)
  {
    vtkWarningMacro("SetLabelMapToVolumeProp: Inputs do not exist.");
    return;
  }

  vtkNew<vtkPiecewiseFunction> opacity;
  vtkNew<vtkColorTransferFunction> colorTransfer;

  vtkLookupTable* lut = vtkLookupTable::SafeDownCast(colors);
  const int colorCount = colors->GetNumberOfAvailableColors();
  double value = colors->GetRange()[0];
  double step = (colors->GetRange()[1] - colors->GetRange()[0] + 1.) / colorCount;
  double color[4] = {0., 0., 0., 1.};
  const double midPoint = 0.5;
  const double sharpness = 1.0;
  for (int i = 0; i < colorCount; ++i, value += step)
  {
    // Short circuit for luts as it is faster
    if (lut)
    {
      lut->GetTableValue(i, color);
    }
    else
    {
      colors->GetColor(value, color);
    }
    opacity->AddPoint(value, color[3], midPoint, sharpness);
    colorTransfer->AddRGBPoint(value, color[0], color[1], color[2], midPoint, sharpness);
  }

  vtkPiecewiseFunction *volumePropOpacity = volumeProp->GetScalarOpacity();
  if (this->IsDifferentFunction(opacity.GetPointer(), volumePropOpacity))
  {
    volumePropOpacity->DeepCopy(opacity.GetPointer());
  }

  vtkColorTransferFunction *volumePropColorTransfer = volumeProp->GetRGBTransferFunction();
  if (this->IsDifferentFunction(colorTransfer.GetPointer(), volumePropColorTransfer))
  {
    volumePropColorTransfer->DeepCopy(colorTransfer.GetPointer());
  }

  volumeProp->SetInterpolationTypeToNearest();
  volumeProp->ShadeOn();
  volumeProp->SetAmbient(0.30);
  volumeProp->SetDiffuse(0.60);
  volumeProp->SetSpecular(0.50);
  volumeProp->SetSpecularPower(40);
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::CopyDisplayToVolumeRenderingDisplayNode(
  vtkDMMLVolumeRenderingDisplayNode* vspNode, vtkDMMLVolumeDisplayNode* displayNode)
{
  if (!vspNode)
  {
    vtkWarningMacro("CopyDisplayToVolumeRenderingDisplayNode: Volume Rendering display node does not exist.");
    return;
  }
  if (!displayNode)
  {
    vtkDMMLVolumeNode* volumeNode = vspNode->GetVolumeNode();
    if (!volumeNode)
    {
      vtkWarningMacro("CopyDisplayToVolumeRenderingDisplayNode: Volume Rendering display node does not reference a volume node.");
      return;
    }
    displayNode = vtkDMMLVolumeDisplayNode::SafeDownCast(volumeNode->GetVolumeDisplayNode());
  }
  if (!displayNode)
  {
    vtkWarningMacro("CopyDisplayToVolumeRenderingDisplayNode: No display node to copy.");
    return;
  }
  if (vtkDMMLScalarVolumeDisplayNode::SafeDownCast(displayNode))
  {
    this->CopyScalarDisplayToVolumeRenderingDisplayNode(vspNode, vtkDMMLScalarVolumeDisplayNode::SafeDownCast(displayNode));
  }
  else if (vtkDMMLLabelMapVolumeDisplayNode::SafeDownCast(displayNode))
  {
    this->CopyLabelMapDisplayToVolumeRenderingDisplayNode(vspNode, vtkDMMLLabelMapVolumeDisplayNode::SafeDownCast(displayNode));
  }
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::CopyScalarDisplayToVolumeRenderingDisplayNode(
  vtkDMMLVolumeRenderingDisplayNode* vspNode, vtkDMMLScalarVolumeDisplayNode* vpNode)
{
  if (!vspNode)
  {
    vtkWarningMacro("CopyScalarDisplayToVolumeRenderingDisplayNode: No volume rendering display node.");
    return;
  }
  if (!vspNode->GetVolumePropertyNode())
  {
    vtkWarningMacro("CopyScalarDisplayToVolumeRenderingDisplayNode: No volume property node.");
    return;
  }

  if (!vpNode)
  {
    vpNode = vtkDMMLScalarVolumeDisplayNode::SafeDownCast(vspNode->GetVolumeNode()->GetVolumeDisplayNode());
  }

  if (!vpNode)
  {
    vtkWarningMacro("CopyScalarDisplayToVolumeRenderingDisplayNode: No volume display node.");
    return;
  }

  bool ignoreVolumeDisplayNodeThreshold = vspNode->GetIgnoreVolumeDisplayNodeThreshold();
  double scalarRange[2];
  vpNode->GetDisplayScalarRange(scalarRange);

  double windowLevel[2];
  windowLevel[0] = vpNode->GetWindow();
  windowLevel[1] = vpNode->GetLevel();

  double threshold[2];
  if (!ignoreVolumeDisplayNodeThreshold)
  {
    threshold[0] = vpNode->GetLowerThreshold();
    threshold[1] = vpNode->GetUpperThreshold();
  }
  else
  {
    threshold[0] = vpNode->GetWindowLevelMin();
    threshold[1] = vpNode->GetWindowLevelMax();
  }

  vtkLookupTable* lut = vpNode->GetColorNode() ? vpNode->GetColorNode()->GetLookupTable() : nullptr;
  vtkVolumeProperty *prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();

  int disabledModify = vspNode->StartModify();
  int vpNodeDisabledModify = vspNode->GetVolumePropertyNode()->StartModify();

  this->SetThresholdToVolumeProp(scalarRange, threshold, prop, this->UseLinearRamp, ignoreVolumeDisplayNodeThreshold);

  this->SetWindowLevelToVolumeProp(scalarRange, windowLevel, lut, prop);
  this->SetGradientOpacityToVolumeProp(scalarRange, prop);

  vspNode->GetVolumePropertyNode()->EndModify(vpNodeDisabledModify);
  vspNode->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::CopyLabelMapDisplayToVolumeRenderingDisplayNode(
  vtkDMMLVolumeRenderingDisplayNode* vspNode, vtkDMMLLabelMapVolumeDisplayNode* vpNode)
{
  if (!vspNode)
  {
    vtkWarningMacro("CopyLabelMapDisplayToVolumeRenderingDisplayNode: No volume rendering display node.");
    return;
  }
  if (!vspNode->GetVolumePropertyNode())
  {
    vtkWarningMacro("CopyLabelMapDisplayToVolumeRenderingDisplayNode: No volume property node.");
    return;
  }

  if (!vpNode)
  {
    vpNode = vtkDMMLLabelMapVolumeDisplayNode::SafeDownCast(vspNode->GetVolumeNode()->GetVolumeDisplayNode());
  }
  if (!vpNode)
  {
    vtkWarningMacro("CopyLabelMapDisplayToVolumeRenderingDisplayNode: No volume display node.");
    return;
  }

  vtkScalarsToColors* colors = vpNode->GetColorNode() ? vpNode->GetColorNode()->GetScalarsToColors() : nullptr;

  vtkVolumeProperty *prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();

  int disabledModify = vspNode->StartModify();
  int vpNodeDisabledModify = vspNode->GetVolumePropertyNode()->StartModify();

  this->SetLabelMapToVolumeProp(colors, prop);
  this->SetGradientOpacityToVolumeProp(colors->GetRange(), prop);

  vspNode->GetVolumePropertyNode()->EndModify(vpNodeDisabledModify);
  vspNode->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::FitROIToVolume(vtkDMMLVolumeRenderingDisplayNode* vspNode)
{
  // Resize the ROI to fit the volume
  vtkDMMLScalarVolumeNode *volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode());

  if (!volumeNode)
    {
    return;
    }

  vtkDMMLAnnotationROINode* roiNode = vspNode->GetAnnotationROINode();
  vtkDMMLMarkupsROINode* markupsROINode = vspNode->GetMarkupsROINode();
  if (markupsROINode)
    {
    DMMLNodeModifyBlocker blocker(markupsROINode);

    double xyz[3] = { 0.0 };
    double center[3] = { 0.0 };

    vtkDMMLSliceLogic::GetVolumeRASBox(volumeNode, xyz, center);
    for (int i = 0; i < 3; i++)
      {
      xyz[i] *= 0.5;
      }

    markupsROINode->GetObjectToNodeMatrix()->Identity();
    markupsROINode->SetXYZ(center);
    markupsROINode->SetRadiusXYZ(xyz);
    }
  else if (roiNode)
    {
    DMMLNodeModifyBlocker blocker(roiNode);

    double xyz[3] = {0.0};
    double center[3] = {0.0};

    vtkDMMLSliceLogic::GetVolumeRASBox(volumeNode, xyz, center);
    for (int i = 0; i < 3; i++)
      {
      xyz[i] *= 0.5;
      }

    roiNode->SetXYZ(center);
    roiNode->SetRadiusXYZ(xyz);
    }
}

//----------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode* vtkCjyxVolumeRenderingLogic::CreateDefaultVolumeRenderingNodes(vtkDMMLVolumeNode* volumeNode)
{
  if (!volumeNode)
  {
    vtkErrorMacro("CreateVolumeRenderingNodesForVolume: No volume node given");
    return nullptr;
  }
  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
  {
    vtkErrorMacro("CreateVolumeRenderingNodesForVolume: Invalid DMML scene");
    return nullptr;
  }

  vtkSmartPointer<vtkDMMLVolumeRenderingDisplayNode> displayNode = this->GetFirstVolumeRenderingDisplayNode(volumeNode);
  if (!displayNode)
  {
    displayNode = vtkSmartPointer<vtkDMMLVolumeRenderingDisplayNode>::Take(this->CreateVolumeRenderingDisplayNode());
    scene->AddNode(displayNode);

    // Add all 3D views to the display node
    std::vector<vtkDMMLNode*> viewNodes;
    scene->GetNodesByClass("vtkDMMLViewNode", viewNodes);
    for (std::vector<vtkDMMLNode*>::iterator nodeIt=viewNodes.begin(); nodeIt != viewNodes.end(); ++nodeIt)
    {
      displayNode->AddViewNodeID((*nodeIt)->GetID());
    }

    volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
  }
  if (!displayNode)
  {
    vtkErrorMacro("CreateVolumeRenderingNodesForVolume: Failed to create volume rendering display node for scalar volume node " << volumeNode->GetName());
    return nullptr;
  }

  vtkDMMLVolumePropertyNode* volumePropertyNode = displayNode->GetVolumePropertyNode();
  if (!volumePropertyNode)
  {
    this->UpdateDisplayNodeFromVolumeNode(displayNode, volumeNode, nullptr, nullptr, false);
    this->SetRecommendedVolumeRenderingProperties(displayNode);
    volumePropertyNode = displayNode->GetVolumePropertyNode();
  }
  if (!volumePropertyNode)
  {
    vtkErrorMacro("CreateVolumeRenderingNodesForVolume: Failed to create volume property node for scalar volume node " << volumeNode->GetName());
    return displayNode;
  }

  return displayNode;
}

//----------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode* vtkCjyxVolumeRenderingLogic::CreateVolumeRenderingDisplayNode(const char* renderingClassName)
{
  vtkDMMLVolumeRenderingDisplayNode *node = nullptr;

  if (this->GetDMMLScene() == nullptr)
  {
    return node;
  }
  bool volumeRenderingUniqueName = true;
  if (renderingClassName == nullptr || strlen(renderingClassName) == 0)
  {
    renderingClassName = this->DefaultRenderingMethod;
  }
  else
  {
    volumeRenderingUniqueName = false;
  }
  if (renderingClassName == nullptr || strlen(renderingClassName) == 0)
  {
    renderingClassName = "vtkDMMLGPURayCastVolumeRenderingDisplayNode";
  }
  node = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(this->GetDMMLScene()->CreateNodeByClass(renderingClassName));
  if (volumeRenderingUniqueName)
  {
    node->SetName(this->GetDMMLScene()->GenerateUniqueName("VolumeRendering").c_str());
  }

  return node;
}

// Description:
// Remove ViewNode from VolumeRenderingDisplayNode for a VolumeNode,
//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::RemoveViewFromVolumeDisplayNodes(
  vtkDMMLVolumeNode *volumeNode, vtkDMMLViewNode *viewNode)
{
  if (viewNode == nullptr || volumeNode == nullptr)
  {
    return;
  }

  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
  {
    vtkDMMLVolumeRenderingDisplayNode *dnode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode)
    {
      dnode->RemoveViewNodeID(viewNode->GetID());
    }
  }
}

// Description:
// Find volume rendering display node reference in the volume
//----------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode* vtkCjyxVolumeRenderingLogic::GetVolumeRenderingDisplayNodeByID(
  vtkDMMLVolumeNode *volumeNode, char *displayNodeID)
{
  if (displayNodeID == nullptr || volumeNode == nullptr)
  {
    return nullptr;
  }

  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
  {
    vtkDMMLVolumeRenderingDisplayNode *dnode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode && !strcmp(displayNodeID, dnode->GetID()))
    {
      return dnode;
    }
  }
  return nullptr;
}

// Description:
// Find first volume rendering display node
//----------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode* vtkCjyxVolumeRenderingLogic::GetFirstVolumeRenderingDisplayNode(vtkDMMLVolumeNode *volumeNode)
{
  if (volumeNode == nullptr)
    {
    return nullptr;
    }
  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < ndnodes; displayNodeIndex++)
    {
    vtkDMMLVolumeRenderingDisplayNode *dnode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(volumeNode->GetNthDisplayNode(displayNodeIndex));
    if (!dnode)
      {
      // not a volume rendering display node
      continue;
      }
    if (dnode->GetVolumeNode() != volumeNode)
      {
      // This display node is associated with another volume node as well.
      // Root cause is fixed in scene loading, so this should not happen anymore, but
      // log an error to help debugging in case this happens anyway.
      vtkErrorMacro("Invalid scene: " << dnode->GetID() << " is used by multiple volume nodes ("
        << (dnode->GetVolumeNode() && dnode->GetVolumeNode()->GetID() ? dnode->GetVolumeNode()->GetID() : "(unknown)")
        << " and " << (volumeNode && volumeNode->GetID() ? volumeNode->GetID() : "(unknown)") << ")");
      }
    return dnode;
    }
  return nullptr;
}

// Description:
// Find volume rendering display node referencing the view node and volume node
//----------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode* vtkCjyxVolumeRenderingLogic::GetVolumeRenderingDisplayNodeForViewNode(
  vtkDMMLVolumeNode *volumeNode, vtkDMMLViewNode *viewNode)
{
  if (viewNode == nullptr || volumeNode == nullptr)
  {
    return nullptr;
  }
  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
  {
    vtkDMMLVolumeRenderingDisplayNode *dnode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(volumeNode->GetNthDisplayNode(i));

    if (dnode // display node is not necessarily volume rendering display node.
      && dnode->IsDisplayableInView(viewNode->GetID()))
    {
      return dnode;
    }
  }
  return nullptr;
}

// Description:
// Find volume rendering display node referencing the view node in the scene
//----------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode* vtkCjyxVolumeRenderingLogic::GetVolumeRenderingDisplayNodeForViewNode(vtkDMMLViewNode *viewNode)
{
  if (viewNode == nullptr || viewNode->GetScene() == nullptr)
  {
    return nullptr;
  }
  std::vector<vtkDMMLNode *> nodes;
  viewNode->GetScene()->GetNodesByClass("vtkDMMLVolumeRenderingDisplayNode", nodes);

  for (unsigned int i=0; i<nodes.size(); i++)
  {
    vtkDMMLVolumeRenderingDisplayNode *dnode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(nodes[i]);
    if (dnode && dnode->IsViewNodeIDPresent(viewNode->GetID()))
    {
      return dnode;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLVolumeRenderingDisplayNode* vtkCjyxVolumeRenderingLogic
::GetFirstVolumeRenderingDisplayNodeByROINode(vtkDMMLNode* roiNode)
{
  if (roiNode == nullptr || roiNode->GetScene() == nullptr)
  {
    return nullptr;
  }
  std::vector<vtkDMMLNode *> nodes;
  roiNode->GetScene()->GetNodesByClass("vtkDMMLVolumeRenderingDisplayNode", nodes);

  for (unsigned int i = 0; i < nodes.size(); ++i)
  {
    vtkDMMLVolumeRenderingDisplayNode *dnode = vtkDMMLVolumeRenderingDisplayNode::SafeDownCast(nodes[i]);
    if (dnode && dnode->GetROINodeID() && !strcmp(dnode->GetROINodeID(), roiNode->GetID()))
    {
      return dnode;
    }
  }
  return nullptr;
}

// Description:
// Update vtkDMMLVolumeRenderingDisplayNode from VolumeNode,
// if needed create vtkDMMLVolumePropertyNode and vtkDMMLMarkupsROINode
// and initialize them from VolumeNode
//----------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::UpdateDisplayNodeFromVolumeNode(
  vtkDMMLVolumeRenderingDisplayNode *displayNode, vtkDMMLVolumeNode *volumeNode,
  vtkDMMLVolumePropertyNode *propNode /*=nullptr*/, vtkDMMLNode *roiNode /*=nullptr*/, bool createROI/*=true*/)
{
  if (displayNode == nullptr)
    {
    vtkErrorMacro("vtkCjyxVolumeRenderingLogic::UpdateDisplayNodeFromVolumeNode: display node pointer is null.");
    return;
    }

  if (volumeNode == nullptr)
    {
    return;
    }

  if (propNode == nullptr && displayNode->GetVolumePropertyNode() == nullptr)
    {
    propNode = vtkDMMLVolumePropertyNode::SafeDownCast(this->GetDMMLScene()->AddNewNodeByClass("vtkDMMLVolumePropertyNode"));
    }
  if (propNode != nullptr)
    {
    displayNode->SetAndObserveVolumePropertyNodeID(propNode->GetID());
    }

  if (roiNode)
    {
    displayNode->SetAndObserveROINodeID(roiNode->GetID());
    }
  else if (displayNode->GetROINode() == nullptr && createROI)
    {
    roiNode = this->CreateROINode(displayNode);
    }

  this->CopyDisplayToVolumeRenderingDisplayNode(displayNode);

  this->FitROIToVolume(displayNode);
}

//----------------------------------------------------------------------------
vtkDMMLDisplayableNode* vtkCjyxVolumeRenderingLogic::CreateROINode(vtkDMMLVolumeRenderingDisplayNode *displayNode)
{
  if (displayNode == nullptr)
    {
    vtkErrorMacro("vtkCjyxVolumeRenderingLogic::CreateROINode: display node pointer is null.");
    return nullptr;
    }

  if (displayNode->GetROINode())
    {
    // already created
    return displayNode->GetROINode();
    }

  vtkDMMLNode* roiNode = this->GetDMMLScene()->AddNewNodeByClass(this->DefaultROIClassName, "Volume rendering ROI");
  vtkDMMLAnnotationROINode* annotationROINode = vtkDMMLAnnotationROINode::SafeDownCast(roiNode);
  vtkDMMLMarkupsROINode* markupsROINode = vtkDMMLMarkupsROINode::SafeDownCast(roiNode);
  if (annotationROINode)
    {
    annotationROINode->CreateDefaultDisplayNodes();
    annotationROINode->SetInteractiveMode(1);
    annotationROINode->SetDisplayVisibility(displayNode->GetCroppingEnabled());
    }
  else if (markupsROINode)
    {
    vtkDMMLMarkupsDisplayNode* markupsDisplayNode = vtkDMMLMarkupsDisplayNode::SafeDownCast(markupsROINode->GetDisplayNode());
    if (markupsDisplayNode)
      {
      markupsDisplayNode->SetHandlesInteractive(true);
      // Turn off filling. Semi-transparent actors may introduce volume rendering artifacts in certain
      // configurations and the filling also makes the views a bit more complex.
      markupsDisplayNode->SetFillVisibility(false);
      }
    // by default, show the ROI only if cropping is enabled
    markupsROINode->SetDisplayVisibility(displayNode->GetCroppingEnabled());
    }
  else
    {
    vtkErrorMacro("vtkCjyxVolumeRenderingLogic::CreateROINode failed: cannot instantiate ROI node");
    return nullptr;
    }
  displayNode->SetAndObserveROINodeID(roiNode->GetID());
  this->FitROIToVolume(displayNode);

  return displayNode->GetROINode();
}

//----------------------------------------------------------------------------
vtkDMMLVolumePropertyNode* vtkCjyxVolumeRenderingLogic::AddVolumePropertyFromFile(const char* filename)
{
  if (!this->GetDMMLScene())
  {
    return nullptr;
  }
  if (!filename || !strcmp(filename, ""))
  {
    vtkErrorMacro("AddVolumePropertyFromFile: can't load volume properties from empty file name");
    return nullptr;
  }

  vtkSmartPointer<vtkDMMLVolumePropertyNode> vpNode = vtkSmartPointer<vtkDMMLVolumePropertyNode>::New();
  vtkSmartPointer<vtkDMMLVolumePropertyStorageNode> vpStorageNode = vtkSmartPointer<vtkDMMLVolumePropertyStorageNode>::New();

  // check for local or remote files
  int useURI = 0; // false;
  if (this->GetDMMLScene()->GetCacheManager() != nullptr)
  {
    useURI = this->GetDMMLScene()->GetCacheManager()->IsRemoteReference(filename);
  }

  const char *localFile;
  if (useURI)
  {
    vpStorageNode->SetURI(filename);
    // reset filename to the local file name
    localFile = ((this->GetDMMLScene())->GetCacheManager())->GetFilenameFromURI(filename);
  }
  else
  {
    vpStorageNode->SetFileName(filename);
    localFile = filename;
  }
  const std::string fname(localFile);

  // check to see which node can read this type of file
  if (!vpStorageNode->SupportedFileType(fname.c_str()))
  {
    vtkDebugMacro("Couldn't read file, returning null volume property node: " << filename);
    return nullptr;
  }

  // the node name is based on the file name
  const std::string name = vpStorageNode->GetFileNameWithoutExtension(fname.c_str());
  std::string uname( this->GetDMMLScene()->GetUniqueNameByString(name.c_str()));
  vpNode->SetName(uname.c_str());
  this->GetDMMLScene()->AddNode(vpNode);
  this->GetDMMLScene()->AddNode(vpStorageNode);
  vpNode->SetAndObserveStorageNodeID(vpStorageNode->GetID());

  // now set up the reading
  int retval = vpStorageNode->ReadData(vpNode);
  if (retval != 1)
  {
    vtkErrorMacro("AddVolumePropertyFromFile: error reading " << filename);
    this->GetDMMLScene()->RemoveNode(vpNode);
    this->GetDMMLScene()->RemoveNode(vpStorageNode);
    return nullptr;
  }

  return vpNode;
}

//---------------------------------------------------------------------------
vtkDMMLShaderPropertyNode* vtkCjyxVolumeRenderingLogic::AddShaderPropertyFromFile(const char* filename)
{
  if (!this->GetDMMLScene())
  {
    return nullptr;
  }
  if (!filename || !strcmp(filename, ""))
  {
    vtkErrorMacro(<<"AddShaderPropertyFromFile: can't load shader properties from empty file name");
    return nullptr;
  }

  vtkNew<vtkDMMLShaderPropertyNode> spNode;
  vtkNew<vtkDMMLShaderPropertyStorageNode> spStorageNode;

  // check for local or remote files
  int useURI = 0; // false;
  if (this->GetDMMLScene()->GetCacheManager() != nullptr)
  {
    useURI = this->GetDMMLScene()->GetCacheManager()->IsRemoteReference(filename);
  }

  const char *localFile = nullptr;
  if (useURI)
  {
    spStorageNode->SetURI(filename);
    // reset filename to the local file name
    localFile = ((this->GetDMMLScene())->GetCacheManager())->GetFilenameFromURI(filename);
  }
  else
  {
    spStorageNode->SetFileName(filename);
    localFile = filename;
  }
  const std::string fname(localFile);
  // the node name is based on the file name
  const std::string name = spStorageNode->GetFileNameWithoutExtension(fname.c_str());

  // check to see which node can read this type of file
  if (spStorageNode->SupportedFileType(fname.c_str()))
  {
    std::string uname( this->GetDMMLScene()->GetUniqueNameByString(name.c_str()));

    spNode->SetName(uname.c_str());

    spNode->SetScene(this->GetDMMLScene());
    spStorageNode->SetScene(this->GetDMMLScene());

    this->GetDMMLScene()->AddNode(spStorageNode);
    spNode->SetAndObserveStorageNodeID(spStorageNode->GetID());

    this->GetDMMLScene()->AddNode(spNode);

    // now set up the reading
    int retval = spStorageNode->ReadData(spNode);
    if (retval != 1)
    {
      vtkErrorMacro("AddVolumePropertyFromFile: error reading " << filename);
      this->GetDMMLScene()->RemoveNode(spNode);
      this->GetDMMLScene()->RemoveNode(spStorageNode);
      return nullptr;
    }
    return spNode;
  }
  else
  {
    vtkDebugMacro("Couldn't read file, returning null model node: " << filename);
    return nullptr;
  }
}

//---------------------------------------------------------------------------
vtkDMMLScene* vtkCjyxVolumeRenderingLogic::GetPresetsScene()
{
  if (!this->PresetsScene)
  {
    this->PresetsScene = vtkDMMLScene::New();
    this->LoadPresets(this->PresetsScene);
  }
  return this->PresetsScene;
}

//---------------------------------------------------------------------------
vtkDMMLVolumePropertyNode* vtkCjyxVolumeRenderingLogic::GetPresetByName(const char* presetName)
{
  vtkDMMLScene * presetsScene = this->GetPresetsScene();
  if (!presetsScene || !presetName)
  {
    return nullptr;
  }
  vtkSmartPointer<vtkCollection> presets;
  presets.TakeReference(presetsScene->GetNodesByClassByName("vtkDMMLVolumePropertyNode", presetName));
  if (presets->GetNumberOfItems() == 0)
  {
    return nullptr;
  }
  return vtkDMMLVolumePropertyNode::SafeDownCast(presets->GetItemAsObject(0));
}

//---------------------------------------------------------------------------
bool vtkCjyxVolumeRenderingLogic::LoadPresets(vtkDMMLScene* scene)
{
  this->PresetsScene->RegisterNodeClass(vtkNew<vtkDMMLVolumePropertyNode>().GetPointer());

  if (this->GetModuleShareDirectory().empty())
  {
    vtkErrorMacro(<< "Failed to load presets: Share directory *NOT* set !");
    return false;
  }

  std::string presetFileName = this->GetModuleShareDirectory() + "/presets.xml";
  scene->SetURL(presetFileName.c_str());
  int connected = scene->Connect();
  if (!connected)
  {
    vtkErrorMacro(<< "Failed to load presets [" << presetFileName << "]");
    return false;
  }
  return true;
}

//---------------------------------------------------------------------------
bool vtkCjyxVolumeRenderingLogic::IsDifferentFunction(vtkPiecewiseFunction* function1, vtkPiecewiseFunction* function2)const
{
  if ((function1 != nullptr) ^ (function2 != nullptr))
  {
    return true;
  }
  if (function1->GetSize() != function2->GetSize())
  {
    return true;
  }
  bool different = false;
  for (int i = 0; i < function1->GetSize(); ++i)
  {
    double node1[4];
    function1->GetNodeValue(i, node1);
    double node2[4];
    function2->GetNodeValue(i, node2);
    for (unsigned int j = 0; j < 4; ++j)
    {
      if (node1[j] != node2[j])
      {
        different = true;
        break;
      }
    }
    if (different)
    {
      break;
    }
  }
  return different;
}

//---------------------------------------------------------------------------
bool vtkCjyxVolumeRenderingLogic::IsDifferentFunction(vtkColorTransferFunction* function1, vtkColorTransferFunction* function2)const
{
  if ((function1 != nullptr) ^ (function2 != nullptr))
  {
    return true;
  }
  if (function1->GetSize() != function2->GetSize())
  {
    return true;
  }
  bool different = false;
  for (int i = 0; i < function1->GetSize(); ++i)
  {
    double node1[6];
    function1->GetNodeValue(i, node1);
    double node2[6];
    function2->GetNodeValue(i, node2);
    for (unsigned int j = 0; j < 6; ++j)
    {
      if (node1[j] != node2[j])
      {
        different = true;
        break;
      }
    }
    if (different)
    {
      break;
    }
  }
  return different;
}

//---------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::AddPreset(vtkDMMLVolumePropertyNode* preset, vtkImageData* icon/*=nullptr*/, bool appendToEnd/*=false*/)
{
  if (preset == nullptr)
  {
    vtkErrorMacro("vtkCjyxVolumeRenderingLogic::AddPreset failed: preset is invalid");
    return;
  }
  if (icon == nullptr)
  {
    // use the icon assigned to the preset node if available
    vtkDMMLVolumeNode* iconNode = vtkDMMLVolumeNode::SafeDownCast(
      preset->GetNodeReference(vtkCjyxVolumeRenderingLogic::GetIconVolumeReferenceRole()));
    if (iconNode)
    {
      icon = iconNode->GetImageData();
    }
  }
  vtkDMMLScene* presetScene = this->GetPresetsScene();
  if (icon != nullptr)
  {
    // vector volume is chosen because usually icons are RGB color images
    vtkNew<vtkDMMLVectorVolumeNode> iconNode;
    iconNode->SetAndObserveImageData(icon);
    vtkDMMLNode* addedIconNode = presetScene->AddNode(iconNode.GetPointer());
    // Need to set the node reference before adding the node to the scene to make sure the icon
    // is available immediately when the node is added (otherwise widgets may add the item without an icon)
    preset->SetNodeReferenceID(vtkCjyxVolumeRenderingLogic::GetIconVolumeReferenceRole(), addedIconNode->GetID());
  }
  if (appendToEnd || presetScene->GetNumberOfNodes() == 0)
    {
    presetScene->AddNode(preset);
    }
  else
    {
    presetScene->InsertBeforeNode(presetScene->GetNthNode(0), preset);
    }
}

//---------------------------------------------------------------------------
void vtkCjyxVolumeRenderingLogic::RemovePreset(vtkDMMLVolumePropertyNode* preset)
{
  if (preset == nullptr)
  {
    return;
  }
  vtkDMMLScene* presetScene = this->GetPresetsScene();
  vtkDMMLNode* iconNode = preset->GetNodeReference(vtkCjyxVolumeRenderingLogic::GetIconVolumeReferenceRole());
  if (iconNode != nullptr)
  {
    presetScene->RemoveNode(iconNode);
  }
  presetScene->RemoveNode(preset);
}

//---------------------------------------------------------------------------
int vtkCjyxVolumeRenderingLogic::LoadCustomPresetsScene(const char* sceneFilePath)
{
  if (!this->PresetsScene)
  {
    this->PresetsScene = vtkDMMLScene::New();
  }
  else
  {
    this->PresetsScene->Clear(1);
  }

  this->PresetsScene->SetURL(sceneFilePath);
  return this->PresetsScene->Import();
}

//---------------------------------------------------------------------------
bool vtkCjyxVolumeRenderingLogic::SetRecommendedVolumeRenderingProperties(vtkDMMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode == nullptr || vspNode->GetVolumePropertyNode() == nullptr)
  {
    vtkErrorMacro("SetRecommendedVolumeRenderingProperties: invalid input display or volume property node");
    return false;
  }
  vtkDMMLScalarVolumeNode* volumeNode = vtkDMMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode());
  if (!volumeNode || !volumeNode->GetImageData())
  {
    vtkErrorMacro("SetRecommendedVolumeRenderingProperties: invalid volume node");
    return false;
  }

  if (volumeNode->IsA("vtkDMMLLabelMapVolumeNode"))
  {
    return false;
  }

  double* scalarRange = volumeNode->GetImageData()->GetScalarRange();
  double scalarRangeSize = scalarRange[1] - scalarRange[0];

  if (volumeNode->GetImageData()->GetScalarType() == VTK_UNSIGNED_CHAR)
  {
    // 8-bit grayscale image, it is probably ultrasound
    vspNode->GetVolumePropertyNode()->Copy(this->GetPresetByName("US-Fetal"));
    return true;
  }

  if (scalarRangeSize > 50.0 && scalarRangeSize < 1500.0 && this->GetPresetByName("MR-Default"))
  {
    // small dynamic range, probably MRI
    vspNode->GetVolumePropertyNode()->Copy(this->GetPresetByName("MR-Default"));
    return true;
  }

  if (scalarRangeSize >= 1500.0 && scalarRangeSize < 10000.0 && this->GetPresetByName("CT-Chest-Contrast-Enhanced"))
  {
    // larger dynamic range, probably CT
    vspNode->GetVolumePropertyNode()->Copy(this->GetPresetByName("CT-Chest-Contrast-Enhanced"));
    return true;
  }

  return false;
}
