/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// Cjyx logic includes
#include "vtkCjyxModelsLogic.h"
#include "vtkDMMLSliceLogic.h"

/// DMML includes
#include <vtkCacheManager.h>
#include <vtkDMMLClipModelsNode.h>
#include "vtkDMMLMessageCollection.h"
#include <vtkDMMLModelDisplayNode.h>
#include <vtkDMMLModelHierarchyNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLModelStorageNode.h>
#include <vtkDMMLSelectionNode.h>
#include <vtkDMMLScene.h>
#include <vtkDMMLSubjectHierarchyNode.h>
#include <vtkDMMLTransformNode.h>

/// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkGeneralTransform.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkTagTable.h>

/// ITK includes
#include <itksys/Directory.hxx>
#include <itksys/SystemTools.hxx>

/// STD includes
#include <cassert>

vtkStandardNewMacro(vtkCjyxModelsLogic);

//----------------------------------------------------------------------------
vtkCjyxModelsLogic::vtkCjyxModelsLogic()=default;

//----------------------------------------------------------------------------
vtkCjyxModelsLogic::~vtkCjyxModelsLogic()=default;

//----------------------------------------------------------------------------
void vtkCjyxModelsLogic::SetDMMLSceneInternal(vtkDMMLScene* newScene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndImportEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, sceneEvents.GetPointer());
}

//----------------------------------------------------------------------------
void vtkCjyxModelsLogic::ObserveDMMLScene()
{
  if (this->GetDMMLScene() &&
    this->GetDMMLScene()->GetFirstNodeByClass("vtkDMMLClipModelsNode") == nullptr)
    {
    // vtkDMMLClipModelsNode is a singleton
    this->GetDMMLScene()->AddNode(vtkSmartPointer<vtkDMMLClipModelsNode>::New());
    }
  this->Superclass::ObserveDMMLScene();
}

//-----------------------------------------------------------------------------
void vtkCjyxModelsLogic::OnDMMLSceneEndImport()
{
  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkErrorMacro("OnDMMLSceneEndImport: Unable to access DMML scene");
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = vtkDMMLSubjectHierarchyNode::ResolveSubjectHierarchy(scene);
  if (!shNode)
    {
    vtkErrorMacro("OnDMMLSceneEndImport: Unable to access subject hierarchy node");
    return;
    }

  // Convert model hierarchy nodes into subject hierarchy folders
  vtkDMMLNode* node = nullptr;
  vtkCollectionSimpleIterator mhIt;
  vtkCollection* mhNodes = scene->GetNodesByClass("vtkDMMLModelHierarchyNode");
  std::string newFolderName = vtkDMMLSubjectHierarchyConstants::GetSubjectHierarchyNewItemNamePrefix()
    + vtkDMMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder();
  std::map<std::string, vtkIdType> mhNodeIdToShItemIdMap;
  std::map<std::string, std::string> mhNodeIdToParentNodeIdMap;
  for (mhNodes->InitTraversal(mhIt); (node = (vtkDMMLNode*)mhNodes->GetNextItemAsObject(mhIt)) ;)
    {
    // Get direct child hierarchy nodes
    vtkDMMLModelHierarchyNode* mhNode = vtkDMMLModelHierarchyNode::SafeDownCast(node);
    std::vector<vtkDMMLHierarchyNode*> childHierarchyNodes = mhNode->GetChildrenNodes();

    vtkIdType folderItemID = vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID;
    if (childHierarchyNodes.size() > 0)
      {
      // Create new folder for the model hierarchy if there are children
      // (otherwise it's a leaf node with an associated model).
      // Have it directly under the scene for now. Rebuild hierarchy later
      // when we have all the items created.
      folderItemID = shNode->CreateFolderItem(shNode->GetSceneItemID(),
        (mhNode->GetName() ? mhNode->GetName() : shNode->GenerateUniqueItemName(newFolderName)) );
      }
    else if (!mhNode->GetAssociatedNodeID())
      {
      // If there are no children but there is no associated node, then something is wrong
      vtkWarningMacro("OnDMMLSceneEndImport: Invalid model hierarchy node found with neither "
        << "children nor associated node: " << mhNode->GetID());
      continue;
      }

    // Remember subject hierarchy item for current model hierarchy node
    // (even if has no actual folder, as this map will be used to remove the hierarchy nodes)
    mhNodeIdToShItemIdMap[mhNode->GetID()] = folderItemID;

    // Remember parent for current model hierarchy node if not leaf
    // (i.e. has a corresponding folder item and so need to be reparented in subject hierarchy)
    if (mhNode->GetParentNodeID() && !mhNode->GetAssociatedNodeID())
      {
      mhNodeIdToParentNodeIdMap[mhNode->GetID()] = mhNode->GetParentNodeID();
      }

    // Move all the direct children of the model hierarchy node under the folder if one was created
    if (folderItemID)
      {
      for (std::vector<vtkDMMLHierarchyNode*>::iterator it = childHierarchyNodes.begin();
        it != childHierarchyNodes.end(); ++it)
        {
        vtkDMMLNode* associatedNode = (*it)->GetAssociatedNode();
        if (associatedNode)
          {
          vtkIdType associatedItemID = shNode->GetItemByDataNode(associatedNode);
          if (associatedItemID)
            {
            shNode->SetItemParent(associatedItemID, folderItemID, true);
            }
          }
        }
      // Request plugin search for the folder that triggers creation of a model display node
      shNode->RequestOwnerPluginSearch(folderItemID);
      }
    } // for all model hierarchy nodes
  mhNodes->Delete();

  // Set up hierarchy between the created folder items
  for (std::map<std::string, std::string>::iterator it = mhNodeIdToParentNodeIdMap.begin();
    it != mhNodeIdToParentNodeIdMap.end(); ++it)
    {
    // Get SH item IDs for the nodes
    vtkIdType currentItemID = mhNodeIdToShItemIdMap[it->first];
    vtkIdType parentItemID = mhNodeIdToShItemIdMap[it->second];

    // Set parent in subject hierarchy
    shNode->SetItemParent(currentItemID, parentItemID, true);
    }

  // Remove model hierarchy nodes from the scene
  for (std::map<std::string, vtkIdType>::iterator it = mhNodeIdToShItemIdMap.begin();
    it != mhNodeIdToShItemIdMap.end(); ++it)
    {
    scene->RemoveNode(scene->GetNodeByID(it->first));
    }
}

//----------------------------------------------------------------------------
vtkDMMLModelNode* vtkCjyxModelsLogic::AddModel(vtkPolyData* polyData)
{
  if (this->GetDMMLScene() == nullptr)
    {
    return nullptr;
    }
  vtkDMMLModelNode* model = vtkDMMLModelNode::SafeDownCast(this->GetDMMLScene()->AddNewNodeByClass("vtkDMMLModelNode"));
  if (!model)
    {
    return nullptr;
    }
  model->SetAndObservePolyData(polyData);
  model->CreateDefaultDisplayNodes();
  return model;
}

//----------------------------------------------------------------------------
vtkDMMLModelNode* vtkCjyxModelsLogic::AddModel(vtkAlgorithmOutput* polyData)
{
  if (this->GetDMMLScene() == nullptr)
    {
    return nullptr;
    }
  vtkDMMLModelNode* model = vtkDMMLModelNode::SafeDownCast(this->GetDMMLScene()->AddNewNodeByClass("vtkDMMLModelNode"));
  if (!model)
    {
    return nullptr;
    }
  model->SetPolyDataConnection(polyData);
  model->CreateDefaultDisplayNodes();
  return model;
}

//----------------------------------------------------------------------------
int vtkCjyxModelsLogic::AddModels (const char* dirname, const char* suffix,
  int coordinateSystem /*=vtkDMMLStorageNode::CoordinateSystemLPS*/,
  vtkDMMLMessageCollection* userMessages/*=nullptr*/)
{
  std::string ssuf = suffix;
  itksys::Directory dir;
  dir.Load(dirname);

  int nfiles = dir.GetNumberOfFiles();
  int res = 1;
  for (int i=0; i<nfiles; i++) {
    const char* filename = dir.GetFile(i);
    std::string sname = filename;
    if (!itksys::SystemTools::FileIsDirectory(filename))
      {
      if ( sname.find(ssuf) != std::string::npos )
        {
        std::string fullPath = std::string(dir.GetPath())
            + "/" + filename;
        if (this->AddModel(fullPath.c_str(), coordinateSystem, userMessages) == nullptr)
          {
          res = 0;
          }
        }
      }
  }
  return res;
}

//----------------------------------------------------------------------------
vtkDMMLModelNode* vtkCjyxModelsLogic::AddModel(const char* filename,
  int coordinateSystem/*=vtkDMMLStorageNode::CoordinateSystemLPS*/,
  vtkDMMLMessageCollection* userMessages/*=nullptr*/)
{
  if (this->GetDMMLScene() == nullptr || filename == nullptr)
    {
    vtkErrorToMessageCollectionMacro(userMessages, "vtkCjyxModelsLogic::AddModel",
      "Invalid scene or filename");
    return nullptr;
    }

  // Determine local filename
  vtkNew<vtkDMMLModelStorageNode> storageNode;
  int useURI = 0; // false;
  if (this->GetDMMLScene()->GetCacheManager() != nullptr)
    {
    useURI = this->GetDMMLScene()->GetCacheManager()->IsRemoteReference(filename);
    vtkDebugMacro("AddModel: file name is remote: " << filename);
    }
  std::string localFile;
  if (useURI)
    {
    storageNode->SetURI(filename);
    // reset filename to the local file name
    const char* localFilePtr = this->GetDMMLScene()->GetCacheManager()->GetFilenameFromURI(filename);
    if (localFilePtr)
      {
      localFile = localFilePtr;
      }
    }
  else
    {
    storageNode->SetFileName(filename);
    localFile = filename;
    }

  // Check if we can read this type of file.
  // The model name is based on the file name (itksys call should work even if
  // file is not on disk yet).
  std::string name = itksys::SystemTools::GetFilenameName(localFile);
  vtkDebugMacro("AddModel: got model name = " << name.c_str());
  if (!storageNode->SupportedFileType(name.c_str()))
    {
    vtkErrorToMessageCollectionMacro(userMessages, "vtkCjyxModelsLogic::AddModel",
      "Could not find a suitable storage node for file '" << filename << "'.");
    return nullptr;
    }

  // Create model node
  std::string baseName = storageNode->GetFileNameWithoutExtension(localFile.c_str());
  std::string uniqueName(this->GetDMMLScene()->GetUniqueNameByString(baseName.c_str()));
  vtkDMMLModelNode* modelNode = vtkDMMLModelNode::SafeDownCast(this->GetDMMLScene()->AddNewNodeByClass("vtkDMMLModelNode", uniqueName.c_str()));
  if (!modelNode)
    {
    return nullptr;
    }

  // Read the model file
  vtkDebugMacro("AddModel: calling read on the storage node");
  storageNode->SetCoordinateSystem(coordinateSystem);
  int success = storageNode->ReadData(modelNode);
  if (!success)
    {
    vtkErrorMacro("AddModel: error reading " << filename);
    if (userMessages)
      {
      userMessages->AddMessages(storageNode->GetUserMessages());
      }
    this->GetDMMLScene()->RemoveNode(modelNode);
    return nullptr;
    }

  // Associate with storage node
  this->GetDMMLScene()->AddNode(storageNode);
  modelNode->SetAndObserveStorageNodeID(storageNode->GetID());

  // Add display node
  modelNode->CreateDefaultDisplayNodes();

  return modelNode;
}

//----------------------------------------------------------------------------
int vtkCjyxModelsLogic::SaveModel (const char* filename, vtkDMMLModelNode *modelNode,
  int coordinateSystem/*=-1*/, vtkDMMLMessageCollection* userMessages/*=nullptr*/)
{
   if (modelNode == nullptr || filename == nullptr)
     {
     vtkErrorToMessageCollectionMacro(userMessages, "vtkCjyxModelsLogic::SaveModel",
       "Failed to save model node " << ((modelNode && modelNode->GetID()) ? modelNode->GetID() : "(null)")
       << " into file '" << (filename ? filename : "(null)") << "'.");
     return 0;
     }

  vtkDMMLModelStorageNode *storageNode = nullptr;
  vtkDMMLStorageNode *snode = modelNode->GetStorageNode();
  if (snode != nullptr)
    {
    storageNode = vtkDMMLModelStorageNode::SafeDownCast(snode);
    }
  if (storageNode == nullptr)
    {
    storageNode = vtkDMMLModelStorageNode::SafeDownCast(this->GetDMMLScene()->AddNewNodeByClass("vtkDMMLModelStorageNode"));
    modelNode->SetAndObserveStorageNodeID(storageNode->GetID());
    }

  if (coordinateSystem >= 0)
    {
    storageNode->SetCoordinateSystem(coordinateSystem);
    }

  // check for a remote file
  if ((this->GetDMMLScene()->GetCacheManager() != nullptr) &&
      this->GetDMMLScene()->GetCacheManager()->IsRemoteReference(filename))
    {
    storageNode->SetURI(filename);
    }
  else
    {
    storageNode->SetFileName(filename);
    }

  int success = storageNode->WriteData(modelNode);
  if (!success)
    {
    vtkErrorMacro("vtkCjyxModelsLogic::SaveModel: error saving " << filename);
    if (userMessages)
      {
      userMessages->AddMessages(storageNode->GetUserMessages());
      }
    }
  return success;
}

//----------------------------------------------------------------------------
void vtkCjyxModelsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkCjyxModelsLogic:             " << this->GetClassName() << "\n";
}

//----------------------------------------------------------------------------
void vtkCjyxModelsLogic::TransformModel(vtkDMMLTransformNode *tnode,
                                          vtkDMMLModelNode *modelNode,
                                          int transformNormals,
                                          vtkDMMLModelNode *modelOut)
{
  if (!modelNode || !modelOut || !tnode)
    {
    return;
    }

  vtkNew<vtkPolyData> poly;
  modelOut->SetAndObservePolyData(poly.GetPointer());

  poly->DeepCopy(modelNode->GetPolyData());

  vtkDMMLTransformNode *mtnode = modelNode->GetParentTransformNode();

  vtkAbstractTransform *transform = tnode->GetTransformToParent();
  modelOut->ApplyTransform(transform);

  if (transformNormals)
    {
    // fix normals
    //--- NOTE: This filter recomputes normals for polygons and
    //--- triangle strips only. Normals are not computed for lines or vertices.
    //--- Triangle strips are broken up into triangle polygons.
    //--- Polygons are not automatically re-stripped.
    vtkNew<vtkPolyDataNormals> normals;
    normals->SetInputData(poly.GetPointer());
    //--- NOTE: This assumes a completely closed surface
    //---(i.e. no boundary edges) and no non-manifold edges.
    //--- If these constraints do not hold, the AutoOrientNormals
    //--- is not guaranteed to work.
    normals->AutoOrientNormalsOn();
    //--- Flipping modifies both the normal direction
    //--- and the order of a cell's points.
    normals->FlipNormalsOn();
    normals->SplittingOff();
    //--- enforce consistent polygon ordering.
    normals->ConsistencyOn();

    normals->Update();
    modelOut->SetPolyDataConnection(normals->GetOutputPort());
   }

  modelOut->SetAndObserveTransformNodeID(mtnode == nullptr ? nullptr : mtnode->GetID());

  return;
}

//----------------------------------------------------------------------------
void vtkCjyxModelsLogic::SetAllModelsVisibility(int flag)
{
  if (this->GetDMMLScene() == nullptr)
    {
    return;
    }

  int numModels = this->GetDMMLScene()->GetNumberOfNodesByClass("vtkDMMLModelNode");

  // go into batch processing mode
  this->GetDMMLScene()->StartState(vtkDMMLScene::BatchProcessState);
  for (int i = 0; i < numModels; i++)
    {
    vtkDMMLNode *dmmlNode = this->GetDMMLScene()->GetNthNodeByClass(i, "vtkDMMLModelNode");
    // Exclude volume slice model nodes.
    // Exclude vtkDMMLModelNode subclasses by comparing classname.
    // Doing so will avoid updating annotation and fiber bundle node
    // visibility since they derive from vtkDMMLModelNode
    // See https://github.com/Slicer/Slicer/issues/2576
    if (dmmlNode != nullptr
        && !vtkDMMLSliceLogic::IsSliceModelNode(dmmlNode)
        && strcmp(dmmlNode->GetClassName(), "vtkDMMLModelNode") == 0)
      {
      vtkDMMLModelNode *modelNode = vtkDMMLModelNode::SafeDownCast(dmmlNode);
      if (modelNode)
        {
        // have a "real" model node, set the display visibility
        modelNode->SetDisplayVisibility(flag);
        }
      }

    if (flag != 2 && dmmlNode != nullptr
        && !vtkDMMLSliceLogic::IsSliceModelNode(dmmlNode) )
      {
      vtkDMMLModelNode *modelNode = vtkDMMLModelNode::SafeDownCast(dmmlNode);
      int ndnodes = modelNode->GetNumberOfDisplayNodes();
      for (int i=0; i<ndnodes; i++)
        {
        vtkDMMLDisplayNode *displayNode = modelNode->GetNthDisplayNode(i);
        if (displayNode && displayNode->IsShowModeDefault())
          {
          displayNode->SetVisibility(flag);
          }
        }
      }
    }
  this->GetDMMLScene()->EndState(vtkDMMLScene::BatchProcessState);
}
