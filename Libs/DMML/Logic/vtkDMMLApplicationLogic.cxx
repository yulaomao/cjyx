/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// vtkCjyx includes
#include "vtkArchive.h" // note: this is not a class

// DMMLLogic includes
#include "vtkDMMLApplicationLogic.h"
#include "vtkDMMLColorLogic.h"
#include "vtkDMMLSliceLogic.h"
#include "vtkDMMLSliceLinkLogic.h"
#include "vtkDMMLViewLogic.h"
#include "vtkDMMLViewLinkLogic.h"

// DMML includes
#include "vtkDMMLInteractionNode.h"
#include "vtkDMMLPlotViewNode.h"
#include "vtkDMMLScene.h"
#include "vtkDMMLSelectionNode.h"
#include "vtkDMMLSliceCompositeNode.h"
#include "vtkDMMLSliceDisplayNode.h"
#include "vtkDMMLSliceNode.h"
#include "vtkDMMLStorableNode.h"
#include "vtkDMMLStorageNode.h"
#include "vtkDMMLSceneViewNode.h"
#include "vtkDMMLTableViewNode.h"
#include "vtkDMMLViewNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>
#include <vtksys/Glob.hxx>

// STD includes
#include <cassert>
#include <map>
#include <sstream>

// For LoadDefaultParameterSets
#ifdef WIN32
# include <windows.h>
#else
# include <dirent.h>
# include <errno.h>
#endif

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDMMLApplicationLogic);

//----------------------------------------------------------------------------
class vtkDMMLApplicationLogic::vtkInternal
{
public:
  vtkInternal(vtkDMMLApplicationLogic* external);
  void PropagateVolumeSelection(int layer, int fit);
  ~vtkInternal();

  vtkDMMLApplicationLogic* External;
  vtkSmartPointer<vtkDMMLSelectionNode> SelectionNode;
  vtkSmartPointer<vtkDMMLInteractionNode> InteractionNode;
  vtkSmartPointer<vtkCollection> SliceLogics;
  vtkSmartPointer<vtkCollection> ViewLogics;
  vtkSmartPointer<vtkDMMLSliceLinkLogic> SliceLinkLogic;
  vtkSmartPointer<vtkDMMLViewLinkLogic> ViewLinkLogic;
  vtkSmartPointer<vtkDMMLColorLogic> ColorLogic;
  std::string TemporaryPath;
  std::map<std::string, vtkWeakPointer<vtkDMMLAbstractLogic> > ModuleLogicMap;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkDMMLApplicationLogic::vtkInternal::vtkInternal(vtkDMMLApplicationLogic* external)
{
  this->External = external;
  this->SliceLinkLogic = vtkSmartPointer<vtkDMMLSliceLinkLogic>::New();
  this->ViewLinkLogic = vtkSmartPointer<vtkDMMLViewLinkLogic>::New();
  this->ColorLogic = vtkSmartPointer<vtkDMMLColorLogic>::New();
}

//----------------------------------------------------------------------------
vtkDMMLApplicationLogic::vtkInternal::~vtkInternal() = default;

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::vtkInternal::PropagateVolumeSelection(int layer, int fit)
{
  if ( !this->SelectionNode || !this->External->GetDMMLScene() )
    {
    return;
    }

  const char* ID = this->SelectionNode->GetActiveVolumeID();
  const char* secondID = this->SelectionNode->GetSecondaryVolumeID();
  const char* labelID = this->SelectionNode->GetActiveLabelVolumeID();

  vtkDMMLSliceCompositeNode* cnode;
  const int nnodes = this->External->GetDMMLScene()->GetNumberOfNodesByClass("vtkDMMLSliceCompositeNode");

  for (int i = 0; i < nnodes; i++)
    {
    cnode = vtkDMMLSliceCompositeNode::SafeDownCast (
      this->External->GetDMMLScene()->GetNthNodeByClass( i, "vtkDMMLSliceCompositeNode" ) );
    if(!cnode->GetDoPropagateVolumeSelection())
      {
      continue;
      }
    if (layer & vtkDMMLApplicationLogic::BackgroundLayer)
      {
      cnode->SetBackgroundVolumeID( ID );
      }
    if (layer & vtkDMMLApplicationLogic::ForegroundLayer)
      {
      cnode->SetForegroundVolumeID( secondID );
      }
    if (layer & vtkDMMLApplicationLogic::LabelLayer)
      {
      cnode->SetLabelVolumeID( labelID );
      }
    }
  if (fit)
    {
    this->External->FitSliceToAll(true);
    }
}
//----------------------------------------------------------------------------
// vtkDMMLApplicationLogic methods

//----------------------------------------------------------------------------
vtkDMMLApplicationLogic::vtkDMMLApplicationLogic()
{
  this->Internal = new vtkInternal(this);
  this->Internal->SliceLinkLogic->SetDMMLApplicationLogic(this);
  this->Internal->ViewLinkLogic->SetDMMLApplicationLogic(this);
  this->Internal->ColorLogic->SetDMMLApplicationLogic(this);
}

//----------------------------------------------------------------------------
vtkDMMLApplicationLogic::~vtkDMMLApplicationLogic()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkDMMLSelectionNode* vtkDMMLApplicationLogic::GetSelectionNode()const
{
  return this->Internal->SelectionNode;
}

//----------------------------------------------------------------------------
vtkDMMLInteractionNode* vtkDMMLApplicationLogic::GetInteractionNode()const
{
  return this->Internal->InteractionNode;
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::SetColorLogic(vtkDMMLColorLogic* colorLogic)
{
  this->Internal->ColorLogic = colorLogic;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkDMMLColorLogic* vtkDMMLApplicationLogic::GetColorLogic()const
{
  return this->Internal->ColorLogic;
}

//----------------------------------------------------------------------------
vtkCollection* vtkDMMLApplicationLogic::GetSliceLogics()const
{
  return this->Internal->SliceLogics;
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::SetSliceLogics(vtkCollection* sliceLogics)
{
  if (sliceLogics == this->Internal->SliceLogics)
    {
    return;
    }
  this->Internal->SliceLogics = sliceLogics;
  this->Modified();
}

//---------------------------------------------------------------------------
vtkDMMLSliceLogic* vtkDMMLApplicationLogic::
GetSliceLogic(vtkDMMLSliceNode* sliceNode) const
{
  if(!sliceNode || !this->Internal->SliceLogics)
    {
    return nullptr;
    }

  vtkDMMLSliceLogic* logic = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* logics = this->Internal->SliceLogics;

  for (logics->InitTraversal(it);
      (logic=vtkDMMLSliceLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
    {
    if (logic->GetSliceNode() == sliceNode)
      {
      break;
      }

    logic = nullptr;
    }

  return logic;
}

//---------------------------------------------------------------------------
vtkDMMLSliceLogic* vtkDMMLApplicationLogic::
GetSliceLogicByLayoutName(const char* layoutName) const
{
  if(!layoutName || !this->Internal->SliceLogics)
    {
    return nullptr;
    }

  vtkDMMLSliceLogic* logic = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* logics = this->Internal->SliceLogics;

  for (logics->InitTraversal(it);
      (logic=vtkDMMLSliceLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
    {
    if (logic->GetSliceNode())
      {
      if ( !strcmp( logic->GetSliceNode()->GetLayoutName(), layoutName) )
        {
        return logic;
        }
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
vtkDMMLSliceLogic* vtkDMMLApplicationLogic::
GetSliceLogicByModelDisplayNode(vtkDMMLModelDisplayNode* displayNode) const
{
  if (!displayNode || !this->Internal->SliceLogics)
    {
    return nullptr;
    }

  vtkDMMLSliceLogic* logic = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* logics = this->Internal->SliceLogics;

  for (logics->InitTraversal(it);
    (logic = vtkDMMLSliceLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
    {
    if (logic->GetSliceModelDisplayNode() == displayNode)
      {
      return logic;
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLApplicationLogic::SetViewLogics(vtkCollection* viewLogics)
{
  if (viewLogics == this->Internal->ViewLogics)
    {
    return;
    }
  this->Internal->ViewLogics = viewLogics;
  this->Modified();
}

//---------------------------------------------------------------------------
vtkCollection* vtkDMMLApplicationLogic::GetViewLogics() const
{
  return this->Internal->ViewLogics;
}

//---------------------------------------------------------------------------
vtkDMMLViewLogic* vtkDMMLApplicationLogic::
GetViewLogic(vtkDMMLViewNode* viewNode) const
{
  if(!viewNode || !this->Internal->ViewLogics)
    {
    return nullptr;
    }

  vtkDMMLViewLogic* logic = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* logics = this->Internal->ViewLogics;

  for (logics->InitTraversal(it);
      (logic=vtkDMMLViewLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
    {
    if (logic->GetViewNode() == viewNode)
      {
      break;
      }

    logic = nullptr;
    }

  return logic;
}

//---------------------------------------------------------------------------
vtkDMMLViewLogic* vtkDMMLApplicationLogic::
GetViewLogicByLayoutName(const char* layoutName) const
{
  if(!layoutName || !this->Internal->ViewLogics)
    {
    return nullptr;
    }

  vtkDMMLViewLogic* logic = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* logics = this->Internal->ViewLogics;

  for (logics->InitTraversal(it);
      (logic=vtkDMMLViewLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
    {
    if (logic->GetViewNode())
      {
      if ( !strcmp( logic->GetViewNode()->GetLayoutName(), layoutName) )
        {
        return logic;
        }
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
void vtkDMMLApplicationLogic::SetDMMLSceneInternal(vtkDMMLScene* newScene)
{
  vtkDMMLNode* selectionNode = nullptr;
  if (newScene)
    {
    // Selection Node
    selectionNode = newScene->GetNodeByID("vtkDMMLSelectionNodeSingleton");
    if (!selectionNode)
      {
      selectionNode = newScene->AddNode(vtkNew<vtkDMMLSelectionNode>().GetPointer());
      }
    assert(vtkDMMLSelectionNode::SafeDownCast(selectionNode));
    }
  this->SetSelectionNode(vtkDMMLSelectionNode::SafeDownCast(selectionNode));

  vtkDMMLNode* interactionNode = nullptr;
  if (newScene)
    {
    // Interaction Node
    interactionNode = newScene->GetNodeByID("vtkDMMLInteractionNodeSingleton");
    if (!interactionNode)
      {
      interactionNode = newScene->AddNode(vtkNew<vtkDMMLInteractionNode>().GetPointer());
      }
    assert(vtkDMMLInteractionNode::SafeDownCast(interactionNode));
    }
  this->SetInteractionNode(vtkDMMLInteractionNode::SafeDownCast(interactionNode));

  // Add default slice orientation presets
  vtkDMMLSliceNode::AddDefaultSliceOrientationPresets(newScene);

  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkDMMLScene::StartBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::StartCloseEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndCloseEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::StartImportEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndImportEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::StartRestoreEvent);
  sceneEvents->InsertNextValue(vtkDMMLScene::EndRestoreEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, sceneEvents.GetPointer());

  this->Internal->SliceLinkLogic->SetDMMLScene(newScene);
  this->Internal->ViewLinkLogic->SetDMMLScene(newScene);
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::SetSelectionNode(vtkDMMLSelectionNode* selectionNode)
{
  if (selectionNode == this->Internal->SelectionNode)
    {
    return;
    }
  this->Internal->SelectionNode = selectionNode;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::SetInteractionNode(vtkDMMLInteractionNode* interactionNode)
{
  if (interactionNode == this->Internal->InteractionNode)
    {
    return;
    }

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLInteractionNode::EditNodeEvent);
  events->InsertNextValue(vtkDMMLInteractionNode::ShowViewContextMenuEvent);
  vtkSetAndObserveDMMLNodeEventsMacro(this->Internal->InteractionNode, interactionNode, events);
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::PropagateVolumeSelection(int fit)
{
  this->PropagateVolumeSelection(
        BackgroundLayer | ForegroundLayer | LabelLayer, fit);
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::PropagateBackgroundVolumeSelection(int fit)
{
  this->PropagateVolumeSelection(BackgroundLayer, fit);
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::PropagateForegroundVolumeSelection(int fit)
{
  this->PropagateVolumeSelection(ForegroundLayer, fit);
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::PropagateLabelVolumeSelection(int fit)
{
  this->PropagateVolumeSelection(LabelLayer, fit);
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::PropagateVolumeSelection(int layer, int fit)
{
  this->Internal->PropagateVolumeSelection(layer, fit);
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::PropagateTableSelection()
{
  if ( !this->Internal->SelectionNode || !this->GetDMMLScene() )
    {
    return;
    }

  const char* tableId = this->Internal->SelectionNode->GetActiveTableID();

  const int nnodes = this->GetDMMLScene()->GetNumberOfNodesByClass("vtkDMMLTableViewNode");
  for (int i = 0; i < nnodes; i++)
    {
    vtkDMMLTableViewNode* tnode = vtkDMMLTableViewNode::SafeDownCast (
      this->GetDMMLScene()->GetNthNodeByClass( i, "vtkDMMLTableViewNode" ) );
    if(!tnode->GetDoPropagateTableSelection())
      {
      continue;
      }
    tnode->SetTableNodeID( tableId );
  }
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::PropagatePlotChartSelection()
{
  if ( !this->Internal->SelectionNode || !this->GetDMMLScene() )
    {
    return;
    }

  const char* PlotChartId = this->Internal->SelectionNode->GetActivePlotChartID();

  const int nnodes = this->GetDMMLScene()->GetNumberOfNodesByClass("vtkDMMLPlotViewNode");
  for (int i = 0; i < nnodes; i++)
    {
    vtkDMMLPlotViewNode* pnode = vtkDMMLPlotViewNode::SafeDownCast (
      this->GetDMMLScene()->GetNthNodeByClass( i, "vtkDMMLPlotViewNode" ) );
    if(!pnode->GetDoPropagatePlotChartSelection())
      {
      continue;
      }
    pnode->SetPlotChartNodeID(PlotChartId);
  }
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::FitSliceToAll(bool onlyIfPropagateVolumeSelectionAllowed /* =false */, bool resetOrientation /* =true */)
{
  if (this->Internal->SliceLogics.GetPointer() == nullptr)
    {
    return;
    }
  vtkDMMLSliceLogic* sliceLogic = nullptr;
  vtkCollectionSimpleIterator it;
  for(this->Internal->SliceLogics->InitTraversal(it);
      (sliceLogic = vtkDMMLSliceLogic::SafeDownCast(
        this->Internal->SliceLogics->GetNextItemAsObject(it)));)
    {
    if (onlyIfPropagateVolumeSelectionAllowed)
      {
      vtkDMMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
      if (sliceCompositeNode!=nullptr && !sliceCompositeNode->GetDoPropagateVolumeSelection())
        {
        // propagate volume selection is disabled, skip this slice
        continue;
        }
      }
    vtkDMMLSliceNode* sliceNode = sliceLogic->GetSliceNode();
    if (resetOrientation)
      {
      // Set to default orientation before rotation so that the view is snapped
      // closest to the default orientation of this slice view.
      sliceNode->SetOrientationToDefault();
      sliceLogic->RotateSliceToLowestVolumeAxes(false);
      }
    int* dims = sliceNode->GetDimensions();
    sliceLogic->FitSliceToAll(dims[0], dims[1]);
    sliceLogic->SnapSliceOffsetToIJK();
    }
}

//----------------------------------------------------------------------------
bool vtkDMMLApplicationLogic::Zip(const char* zipFileName, const char* directoryToZip)
{
  return vtkArchive::Zip(zipFileName, directoryToZip);
}

//----------------------------------------------------------------------------
bool vtkDMMLApplicationLogic::Unzip(const char* zipFileName, const char* destinationDirectory)
{
  return vtkArchive::UnZip(zipFileName, destinationDirectory);
}

//----------------------------------------------------------------------------
std::string vtkDMMLApplicationLogic::UnpackCjyxDataBundle(const char *sdbFilePath, const char *temporaryDirectory)
{
  return vtkDMMLScene::UnpackCjyxDataBundle(sdbFilePath, temporaryDirectory);
}

//----------------------------------------------------------------------------
bool vtkDMMLApplicationLogic::OpenCjyxDataBundle(const char* sdbFilePath, const char* temporaryDirectory)
{
  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("no scene");
    return false;
    }

  std::string dmmlFile = this->UnpackCjyxDataBundle(sdbFilePath, temporaryDirectory);

  if ( dmmlFile.empty() )
    {
    vtkErrorMacro("Could not unpack dmml scene");
    return false;
    }

  this->GetDMMLScene()->SetURL( dmmlFile.c_str() );
  int success = this->GetDMMLScene()->Connect();
  if ( !success )
    {
    vtkErrorMacro("Could not connect to scene");
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
std::string vtkDMMLApplicationLogic::PercentEncode(std::string s)
{
  return vtkDMMLScene::PercentEncode(s);
}

//----------------------------------------------------------------------------
bool vtkDMMLApplicationLogic::SaveSceneToCjyxDataBundleDirectory(const char* sdbDir, vtkImageData* screenShot)
{
  // Overview:
  // - confirm the arguments are valid and create directories if needed
  // - save all current file storage paths in the scene
  // - replace all file storage folders by sdbDir/Data
  // - create a screenshot of the scene (for allowing preview of scene content without opening in Cjyx)
  // - save the scene (dmml files and all storable nodes)
  // - revert all file storage paths to the original
  //
  // At the end, the scene should be restored to its original state
  // except that some storables will have default storage nodes.

  if (!this->GetDMMLScene())
    {
    vtkErrorMacro("SaveSceneToCjyxDataBundleDirectory failed: invalid scene");
    return false;
    }
  return this->GetDMMLScene()->SaveSceneToCjyxDataBundleDirectory(sdbDir, screenShot);
}

//----------------------------------------------------------------------------
std::string vtkDMMLApplicationLogic::CreateUniqueFileName(const std::string &filename, const std::string& knownExtension)
{
  return vtkDMMLScene::CreateUniqueFileName(filename, knownExtension);
}

//----------------------------------------------------------------------------
int vtkDMMLApplicationLogic::LoadDefaultParameterSets(vtkDMMLScene* scene,
                                                      const std::vector<std::string>& directories)
{

  // build up the vector
  std::vector<std::string> filesVector;
  std::vector<std::string> filesToLoad;
  //filesVector.push_back(""); // for relative path

// Didn't port this next block of code yet.  Would need to add a
//   UserParameterSetsPath to the object and some window
//
//   // add the list of dirs set from the application
//   if (this->UserColorFilePaths != nullptr)
//     {
//     vtkDebugMacro("\nFindColorFiles: got user color file paths = " << this->UserColorFilePaths);
//     // parse out the list, breaking at delimiter strings
// #ifdef WIN32
//     const char *delim = ";";
// #else
//     const char *delim = ":";
// #endif
//     char *ptr = strtok(this->UserColorFilePaths, delim);
//     while (ptr != nullptr)
//       {
//       std::string dir = std::string(ptr);
//       vtkDebugMacro("\nFindColorFiles: Adding user dir " << dir.c_str() << " to the directories to check");
//       DirectoriesToCheck.push_back(dir);
//       ptr = strtok(nullptr, delim);
//       }
//     } else { vtkDebugMacro("\nFindColorFiles: oops, the user color file paths aren't set!"); }


  // Get the list of parameter sets in these dir
  for (unsigned int d = 0; d < directories.size(); d++)
    {
    std::string dirString = directories[d];
    //vtkDebugMacro("\nLoadDefaultParameterSets: checking for parameter sets in dir " << d << " = " << dirString.c_str());

    filesVector.clear();
    filesVector.push_back(dirString);
    filesVector.emplace_back("/");

#ifdef WIN32
    WIN32_FIND_DATA findData;
    HANDLE fileHandle;
    int flag = 1;
    std::string search ("*.*");
    dirString += "/";
    search = dirString + search;

    fileHandle = FindFirstFile(search.c_str(), &findData);
    if (fileHandle != INVALID_HANDLE_VALUE)
      {
      while (flag)
        {
        // add this file to the vector holding the base dir name so check the
        // file type using the full path
        filesVector.push_back(std::string(findData.cFileName));
#else
    DIR* dp;
    struct dirent* dirp;
    if ((dp  = opendir(dirString.c_str())) == nullptr)
      {
      vtkGenericWarningMacro("Error(" << errno << ") opening " << dirString.c_str());
      }
    else
      {
      while ((dirp = readdir(dp)) != nullptr)
        {
        // add this file to the vector holding the base dir name
        filesVector.emplace_back(dirp->d_name);
#endif

        std::string fileToCheck = vtksys::SystemTools::JoinPath(filesVector);
        int fileType = vtksys::SystemTools::DetectFileType(fileToCheck.c_str());
        if (fileType == vtksys::SystemTools::FileTypeText)
          {
          //vtkDebugMacro("\nAdding " << fileToCheck.c_str() << " to list of potential parameter sets. Type = " << fileType);
          filesToLoad.push_back(fileToCheck);
          }
        else
          {
          //vtkDebugMacro("\nSkipping potential parameter set " << fileToCheck.c_str() << ", file type = " << fileType);
          }
        // take this file off so that can build the next file name
        filesVector.pop_back();

#ifdef WIN32
        flag = FindNextFile(fileHandle, &findData);
        } // end of while flag
      FindClose(fileHandle);
      } // end of having a valid fileHandle
#else
        } // end of while loop over reading the directory entries
      closedir(dp);
      } // end of able to open dir
#endif

    } // end of looping over dirs

  // Save the URL and root directory of the scene so it can
  // be restored after loading presets
  std::string url = scene->GetURL();
  std::string rootdir = scene->GetRootDirectory();

  // Finally, load each of the parameter sets
  std::vector<std::string>::iterator fit;
  for (fit = filesToLoad.begin(); fit != filesToLoad.end(); ++fit)
    {
    scene->SetURL( (*fit).c_str() );
    scene->Import();
    }

  // restore URL and root dir
  scene->SetURL(url.c_str());
  scene->SetRootDirectory(rootdir.c_str());

  return static_cast<int>(filesToLoad.size());
}

//----------------------------------------------------------------------------
vtkDMMLApplicationLogic
::InvokeRequest::InvokeRequest() = default;

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic
::InvokeEventWithDelay(unsigned int delayInMs, vtkObject* caller, unsigned long eventID,
              void* callData)
{
  InvokeRequest request;
  request.Delay = delayInMs;
  request.Caller = caller;
  request.EventID = eventID;
  request.CallData = callData;
  this->InvokeEvent(vtkDMMLApplicationLogic::RequestInvokeEvent, &request);
}

//----------------------------------------------------------------------------
const char* vtkDMMLApplicationLogic::GetTemporaryPath()
{
  return this->Internal->TemporaryPath.c_str();
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::SetTemporaryPath(const char* path)
{
    if (path == nullptr)
      {
      this->Internal->TemporaryPath.clear();
      }
    else if (this->Internal->TemporaryPath == std::string(path))
      {
      return;
      }
    else
      {
      this->Internal->TemporaryPath = std::string(path);
      }
    this->Modified();
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::SaveSceneScreenshot(vtkImageData* screenshot)
{
  if (this->GetDMMLScene() == nullptr)
    {
    vtkErrorMacro("vtkDMMLApplicationLogic::SaveSceneScreenshot failed: invalid scene or URL");
    return;
    }
  this->GetDMMLScene()->SaveSceneScreenshot(screenshot);
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::PauseRender()
{
  // Observers in qCjyxCoreApplication listen for PauseRenderEvent and call pauseRender
  // to temporarily stop rendering in all views in the current layout
  this->InvokeEvent(PauseRenderEvent);
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::ResumeRender()
{
  // Observers in qCjyxCoreApplication listen for ResumeRenderEvent and call resumeRender
  // to resume rendering in all views in the current layout
  this->InvokeEvent(ResumeRenderEvent);
}

//---------------------------------------------------------------------------
void vtkDMMLApplicationLogic::ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  if (vtkDMMLInteractionNode::SafeDownCast(caller) && event == vtkDMMLInteractionNode::EditNodeEvent)
    {
    if (callData != nullptr)
      {
      vtkDMMLNode* nodeToBeEdited = reinterpret_cast<vtkDMMLNode*>(callData);
      this->EditNode(nodeToBeEdited);
      }
    }
  else if (vtkDMMLInteractionNode::SafeDownCast(caller) && event == vtkDMMLInteractionNode::ShowViewContextMenuEvent)
    {
    this->InvokeEvent(ShowViewContextMenuEvent, callData);
    }
  else
    {
    this->Superclass::ProcessDMMLNodesEvents(caller, event, callData);
    }
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::EditNode(vtkDMMLNode* node)
{
  // Observers in qCjyxCoreApplication listen for this event
  this->InvokeEvent(EditNodeEvent, node);
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::SetModuleLogic(const char* moduleName,
                                             vtkDMMLAbstractLogic* moduleLogic)
{
  if (!moduleName)
    {
    vtkErrorMacro("AddModuleLogic: invalid module name.");
    return;
    }
  if (moduleLogic)
    {
    this->Internal->ModuleLogicMap[moduleName] = moduleLogic;
    }
  else
    {
    // If no logic is provided, erase the module-logic association.
    this->Internal->ModuleLogicMap.erase(moduleName);
    }
}

//----------------------------------------------------------------------------
vtkDMMLAbstractLogic* vtkDMMLApplicationLogic::GetModuleLogic(const char* moduleName) const
{
  if (!moduleName)
    {
    vtkErrorMacro("GetModuleLogic: invalid module name");
    return nullptr;
    }
  //Check that the logic is registered.
  if (this->Internal->ModuleLogicMap.count(moduleName) == 0)
    {
    return nullptr;
    }
  return this->Internal->ModuleLogicMap[moduleName];
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::OnDMMLSceneStartBatchProcess()
{
  this->PauseRender();
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::OnDMMLSceneEndBatchProcess()
{
  this->ResumeRender();
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::OnDMMLSceneStartImport()
{
  this->PauseRender();
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::OnDMMLSceneEndImport()
{
  this->ResumeRender();
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::OnDMMLSceneStartRestore()
{
  this->PauseRender();
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::OnDMMLSceneEndRestore()
{
  this->ResumeRender();
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::SetIntersectingSlicesEnabled(
  vtkDMMLApplicationLogic::IntersectingSlicesOperation operation, bool enabled)
{
  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkWarningMacro("vtkDMMLApplicationLogic::SetIntersectingSlicesEnabled failed: invalid scene");
    return;
    }

  vtkSmartPointer<vtkCollection> sliceDisplayNodes =
    vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkDMMLSliceDisplayNode"));
  if (sliceDisplayNodes.GetPointer())
    {
    vtkDMMLSliceDisplayNode* sliceDisplayNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (sliceDisplayNodes->InitTraversal(it);
      (sliceDisplayNode = static_cast<vtkDMMLSliceDisplayNode*>(sliceDisplayNodes->GetNextItemAsObject(it)));)
      {
      switch (operation)
        {
        case vtkDMMLApplicationLogic::IntersectingSlicesVisibility:
          sliceDisplayNode->SetIntersectingSlicesVisibility(enabled);
          break;
        case vtkDMMLApplicationLogic::IntersectingSlicesInteractive:
          sliceDisplayNode->SetIntersectingSlicesInteractive(enabled);
          break;
        case vtkDMMLApplicationLogic::IntersectingSlicesTranslation:
          sliceDisplayNode->SetIntersectingSlicesTranslationEnabled(enabled);
          break;
        case vtkDMMLApplicationLogic::IntersectingSlicesRotation:
          sliceDisplayNode->SetIntersectingSlicesRotationEnabled(enabled);
          break;
        }
      }
    }

  // The vtkDMMLSliceIntersectionWidget should observe slice display node modifications
  // but as a workaround for now, trigger update by modifying all the slice nodes.
  vtkSmartPointer<vtkCollection> sliceNodes =
    vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkDMMLSliceNode"));
  if (sliceNodes.GetPointer())
    {
    vtkDMMLSliceNode* sliceNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (sliceNodes->InitTraversal(it);
      (sliceNode = static_cast<vtkDMMLSliceNode*>(sliceNodes->GetNextItemAsObject(it)));)
      {
      sliceNode->Modified();
      }
    }
}

//----------------------------------------------------------------------------
bool vtkDMMLApplicationLogic::GetIntersectingSlicesEnabled(
  vtkDMMLApplicationLogic::IntersectingSlicesOperation operation)
{
  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkWarningMacro("vtkDMMLApplicationLogic::GetIntersectingSlicesEnabled failed: invalid scene");
    return false;
    }
  vtkDMMLSliceDisplayNode* sliceDisplayNode = vtkDMMLSliceDisplayNode::SafeDownCast(
    scene->GetFirstNodeByClass("vtkDMMLSliceDisplayNode"));
  if (!sliceDisplayNode)
    {
    // No slice display nodes are in the scene yet, use the scene default node instead.
    // Developers can set the default appearance of intersecting slices by modifying the
    // default slice display node.
    vtkSmartPointer<vtkDMMLSliceDisplayNode> defaultSliceDisplayNode =
      vtkDMMLSliceDisplayNode::SafeDownCast(scene->GetDefaultNodeByClass("vtkDMMLSliceDisplayNode"));
    if (!defaultSliceDisplayNode.GetPointer())
      {
      defaultSliceDisplayNode = vtkSmartPointer<vtkDMMLSliceDisplayNode>::New();
      scene->AddDefaultNode(defaultSliceDisplayNode);
      }
    sliceDisplayNode = defaultSliceDisplayNode;
    if (!sliceDisplayNode)
      {
      vtkErrorMacro("vtkDMMLApplicationLogic::GetIntersectingSlicesEnabled failed: cannot get slice display node");
      return false;
      }
    }

  switch (operation)
    {
    case vtkDMMLApplicationLogic::IntersectingSlicesVisibility:
      return sliceDisplayNode->GetIntersectingSlicesVisibility();
    case vtkDMMLApplicationLogic::IntersectingSlicesInteractive:
      return sliceDisplayNode->GetIntersectingSlicesInteractive();
    case vtkDMMLApplicationLogic::IntersectingSlicesTranslation:
      return sliceDisplayNode->GetIntersectingSlicesTranslationEnabled();
    case vtkDMMLApplicationLogic::IntersectingSlicesRotation:
      return sliceDisplayNode->GetIntersectingSlicesRotationEnabled();
    }

  return false;
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::SetIntersectingSlicesIntersectionMode(int mode)
{
  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkWarningMacro("vtkDMMLApplicationLogic::SetIntersectingSlicesEnabled failed: invalid scene");
    return;
    }

  vtkSmartPointer<vtkCollection> sliceDisplayNodes =
    vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkDMMLSliceDisplayNode"));
  if (sliceDisplayNodes.GetPointer())
    {
    vtkDMMLSliceDisplayNode* sliceDisplayNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (sliceDisplayNodes->InitTraversal(it);
      (sliceDisplayNode = static_cast<vtkDMMLSliceDisplayNode*>(sliceDisplayNodes->GetNextItemAsObject(it)));)
      {
      sliceDisplayNode->SetIntersectingSlicesIntersectionMode(mode);
      }
    }

  // The vtkDMMLSliceIntersectionWidget should observe slice display node modifications
  // but as a workaround for now, trigger update by modifying all the slice nodes.
  vtkSmartPointer<vtkCollection> sliceNodes =
    vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkDMMLSliceNode"));
  if (sliceNodes.GetPointer())
  {
    vtkDMMLSliceNode* sliceNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (sliceNodes->InitTraversal(it);
      (sliceNode = static_cast<vtkDMMLSliceNode*>(sliceNodes->GetNextItemAsObject(it)));)
    {
      sliceNode->Modified();
    }
  }
}

//----------------------------------------------------------------------------
int vtkDMMLApplicationLogic::GetIntersectingSlicesIntersectionMode()
{
  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkWarningMacro("vtkDMMLApplicationLogic::GetIntersectingSlicesEnabled failed: invalid scene");
    return false;
    }
  vtkDMMLSliceDisplayNode* sliceDisplayNode = vtkDMMLSliceDisplayNode::SafeDownCast(
    scene->GetFirstNodeByClass("vtkDMMLSliceDisplayNode"));
  if (!sliceDisplayNode)
    {
    // No slice display nodes are in the scene yet, use the scene default node instead.
    // Developers can set the default appearance of intersecting slices by modifying the
    // default slice display node.
    vtkSmartPointer<vtkDMMLSliceDisplayNode> defaultSliceDisplayNode =
      vtkDMMLSliceDisplayNode::SafeDownCast(scene->GetDefaultNodeByClass("vtkDMMLSliceDisplayNode"));
    if (!defaultSliceDisplayNode.GetPointer())
      {
      defaultSliceDisplayNode = vtkSmartPointer<vtkDMMLSliceDisplayNode>::New();
      scene->AddDefaultNode(defaultSliceDisplayNode);
      }
    sliceDisplayNode = defaultSliceDisplayNode;
    if (!sliceDisplayNode)
      {
      vtkErrorMacro("vtkDMMLApplicationLogic::GetIntersectingSlicesEnabled failed: cannot get slice display node");
      return false;
      }
    }

  return sliceDisplayNode->GetIntersectingSlicesIntersectionMode();
}

//----------------------------------------------------------------------------
void vtkDMMLApplicationLogic::SetIntersectingSlicesLineThicknessMode(int mode)
{
  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkWarningMacro("vtkDMMLApplicationLogic::SetIntersectingSlicesEnabled failed: invalid scene");
    return;
    }

  vtkSmartPointer<vtkCollection> sliceDisplayNodes =
    vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkDMMLSliceDisplayNode"));
  if (sliceDisplayNodes.GetPointer())
    {
    vtkDMMLSliceDisplayNode* sliceDisplayNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (sliceDisplayNodes->InitTraversal(it);
      (sliceDisplayNode = static_cast<vtkDMMLSliceDisplayNode*>(sliceDisplayNodes->GetNextItemAsObject(it)));)
      {
      sliceDisplayNode->SetIntersectingSlicesLineThicknessMode(mode);
      }
    }

  // The vtkDMMLSliceIntersectionWidget should observe slice display node modifications
  // but as a workaround for now, trigger update by modifying all the slice nodes.
  vtkSmartPointer<vtkCollection> sliceNodes =
    vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkDMMLSliceNode"));
  if (sliceNodes.GetPointer())
    {
    vtkDMMLSliceNode* sliceNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (sliceNodes->InitTraversal(it);
      (sliceNode = static_cast<vtkDMMLSliceNode*>(sliceNodes->GetNextItemAsObject(it)));)
      {
      sliceNode->Modified();
      }
    }
}

//----------------------------------------------------------------------------
int vtkDMMLApplicationLogic::GetIntersectingSlicesLineThicknessMode()
{
  vtkDMMLScene* scene = this->GetDMMLScene();
  if (!scene)
    {
    vtkWarningMacro("vtkDMMLApplicationLogic::GetIntersectingSlicesEnabled failed: invalid scene");
    return false;
    }
  vtkDMMLSliceDisplayNode* sliceDisplayNode = vtkDMMLSliceDisplayNode::SafeDownCast(
    scene->GetFirstNodeByClass("vtkDMMLSliceDisplayNode"));
  if (!sliceDisplayNode)
    {
    // No slice display nodes are in the scene yet, use the scene default node instead.
    // Developers can set the default appearance of intersecting slices by modifying the
    // default slice display node.
    vtkSmartPointer<vtkDMMLSliceDisplayNode> defaultSliceDisplayNode =
      vtkDMMLSliceDisplayNode::SafeDownCast(scene->GetDefaultNodeByClass("vtkDMMLSliceDisplayNode"));
    if (!defaultSliceDisplayNode.GetPointer())
      {
      defaultSliceDisplayNode = vtkSmartPointer<vtkDMMLSliceDisplayNode>::New();
      scene->AddDefaultNode(defaultSliceDisplayNode);
      }
    sliceDisplayNode = defaultSliceDisplayNode;
    if (!sliceDisplayNode)
      {
      vtkErrorMacro("vtkDMMLApplicationLogic::GetIntersectingSlicesEnabled failed: cannot get slice display node");
      return false;
      }
    }

  return sliceDisplayNode->GetIntersectingSlicesLineThicknessMode();
}
