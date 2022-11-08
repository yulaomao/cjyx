/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women�s Hospital through NIH grant R01MH112748.

==============================================================================*/

// Qt includes
#include <QDebug>

// QtGUI includes
#include "qCjyxMarkupsWriter.h"

// QTCore includes
#include "qCjyxCoreApplication.h"
#include "qCjyxCoreIOManager.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLSceneViewNode.h>
#include <vtkDMMLMarkupsJsonStorageNode.h>
#include <vtkDMMLMarkupsFiducialStorageNode.h>
#include <vtkDMMLStorableNode.h>
#include <vtkDMMLStorageNode.h>

// VTK includes
#include <vtkStdString.h>
#include <vtkStringArray.h>

//----------------------------------------------------------------------------
qCjyxMarkupsWriter::qCjyxMarkupsWriter(QObject* parentObject)
  : qCjyxNodeWriter("Markups", QString("MarkupsFile"), QStringList() << "vtkDMMLMarkupsNode", true, parentObject)
{
}

//----------------------------------------------------------------------------
qCjyxMarkupsWriter::~qCjyxMarkupsWriter() = default;

//----------------------------------------------------------------------------
QStringList qCjyxMarkupsWriter::extensions(vtkObject* vtkNotUsed(object))const
{
  QStringList supportedExtensions;

  vtkNew<vtkDMMLMarkupsJsonStorageNode> jsonStorageNode;
  const int formatCount = jsonStorageNode->GetSupportedWriteFileTypes()->GetNumberOfValues();
  for (int formatIt = 0; formatIt < formatCount; ++formatIt)
    {
    vtkStdString format = jsonStorageNode->GetSupportedWriteFileTypes()->GetValue(formatIt);
    supportedExtensions << QString::fromStdString(format);
    }

  vtkNew<vtkDMMLMarkupsFiducialStorageNode> fcsvStorageNode;
  const int fidsFormatCount = fcsvStorageNode->GetSupportedWriteFileTypes()->GetNumberOfValues();
  for (int formatIt = 0; formatIt < fidsFormatCount; ++formatIt)
    {
    vtkStdString format = fcsvStorageNode->GetSupportedWriteFileTypes()->GetValue(formatIt);
    supportedExtensions << QString::fromStdString(format);
    }

  return supportedExtensions;
}

//----------------------------------------------------------------------------
void qCjyxMarkupsWriter::setStorageNodeClass(vtkDMMLStorableNode* storableNode, const QString& storageNodeClassName)
{
  if (!storableNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: invalid storable node";
    return;
    }
  vtkDMMLScene* scene = storableNode->GetScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << " failed: invalid scene";
    return;
    }

  vtkDMMLStorageNode* currentStorageNode = storableNode->GetStorageNode();
  std::string storageNodeClassNameStr = storageNodeClassName.toStdString();
  if (currentStorageNode != nullptr && currentStorageNode->IsA(storageNodeClassNameStr.c_str()))
    {
    // requested storage node class is the same as current class, there is nothing to do
    return;
    }

  // Create and use new storage node of the correct class
  vtkDMMLStorageNode* newStorageNode = vtkDMMLStorageNode::SafeDownCast(scene->AddNewNodeByClass(storageNodeClassNameStr));
  if (!newStorageNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: cannot create new storage node of class " << storageNodeClassName;
    return;
    }
  storableNode->SetAndObserveStorageNodeID(newStorageNode->GetID());

  // Remove old storage node
  if (currentStorageNode)
    {
    scene->RemoveNode(currentStorageNode);
    }
}

//----------------------------------------------------------------------------
bool qCjyxMarkupsWriter::write(const qCjyxIO::IOProperties& properties)
{
  vtkDMMLStorableNode* node = vtkDMMLStorableNode::SafeDownCast(this->getNodeByID(properties["nodeID"].toString().toUtf8().data()));
  std::string fileName = properties["fileName"].toString().toStdString();

  vtkNew<vtkDMMLMarkupsFiducialStorageNode> fcsvStorageNode;
  std::string fcsvCompatibleFileExtension = fcsvStorageNode->GetSupportedFileExtension(fileName.c_str(), false, true);
  if (!fcsvCompatibleFileExtension.empty())
    {
    // fcsv file needs to be written
    this->setStorageNodeClass(node, "vtkDMMLMarkupsFiducialStorageNode");
    }
  else
    {
    // json file needs to be written
    this->setStorageNodeClass(node, "vtkDMMLMarkupsJsonStorageNode");
    }

  return Superclass::write(properties);
}
