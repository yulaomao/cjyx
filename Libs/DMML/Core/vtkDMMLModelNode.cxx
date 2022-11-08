/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Cjyx
Module:    $RCSfile: vtkDMMLModelNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// DMML includes
#include "vtkEventBroker.h"
#include "vtkDMMLModelNode.h"
#include "vtkDMMLModelDisplayNode.h"
#include "vtkDMMLModelStorageNode.h"
#include <vtkDMMLProceduralColorNode.h>
#include "vtkDMMLTransformNode.h"
#include "vtkDMMLScene.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkCallbackCommand.h>
#include <vtkCellData.h>
#include <vtkColorTransferFunction.h>
#include <vtkEventForwarderCommand.h>
#include <vtkFloatArray.h>
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTransformFilter.h>
#include <vtkTrivialProducer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVersion.h>

// STD includes
#include <cassert>
#include <sstream>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLModelNode);

//----------------------------------------------------------------------------
vtkDMMLModelNode::vtkDMMLModelNode()
{
  this->MeshConnection = nullptr;
  this->DataEventForwarder = nullptr;

  // for backward compatibility, we assume that if no
  // mesh were set, it is a polydata.
  this->MeshType = vtkDMMLModelNode::PolyDataMeshType;

  this->ContentModifiedEvents->InsertNextValue(vtkDMMLModelNode::PolyDataModifiedEvent);
}

//----------------------------------------------------------------------------
vtkDMMLModelNode::~vtkDMMLModelNode()
{
  this->SetMeshConnection(nullptr);
  if (this->DataEventForwarder)
    {
    this->DataEventForwarder->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkDMMLModelNode::CopyContent(vtkDMMLNode* anode, bool deepCopy/*=true*/)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkDMMLModelNode* node = vtkDMMLModelNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }
  if (deepCopy)
    {
    if (node->GetMesh())
      {
      if (this->GetMesh() && strcmp(this->GetMesh()->GetClassName(), node->GetMesh()->GetClassName())==0)
        {
        this->GetMesh()->DeepCopy(node->GetMesh());
        }
      else
        {
        vtkSmartPointer<vtkPointSet> newMesh
          = vtkSmartPointer<vtkPointSet>::Take(node->GetMesh()->NewInstance());
        newMesh->DeepCopy(node->GetMesh());
        this->SetAndObserveMesh(newMesh);
        }
      }
    else
      {
      // input was nullptr
      this->SetAndObserveMesh(nullptr);
      }
    }
  else
    {
    // shallow-copy
    this->SetMeshConnection(node->GetMeshConnection());
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode::ProcessDMMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  this->Superclass::ProcessDMMLEvents(caller, event, callData);

  if (this->MeshConnection &&
      this->MeshConnection->GetProducer() == vtkAlgorithm::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->StorableModifiedTime.Modified();
    this->InvokeCustomModifiedEvent(vtkDMMLModelNode::MeshModifiedEvent, nullptr);
    }
}

//----------------------------------------------------------------------------
vtkDMMLModelDisplayNode* vtkDMMLModelNode::GetModelDisplayNode()
{
  return vtkDMMLModelDisplayNode::SafeDownCast(this->GetDisplayNode());
}

//----------------------------------------------------------------------------
void vtkDMMLModelNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintObjectMacro(UnstructuredGrid);
  vtkDMMLPrintObjectMacro(PolyData);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLModelNode::SetAndObserveMesh(vtkPointSet *mesh)
{
  if (mesh == nullptr)
    {
    this->SetMeshConnection(nullptr);
    }
  else
    {
    // Check that mesh is polydata or unstructuredgrid
    vtkPolyData* polydata = vtkPolyData::SafeDownCast(mesh);
    if( !polydata && !vtkUnstructuredGrid::SafeDownCast(mesh))
      {
      vtkErrorMacro("SetAndObserveMesh: mesh should be a"
                    "vtkPolyData or an vtkUnstructuredGrid.");
      return;
      }

    vtkTrivialProducer* oldProducer = vtkTrivialProducer::SafeDownCast(
      this->MeshConnection ? this->MeshConnection->GetProducer() : nullptr);
    if (oldProducer && oldProducer->GetOutputDataObject(0) == mesh)
      {
      return;
      }
    else if (oldProducer && oldProducer->GetOutputDataObject(0))
      {
      oldProducer->GetOutputDataObject(0)->RemoveObservers(
        vtkCommand::ModifiedEvent, this->DataEventForwarder);
      }

    vtkNew<vtkTrivialProducer> tp;
    tp->SetOutput(mesh);
    // Propagate ModifiedEvent onto the trivial producer to make sure
    // MeshModifiedEvent is triggered.
    if (!this->DataEventForwarder)
      {
      this->DataEventForwarder = vtkEventForwarderCommand::New();
      }
    this->DataEventForwarder->SetTarget(tp.GetPointer());
    mesh->AddObserver(vtkCommand::ModifiedEvent, this->DataEventForwarder);

    if (polydata)
      {
      this->SetPolyDataConnection(tp->GetOutputPort());
      }
    else
      {
      this->SetUnstructuredGridConnection(tp->GetOutputPort());
      }
    }
}

//----------------------------------------------------------------------------
void vtkDMMLModelNode::SetAndObservePolyData(vtkPolyData *polyData)
{
  this->SetAndObserveMesh(polyData);
}

//---------------------------------------------------------------------------
vtkPointSet *vtkDMMLModelNode::GetMesh()
{
  if (!this->MeshConnection)
    {
    return nullptr;
    }

  vtkAlgorithm* producer = this->MeshConnection->GetProducer();

  if (!producer)
    {
    vtkErrorMacro("Could not retrieve mesh connection producer.");
    return nullptr;
    }

  producer->Update();

  return vtkPointSet::SafeDownCast(
    producer->GetOutputDataObject(this->MeshConnection->GetIndex()));
}

//---------------------------------------------------------------------------
vtkPolyData* vtkDMMLModelNode::GetPolyData()
{
  vtkPointSet* mesh = this->GetMesh();
  vtkPolyData* poly = vtkPolyData::SafeDownCast(mesh);
  if (poly && this->MeshType == vtkDMMLModelNode::UnstructuredGridMeshType)
    {
    vtkWarningMacro("GetPolyData() successfully returned a vtkPolyData while "
                    << "the MeshType was set to UnstructuredGridMeshType. This "
                    << "could happen if the pipeline input was wrongfully set "
                    << "with SetUnstructuredGridConnection() instead of "
                    << "SetPolyDataConnection().");
    }
  else if (!poly && this->MeshType == vtkDMMLModelNode::PolyDataMeshType &&
           vtkUnstructuredGrid::SafeDownCast(mesh))
    {
    vtkWarningMacro("GetPolyData() correctly returned a null pointer "
                    << "since the model's mesh is a vtkUnstructuredGrid, but "
                    << "the MeshType was set to PolyDataMeshType. This "
                    << "could happen if the pipeline input was wrongfully set "
                    << "with SetPolyDataConnection() instead of "
                    << "SetUnstructuredGridConnection().");
    }
  return poly;
}

//---------------------------------------------------------------------------
vtkUnstructuredGrid* vtkDMMLModelNode::GetUnstructuredGrid()
{
  vtkPointSet* mesh = this->GetMesh();
  vtkUnstructuredGrid* ug = vtkUnstructuredGrid::SafeDownCast(mesh);
  if (ug && this->MeshType == vtkDMMLModelNode::PolyDataMeshType)
    {
    vtkWarningMacro("GetUnstructuredGrid() successfully returned a vtkUnstructuredGrid"
                    << "while the MeshType was set to PolyDataMeshType. This "
                    << "could happen if the pipeline input was wrongfully set "
                    << "with SetPolyDataConnection() instead of "
                    << "SetUnstructuredGridConnection().");
    }
  else if (!ug && this->MeshType == vtkDMMLModelNode::UnstructuredGridMeshType &&
           vtkPolyData::SafeDownCast(mesh))
    {
    vtkWarningMacro("GetUnstructuredGrid() correctly returned a null pointer "
                    << "since the model's mesh is a vtkPolyData, but "
                    << "the MeshType was set to UnstructuredGridMeshType. This "
                    << "could happen if the pipeline input was wrongfully set "
                    << "with SetUnstructuredGridConnection() instead of "
                    << "SetPolyDataConnection().");
    }
  return ug;
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode
::SetMeshConnection(vtkAlgorithmOutput *newMeshConnection)
{
  if (newMeshConnection == this->MeshConnection)
    {
    return;
    }

  vtkAlgorithm* oldMeshAlgorithm = this->MeshConnection ?
    this->MeshConnection->GetProducer() : nullptr;

  this->MeshConnection = newMeshConnection;

  vtkAlgorithm* newMeshAlgorithm = this->MeshConnection ?
    this->MeshConnection->GetProducer() : nullptr;
  if (newMeshAlgorithm != nullptr)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      newMeshAlgorithm, vtkCommand::ModifiedEvent, this, this->DMMLCallbackCommand );
    newMeshAlgorithm->Register(this);
    }

  this->SetMeshToDisplayNodes();

  if (oldMeshAlgorithm != nullptr)
    {
    vtkEventBroker::GetInstance()->RemoveObservations (
      oldMeshAlgorithm, vtkCommand::ModifiedEvent, this, this->DMMLCallbackCommand );
    oldMeshAlgorithm->UnRegister(this);
    }

  this->StorableModifiedTime.Modified();
  this->Modified();
  this->InvokeCustomModifiedEvent( vtkDMMLModelNode::MeshModifiedEvent , this);
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode
::SetPolyDataConnection(vtkAlgorithmOutput *newPolyDataConnection)
{
  this->MeshType = vtkDMMLModelNode::PolyDataMeshType;
  this->SetMeshConnection(newPolyDataConnection);
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode
::SetUnstructuredGridConnection(vtkAlgorithmOutput *newUnstructuredGridConnection)
{
  this->MeshType = vtkDMMLModelNode::UnstructuredGridMeshType;
  this->SetMeshConnection(newUnstructuredGridConnection);
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLModelNode::GetPolyDataConnection()
{
  return (this->MeshType == vtkDMMLModelNode::PolyDataMeshType) ?
    this->GetMeshConnection() : nullptr;
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDMMLModelNode::GetUnstructuredGridConnection()
{
  return (this->MeshType == vtkDMMLModelNode::UnstructuredGridMeshType) ?
    this->GetMeshConnection() : nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode::AddPointScalars(vtkDataArray *array)
{
  this->AddScalars(array, vtkAssignAttribute::POINT_DATA);
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode::AddCellScalars(vtkDataArray *array)
{
  this->AddScalars(array, vtkAssignAttribute::CELL_DATA);
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode::AddScalars(vtkDataArray *array, int location)
{
  if (array == nullptr)
    {
    return;
    }
  if (this->GetMesh() == nullptr)
    {
    vtkErrorMacro("AddScalars: No mesh on model "
                  << (this->GetName() ? this->GetName() : "no_name"));
    return;
    }
  vtkDataSetAttributes* data =
    (location == vtkAssignAttribute::POINT_DATA ?
     vtkDataSetAttributes::SafeDownCast(this->GetMesh()->GetPointData()) :
     vtkDataSetAttributes::SafeDownCast(this->GetMesh()->GetCellData()));

  int numScalars = data->GetNumberOfArrays();
  vtkDebugMacro("Model node has " << numScalars << " scalars now, "
                << "adding " << array->GetName());

  data->AddArray(array);
  if (numScalars > 0)
    {
    // set the scalars
    data->SetActiveScalars(array->GetName());
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode::RemoveScalars(const char *scalarName)
{
  if (scalarName == nullptr)
    {
    vtkErrorMacro("Scalar name is null");
    return;
    }
  if (this->GetMesh() == nullptr)
    {
    vtkErrorMacro("RemoveScalars: No poly data on model "
                  << (this->GetName() ? this->GetName() : "no_name"));
    return;
    }
  // try removing the array from the points first
  if (this->GetMesh()->GetPointData())
    {
    this->GetMesh()->GetPointData()->RemoveArray(scalarName);
    // it's a void method, how to check if it succeeded?
    }
  // try the cells
  if (this->GetMesh()->GetCellData())
    {
    this->GetMesh()->GetCellData()->RemoveArray(scalarName);
    }
}

//---------------------------------------------------------------------------
const char * vtkDMMLModelNode::GetActivePointScalarName(int type)
{
  if (this->GetMesh() == nullptr ||
      this->GetMesh()->GetPointData() == nullptr)
    {
    return nullptr;
    }
  vtkAbstractArray* attributeArray =
    this->GetMesh()->GetPointData()->GetAbstractAttribute(type);
  return attributeArray ? attributeArray->GetName() : nullptr;
}

//---------------------------------------------------------------------------
const char * vtkDMMLModelNode::GetActiveCellScalarName(int type)
{
  if (this->GetMesh() == nullptr ||
      this->GetMesh()->GetCellData() == nullptr)
    {
    return nullptr;
    }
  vtkAbstractArray* attributeArray =
    this->GetMesh()->GetCellData()->GetAbstractAttribute(type);
  return attributeArray ? attributeArray->GetName() : nullptr;
}

//---------------------------------------------------------------------------
bool vtkDMMLModelNode::HasPointScalarName(const char* scalarName)
{
  if (this->GetMesh() == nullptr ||
      this->GetMesh()->GetPointData() == nullptr)
    {
    return false;
    }
  return static_cast<bool>(
    this->GetMesh()->GetPointData()->HasArray(scalarName));
}

//---------------------------------------------------------------------------
bool vtkDMMLModelNode::HasCellScalarName(const char* scalarName)
{
  if (this->GetMesh() == nullptr ||
      this->GetMesh()->GetCellData() == nullptr)
    {
    return false;
    }
  return static_cast<bool>(
    this->GetMesh()->GetCellData()->HasArray(scalarName));
}

//---------------------------------------------------------------------------
int vtkDMMLModelNode::GetAttributeTypeFromString(const char* typeName)
{
  if (typeName != nullptr && (strcmp(typeName, "") != 0))
    {
    for (int a = 0; a < vtkDataSetAttributes::NUM_ATTRIBUTES; a++)
      {
      if (strcmp(typeName, vtkDataSetAttributes::GetAttributeTypeAsString(a)) == 0)
        {
        return a;
        }
      }
    }
  return -1;
}

//---------------------------------------------------------------------------
int vtkDMMLModelNode::SetActivePointScalars(const char *scalarName, int attributeType)
{
  if (this->GetMesh() == nullptr)
    {
    return -1;
    }
  this->MeshConnection->GetProducer()->Update();
  vtkPointData* pointData = this->GetMesh()->GetPointData();
  return pointData? pointData->SetActiveAttribute(
    scalarName, attributeType) : 0;
}

//---------------------------------------------------------------------------
int vtkDMMLModelNode::SetActiveCellScalars(const char *scalarName, int attributeType)
{
  if (this->GetMesh() == nullptr)
    {
    return -1;
    }
  this->MeshConnection->GetProducer()->Update();
  vtkCellData* cellData = this->GetMesh()->GetCellData();
  return cellData ? cellData->SetActiveAttribute(
    scalarName, attributeType) : 0;
}

//---------------------------------------------------------------------------
int vtkDMMLModelNode::CompositeScalars(const char* backgroundName, const char* overlayName,
                                       float overlayMin, float overlayMax,
                                       int showOverlayPositive, int showOverlayNegative,
                                       int reverseOverlay)
{
    if (backgroundName == nullptr || overlayName == nullptr)
      {
      vtkErrorMacro("CompositeScalars: one of the input array names is null");
      return 0;
      }

    bool haveCurvScalars = false;
    // is there a curv scalar in the composite?
    if (strstr(backgroundName, "curv") != nullptr ||
        strstr(overlayName, "curv") != nullptr)
      {
      haveCurvScalars = true;
      }

    // get the scalars to composite, putting any curv file in scalars 1
    vtkDataArray *scalars1, *scalars2;
    if (!haveCurvScalars ||
        strstr(backgroundName, "curv") != nullptr)
      {
      scalars1 = this->GetMesh()->GetPointData()->GetScalars(backgroundName);
      scalars2 = this->GetMesh()->GetPointData()->GetScalars(overlayName);
      }
    else
      {
      scalars1 = this->GetMesh()->GetPointData()->GetScalars(overlayName);
      scalars2 = this->GetMesh()->GetPointData()->GetScalars(backgroundName);
      }
    if (scalars1 == nullptr || scalars2 == nullptr)
      {
      vtkErrorMacro("CompositeScalars: unable to find the named scalar arrays " << backgroundName << " and/or " << overlayName);
      return 0;
      }
    if (scalars1->GetNumberOfTuples() != scalars2->GetNumberOfTuples())
      {
      vtkErrorMacro("CompositeScalars: sizes of scalar arrays don't match");
      return 0;
      }
    // Get the number of elements and initialize the composed scalar
    // array.
    int cValues = 0;
    cValues = scalars1->GetNumberOfTuples();

    vtkFloatArray* composedScalars = vtkFloatArray::New();

    std::stringstream ss;
    ss << backgroundName;
    ss << "+";
    ss << overlayName;
    std::string composedName = std::string(ss.str());
    composedScalars->SetName(composedName.c_str());
    composedScalars->Allocate( cValues );
    composedScalars->SetNumberOfComponents( 1 );

    // For each value, check the overlay value. If it's < min, use
    // the background value. If we're reversing, reverse the overlay
    // value. If we're not showing one side, use the background
    // value. If we are showing curvature (and have it), the
    // background value is our curvature value.
    float overlayMid = 0.5 * (overlayMax - overlayMin) + overlayMin; // 2.0;
    vtkDebugMacro("CompositeScalars: using overlay mid = " << overlayMid);
    float overlay = 0.0;
    float background = 0.0;
    for( int nValue = 0; nValue < cValues; nValue++ )
      {
      background = scalars1->GetTuple1(nValue);
      overlay = scalars2->GetTuple1(nValue);

      if( reverseOverlay )
        {
        overlay = -overlay;
        }
      if( overlay > 0 && !showOverlayPositive )
        {
        overlay = 0;
        }

      if( overlay < 0 && !showOverlayNegative )
        {
        overlay = 0;
        }

      // Insert the appropriate color into the composed array.
      if( overlay < overlayMin &&
          overlay > -overlayMin )
        {
        composedScalars->InsertNextValue( background );
        }
      else
        {
        composedScalars->InsertNextValue( overlay );
        }
      }

    // set up a color node
    vtkDMMLProceduralColorNode *colorNode = vtkDMMLProceduralColorNode::New();
    colorNode->SetName(composedName.c_str());
    // set the type to avoid error messages when copy it, as the default is -1
    colorNode->SetType(vtkDMMLProceduralColorNode::User);
    vtkColorTransferFunction *func = colorNode->GetColorTransferFunction();

    // adapted from FS code that assumed that one scalar was curvature, the
    // other heat overlay
    const double EPS = 0.00001; // epsilon
    double curvatureMin = 0;

    if (haveCurvScalars)
      {
      curvatureMin = 0.5;
      }
    bool bUseGray = true;
    if( overlayMin <= curvatureMin )
      {
      curvatureMin = overlayMin - EPS;
      bUseGray = false;
      }
    func->AddRGBPoint( -overlayMax, 0, 1, 1 );
    func->AddRGBPoint( -overlayMid, 0, 0, 1 );
    func->AddRGBPoint( -overlayMin, 0, 0, 1 );

    if( bUseGray && overlayMin != 0 )
      {
      func->AddRGBPoint( -overlayMin + EPS, 0.5, 0.5, 0.5 );
      if( haveCurvScalars)
        {
        func->AddRGBPoint( -curvatureMin - EPS, 0.5, 0.5, 0.5 );
        }
      }
    if( haveCurvScalars && overlayMin != 0 )
      {
      func->AddRGBPoint( -curvatureMin, 0.6, 0.6, 0.6 );
      func->AddRGBPoint(  0,            0.6, 0.6, 0.6 );
      func->AddRGBPoint(  EPS,          0.4, 0.4, 0.4 );
      func->AddRGBPoint(  curvatureMin, 0.4, 0.4, 0.4 );
      }

    if ( bUseGray && overlayMin != 0 )
      {
      if( haveCurvScalars )
        {
        func->AddRGBPoint( curvatureMin + EPS, 0.5, 0.5, 0.5 );
        }
      func->AddRGBPoint( overlayMin - EPS, 0.5, 0.5, 0.5 );
      }

    func->AddRGBPoint( overlayMin, 1, 0, 0 );
    func->AddRGBPoint( overlayMid, 1, 0, 0 );
    func->AddRGBPoint( overlayMax, 1, 1, 0 );

    func->Build();

    // use the new colornode
    this->Scene->AddNode(colorNode);
    vtkDebugMacro("CompositeScalars: created color transfer function, and added proc color node to scene, id = " << colorNode->GetID());
    if (colorNode->GetID() != nullptr)
      {
      this->GetModelDisplayNode()->SetAndObserveColorNodeID(colorNode->GetID());
      this->GetModelDisplayNode()->SetScalarRange(-overlayMax, overlayMax);
      }

    // add the new scalars
    this->AddPointScalars(composedScalars);

    // make them active
    this->GetModelDisplayNode()->SetActiveScalarName(composedName.c_str());

    // clean up
    colorNode->Delete();
    colorNode = nullptr;
    composedScalars->Delete();
    composedScalars = nullptr;

    return 1;
}

//---------------------------------------------------------------------------
bool vtkDMMLModelNode::CanApplyNonLinearTransforms()const
{
  return true;
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode::ApplyTransform(vtkAbstractTransform* transform)
{
  if (this->GetMesh() == nullptr)
    {
    return;
    }

  vtkTransformFilter* transformFilter = vtkTransformFilter::New();
  transformFilter->SetInputConnection(this->MeshConnection);
  transformFilter->SetTransform(transform);

  bool isInPipeline = !vtkTrivialProducer::SafeDownCast(
     this->MeshConnection ? this->MeshConnection->GetProducer() : nullptr);

  // If mesh was set through pipeline (SetMeshConnection), append
  // transform filter to that pipeline
  if (isInPipeline)
    {
    this->SetMeshConnection(transformFilter->GetOutputPort());
    }
  // Else, if mesh was set as data object (SetAndObserveMesh: uses
  // vtkTrivialProducer to produce the mesh connection), apply the
  // transformation to the data object directly
  else
    {
    transformFilter->Update();
    vtkPointSet * mesh = this->GetMesh();
    mesh->DeepCopy(transformFilter->GetOutput());
    }
  transformFilter->Delete();
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode::GetRASBounds(double bounds[6])
{
  double localBounds[6];
  this->GetBounds(localBounds);
  this->TransformBoundsToRAS(localBounds, bounds);
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode::GetBounds(double bounds[6])
{
  this->Superclass::GetBounds(bounds);

  if (this->GetMesh() == nullptr)
    {
    return;
    }

  this->GetMesh()->ComputeBounds();
  this->GetMesh()->GetBounds(bounds);
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode::TransformBoundsToRAS(double inputBounds_Local[6], double outputBounds_RAS[6])
{
  vtkDMMLTransformNode *transformNode = this->GetParentTransformNode();
  if ( !transformNode )
    {
    // node is not transformed, therefore RAS=local
    for (int i=0; i<6; i++)
      {
      outputBounds_RAS[i]=inputBounds_Local[i];
      }
    return;
    }

  vtkNew<vtkGeneralTransform> transformLocalToRAS;
  transformNode->GetTransformToWorld(transformLocalToRAS.GetPointer());

  double cornerPoints_Local[8][4] =
    {
    {inputBounds_Local[0], inputBounds_Local[2], inputBounds_Local[4], 1},
    {inputBounds_Local[0], inputBounds_Local[3], inputBounds_Local[4], 1},
    {inputBounds_Local[0], inputBounds_Local[2], inputBounds_Local[5], 1},
    {inputBounds_Local[0], inputBounds_Local[3], inputBounds_Local[5], 1},
    {inputBounds_Local[1], inputBounds_Local[2], inputBounds_Local[4], 1},
    {inputBounds_Local[1], inputBounds_Local[3], inputBounds_Local[4], 1},
    {inputBounds_Local[1], inputBounds_Local[2], inputBounds_Local[5], 1},
    {inputBounds_Local[1], inputBounds_Local[3], inputBounds_Local[5], 1}
    };

  // initialize bounds with point 0
  double* cornerPoint_RAS = transformLocalToRAS->TransformDoublePoint(cornerPoints_Local[0]);
  for ( int i=0; i<3; i++)
    {
    outputBounds_RAS[2*i]   = cornerPoint_RAS[i];
    outputBounds_RAS[2*i+1] = cornerPoint_RAS[i];
    }

  // update bounds with the rest of the points
  for ( int i=1; i<8; i++)
    {
    cornerPoint_RAS = transformLocalToRAS->TransformPoint( cornerPoints_Local[i] );
    for (int n=0; n<3; n++)
      {
      if (cornerPoint_RAS[n] < outputBounds_RAS[2*n]) // min bound
        {
        outputBounds_RAS[2*n] = cornerPoint_RAS[n];
        }
      if (cornerPoint_RAS[n] > outputBounds_RAS[2*n+1]) // max bound
        {
        outputBounds_RAS[2*n+1] = cornerPoint_RAS[n];
        }
      }
   }
}

//---------------------------------------------------------------------------
vtkDMMLStorageNode* vtkDMMLModelNode::CreateDefaultStorageNode()
{
  vtkDMMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkDMMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkDMMLModelStorageNode"));
}

//---------------------------------------------------------------------------
std::string vtkDMMLModelNode::GetDefaultStorageNodeClassName(const char* vtkNotUsed(filename) /* =nullptr */)
{
  return "vtkDMMLModelStorageNode";
}

//----------------------------------------------------------------------------
void vtkDMMLModelNode::CreateDefaultDisplayNodes()
{
  if (vtkDMMLModelDisplayNode::SafeDownCast(this->GetDisplayNode())!=nullptr)
    {
    // display node already exists
    return;
    }
  vtkDMMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("vtkDMMLModelNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkDMMLModelDisplayNode* dispNode = vtkDMMLModelDisplayNode::SafeDownCast(
    scene->AddNewNodeByClass("vtkDMMLModelDisplayNode") );

  if (this->GetMesh())
    {
    dispNode->SetInputMeshConnection(this->MeshConnection);
    }

  this->SetAndObserveDisplayNodeID( dispNode->GetID() );

  this->ShowDefaultScalarData();
}

//----------------------------------------------------------------------------
bool vtkDMMLModelNode::ShowDefaultScalarData()
{
  if (!this->GetMesh())
    {
    return false;
    }
  vtkDMMLDisplayNode* displayNode = this->GetDisplayNode();
  if (!displayNode)
    {
    vtkWarningMacro("Cannot show scalar data: no display node is available");
    return false;
    }

  // If an RGB or RGBA array (3 or 4 component unsigned char array) is found
  // then use that directly as point or cell colors. This is most commonly used
  // in VTK, VTP, and PLY files.

  // Try cell data first because it is slightly preferable, as
  // it does not need to interpolate the data for coloring the surface
  // (colored region boundaries appear sharp, not blurred).
  vtkCellData* cellData = this->GetMesh()->GetCellData();
  if (cellData && cellData->GetNumberOfArrays() > 0)
    {
    // RGB(A) color
    for (int arrayIndex = 0; arrayIndex < cellData->GetNumberOfArrays(); arrayIndex++)
      {
      vtkUnsignedCharArray* dataArray = vtkUnsignedCharArray::SafeDownCast(cellData->GetArray(arrayIndex));
      if (!dataArray || !dataArray->GetName())
        {
        continue;
        }
      if (dataArray->GetNumberOfComponents() == 3 || dataArray->GetNumberOfComponents() == 4)
        {
        // Found a suitable RGB or RGBA array
        displayNode->SetActiveScalar(dataArray->GetName(), vtkAssignAttribute::CELL_DATA);
        displayNode->SetScalarRangeFlag(vtkDMMLDisplayNode::UseDirectMapping);
        displayNode->SetScalarVisibility(true);
        return true;
        }
      }
    // Simple scalar
    if (cellData->GetScalars())
      {
      vtkDebugMacro("Made a new model display node, there are cell scalars defined "
          "on the model - setting them visible and using the first one as the selected overlay");
      displayNode->SetScalarVisibility(1);
      displayNode->SetActiveScalar(cellData->GetAttribute(0)->GetName(), vtkAssignAttribute::CELL_DATA);
      displayNode->SetAndObserveColorNodeID("vtkDMMLColorTableNodeFileViridis.txt");
      return true;
      }
    }

  vtkPointData* pointData = this->GetMesh()->GetPointData();
  if (pointData && pointData->GetNumberOfArrays() > 0)
    {
    // RGB(A) color
    for (int arrayIndex = 0; arrayIndex < pointData->GetNumberOfArrays(); arrayIndex++)
      {
      vtkUnsignedCharArray* dataArray = vtkUnsignedCharArray::SafeDownCast(pointData->GetArray(arrayIndex));
      if (!dataArray || !dataArray->GetName())
        {
        continue;
        }
      if (dataArray->GetNumberOfComponents() == 3 || dataArray->GetNumberOfComponents() == 4)
        {
        // Found a suitable RGB or RGBA array
        displayNode->SetActiveScalar(dataArray->GetName(), vtkAssignAttribute::POINT_DATA);
        displayNode->SetScalarRangeFlag(vtkDMMLDisplayNode::UseDirectMapping);
        displayNode->SetScalarVisibility(true);
        return true;
        }
      }
    // Simple scalar
    if (pointData->GetScalars())
      {
      vtkDebugMacro("Made a new model display node, there are point scalars defined "
          "on the model - setting them visible and using the first one as the selected overlay");
      displayNode->SetScalarVisibility(1);
      displayNode->SetActiveScalar(pointData->GetAttribute(0)->GetName(), vtkAssignAttribute::POINT_DATA);
      displayNode->SetAndObserveColorNodeID("vtkDMMLColorTableNodeFileViridis.txt");
      return true;
      }
    }

  return false;
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode::OnNodeReferenceAdded(vtkDMMLNodeReference *reference)
{
  if (std::string(reference->GetReferenceRole()) == this->DisplayNodeReferenceRole)
    {
    this->UpdateDisplayNodeMesh(vtkDMMLDisplayNode::SafeDownCast(reference->GetReferencedNode()));
    }
  Superclass::OnNodeReferenceAdded(reference);
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode::OnNodeReferenceModified(vtkDMMLNodeReference *reference)
{
   this->UpdateDisplayNodeMesh(vtkDMMLDisplayNode::SafeDownCast(reference->GetReferencedNode()));
   Superclass::OnNodeReferenceModified(reference);
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode::UpdateDisplayNodeMesh(vtkDMMLDisplayNode *dnode)
{
  vtkDMMLModelDisplayNode* modelDisplayNode =
    vtkDMMLModelDisplayNode::SafeDownCast(dnode);
  if (modelDisplayNode)
    {
    this->SetMeshToDisplayNode(modelDisplayNode);
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode
::SetMeshToDisplayNodes()
{
  int ndisp = this->GetNumberOfDisplayNodes();
  for (int n=0; n<ndisp; n++)
    {
    vtkDMMLModelDisplayNode *dnode = vtkDMMLModelDisplayNode::SafeDownCast(
      this->GetNthDisplayNode(n));
    if (dnode)
      {
      this->SetMeshToDisplayNode(dnode);
      }
    }
}

//---------------------------------------------------------------------------
void vtkDMMLModelNode
::SetMeshToDisplayNode(vtkDMMLModelDisplayNode* modelDisplayNode)
{
  assert(modelDisplayNode);
  modelDisplayNode->SetInputMeshConnection(this->MeshConnection);
}

//---------------------------------------------------------------------------
bool vtkDMMLModelNode::GetModifiedSinceRead()
{
  return this->Superclass::GetModifiedSinceRead() ||
    (this->GetMesh() && this->GetMesh()->GetMTime() > this->GetStoredTime());
}
