/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// DMML includes
#include "vtkDMMLMarkupsJsonStorageNode_Private.h"

//---------------------------------------------------------------------------
class vtkDMMLMarkupsPlaneJsonStorageNode::vtkInternalPlane : public vtkDMMLMarkupsJsonStorageNode::vtkInternal
{
public:
  vtkInternalPlane(vtkDMMLMarkupsPlaneJsonStorageNode* external);
  ~vtkInternalPlane();

  bool WriteMarkup(rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, vtkDMMLMarkupsNode* markupsNode) override;
  bool UpdateMarkupsNodeFromJsonValue(vtkDMMLMarkupsNode* markupsNode, rapidjson::Value& markupObject) override;
  bool WritePlaneProperties(rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, vtkDMMLMarkupsPlaneNode* markupsNode);
};
