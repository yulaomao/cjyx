/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLViewNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// DMML includes
#include "vtkDMMLScene.h"
#include "vtkDMMLViewNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLViewNode);

//----------------------------------------------------------------------------
vtkDMMLViewNode::vtkDMMLViewNode()
{
  this->BoxVisible = 1;
  this->AxisLabelsVisible = 1;
  this->AxisLabelsCameraDependent = 1;
  this->FiducialsVisible = 1;
  this->FiducialLabelsVisible = 1;
  this->FieldOfView = 200;
  this->LetterSize = 0.05;
  this->AnimationMode = vtkDMMLViewNode::Off;
  this->ViewAxisMode = vtkDMMLViewNode::LookFrom;
  this->SpinDegrees = 2.0;
  this->RotateDegrees = 5.0;
  this->SpinDirection = vtkDMMLViewNode::YawLeft;
  this->AnimationMs = 5;
  this->RockLength = 200;
  this->RockCount = 0;
  this->StereoType = vtkDMMLViewNode::NoStereo;
  this->RenderMode = vtkDMMLViewNode::Perspective;
  this->BackgroundColor[0] = this->defaultBackgroundColor()[0];
  this->BackgroundColor[1] = this->defaultBackgroundColor()[1];
  this->BackgroundColor[2] = this->defaultBackgroundColor()[2];
  this->BackgroundColor2[0] = this->defaultBackgroundColor2()[0];
  this->BackgroundColor2[1] = this->defaultBackgroundColor2()[1];
  this->BackgroundColor2[2] = this->defaultBackgroundColor2()[2];
  this->UseDepthPeeling = 1;
  this->FPSVisible = 0;
  this->OrientationMarkerEnabled = true;
  this->RulerEnabled = true;
  this->GPUMemorySize = 0; // Means application default
  this->AutoReleaseGraphicsResources = false;
  this->ExpectedFPS = 8.;
  this->VolumeRenderingQuality = vtkDMMLViewNode::Normal;
  this->RaycastTechnique = vtkDMMLViewNode::Composite;
  this->VolumeRenderingSurfaceSmoothing = false;
  this->VolumeRenderingOversamplingFactor = 2.0;
  this->LinkedControl = 0;
  this->Interacting = 0;
  this->InteractionFlags = 0;
}

//----------------------------------------------------------------------------
vtkDMMLViewNode::~vtkDMMLViewNode() = default;

//----------------------------------------------------------------------------
const char* vtkDMMLViewNode::GetNodeTagName()
{
  return "View";
}

//----------------------------------------------------------------------------
void vtkDMMLViewNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLFloatMacro(fieldOfView, FieldOfView);
  vtkDMMLWriteXMLFloatMacro(letterSize, LetterSize);
  vtkDMMLWriteXMLBooleanMacro(boxVisible, BoxVisible);
  vtkDMMLWriteXMLBooleanMacro(fiducialsVisible, FiducialsVisible);
  vtkDMMLWriteXMLBooleanMacro(fiducialLabelsVisible, FiducialLabelsVisible);
  vtkDMMLWriteXMLBooleanMacro(axisLabelsVisible, AxisLabelsVisible);
  vtkDMMLWriteXMLBooleanMacro(axisLabelsCameraDependent, AxisLabelsCameraDependent);
  vtkDMMLWriteXMLEnumMacro(animationMode, AnimationMode);
  vtkDMMLWriteXMLEnumMacro(viewAxisMode, ViewAxisMode);
  vtkDMMLWriteXMLFloatMacro(spinDegrees, SpinDegrees);
  vtkDMMLWriteXMLFloatMacro(spinMs, AnimationMs);
  vtkDMMLWriteXMLEnumMacro(spinDirection, SpinDirection);
  vtkDMMLWriteXMLFloatMacro(rotateDegrees, RotateDegrees);
  vtkDMMLWriteXMLIntMacro(rockLength, RockLength);
  vtkDMMLWriteXMLIntMacro(rockCount, RockCount);
  vtkDMMLWriteXMLEnumMacro(stereoType, StereoType);
  vtkDMMLWriteXMLEnumMacro(renderMode, RenderMode);
  vtkDMMLWriteXMLIntMacro(useDepthPeeling, UseDepthPeeling);
  vtkDMMLWriteXMLIntMacro(gpuMemorySize, GPUMemorySize);
  vtkDMMLWriteXMLBooleanMacro(autoReleaseGraphicsResources, AutoReleaseGraphicsResources);
  vtkDMMLWriteXMLFloatMacro(expectedFPS, ExpectedFPS);
  vtkDMMLWriteXMLEnumMacro(volumeRenderingQuality, VolumeRenderingQuality);
  vtkDMMLWriteXMLEnumMacro(raycastTechnique, RaycastTechnique);
  vtkDMMLWriteXMLIntMacro(volumeRenderingSurfaceSmoothing, VolumeRenderingSurfaceSmoothing);
  vtkDMMLWriteXMLFloatMacro(volumeRenderingOversamplingFactor, VolumeRenderingOversamplingFactor);
  vtkDMMLWriteXMLIntMacro(linkedControl, LinkedControl);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLFloatMacro(fieldOfView, FieldOfView);
  vtkDMMLReadXMLFloatMacro(letterSize, LetterSize);
  vtkDMMLReadXMLBooleanMacro(boxVisible, BoxVisible);
  vtkDMMLReadXMLBooleanMacro(fiducialsVisible, FiducialsVisible);
  vtkDMMLReadXMLBooleanMacro(fiducialLabelsVisible, FiducialLabelsVisible);
  vtkDMMLReadXMLBooleanMacro(axisLabelsVisible, AxisLabelsVisible);
  vtkDMMLReadXMLBooleanMacro(axisLabelsCameraDependent, AxisLabelsCameraDependent);
  vtkDMMLReadXMLEnumMacro(animationMode, AnimationMode);
  vtkDMMLReadXMLEnumMacro(viewAxisMode, ViewAxisMode);
  vtkDMMLReadXMLFloatMacro(spinDegrees, SpinDegrees);
  vtkDMMLReadXMLFloatMacro(spinMs, AnimationMs);
  vtkDMMLReadXMLEnumMacro(spinDirection, SpinDirection);
  vtkDMMLReadXMLFloatMacro(rotateDegrees, RotateDegrees);
  vtkDMMLReadXMLIntMacro(rockLength, RockLength);
  vtkDMMLReadXMLIntMacro(rockCount, RockCount);
  vtkDMMLReadXMLEnumMacro(stereoType, StereoType);
  vtkDMMLReadXMLEnumMacro(renderMode, RenderMode);
  vtkDMMLReadXMLIntMacro(useDepthPeeling, UseDepthPeeling);
  vtkDMMLReadXMLIntMacro(gpuMemorySize, GPUMemorySize);
  vtkDMMLReadXMLBooleanMacro(autoReleaseGraphicsResources, AutoReleaseGraphicsResources);
  vtkDMMLReadXMLFloatMacro(expectedFPS, ExpectedFPS);
  vtkDMMLReadXMLEnumMacro(volumeRenderingQuality, VolumeRenderingQuality);
  vtkDMMLReadXMLEnumMacro(raycastTechnique, RaycastTechnique);
  vtkDMMLReadXMLIntMacro(volumeRenderingSurfaceSmoothing, VolumeRenderingSurfaceSmoothing);
  vtkDMMLReadXMLFloatMacro(volumeRenderingOversamplingFactor, VolumeRenderingOversamplingFactor);
  vtkDMMLReadXMLIntMacro(linkedControl, LinkedControl);
  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLViewNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyFloatMacro(FieldOfView);
  vtkDMMLCopyFloatMacro(LetterSize);
  vtkDMMLCopyBooleanMacro(BoxVisible);
  vtkDMMLCopyBooleanMacro(FiducialsVisible);
  vtkDMMLCopyBooleanMacro(FiducialLabelsVisible);
  vtkDMMLCopyBooleanMacro(AxisLabelsVisible);
  vtkDMMLCopyBooleanMacro(AxisLabelsCameraDependent);
  vtkDMMLCopyEnumMacro(AnimationMode);
  vtkDMMLCopyEnumMacro(ViewAxisMode);
  vtkDMMLCopyFloatMacro(SpinDegrees);
  vtkDMMLCopyFloatMacro(AnimationMs);
  vtkDMMLCopyEnumMacro(SpinDirection);
  vtkDMMLCopyFloatMacro(RotateDegrees);
  vtkDMMLCopyIntMacro(RockLength);
  vtkDMMLCopyIntMacro(RockCount);
  vtkDMMLCopyEnumMacro(StereoType);
  vtkDMMLCopyEnumMacro(RenderMode);
  vtkDMMLCopyIntMacro(UseDepthPeeling);
  vtkDMMLCopyIntMacro(GPUMemorySize);
  vtkDMMLCopyBooleanMacro(AutoReleaseGraphicsResources);
  vtkDMMLCopyFloatMacro(ExpectedFPS);
  vtkDMMLCopyIntMacro(VolumeRenderingQuality);
  vtkDMMLCopyIntMacro(RaycastTechnique);
  vtkDMMLCopyIntMacro(VolumeRenderingSurfaceSmoothing);
  vtkDMMLCopyFloatMacro(VolumeRenderingOversamplingFactor);
  vtkDMMLCopyIntMacro(LinkedControl);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintFloatMacro(FieldOfView);
  vtkDMMLPrintFloatMacro(LetterSize);
  vtkDMMLPrintBooleanMacro(BoxVisible);
  vtkDMMLPrintBooleanMacro(FiducialsVisible);
  vtkDMMLPrintBooleanMacro(FiducialLabelsVisible);
  vtkDMMLPrintBooleanMacro(AxisLabelsVisible);
  vtkDMMLPrintBooleanMacro(AxisLabelsCameraDependent);
  vtkDMMLPrintEnumMacro(AnimationMode);
  vtkDMMLPrintEnumMacro(ViewAxisMode);
  vtkDMMLPrintFloatMacro(SpinDegrees);
  vtkDMMLPrintFloatMacro(AnimationMs);
  vtkDMMLPrintEnumMacro(SpinDirection);
  vtkDMMLPrintFloatMacro(RotateDegrees);
  vtkDMMLPrintIntMacro(RockLength);
  vtkDMMLPrintIntMacro(RockCount);
  vtkDMMLPrintEnumMacro(StereoType);
  vtkDMMLPrintEnumMacro(RenderMode);
  vtkDMMLPrintIntMacro(UseDepthPeeling);
  vtkDMMLPrintIntMacro(GPUMemorySize);
  vtkDMMLPrintBooleanMacro(AutoReleaseGraphicsResources);
  vtkDMMLPrintFloatMacro(ExpectedFPS);
  vtkDMMLPrintIntMacro(VolumeRenderingQuality);
  vtkDMMLPrintIntMacro(RaycastTechnique);
  vtkDMMLPrintIntMacro(VolumeRenderingSurfaceSmoothing);
  vtkDMMLPrintFloatMacro(VolumeRenderingOversamplingFactor);
  vtkDMMLPrintIntMacro(Interacting);
  vtkDMMLPrintIntMacro(LinkedControl);
  vtkDMMLPrintEndMacro();
}

//------------------------------------------------------------------------------
double* vtkDMMLViewNode::defaultBackgroundColor()
{
  //static double backgroundColor[3] = {0.70196, 0.70196, 0.90588};
  static double backgroundColor[3] = {0.7568627450980392,
                                      0.7647058823529412,
                                      0.9098039215686275};
  return backgroundColor;
}

//------------------------------------------------------------------------------
double* vtkDMMLViewNode::defaultBackgroundColor2()
{
  static double backgroundColor2[3] = {0.4549019607843137,
                                       0.4705882352941176,
                                       0.7450980392156863};
  return backgroundColor2;
}

//---------------------------------------------------------------------------
const char* vtkDMMLViewNode::GetAnimationModeAsString(int id)
{
  switch (id)
    {
    case Off: return "Off";
    case Spin: return "Spin";
    case Rock: return "Rock";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLViewNode::GetAnimationModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < AnimationMode_Last; ii++)
    {
    if (strcmp(name, GetAnimationModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkDMMLViewNode::GetViewAxisModeAsString(int id)
{
  switch (id)
    {
    case LookFrom: return "LookFrom";
    case RotateAround: return "RotateAround";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLViewNode::GetViewAxisModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < ViewAxisMode_Last; ii++)
    {
    if (strcmp(name, GetViewAxisModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkDMMLViewNode::GetSpinDirectionAsString(int id)
{
  switch (id)
    {
    case PitchUp: return "PitchUp";
    case PitchDown: return "PitchDown";
    case RollLeft: return "RollLeft";
    case RollRight: return "RollRight";
    case YawLeft: return "YawLeft";
    case YawRight: return "YawRight";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLViewNode::GetSpinDirectionFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < SpinDirection_Last; ii++)
    {
    if (strcmp(name, GetSpinDirectionAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkDMMLViewNode::GetStereoTypeAsString(int id)
{
  switch (id)
    {
    case NoStereo: return "NoStereo";
    case RedBlue: return "RedBlue";
    case Anaglyph: return "Anaglyph";
    case QuadBuffer: return "QuadBuffer";
    case Interlaced: return "Interlaced";
    case UserDefined_1: return "UserDefined_1";
    case UserDefined_2: return "UserDefined_2";
    case UserDefined_3: return "UserDefined_3";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLViewNode::GetStereoTypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < StereoType_Last; ii++)
    {
    if (strcmp(name, GetStereoTypeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkDMMLViewNode::GetRenderModeAsString(int id)
{
  switch (id)
    {
    case Perspective: return "Perspective";
    case Orthographic: return "Orthographic";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLViewNode::GetRenderModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < RenderMode_Last; ii++)
    {
    if (strcmp(name, GetRenderModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkDMMLViewNode::GetVolumeRenderingQualityAsString(int id)
{
  switch (id)
    {
    case Adaptive: return "Adaptive";
    case Normal: return "Normal";
    case Maximum: return "Maximum";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLViewNode::GetVolumeRenderingQualityFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < VolumeRenderingQuality_Last; ii++)
    {
    if (strcmp(name, GetVolumeRenderingQualityAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkDMMLViewNode::GetRaycastTechniqueAsString(int id)
{
  switch (id)
    {
    case Composite: return "Composite";
    case CompositeEdgeColoring: return "CompositeEdgeColoring";
    case MaximumIntensityProjection: return "MaximumIntensityProjection";
    case MinimumIntensityProjection: return "MinimumIntensityProjection";
    case GradiantMagnitudeOpacityModulation: return "GradiantMagnitudeOpacityModulation";
    case IllustrativeContextPreservingExploration: return "IllustrativeContextPreservingExploration";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLViewNode::GetRaycastTechniqueFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < RaycastTechnique_Last; ii++)
    {
    if (strcmp(name, GetRaycastTechniqueAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
void vtkDMMLViewNode::SetInteracting(int interacting)
{
  // Don't call Modified()
  this->Interacting = interacting;
}

//-----------------------------------------------------------
void vtkDMMLViewNode::SetInteractionFlags(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlags = flags;
}
