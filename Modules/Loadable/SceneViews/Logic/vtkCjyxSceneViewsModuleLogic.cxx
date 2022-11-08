// CjyxLogic includes
#include "vtkCjyxSceneViewsModuleLogic.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLSceneViewNode.h>
#include <vtkDMMLSceneViewStorageNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <string>
#include <iostream>
#include <sstream>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxSceneViewsModuleLogic)

//-----------------------------------------------------------------------------
// vtkCjyxSceneViewsModuleLogic methods
//-----------------------------------------------------------------------------
vtkCjyxSceneViewsModuleLogic::vtkCjyxSceneViewsModuleLogic() = default;

//-----------------------------------------------------------------------------
vtkCjyxSceneViewsModuleLogic::~vtkCjyxSceneViewsModuleLogic() = default;

//-----------------------------------------------------------------------------
void vtkCjyxSceneViewsModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkCjyxSceneViewsModuleLogic::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  vtkDebugMacro("SetDMMLSceneInternal - listening to scene events");

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
//  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkDMMLScene::EndCloseEvent);
  events->InsertNextValue(vtkDMMLScene::EndImportEvent);
  events->InsertNextValue(vtkDMMLScene::EndRestoreEvent);
//  events->InsertNextValue(vtkDMMLScene::SceneAboutToBeRestoredEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkCjyxSceneViewsModuleLogic::OnDMMLSceneNodeAdded(vtkDMMLNode* vtkNotUsed(node))
{
  vtkDebugMacro("OnDMMLSceneNodeAddedEvent");
}

//-----------------------------------------------------------------------------
void vtkCjyxSceneViewsModuleLogic::OnDMMLSceneEndImport()
{
  vtkDebugMacro("OnDMMLSceneEndImport");
}

//-----------------------------------------------------------------------------
void vtkCjyxSceneViewsModuleLogic::OnDMMLSceneEndRestore()
{
  vtkDebugMacro("OnDMMLSceneEndRestore");
}

//-----------------------------------------------------------------------------
void vtkCjyxSceneViewsModuleLogic::OnDMMLNodeModified(vtkDMMLNode* vtkNotUsed(node))
{
}

//-----------------------------------------------------------------------------
void vtkCjyxSceneViewsModuleLogic::OnDMMLSceneEndClose()
{
}


//-----------------------------------------------------------------------------
void vtkCjyxSceneViewsModuleLogic::RegisterNodes()
{

  if (!this->GetDMMLScene())
    {
    std::cerr << "RegisterNodes: no scene on which to register nodes" << std::endl;
    return;
    }

  vtkDMMLSceneViewNode* viewNode = vtkDMMLSceneViewNode::New();
  this->GetDMMLScene()->RegisterNodeClass(viewNode);
  // SceneSnapshot backward compatibility
#if DMML_APPLICATION_SUPPORT_VERSION < DMML_VERSION_CHECK(4, 0, 0)
  this->GetDMMLScene()->RegisterNodeClass(viewNode, "SceneSnapshot");
#endif
  viewNode->Delete();

  vtkDMMLSceneViewStorageNode *storageNode = vtkDMMLSceneViewStorageNode::New();
  this->GetDMMLScene()->RegisterNodeClass ( storageNode );
  storageNode->Delete();
}

//---------------------------------------------------------------------------
void vtkCjyxSceneViewsModuleLogic::CreateSceneView(const char* name, const char* description, int screenshotType, vtkImageData* screenshot)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene set.");
    return;
    }

  if (!screenshot)
    {
    vtkErrorMacro("CreateSceneView: No screenshot was set.");
    return;
    }

  vtkStdString nameString = vtkStdString(name);

  vtkNew<vtkDMMLSceneViewNode> newSceneViewNode;
  newSceneViewNode->SetScene(this->GetDMMLScene());
  if (strcmp(nameString,""))
    {
    // a name was specified
    newSceneViewNode->SetName(nameString.c_str());
    }
  else
    {
    // if no name is specified, generate a new unique one
    newSceneViewNode->SetName(this->GetDMMLScene()->GetUniqueNameByString("SceneView"));
    }

  vtkStdString descriptionString = vtkStdString(description);

  newSceneViewNode->SetSceneViewDescription(descriptionString);
  newSceneViewNode->SetScreenShotType(screenshotType);

  // make a new vtk image data, as the set macro is taking the pointer
  vtkNew<vtkImageData> copyScreenShot;
  copyScreenShot->DeepCopy(screenshot);
  newSceneViewNode->SetScreenShot(copyScreenShot.GetPointer());
  newSceneViewNode->StoreScene();
  //newSceneViewNode->HideFromEditorsOff();
  // mark it modified since read so that the screen shot will get saved to disk

  this->GetDMMLScene()->AddNode(newSceneViewNode.GetPointer());
}

//---------------------------------------------------------------------------
void vtkCjyxSceneViewsModuleLogic::
         ModifySceneView(vtkStdString id,
                         const char* name,
                         const char* description,
                         int vtkNotUsed(screenshotType),
                         vtkImageData* screenshot)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene set.");
    return;
    }

  if (!screenshot)
    {
    vtkErrorMacro("ModifySceneView: No screenshot was set.");
    return;
    }

  vtkDMMLSceneViewNode* viewNode = vtkDMMLSceneViewNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(id.c_str()));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewName: Could not get sceneView node!");
    return;
    }

  vtkStdString nameString = vtkStdString(name);
  if (strcmp(nameString,""))
    {
    // a name was specified
    viewNode->SetName(nameString.c_str());
    }
  else
    {
    // if no name is specified, generate a new unique one
    viewNode->SetName(this->GetDMMLScene()->GetUniqueNameByString("SceneView"));
    }

  vtkStdString descriptionString = vtkStdString(description);
  viewNode->SetSceneViewDescription(descriptionString);
  // only the text is allowed to be modified, not the screen shot type nor the
  // screen shot image, so don't resave them
  // see also qDMMLScreenShotDialog::grabScreenShot()

  // TODO: Listen to the node directly, probably in OnDMMLSceneNodeAddedEvent
  this->OnDMMLNodeModified(viewNode);
}

//---------------------------------------------------------------------------
vtkStdString vtkCjyxSceneViewsModuleLogic::GetSceneViewName(const char* id)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene set.");
    return nullptr;
    }

  vtkDMMLSceneViewNode* viewNode = vtkDMMLSceneViewNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewName: Could not get sceneView node!");
    return nullptr;
    }

  return vtkStdString(viewNode->GetName());
}

//---------------------------------------------------------------------------
vtkStdString vtkCjyxSceneViewsModuleLogic::GetSceneViewDescription(const char* id)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene set.");
    return nullptr;
    }

  vtkDMMLSceneViewNode* viewNode = vtkDMMLSceneViewNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewDescription: Could not get sceneView node!");
    return nullptr;
    }

  return viewNode->GetSceneViewDescription();
}

//---------------------------------------------------------------------------
int vtkCjyxSceneViewsModuleLogic::GetSceneViewScreenshotType(const char* id)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene set.");
    return -1;
    }

  vtkDMMLSceneViewNode* viewNode = vtkDMMLSceneViewNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewScreenshotType: Could not get sceneView node!");
    return -1;
    }

  return viewNode->GetScreenShotType();
}

//---------------------------------------------------------------------------
vtkImageData* vtkCjyxSceneViewsModuleLogic::GetSceneViewScreenshot(const char* id)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene set.");
    return nullptr;
    }

  vtkDMMLSceneViewNode* viewNode = vtkDMMLSceneViewNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewScreenshot: Could not get sceneView node!");
    return nullptr;
    }

  return viewNode->GetScreenShot();
}

//---------------------------------------------------------------------------
bool vtkCjyxSceneViewsModuleLogic::RestoreSceneView(const char* id, bool removeNodes)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("No scene set.");
    return true;
    }

  vtkDMMLSceneViewNode* viewNode = vtkDMMLSceneViewNode::SafeDownCast(this->GetDMMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("RestoreSceneView: Could not get sceneView node!");
    return true;
    }

  return viewNode->RestoreScene(removeNodes);
}

//---------------------------------------------------------------------------
const char* vtkCjyxSceneViewsModuleLogic::MoveSceneViewUp(const char* vtkNotUsed(id))
{
  // reset stringHolder
  this->m_StringHolder = "";

  vtkErrorMacro("MoveSceneViewUp: operation not supported!");
  return this->m_StringHolder.c_str();
}

//---------------------------------------------------------------------------
const char* vtkCjyxSceneViewsModuleLogic::MoveSceneViewDown(const char* vtkNotUsed(id))
{
  // reset stringHolder
  this->m_StringHolder = "";

  vtkErrorMacro("MoveSceneViewDown: operation not supported!");
  return this->m_StringHolder.c_str();
}

//---------------------------------------------------------------------------
void vtkCjyxSceneViewsModuleLogic::RemoveSceneViewNode(vtkDMMLSceneViewNode *sceneViewNode)
{
  if (!sceneViewNode)
    {
    vtkErrorMacro("RemoveSceneViewNode: No node to remove");
    return;
    }

  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("RemoveSceneViewNode: No DMML Scene found from which to remove the node");
    return;
    }

  this->GetDMMLScene()->RemoveNode(sceneViewNode);
}
