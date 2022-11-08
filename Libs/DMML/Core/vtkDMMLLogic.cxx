
// DMML includes
#include "vtkDMMLScene.h"
#include "vtkDMMLStorageNode.h"
#include "vtkDMMLDisplayableNode.h"
#include "vtkDMMLDisplayNode.h"

// DMMLLogic includes
#include "vtkDMMLLogic.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <set>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLLogic);

//------------------------------------------------------------------------------
vtkDMMLLogic::vtkDMMLLogic()
{
  this->Scene = nullptr;
}

//------------------------------------------------------------------------------
vtkDMMLLogic::~vtkDMMLLogic() = default;

void vtkDMMLLogic::RemoveUnreferencedStorageNodes()
{
  if (this->Scene == nullptr)
    {
    return;
    }
  std::set<vtkDMMLNode *> referencedNodes;
  std::set<vtkDMMLNode *>::iterator iter;
  std::vector<vtkDMMLNode *> storableNodes;
  std::vector<vtkDMMLNode *> storageNodes;
  this->Scene->GetNodesByClass("vtkDMMLStorableNode", storableNodes);
  this->Scene->GetNodesByClass("vtkDMMLStorageNode", storageNodes);

  vtkDMMLNode *node = nullptr;
  vtkDMMLStorableNode *storableNode = nullptr;
  vtkDMMLStorageNode *storageNode = nullptr;
  unsigned int i;
  for (i=0; i<storableNodes.size(); i++)
    {
    node = storableNodes[i];
    if (node)
      {
      storableNode = vtkDMMLStorableNode::SafeDownCast(node);
      }
    else
      {
      continue;
      }
    storageNode = storableNode->GetStorageNode();
    if (storageNode)
      {
      referencedNodes.insert(storageNode);
      }
    }

  for (i=0; i<storageNodes.size(); i++)
    {
    node = storageNodes[i];
    if (node)
      {
      storageNode = vtkDMMLStorageNode::SafeDownCast(node);
      }
    else
      {
      continue;
      }
    iter = referencedNodes.find(storageNode);
    if (iter == referencedNodes.end())
      {
      this->Scene->RemoveNode(storageNode);
      }
    }
}

void vtkDMMLLogic::RemoveUnreferencedDisplayNodes()
{
  if (this->Scene == nullptr)
    {
    return;
    }
  std::set<vtkDMMLNode *> referencedNodes;
  std::set<vtkDMMLNode *>::iterator iter;
  std::vector<vtkDMMLNode *> displayableNodes;
  std::vector<vtkDMMLNode *> displayNodes;
  this->Scene->GetNodesByClass("vtkDMMLDisplayableNode", displayableNodes);
  this->Scene->GetNodesByClass("vtkDMMLDisplayNode", displayNodes);

  vtkDMMLNode *node = nullptr;
  vtkDMMLDisplayableNode *displayableNode = nullptr;
  vtkDMMLDisplayNode *displayNode = nullptr;
  unsigned int i;
  for (i=0; i<displayableNodes.size(); i++)
    {
    node = displayableNodes[i];
    if (node)
      {
      displayableNode = vtkDMMLDisplayableNode::SafeDownCast(node);
      }
    else
      {
      continue;
      }
    int numDisplayNodes = displayableNode->GetNumberOfDisplayNodes();
    for (int n=0; n<numDisplayNodes; n++)
      {
      displayNode = displayableNode->GetNthDisplayNode(n);
      if (displayNode)
        {
        referencedNodes.insert(displayNode);
        }
      }
    }

  for (i=0; i<displayNodes.size(); i++)
    {
    node = displayNodes[i];
    if (node)
      {
      displayNode = vtkDMMLDisplayNode::SafeDownCast(node);
      }
    else
      {
      continue;
      }
    iter = referencedNodes.find(displayNode);
    if (iter == referencedNodes.end())
      {
      this->Scene->RemoveNode(displayNode);
      }
    }
}

//----------------------------------------------------------------------------
std::string vtkDMMLLogic::GetApplicationHomeDirectory()
{
  std::string applicationHome;
  if (vtksys::SystemTools::GetEnv(DMML_APPLICATION_HOME_DIR_ENV) != nullptr)
    {
    applicationHome = std::string(vtksys::SystemTools::GetEnv(DMML_APPLICATION_HOME_DIR_ENV));
    }
  else
    {
    if (vtksys::SystemTools::GetEnv("PWD") != nullptr)
      {
      applicationHome =  std::string(vtksys::SystemTools::GetEnv("PWD"));
      }
    else
      {
      applicationHome =  std::string("");
      }
    }
  return applicationHome;
}

//----------------------------------------------------------------------------
std::string vtkDMMLLogic::GetApplicationShareDirectory()
{
  std::string applicationHome = vtkDMMLLogic::GetApplicationHomeDirectory();
  std::vector<std::string> filesVector;
  filesVector.emplace_back(""); // for relative path
  filesVector.push_back(applicationHome);
  filesVector.emplace_back(DMML_APPLICATION_SHARE_SUBDIR);
  std::string applicationShare = vtksys::SystemTools::JoinPath(filesVector);

  return applicationShare;
}
