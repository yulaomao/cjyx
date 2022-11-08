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

// Qt includes
#include <QDebug>
#include <QTimer>

// Cjyx includes
#include "qCjyxApplication.h"
#include "qCjyxCoreIOManager.h"
#include "qCjyxSaveDataDialog.h"
#include "qCjyxFileWriter.h"

// DMML includes
#include <vtkDMMLLinearTransformNode.h>
#include <vtkDMMLTransformStorageNode.h>
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkNew.h>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_Dummy
class qCjyxDummyFileWriter: public qCjyxFileWriter
{
public:
  qCjyxDummyFileWriter(qCjyxIO::IOFileType fileType, QObject* parent = nullptr)
    : qCjyxFileWriter(parent)
    , FileType(fileType)
    {}
  ~qCjyxDummyFileWriter() override = default;
  virtual QStringList nodeTags()const {return QStringList() << "LinearTransform";}
  QString description()const override{return "Dummy";}
  qCjyxIO::IOFileType fileType()const override{return this->FileType;}
  QStringList extensions(vtkObject*)const override{return QStringList(QString("MyType(*.mhd *.vtk)"));}

  bool write(const IOProperties& properties) override;

  qCjyxIO::IOFileType FileType;
};

//-----------------------------------------------------------------------------
bool qCjyxDummyFileWriter::write(const IOProperties& properties)
{
  QStringList nodeIDs = QStringList() << properties["nodeID"].toString();
  qDebug() << "write" << properties["nodeID"].toString();
  this->setWrittenNodes(nodeIDs);
  return true;
}

//-----------------------------------------------------------------------------
int qCjyxSaveDataDialogCustomFileWriterTest(int argc, char * argv[] )
{
  qCjyxApplication app(argc, argv);
  app.coreIOManager()->registerIO(new qCjyxDummyFileWriter(QString("TransformFile"), nullptr));

  vtkNew<vtkDMMLTransformStorageNode> storageNode;
  app.dmmlScene()->AddNode(storageNode.GetPointer());
  vtkNew<vtkDMMLLinearTransformNode> transformNode;
  app.dmmlScene()->AddNode(transformNode.GetPointer());
  transformNode->SetAndObserveStorageNodeID(storageNode->GetID());

  qCjyxSaveDataDialog saveDataDialog;

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    // Quit the dialog
    QTimer::singleShot(100, &app, SLOT(quit()));
    // Quit the app
    QTimer::singleShot(120, &app, SLOT(quit()));
    }

  return saveDataDialog.exec();
}

