/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLModelDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
// DMML includes
#include "vtkDMMLModelDisplayNode.h"
#include "vtkDMMLColorNode.h"
#include "vtkDMMLModelNode.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkGeometryFilter.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPassThrough.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkThreshold.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVersion.h>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLModelDisplayNode);

static const char* SliceDistanceEncodedProjectionColorNodeReferenceRole = "distanceEncodedProjectionColor";

//-----------------------------------------------------------------------------
vtkDMMLModelDisplayNode::vtkDMMLModelDisplayNode()
{
  this->PassThrough = vtkPassThrough::New();
  this->AssignAttribute = vtkAssignAttribute::New();
  this->ThresholdFilter = vtkThreshold::New();
  this->ThresholdFilter->SetLowerThreshold(0.0);
  this->ThresholdFilter->SetUpperThreshold(-1.0); // indicates uninitialized
  this->ThresholdFilter->SetThresholdFunction(vtkThreshold::THRESHOLD_BETWEEN);
  this->ThresholdRangeTemp[0] = 0.0;
  this->ThresholdRangeTemp[1] = -1.0;
  this->ConvertToPolyDataFilter = vtkGeometryFilter::New();
  this->ThresholdEnabled = false;
  this->SliceDisplayMode = SliceDisplayIntersection;
  this->BackfaceCulling = 0;

  // Backface color has slightly different hue and less saturated compared to frontface
  this->BackfaceColorHSVOffset[0] = -0.05;
  this->BackfaceColorHSVOffset[1] = -0.1;
  this->BackfaceColorHSVOffset[2] = 0.0;

  // the default behavior for models is to use the scalar range of the data
  // to reset the display scalar range, so use the Data flag
  this->SetScalarRangeFlag(vtkDMMLDisplayNode::UseDataScalarRange);

  this->ThresholdFilter->SetInputConnection(this->AssignAttribute->GetOutputPort());
  this->ConvertToPolyDataFilter->SetInputConnection(this->ThresholdFilter->GetOutputPort());

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  this->AddNodeReferenceRole(SliceDistanceEncodedProjectionColorNodeReferenceRole, nullptr, events.GetPointer());
}

//-----------------------------------------------------------------------------
vtkDMMLModelDisplayNode::~vtkDMMLModelDisplayNode()
{
  this->PassThrough->Delete();
  this->AssignAttribute->Delete();
  this->ThresholdFilter->Delete();
  this->ConvertToPolyDataFilter->Delete();
}

//----------------------------------------------------------------------------
void vtkDMMLModelDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintEnumMacro(SliceDisplayMode);
  vtkDMMLPrintBooleanMacro(ThresholdEnabled);
  vtkDMMLPrintVectorMacro(ThresholdRange, double, 2);
  vtkDMMLPrintVectorMacro(BackfaceColorHSVOffset, double, 3);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLModelDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLEnumMacro(sliceDisplayMode, SliceDisplayMode);
  vtkDMMLWriteXMLBooleanMacro(thresholdEnabled, ThresholdEnabled);
  vtkDMMLWriteXMLVectorMacro(thresholdRange, ThresholdRange, double, 2);
  vtkDMMLWriteXMLVectorMacro(backfaceColorHSVOffset, BackfaceColorHSVOffset, double, 3);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLModelDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->Superclass::ReadXMLAttributes(atts);

  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLEnumMacro(sliceDisplayMode, SliceDisplayMode);
  vtkDMMLReadXMLBooleanMacro(thresholdEnabled, ThresholdEnabled);
  vtkDMMLReadXMLVectorMacro(thresholdRange, ThresholdRange, double, 2);
  vtkDMMLReadXMLVectorMacro(backfaceColorHSVOffset, BackfaceColorHSVOffset, double, 3);
  vtkDMMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkDMMLModelDisplayNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLModelDisplayNode* node = vtkDMMLModelDisplayNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyEnumMacro(SliceDisplayMode);
  vtkDMMLCopyBooleanMacro(ThresholdEnabled);
  vtkDMMLCopyVectorMacro(ThresholdRange, double, 2);
  vtkDMMLCopyVectorMacro(BackfaceColorHSVOffset, double, 3);
  vtkDMMLCopyEndMacro();
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayNode::ProcessDMMLEvents(vtkObject *caller,
                                                unsigned long event,
                                                void *callData )
{
  this->Superclass::ProcessDMMLEvents(caller, event, callData);

  vtkDMMLColorNode* cnode = vtkDMMLColorNode::SafeDownCast(caller);
  if (cnode != nullptr && cnode == this->GetDistanceEncodedProjectionColorNode()
    && event == vtkCommand::ModifiedEvent)
    {
    // Slice distance encoded projection color node changed
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }
  else if (event == vtkCommand::ModifiedEvent)
    {
    this->UpdateScalarRange();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayNode
::SetInputMeshConnection(vtkAlgorithmOutput* meshConnection)
{
  if (this->GetInputMeshConnection() == meshConnection)
    {
    return;
    }
  this->SetInputToMeshPipeline(meshConnection);
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayNode
::SetInputPolyDataConnection(vtkAlgorithmOutput* polyDataConnection)
{
  // Wrapping `SetInputMeshConnection` for backward compatibility
  this->SetInputMeshConnection(polyDataConnection);
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayNode
::SetInputToMeshPipeline(vtkAlgorithmOutput* meshConnection)
{
  this->PassThrough->SetInputConnection(meshConnection);
  this->AssignAttribute->SetInputConnection(meshConnection);
  this->Modified();
}

//---------------------------------------------------------------------------
vtkPointSet* vtkDMMLModelDisplayNode::GetInputMesh()
{
  return vtkPointSet::SafeDownCast(this->AssignAttribute->GetInput());
}

//---------------------------------------------------------------------------
vtkPolyData* vtkDMMLModelDisplayNode::GetInputPolyData()
{
  return vtkPolyData::SafeDownCast(this->GetInputMesh());
}

//---------------------------------------------------------------------------
vtkUnstructuredGrid* vtkDMMLModelDisplayNode::GetInputUnstructuredGrid()
{
  return vtkUnstructuredGrid::SafeDownCast(this->GetInputMesh());
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLModelDisplayNode::GetInputMeshConnection()
{
  return this->AssignAttribute->GetNumberOfInputConnections(0) ?
    this->AssignAttribute->GetInputConnection(0,0) : nullptr;
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLModelDisplayNode::GetInputPolyDataConnection()
{
  // Wrapping `GetInputMeshConnection` for backward compatibility
  return this->GetInputMeshConnection();
}

//---------------------------------------------------------------------------
vtkPointSet* vtkDMMLModelDisplayNode::GetOutputMesh()
{
  if (!this->GetInputMeshConnection())
    {
    return nullptr;
    }

  vtkAlgorithmOutput* outputConnection = this->GetOutputMeshConnection();
  if (!outputConnection)
    {
    return nullptr;
    }

  vtkAlgorithm* producer = outputConnection->GetProducer();
  if (!producer)
    {
    return nullptr;
    }

  producer->Update();
  return vtkPointSet::SafeDownCast(
           producer->GetOutputDataObject(outputConnection->GetIndex()));
}

//---------------------------------------------------------------------------
vtkPolyData* vtkDMMLModelDisplayNode::GetOutputPolyData()
{
  return vtkPolyData::SafeDownCast(this->GetOutputMesh());
}

//---------------------------------------------------------------------------
vtkUnstructuredGrid* vtkDMMLModelDisplayNode::GetOutputUnstructuredGrid()
{
  return vtkUnstructuredGrid::SafeDownCast(this->GetOutputMesh());
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLModelDisplayNode::GetOutputMeshConnection()
{
  if (this->GetActiveScalarName() &&
      this->GetScalarVisibility() && // do not threshold if scalars hidden
      this->ThresholdEnabled)
    {
    vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(this->GetDisplayableNode());
    if (modelNode && modelNode->GetMeshType() == vtkDMMLModelNode::PolyDataMeshType)
      {
      // Threshold filter generates unstructured grid output. If input is a polydata mesh
      // then the pipeline expects polydata as output mesh, therefore we need to use
      // ConvertToPolyDataFilter output.
      return this->ConvertToPolyDataFilter->GetOutputPort();
      }
    else
      {
      return this->ThresholdFilter->GetOutputPort();
      }
    }
  else if (this->GetActiveScalarName())
    {
    return this->AssignAttribute->GetOutputPort();
    }
  else
    {
    return this->PassThrough->GetOutputPort();
    }
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLModelDisplayNode::GetOutputPolyDataConnection()
{
  vtkWarningMacro("vtkDMMLModelDisplayNode::GetOutputPolyDataConnection is "
                  << "deprecated. Favor GetOutputMeshConnection().");
  return this->GetOutputMeshConnection();
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayNode::SetThresholdEnabled(bool enabled)
{
  if (this->ThresholdEnabled == enabled)
    {
    return;
    }

  int wasModified = this->StartModify();

  this->ThresholdEnabled = enabled;
  this->Modified();

  // initialize threshold range if it has not been initialized yet
  if (enabled && this->GetThresholdMin() > this->GetThresholdMax())
    {
    double dataRange[2] = { 0.0, -1.0 };
    vtkDataArray *dataArray = this->GetActiveScalarArray();
    if (dataArray)
      {
      dataArray->GetRange(dataRange);
      }
    if (dataRange[0] <= dataRange[1])
      {
      this->SetThresholdRange(dataRange);
      }
    }

  this->EndModify(wasModified);
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayNode::SetThresholdRange(double min, double max)
{
  vtkMTimeType mtime = this->ThresholdFilter->GetMTime();
  this->ThresholdFilter->SetLowerThreshold(min);
  this->ThresholdFilter->SetUpperThreshold(max);
  this->ThresholdFilter->SetThresholdFunction(vtkThreshold::THRESHOLD_BETWEEN);
  if (this->ThresholdFilter->GetMTime() > mtime)
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayNode::SetThresholdRange(double range[2])
{
  this->SetThresholdRange(range[0], range[1]);
}

//---------------------------------------------------------------------------
double vtkDMMLModelDisplayNode::GetThresholdMin()
{
  return this->ThresholdFilter->GetLowerThreshold();
}

//---------------------------------------------------------------------------
double vtkDMMLModelDisplayNode::GetThresholdMax()
{
  return this->ThresholdFilter->GetUpperThreshold();
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayNode::GetThresholdRange(double range[2])
{
  range[0] = this->GetThresholdMin();
  range[1] = this->GetThresholdMax();
}

//---------------------------------------------------------------------------
double* vtkDMMLModelDisplayNode::GetThresholdRange()
{
  this->GetThresholdRange(this->ThresholdRangeTemp);
  return this->ThresholdRangeTemp;
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayNode::SetActiveScalarName(const char *scalarName)
{
  if (scalarName && this->ActiveScalarName && !strcmp(scalarName, this->ActiveScalarName))
    {
    return;
    }
  if (scalarName == nullptr && this->ActiveScalarName == nullptr)
    {
    return;
    }
  int wasModifying = this->StartModify();
  this->Superclass::SetActiveScalarName(scalarName);
  this->UpdateAssignedAttribute();
  this->EndModify(wasModifying);
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayNode::SetActiveAttributeLocation(int location)
{
  if (location == this->ActiveAttributeLocation)
    {
    return;
    }
  int wasModifying = this->StartModify();
  this->Superclass::SetActiveAttributeLocation(location);
  this->UpdateAssignedAttribute();
  this->EndModify(wasModifying);
}

//---------------------------------------------------------------------------
void vtkDMMLModelDisplayNode::UpdateAssignedAttribute()
{
  this->AssignAttribute->Assign(
    this->GetActiveScalarName(),
    vtkDataSetAttributes::SCALARS,
    this->GetActiveAttributeLocation() >= 0 ? this->GetActiveAttributeLocation() : vtkAssignAttribute::POINT_DATA);

  if (this->GetActiveAttributeLocation() == vtkAssignAttribute::POINT_DATA)
    {
    this->ThresholdFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
    }
  else if (this->GetActiveAttributeLocation() == vtkAssignAttribute::CELL_DATA)
    {
    this->ThresholdFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, vtkDataSetAttributes::SCALARS);
    }

  this->UpdateScalarRange();
}

//-----------------------------------------------------------
vtkDataSet* vtkDMMLModelDisplayNode::GetScalarDataSet()
{
  return this->GetInputMesh();
}

//-----------------------------------------------------------
vtkDataArray* vtkDMMLModelDisplayNode::GetActiveScalarArray()
{
  if (this->GetActiveScalarName() == nullptr || strcmp(this->GetActiveScalarName(),"") == 0)
    {
    return nullptr;
    }
  if (!this->GetInputMesh())
    {
    return nullptr;
    }

  // Use output of AssignAttribute instead of this->GetOutputMesh()
  // since the data scalar range should not be retrieved from a
  // thresholded mesh even when thresholding is enabled.
  // Need to call Update() since we need to use GetOutput() on the
  // AssignAttribuet filter to retrieve its output mesh scalar range.
  this->AssignAttribute->Update();
  vtkPointSet* mesh = vtkPointSet::SafeDownCast(this->AssignAttribute->GetOutput());
  if (mesh == nullptr)
    {
    return nullptr;
    }

  vtkDataSetAttributes* attributes = nullptr;
  switch (this->GetActiveAttributeLocation())
    {
    case vtkAssignAttribute::POINT_DATA:
      attributes = mesh->GetPointData();
      break;
    case vtkAssignAttribute::CELL_DATA:
      attributes = mesh->GetCellData();
      break;
    default:
      vtkWarningMacro("vtkDMMLModelDisplayNode::GetActiveScalarArray failed: unsupported attribute location: "
        << this->GetActiveAttributeLocation());
      break;
    }
  if (attributes == nullptr)
    {
    return nullptr;
    }
  vtkDataArray *dataArray = attributes->GetArray(this->GetActiveScalarName());
  return dataArray;
}

//-----------------------------------------------------------
void vtkDMMLModelDisplayNode::SetSliceDisplayModeToIntersection()
{
  this->SetSliceDisplayMode(SliceDisplayIntersection);
};

//-----------------------------------------------------------
void vtkDMMLModelDisplayNode::SetSliceDisplayModeToProjection()
{
  this->SetSliceDisplayMode(SliceDisplayProjection);
};

//-----------------------------------------------------------
void vtkDMMLModelDisplayNode::SetSliceDisplayModeToDistanceEncodedProjection()
{
  this->SetSliceDisplayMode(SliceDisplayDistanceEncodedProjection);
};

//-----------------------------------------------------------
const char* vtkDMMLModelDisplayNode::GetSliceDisplayModeAsString(int id)
{
  switch (id)
    {
    case SliceDisplayIntersection: return "intersection";
    case SliceDisplayProjection: return "projection";
    case SliceDisplayDistanceEncodedProjection: return "distanceEncodedProjection";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkDMMLModelDisplayNode::GetSliceDisplayModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i<SliceDisplayMode_Last; i++)
    {
    if (strcmp(name, GetSliceDisplayModeAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
void vtkDMMLModelDisplayNode::SetAndObserveDistanceEncodedProjectionColorNodeID(const char *colorNodeID)
{
  this->SetAndObserveNodeReferenceID(SliceDistanceEncodedProjectionColorNodeReferenceRole, colorNodeID);
}

//-----------------------------------------------------------
const char* vtkDMMLModelDisplayNode::GetDistanceEncodedProjectionColorNodeID()
{
  return this->GetNodeReferenceID(SliceDistanceEncodedProjectionColorNodeReferenceRole);
}

//-----------------------------------------------------------
vtkDMMLColorNode* vtkDMMLModelDisplayNode::GetDistanceEncodedProjectionColorNode()
{
  return vtkDMMLColorNode::SafeDownCast(this->GetNodeReference(SliceDistanceEncodedProjectionColorNodeReferenceRole));
}
