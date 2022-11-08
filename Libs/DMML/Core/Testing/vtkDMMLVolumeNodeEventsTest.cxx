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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// DMML includes
#include "vtkDMMLCoreTestingMacros.h"
#include "vtkDMMLVolumeNode.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
class vtkDMMLTestVolumeNode
  : public vtkDMMLVolumeNode
{
public:
  // Provide a concrete New.
  static vtkDMMLTestVolumeNode *New();
  vtkTypeMacro(vtkDMMLTestVolumeNode,vtkDMMLVolumeNode);
  vtkDMMLNode* CreateNodeInstance() override {return  vtkDMMLTestVolumeNode::New();}
  const char* GetNodeTagName() override {return "vtkDMMLTestVolumeNode";}
};
vtkStandardNewMacro(vtkDMMLTestVolumeNode);

//---------------------------------------------------------------------------
int vtkDMMLVolumeNodeEventsTest(int , char * [] )
{
  vtkNew<vtkDMMLTestVolumeNode> volumeNode;

  vtkNew<vtkDMMLCoreTestingUtilities::vtkDMMLNodeCallback> callback;

  volumeNode->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  // Test vtkDMMLVolumeNode::SetAndObserveImageData()
  vtkNew<vtkImageData> imageData;
  volumeNode->SetAndObserveImageData(imageData.GetPointer());

  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent) != 1)
    {
    std::cerr << __LINE__ << ": vtkDMMLVolumeNode::SetAndObserveImageData failed: "
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: "
              << callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << "Number of ImageDataModifiedEvent: "
              << callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEvents();

  // Set the same image data:
  volumeNode->SetAndObserveImageData(imageData.GetPointer());

  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 0 ||
      callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent) != 0)
    {
    std::cerr << __LINE__ << ": vtkDMMLVolumeNode::SetAndObserveImageData failed: "
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: "
              << callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << "Number of ImageDataModifiedEvent: "
              << callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEvents();

  // Update image data
  imageData->Modified();
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 0 ||
      callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent) != 1)
    {
    std::cerr << "vtkImageData::Modified failed."
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: "
              << callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << "Number of ImageDataModifiedEvent: "
              << callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEvents();

  // Update volume node
  volumeNode->Modified();
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent) != 0)
    {
    std::cerr << __LINE__ << ": vtkDMMLVolumeNode::Modified failed: "
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: "
              << callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << "Number of ImageDataModifiedEvent: "
              << callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEvents();

  // StartModify
  int wasModifying = volumeNode->StartModify();
  volumeNode->Modified();
  imageData->Modified();
  if (!callback->GetErrorString().empty() ||
    callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 0 ||
    callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent) != 0)
  {
    std::cerr << __LINE__ << ": vtkDMMLVolumeNode::StartModify failed: "
      << callback->GetErrorString().c_str() << " "
      << "Number of ModifiedEvent: "
      << callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
      << "Number of ImageDataModifiedEvent: "
      << callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent)
      << std::endl;
    return EXIT_FAILURE;
  }
  callback->ResetNumberOfEvents();

  // EndModify
  volumeNode->EndModify(wasModifying);
  if (!callback->GetErrorString().empty() ||
    callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
    callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent) != 1)
  {
    std::cerr << __LINE__ << ": vtkDMMLVolumeNode::EndModify failed: "
      << callback->GetErrorString().c_str() << " "
      << "Number of ModifiedEvent: "
      << callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
      << "Number of ImageDataModifiedEvent: "
      << callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent)
      << std::endl;
    return EXIT_FAILURE;
  }
  callback->ResetNumberOfEvents();

  // Set new image data
  vtkNew<vtkImageData> imageData2;
  volumeNode->SetAndObserveImageData(imageData2.GetPointer());

  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent) != 1)
    {
    std::cerr << __LINE__ << ": vtkDMMLVolumeNode::SetAndObserveImageData failed: "
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: "
              << callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << "Number of ImageDataModifiedEvent: "
              << callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEvents();

  // Update old image data
  imageData->Modified();

  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 0 ||
      callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent) != 0)
    {
    std::cerr << __LINE__ << ": vtkDMMLVolumeNode::SetAndObserveImageData failed: "
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: "
              << callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << "Number of ImageDataModifiedEvent: "
              << callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEvents();

  // Update new image data
  imageData2->Modified();

  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 0 ||
      callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent) != 1)
    {
    std::cerr << __LINE__ << ": vtkDMMLVolumeNode::SetAndObserveImageData failed: "
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: "
              << callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << "Number of ImageDataModifiedEvent: "
              << callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEvents();

  // Clear image data
  volumeNode->SetAndObserveImageData(nullptr);

  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1 ||
      callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent) != 1)
    {
    std::cerr << __LINE__ << ": vtkDMMLVolumeNode::SetAndObserveImageData failed: "
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: "
              << callback->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " "
              << "Number of ImageDataModifiedEvent: "
              << callback->GetNumberOfEvents(vtkDMMLVolumeNode::ImageDataModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEvents();

  return EXIT_SUCCESS;
}
