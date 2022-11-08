/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Cjyx
  Module:    $RCSfile: vtkDMMLColorLogic.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

// DMMLLogic includes
#include "vtkDMMLColorLogic.h"

// DMML includes
#include "vtkDMMLColorTableNode.h"
#include "vtkDMMLColorTableStorageNode.h"
#include "vtkDMMLdGEMRICProceduralColorNode.h"
#include "vtkDMMLPETProceduralColorNode.h"
#include "vtkDMMLProceduralColorStorageNode.h"
#include "vtkDMMLScalarVolumeDisplayNode.h"
#include "vtkDMMLScene.h"

// VTK sys includes
#include <vtkLookupTable.h>
#include <vtksys/SystemTools.hxx>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <ctype.h> // For isspace
#include <functional>
#include <random>
#include <sstream>

//----------------------------------------------------------------------------
std::string vtkDMMLColorLogic::TempColorNodeID;

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLColorLogic);

//----------------------------------------------------------------------------
vtkDMMLColorLogic::vtkDMMLColorLogic()
{
  this->UserColorFilePaths = nullptr;
}

//----------------------------------------------------------------------------
vtkDMMLColorLogic::~vtkDMMLColorLogic()
{
  // remove the default color nodes
  this->RemoveDefaultColorNodes();

  // clear out the lists of files
  this->ColorFiles.clear();
  this->UserColorFiles.clear();

  if (this->UserColorFilePaths)
    {
    delete [] this->UserColorFilePaths;
    this->UserColorFilePaths = nullptr;
    }
}

//------------------------------------------------------------------------------
void vtkDMMLColorLogic::SetDMMLSceneInternal(vtkDMMLScene* newScene)
{
  // We are solely interested in vtkDMMLScene::NewSceneEvent,
  // we don't want to listen to any other events.
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkDMMLScene::NewSceneEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, sceneEvents.GetPointer());

  if (newScene)
    {
    this->OnDMMLSceneNewEvent();
    }
}

//------------------------------------------------------------------------------
void vtkDMMLColorLogic::OnDMMLSceneNewEvent()
{
  this->AddDefaultColorNodes();
}

//----------------------------------------------------------------------------
void vtkDMMLColorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkDMMLColorLogic:             " << this->GetClassName() << "\n";

  os << indent << "UserColorFilePaths: " << this->GetUserColorFilePaths() << "\n";
  os << indent << "Color Files:\n";
  for (size_t i = 0; i < this->ColorFiles.size(); i++)
    {
    os << indent.GetNextIndent() << i << " " << this->ColorFiles[i].c_str() << "\n";
    }
  os << indent << "User Color Files:\n";
  for (size_t i = 0; i < this->UserColorFiles.size(); i++)
    {
    os << indent.GetNextIndent() << i << " " << this->UserColorFiles[i].c_str() << "\n";
    }
}

//----------------------------------------------------------------------------
void vtkDMMLColorLogic::AddDefaultColorNodes()
{
  // create the default color nodes, they don't get saved with the scenes as
  // they'll be created on start up, and when a new
  // scene is opened
  if (this->GetDMMLScene() == nullptr)
    {
    vtkWarningMacro("vtkDMMLColorLogic::AddDefaultColorNodes: no scene to which to add nodes\n");
    return;
    }

  this->GetDMMLScene()->StartState(vtkDMMLScene::BatchProcessState);

  // add the labels first
  this->AddLabelsNode();

  // add the rest of the default color table nodes
  this->AddDefaultTableNodes();

  // add default procedural nodes, including a random one
  this->AddDefaultProceduralNodes();

  // add the PET nodes
  this->AddPETNodes();

  // add the dGEMRIC nodes
  this->AddDGEMRICNodes();

  // file based labels
  // first check for any new ones

  // load the one from the default resources directory
  this->AddDefaultFileNodes();

  // now add ones in files that the user pointed to, these ones are not hidden
  // from the editors
  this->AddUserFileNodes();

  vtkDebugMacro("Done adding default color nodes");
  this->GetDMMLScene()->EndState(vtkDMMLScene::BatchProcessState);
}

//----------------------------------------------------------------------------
void vtkDMMLColorLogic::RemoveDefaultColorNodes()
{
  // try to find any of the default color nodes that are still in the scene
  if (this->GetDMMLScene() == nullptr)
    {
    // nothing can do, it's gone
    return;
    }

  this->GetDMMLScene()->StartState(vtkDMMLScene::BatchProcessState);

  vtkDMMLColorTableNode *basicNode = vtkDMMLColorTableNode::New();
  vtkDMMLColorTableNode *node;
  for (int i = basicNode->GetFirstType(); i <= basicNode->GetLastType(); i++)
    {
    // don't have a File node...
    if (i != vtkDMMLColorTableNode::File
        && i != vtkDMMLColorTableNode::Obsolete)
      {
      //std::string id = std::string(this->GetColorTableNodeID(i));
      const char* id = this->GetColorTableNodeID(i);
      vtkDebugMacro("vtkDMMLColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id << endl);
      node = vtkDMMLColorTableNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(id));
      if (node != nullptr)
        {
        this->GetDMMLScene()->RemoveNode(node);
        }
      }
    }
  basicNode->Delete();

   // remove the procedural color nodes (after the fs proc nodes as
   // getting them by class)
  std::vector<vtkDMMLNode *> procNodes;
  int numProcNodes = this->GetDMMLScene()->GetNodesByClass("vtkDMMLProceduralColorNode", procNodes);
  for (int i = 0; i < numProcNodes; i++)
    {
    vtkDMMLProceduralColorNode* procNode = vtkDMMLProceduralColorNode::SafeDownCast(procNodes[i]);
    if (procNode != nullptr &&
        strcmp(procNode->GetID(), this->GetProceduralColorNodeID(procNode->GetName())) == 0)
      {
      // it's one we added
      this->GetDMMLScene()->RemoveNode(procNode);
      }
    }

  // remove the PET nodes
  vtkDMMLPETProceduralColorNode *basicPETNode = vtkDMMLPETProceduralColorNode::New();
  vtkDMMLPETProceduralColorNode *PETnode;
  for (int i = basicPETNode->GetFirstType(); i <= basicPETNode->GetLastType(); i++)
    {
    basicPETNode->SetType(i);
    const char* id = this->GetPETColorNodeID(i);
    vtkDebugMacro("vtkDMMLColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id << endl);
    PETnode =  vtkDMMLPETProceduralColorNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(id));
    if (PETnode != nullptr)
      {
      this->GetDMMLScene()->RemoveNode(PETnode);
      }
    }
  basicPETNode->Delete();

  // remove the dGEMRIC nodes
  vtkDMMLdGEMRICProceduralColorNode *basicdGEMRICNode = vtkDMMLdGEMRICProceduralColorNode::New();
  vtkDMMLdGEMRICProceduralColorNode *dGEMRICnode;
  for (int i = basicdGEMRICNode->GetFirstType(); i <= basicdGEMRICNode->GetLastType(); i++)
    {
    basicdGEMRICNode->SetType(i);
    const char* id = this->GetdGEMRICColorNodeID(i);
    vtkDebugMacro("vtkDMMLColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id << endl);
    dGEMRICnode =  vtkDMMLdGEMRICProceduralColorNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(id));
    if (dGEMRICnode != nullptr)
      {
      this->GetDMMLScene()->RemoveNode(dGEMRICnode);
      }
    }
  basicdGEMRICNode->Delete();

  // remove the file based labels node
  for (unsigned int i = 0; i < this->ColorFiles.size(); i++)
    {
    node =  vtkDMMLColorTableNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(this->GetFileColorNodeID(this->ColorFiles[i].c_str())));
    if (node != nullptr)
      {
      this->GetDMMLScene()->RemoveNode(node);
      }
    }
  for (unsigned int i = 0; i < this->UserColorFiles.size(); i++)
    {
    node =  vtkDMMLColorTableNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(this->GetFileColorNodeID(this->UserColorFiles[i].c_str())));
    if (node != nullptr)
      {
      this->GetDMMLScene()->RemoveNode(node);
      }
    }
  this->GetDMMLScene()->EndState(vtkDMMLScene::BatchProcessState);
}

//----------------------------------------------------------------------------
const char *vtkDMMLColorLogic::GetColorTableNodeID(int type)
{
  vtkNew<vtkDMMLColorTableNode> basicNode;
  basicNode->SetType(type);
  return vtkDMMLColorLogic::GetColorNodeID(basicNode.GetPointer());
}

//----------------------------------------------------------------------------
const char * vtkDMMLColorLogic::GetPETColorNodeID (int type )
{
  vtkNew<vtkDMMLPETProceduralColorNode> basicNode;
  basicNode->SetType(type);
  return vtkDMMLColorLogic::GetColorNodeID(basicNode.GetPointer());
}

//----------------------------------------------------------------------------
const char * vtkDMMLColorLogic::GetdGEMRICColorNodeID(int type)
{
  vtkNew<vtkDMMLdGEMRICProceduralColorNode> basicNode;
  basicNode->SetType(type);
  return vtkDMMLColorLogic::GetColorNodeID(basicNode.GetPointer());
}

//----------------------------------------------------------------------------
const char *vtkDMMLColorLogic::GetColorNodeID(vtkDMMLColorNode* colorNode)
{
  assert(colorNode);
  std::string id = std::string(colorNode->GetClassName()) +
                   std::string(colorNode->GetTypeAsString());
  vtkDMMLColorLogic::TempColorNodeID = id;
  return vtkDMMLColorLogic::TempColorNodeID.c_str();
}

//----------------------------------------------------------------------------
const char * vtkDMMLColorLogic::GetProceduralColorNodeID(const char *name)
{
  std::string id = std::string("vtkDMMLProceduralColorNode") + std::string(name);
  vtkDMMLColorLogic::TempColorNodeID = id;
  return vtkDMMLColorLogic::TempColorNodeID.c_str();
}

//----------------------------------------------------------------------------
std::string vtkDMMLColorLogic::GetFileColorNodeSingletonTag(const char * fileName)
{
  std::string singleton = std::string("File") +
    vtksys::SystemTools::GetFilenameName(fileName);
  return singleton;
}

//----------------------------------------------------------------------------
const char *vtkDMMLColorLogic::GetFileColorNodeID(const char * fileName)
{
  std::string id = std::string("vtkDMMLColorTableNode") +
                   vtkDMMLColorLogic::GetFileColorNodeSingletonTag(fileName);
  vtkDMMLColorLogic::TempColorNodeID = id;
  return vtkDMMLColorLogic::TempColorNodeID.c_str();
}

//----------------------------------------------------------------------------
const char *vtkDMMLColorLogic::GetDefaultVolumeColorNodeID()
{
  // If color node is specified in default vtkDMMLScalarVolumeDisplayNode then use that.
  vtkDMMLScene* scene = this->GetDMMLScene();
  if (scene)
    {
    vtkDMMLScalarVolumeDisplayNode* defaultDisplayNode =
      vtkDMMLScalarVolumeDisplayNode::SafeDownCast(scene->GetDefaultNodeByClass("vtkDMMLScalarVolumeDisplayNode"));
    if (defaultDisplayNode && defaultDisplayNode->GetColorNodeID())
      {
      return defaultDisplayNode->GetColorNodeID();
      }
    }
  return vtkDMMLColorLogic::GetColorTableNodeID(vtkDMMLColorTableNode::Grey);
}

//----------------------------------------------------------------------------
const char *vtkDMMLColorLogic::GetDefaultLabelMapColorNodeID()
{
  return vtkDMMLColorLogic::GetProceduralColorNodeID("RandomIntegers");
}

//----------------------------------------------------------------------------
const char *vtkDMMLColorLogic::GetDefaultEditorColorNodeID()
{
  return vtkDMMLColorLogic::GetProceduralColorNodeID("RandomIntegers");
}

//----------------------------------------------------------------------------
const char *vtkDMMLColorLogic::GetDefaultModelColorNodeID()
{
  return vtkDMMLColorLogic::GetProceduralColorNodeID("RedGreenBlue");
}

//----------------------------------------------------------------------------
const char *vtkDMMLColorLogic::GetDefaultChartColorNodeID()
{
  return vtkDMMLColorLogic::GetProceduralColorNodeID("RandomIntegers");
}

//----------------------------------------------------------------------------
const char *vtkDMMLColorLogic::GetDefaultPlotColorNodeID()
{
  return vtkDMMLColorLogic::GetProceduralColorNodeID("RandomIntegers");
}

//----------------------------------------------------------------------------
void vtkDMMLColorLogic::AddColorFile(const char *fileName, std::vector<std::string> *Files)
{
  if (fileName == nullptr)
    {
    vtkErrorMacro("AddColorFile: can't add a null color file name");
    return;
    }
  if (Files == nullptr)
    {
    vtkErrorMacro("AddColorFile: no array to which to add color file to!");
    return;
    }
  // check if it's in the vector already
  std::string fileNameStr = std::string(fileName);
  for (unsigned int i = 0; i <  Files->size(); i++)
    {
    std::string fileToCheck;
    try
      {
      fileToCheck = Files->at(i);
      }
    catch (...)
      {
      // an out_of_range exception can be thrown.
      }
    if (fileToCheck.compare(fileNameStr) == 0)
      {
      vtkDebugMacro("AddColorFile: already have this file at index " << i << ", not adding it again: " << fileNameStr.c_str());
      return;
      }
    }
  vtkDebugMacro("AddColorFile: adding file name to Files: " << fileNameStr.c_str());
  Files->push_back(fileNameStr);
}

//----------------------------------------------------------------------------
vtkDMMLColorNode* vtkDMMLColorLogic::LoadColorFile(const char *fileName, const char *nodeName)
{
  // try loading it as a color table node first
  vtkDMMLColorTableNode* node = this->CreateFileNode(fileName);
  vtkDMMLColorNode * addedNode = nullptr;

  if (node)
    {
    node->SetAttribute("Category", "File");
    node->SaveWithSceneOn();
    node->GetStorageNode()->SaveWithSceneOn();
    node->HideFromEditorsOff();
    node->SetSingletonTag(nullptr);

    if (nodeName != nullptr)
      {
      std::string uname( this->GetDMMLScene()->GetUniqueNameByString(nodeName));
      node->SetName(uname.c_str());
      }
    addedNode =
      vtkDMMLColorNode::SafeDownCast(this->GetDMMLScene()->AddNode(node));
    vtkDebugMacro("LoadColorFile: Done: Read and added file node: " <<  fileName);
    node->Delete();
    }
  else
    {
    // try loading it as a procedural node
    vtkWarningMacro("Trying to read color file as a procedural color node");
    vtkDMMLProceduralColorNode *procNode = this->CreateProceduralFileNode(fileName);
    if (procNode)
      {
      procNode->SetAttribute("Category", "File");
      procNode->SaveWithSceneOn();
      procNode->GetStorageNode()->SaveWithSceneOn();
      procNode->HideFromEditorsOff();
      procNode->SetSingletonTag(nullptr);

      if (nodeName != nullptr)
        {
        std::string uname( this->GetDMMLScene()->GetUniqueNameByString(nodeName));
        procNode->SetName(uname.c_str());
        }
      addedNode =
        vtkDMMLColorNode::SafeDownCast(this->GetDMMLScene()->AddNode(procNode));
      vtkDebugMacro("LoadColorFile: Done: Read and added file procNode: " <<  fileName);
      procNode->Delete();
      }
    }
  return addedNode;
}

//------------------------------------------------------------------------------
vtkDMMLColorTableNode* vtkDMMLColorLogic::CreateLabelsNode()
{
  vtkDMMLColorTableNode *labelsNode = vtkDMMLColorTableNode::New();
  labelsNode->SetTypeToLabels();
  labelsNode->SetAttribute("Category", "Discrete");
  labelsNode->SaveWithSceneOff();
  labelsNode->SetName(labelsNode->GetTypeAsString());
  labelsNode->SetSingletonTag(labelsNode->GetTypeAsString());
  return labelsNode;
}

//------------------------------------------------------------------------------
vtkDMMLColorTableNode* vtkDMMLColorLogic::CreateDefaultTableNode(int type)
{
  vtkDMMLColorTableNode *node = vtkDMMLColorTableNode::New();
  node->SetType(type);
  const char* typeName = node->GetTypeAsString();
  if (strstr(typeName, "Tint") != nullptr)
    {
    node->SetAttribute("Category", "Tint");
    }
  else if (strstr(typeName, "Shade") != nullptr)
    {
    node->SetAttribute("Category", "Shade");
    }
  else
    {
    node->SetAttribute("Category", "Discrete");
    }
  if (strcmp(typeName, "(unknown)") == 0)
    {
    return node;
    }
  node->SaveWithSceneOff();
  node->SetName(node->GetTypeAsString());
  node->SetSingletonTag(node->GetTypeAsString());
  return node;
}

//------------------------------------------------------------------------------
vtkDMMLProceduralColorNode* vtkDMMLColorLogic::CreateRandomNode()
{
  vtkDebugMacro("vtkDMMLColorLogic::CreateRandomNode: making a random  dmml proc color node");
  vtkDMMLProceduralColorNode *procNode = vtkDMMLProceduralColorNode::New();
  procNode->SetName("RandomIntegers");
  procNode->SetAttribute("Category", "Discrete");
  procNode->SaveWithSceneOff();
  procNode->SetSingletonTag(procNode->GetTypeAsString());

  std::default_random_engine randomGenerator(std::random_device{}());

  vtkColorTransferFunction *func = procNode->GetColorTransferFunction();
  const int dimension = 1000;
  double table[3*dimension];
  double* tablePtr = table;
  for (int i = 0; i < dimension; ++i)
    {
    *tablePtr++ = static_cast<double>(randomGenerator()) / randomGenerator.max();
    *tablePtr++ = static_cast<double>(randomGenerator()) / randomGenerator.max();
    *tablePtr++ = static_cast<double>(randomGenerator()) / randomGenerator.max();
    }
  func->BuildFunctionFromTable(VTK_INT_MIN, VTK_INT_MAX, dimension, table);
  func->Build();
  procNode->SetNamesFromColors();

  return procNode;
}

//------------------------------------------------------------------------------
vtkDMMLProceduralColorNode* vtkDMMLColorLogic::CreateRedGreenBlueNode()
{
  vtkDebugMacro("vtkDMMLColorLogic::AddDefaultColorNodes: making a red - green - blue dmml proc color node");
  vtkDMMLProceduralColorNode *procNode = vtkDMMLProceduralColorNode::New();
  procNode->SetName("RedGreenBlue");
  procNode->SetAttribute("Category", "Continuous");
  procNode->SaveWithSceneOff();
  procNode->SetSingletonTag(procNode->GetTypeAsString());
  procNode->SetDescription("A color transfer function that maps from -6 to 6, red through green to blue");
  vtkColorTransferFunction *func = procNode->GetColorTransferFunction();
  func->SetColorSpaceToRGB();
  func->AddRGBPoint(-6.0, 1.0, 0.0, 0.0);
  func->AddRGBPoint(0.0, 0.0, 1.0, 0.0);
  func->AddRGBPoint(6.0, 0.0, 0.0, 1.0);

  procNode->SetNamesFromColors();

  return procNode;
}

//--------------------------------------------------------------------------------
vtkDMMLPETProceduralColorNode* vtkDMMLColorLogic::CreatePETColorNode(int type)
{
  vtkDMMLPETProceduralColorNode *nodepcn = vtkDMMLPETProceduralColorNode::New();
  nodepcn->SetType(type);
  nodepcn->SetAttribute("Category", "PET");
  nodepcn->SaveWithSceneOff();

  if (nodepcn->GetTypeAsString() == nullptr)
    {
    vtkWarningMacro("Node type as string is null");
    nodepcn->SetName("NoName");
    }
  else
    {
    vtkDebugMacro("Got node type as string " << nodepcn->GetTypeAsString());
    nodepcn->SetName(nodepcn->GetTypeAsString());
    }

  nodepcn->SetSingletonTag(nodepcn->GetTypeAsString());

  return nodepcn;
}

//---------------------------------------------------------------------------------
vtkDMMLdGEMRICProceduralColorNode* vtkDMMLColorLogic::CreatedGEMRICColorNode(int type)
{
  vtkDMMLdGEMRICProceduralColorNode *pcnode = vtkDMMLdGEMRICProceduralColorNode::New();
  pcnode->SetType(type);
  pcnode->SetAttribute("Category", "Cartilage MRI");
  pcnode->SaveWithSceneOff();
  if (pcnode->GetTypeAsString() == nullptr)
    {
    vtkWarningMacro("Node type as string is null");
    pcnode->SetName("NoName");
    }
  else
    {
    vtkDebugMacro("Got node type as string " << pcnode->GetTypeAsString());
    pcnode->SetName(pcnode->GetTypeAsString());
    }

  pcnode->SetSingletonTag(pcnode->GetTypeAsString());

  return pcnode;
}

//---------------------------------------------------------------------------------
vtkDMMLColorTableNode* vtkDMMLColorLogic::CreateDefaultFileNode(const std::string& colorFileName)
{
  vtkDMMLColorTableNode* ctnode = this->CreateFileNode(colorFileName.c_str());

  if (!ctnode)
    {
    return nullptr;
    }

  if (strcmp(ctnode->GetName(),"GenericColors") == 0 ||
      strcmp(ctnode->GetName(),"GenericAnatomyColors") == 0)
    {
    vtkDebugMacro("Found default lut node");
    // No category to float to the top of the node
    // can't unset an attribute, so just don't set it at all
    }
  else
    {
    ctnode->SetAttribute("Category", "Default Labels from File");
    }

  return ctnode;
}

//---------------------------------------------------------------------------------
vtkDMMLColorTableNode* vtkDMMLColorLogic::CreateUserFileNode(const std::string& colorFileName)
{
  vtkDMMLColorTableNode * ctnode = this->CreateFileNode(colorFileName.c_str());
  if (ctnode == nullptr)
    {
    return nullptr;
    }
  ctnode->SetAttribute("Category", "Auto Loaded User Color Files");
  ctnode->SaveWithSceneOn();
  ctnode->HideFromEditorsOff();

  return ctnode;
}

//--------------------------------------------------------------------------------
std::vector<std::string> vtkDMMLColorLogic::FindDefaultColorFiles()
{
  return std::vector<std::string>();
}

//--------------------------------------------------------------------------------
std::vector<std::string> vtkDMMLColorLogic::FindUserColorFiles()
{
  return std::vector<std::string>();
}

//--------------------------------------------------------------------------------
vtkDMMLColorTableNode* vtkDMMLColorLogic::CreateFileNode(const char* fileName)
{
  vtkDMMLColorTableNode * ctnode =  vtkDMMLColorTableNode::New();
  ctnode->SetTypeToFile();
  ctnode->SaveWithSceneOff();
  ctnode->HideFromEditorsOn();
  ctnode->SetScene(this->GetDMMLScene());

  // make a storage node
  vtkNew<vtkDMMLColorTableStorageNode> colorStorageNode;
  colorStorageNode->SaveWithSceneOff();
  if (this->GetDMMLScene())
    {
    this->GetDMMLScene()->AddNode(colorStorageNode.GetPointer());
    ctnode->SetAndObserveStorageNodeID(colorStorageNode->GetID());
    }

  ctnode->GetStorageNode()->SetFileName(fileName);
  std::string basename = ctnode->GetStorageNode()->GetFileNameWithoutExtension(fileName);
  if (this->GetDMMLScene())
    {
    std::string uname(this->GetDMMLScene()->GetUniqueNameByString(basename.c_str()));
    ctnode->SetName(uname.c_str());
    }
  else
    {
    ctnode->SetName(basename.c_str());
    }
  vtkDebugMacro("CreateFileNode: About to read user file " << fileName);

  if (ctnode->GetStorageNode()->ReadData(ctnode) == 0)
    {
    vtkErrorMacro("Unable to read file as color table " << (ctnode->GetFileName() ? ctnode->GetFileName() : ""));

    if (this->GetDMMLScene())
      {
      ctnode->SetAndObserveStorageNodeID(nullptr);
      ctnode->SetScene(nullptr);
      this->GetDMMLScene()->RemoveNode(colorStorageNode.GetPointer());
      }

      ctnode->Delete();
      return nullptr;
    }
  vtkDebugMacro("CreateFileNode: finished reading user file " << fileName);
  ctnode->SetSingletonTag(
    this->GetFileColorNodeSingletonTag(fileName).c_str());

  return ctnode;
}

//--------------------------------------------------------------------------------
vtkDMMLProceduralColorNode* vtkDMMLColorLogic::CreateProceduralFileNode(const char* fileName)
{
  vtkDMMLProceduralColorNode * cpnode =  vtkDMMLProceduralColorNode::New();
  cpnode->SetTypeToFile();
  cpnode->SaveWithSceneOff();
  cpnode->HideFromEditorsOn();
  cpnode->SetScene(this->GetDMMLScene());

  // make a storage node
  vtkDMMLProceduralColorStorageNode *colorStorageNode = vtkDMMLProceduralColorStorageNode::New();
  colorStorageNode->SaveWithSceneOff();
  if (this->GetDMMLScene())
    {
    this->GetDMMLScene()->AddNode(colorStorageNode);
    cpnode->SetAndObserveStorageNodeID(colorStorageNode->GetID());
    }
  colorStorageNode->Delete();

  cpnode->GetStorageNode()->SetFileName(fileName);
  std::string basename = cpnode->GetStorageNode()->GetFileNameWithoutExtension(fileName);
  if (this->GetDMMLScene())
    {
    std::string uname(this->GetDMMLScene()->GetUniqueNameByString(basename.c_str()));
    cpnode->SetName(uname.c_str());
    }
  else
    {
    cpnode->SetName(basename.c_str());
    }

  vtkDebugMacro("CreateProceduralFileNode: About to read user file " << fileName);

  if (cpnode->GetStorageNode()->ReadData(cpnode) == 0)
    {
    vtkErrorMacro("Unable to read procedural color file " << (cpnode->GetFileName() ? cpnode->GetFileName() : ""));
    if (this->GetDMMLScene())
      {
      cpnode->SetAndObserveStorageNodeID(nullptr);
      cpnode->SetScene(nullptr);
      this->GetDMMLScene()->RemoveNode(colorStorageNode);
      }
      cpnode->Delete();
      return nullptr;
    }
  vtkDebugMacro("CreateProceduralFileNode: finished reading user procedural color file " << fileName);
  cpnode->SetSingletonTag(
    this->GetFileColorNodeSingletonTag(fileName).c_str());

  return cpnode;
}

//----------------------------------------------------------------------------------------
void vtkDMMLColorLogic::AddLabelsNode()
{
  vtkDMMLColorTableNode* labelsNode = this->CreateLabelsNode();
  //if (this->GetDMMLScene()->GetNodeByID(labelsNode->GetSingletonTag()) == nullptr)
    {
    //this->GetDMMLScene()->RequestNodeID(labelsNode, labelsNode->GetSingletonTag());
    this->GetDMMLScene()->AddNode(labelsNode);
    }
  labelsNode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkDMMLColorLogic::AddDefaultTableNode(int i)
{
  vtkDMMLColorTableNode* node = this->CreateDefaultTableNode(i);
  //if (node->GetSingletonTag())
    {
    //if (this->GetDMMLScene()->GetNodeByID(node->GetSingletonTag()) == nullptr)
      {
      vtkDebugMacro("vtkDMMLColorLogic::AddDefaultColorNodes: requesting id " << node->GetSingletonTag() << endl);
      //this->GetDMMLScene()->RequestNodeID(node, node->GetSingletonTag());
      this->GetDMMLScene()->AddNode(node);
      vtkDebugMacro("vtkDMMLColorLogic::AddDefaultColorNodes: added node " << node->GetID() << ", requested id was " << node->GetSingletonTag() << ", type = " << node->GetTypeAsString() << endl);
      }
    //else
    //  {
    //  vtkDebugMacro("vtkDMMLColorLogic::AddDefaultColorNodes: didn't add node " << node->GetID() << " as it was already in the scene.\n");
    //  }
    }
  node->Delete();
}

//----------------------------------------------------------------------------------------
void vtkDMMLColorLogic::AddDefaultProceduralNodes()
{
  // random one
  vtkDMMLProceduralColorNode* randomNode = this->CreateRandomNode();
  this->GetDMMLScene()->AddNode(randomNode);
  randomNode->Delete();

  // red green blue one
  vtkDMMLProceduralColorNode* rgbNode = this->CreateRedGreenBlueNode();
  this->GetDMMLScene()->AddNode(rgbNode);
  rgbNode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkDMMLColorLogic::AddPETNode(int type)
{
  vtkDebugMacro("AddDefaultColorNodes: adding PET nodes");
  vtkDMMLPETProceduralColorNode *nodepcn = this->CreatePETColorNode(type);
  //if (this->GetDMMLScene()->GetNodeByID( nodepcn->GetSingletonTag() ) == nullptr)
    {
    //this->GetDMMLScene()->RequestNodeID(nodepcn, nodepcn->GetSingletonTag() );
    this->GetDMMLScene()->AddNode(nodepcn);
    }
  nodepcn->Delete();
}

//----------------------------------------------------------------------------------------
void vtkDMMLColorLogic::AddDGEMRICNode(int type)
{
  vtkDebugMacro("AddDefaultColorNodes: adding dGEMRIC nodes");
  vtkDMMLdGEMRICProceduralColorNode *pcnode = this->CreatedGEMRICColorNode(type);
  //if (this->GetDMMLScene()->GetNodeByID(pcnode->GetSingletonTag()) == nullptr)
    {
    //this->GetDMMLScene()->RequestNodeID(pcnode, pcnode->GetSingletonTag());
    this->GetDMMLScene()->AddNode(pcnode);
    }
  pcnode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkDMMLColorLogic::AddDefaultFileNode(int i)
{
  vtkDMMLColorTableNode* ctnode =  this->CreateDefaultFileNode(this->ColorFiles[i]);
  if (ctnode)
    {
    //if (this->GetDMMLScene()->GetNodeByID(ctnode->GetSingletonTag()) == nullptr)
      {
        //this->GetDMMLScene()->RequestNodeID(ctnode, ctnode->GetSingletonTag());
        this->GetDMMLScene()->AddNode(ctnode);
        ctnode->Delete();
        vtkDebugMacro("AddDefaultColorFiles: Read and added file node: " <<  this->ColorFiles[i].c_str());
      }
    //else
    //  {
    //  vtkDebugMacro("AddDefaultColorFiles: node " << ctnode->GetSingletonTag() << " already in scene");
    //  }
    }
  else
    {
    vtkWarningMacro("Unable to read color file " << this->ColorFiles[i].c_str());
    }
}

//----------------------------------------------------------------------------------------
void vtkDMMLColorLogic::AddUserFileNode(int i)
{
  vtkDMMLColorTableNode* ctnode = this->CreateUserFileNode(this->UserColorFiles[i]);
  if (ctnode)
    {
    //if (this->GetDMMLScene()->GetNodeByID(ctnode->GetSingletonTag()) == nullptr)
      {
      //this->GetDMMLScene()->RequestNodeID(ctnode, ctnode->GetSingletonTag());
      this->GetDMMLScene()->AddNode(ctnode);
      vtkDebugMacro("AddDefaultColorFiles: Read and added user file node: " <<  this->UserColorFiles[i].c_str());
      }
    //else
    //  {
    //  vtkDebugMacro("AddDefaultColorFiles: node " << ctnode->GetSingletonTag() << " already in scene");
    //  }
    }
  else
    {
    vtkWarningMacro("Unable to read user color file " << this->UserColorFiles[i].c_str());
    }
  ctnode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkDMMLColorLogic::AddDefaultTableNodes()
{
  vtkDMMLColorTableNode* basicNode = vtkDMMLColorTableNode::New();
  for (int i = basicNode->GetFirstType(); i <= basicNode->GetLastType(); i++)
    {
    // don't add a second Labels node, File node or the old atlas node
    if (i != vtkDMMLColorTableNode::Labels &&
        i != vtkDMMLColorTableNode::File &&
        i != vtkDMMLColorTableNode::Obsolete &&
        i != vtkDMMLColorTableNode::User)
      {
      this->AddDefaultTableNode(i);
      }
    }
  basicNode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkDMMLColorLogic::AddPETNodes()
{
  vtkDMMLPETProceduralColorNode* basicPETNode = vtkDMMLPETProceduralColorNode::New();
  for (int type = basicPETNode->GetFirstType(); type <= basicPETNode->GetLastType(); ++type)
    {
    this->AddPETNode(type);
    }
  basicPETNode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkDMMLColorLogic::AddDGEMRICNodes()
{
  vtkDMMLdGEMRICProceduralColorNode* basicdGEMRICNode = vtkDMMLdGEMRICProceduralColorNode::New();
  for (int type = basicdGEMRICNode->GetFirstType(); type <= basicdGEMRICNode->GetLastType(); ++type)
    {
    this->AddDGEMRICNode(type);
    }
  basicdGEMRICNode->Delete();
}

//----------------------------------------------------------------------------------------
void vtkDMMLColorLogic::AddDefaultFileNodes()
{
  this->ColorFiles = this->FindDefaultColorFiles();
  vtkDebugMacro("AddDefaultColorNodes: found " <<  this->ColorFiles.size() << " default color files");
  for (unsigned int i = 0; i < this->ColorFiles.size(); i++)
    {
    this->AddDefaultFileNode(i);
    }
}

//----------------------------------------------------------------------------------------
void vtkDMMLColorLogic::AddUserFileNodes()
{
  this->UserColorFiles = this->FindUserColorFiles();
  vtkDebugMacro("AddDefaultColorNodes: found " <<  this->UserColorFiles.size() << " user color files");
  for (unsigned int i = 0; i < this->UserColorFiles.size(); i++)
    {
    this->AddUserFileNode(i);
    }

}

//----------------------------------------------------------------------------------------
vtkDMMLColorTableNode* vtkDMMLColorLogic::CopyNode(vtkDMMLColorNode* nodeToCopy, const char* copyName)
{
  vtkDMMLColorTableNode *colorNode = vtkDMMLColorTableNode::New();
  colorNode->SetName(copyName);
  colorNode->SetTypeToUser();
  colorNode->SetAttribute("Category", "User Generated");
  colorNode->SetHideFromEditors(false);
  colorNode->SetNamesInitialised(nodeToCopy->GetNamesInitialised());
  if (nodeToCopy->GetLookupTable())
    {
    double* range = nodeToCopy->GetLookupTable()->GetRange();
    colorNode->GetLookupTable()->SetRange(range[0], range[1]);
    }
  colorNode->SetNumberOfColors(nodeToCopy->GetNumberOfColors());
  for (int i = 0; i < nodeToCopy->GetNumberOfColors(); ++i)
    {
    double color[4];
    nodeToCopy->GetColor(i, color);
    colorNode->SetColor(i, nodeToCopy->GetColorName(i), color[0], color[1], color[2], color[3]);
    }
  return colorNode;
}

//----------------------------------------------------------------------------------------
vtkDMMLProceduralColorNode* vtkDMMLColorLogic::CopyProceduralNode(vtkDMMLColorNode* nodeToCopy, const char* copyName)
{
  vtkDMMLProceduralColorNode *colorNode = vtkDMMLProceduralColorNode::New();
  if (nodeToCopy->IsA("vtkDMMLProceduralColorNode"))
    {
    colorNode->Copy(nodeToCopy);
    // the copy will copy any singleton tag, make sure it's unset
    colorNode->SetSingletonTag(nullptr);
    }

  colorNode->SetName(copyName);
  colorNode->SetTypeToUser();
  colorNode->SetAttribute("Category", "User Generated");
  colorNode->SetHideFromEditors(false);

  return colorNode;
}
