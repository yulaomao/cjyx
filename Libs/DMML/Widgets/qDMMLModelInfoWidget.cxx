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

// qDMML includes
#include "qDMMLModelInfoWidget.h"
#include "ui_qDMMLModelInfoWidget.h"

// DMML includes
#include <vtkDMMLStorageNode.h>
#include <vtkDMMLModelNode.h>

// VTK includes
#include <vtkCellData.h>
#include <vtkGeometryFilter.h>
#include <vtkMassProperties.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVersion.h>

//------------------------------------------------------------------------------
class qDMMLModelInfoWidgetPrivate: public Ui_qDMMLModelInfoWidget
{
  Q_DECLARE_PUBLIC(qDMMLModelInfoWidget);

protected:
  qDMMLModelInfoWidget* const q_ptr;

public:
  qDMMLModelInfoWidgetPrivate(qDMMLModelInfoWidget& object);
  void init();

  vtkDMMLModelNode* DMMLModelNode;
  vtkSmartPointer<vtkTriangleFilter> TriangleFilter;
  vtkSmartPointer<vtkGeometryFilter> GeometryFilter;
  vtkSmartPointer<vtkMassProperties> MassProperties;
};

//------------------------------------------------------------------------------
qDMMLModelInfoWidgetPrivate::qDMMLModelInfoWidgetPrivate(qDMMLModelInfoWidget& object)
  : q_ptr(&object)
{
  this->DMMLModelNode = nullptr;
  this->GeometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
  this->TriangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  this->TriangleFilter->SetPassLines(0);
  this->MassProperties = vtkSmartPointer<vtkMassProperties>::New();
}

//------------------------------------------------------------------------------
void qDMMLModelInfoWidgetPrivate::init()
{
  Q_Q(qDMMLModelInfoWidget);
  this->setupUi(q);
  this->ExpandButton->setOrientation(Qt::Vertical);
  this->ExpandButton->setChecked(false);
  q->setEnabled(this->DMMLModelNode != nullptr);
}

//------------------------------------------------------------------------------
qDMMLModelInfoWidget::qDMMLModelInfoWidget(QWidget *_parent)
  : QWidget(_parent)
  , d_ptr(new qDMMLModelInfoWidgetPrivate(*this))
{
  Q_D(qDMMLModelInfoWidget);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLModelInfoWidget::~qDMMLModelInfoWidget() = default;


//------------------------------------------------------------------------------
vtkDMMLModelNode* qDMMLModelInfoWidget::dmmlModelNode()const
{
  Q_D(const qDMMLModelInfoWidget);
  return d->DMMLModelNode;
}

//------------------------------------------------------------------------------
void qDMMLModelInfoWidget::setDMMLModelNode(vtkDMMLNode* node)
{
  this->setDMMLModelNode(vtkDMMLModelNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qDMMLModelInfoWidget::setDMMLModelNode(vtkDMMLModelNode* modelNode)
{
  Q_D(qDMMLModelInfoWidget);
  qvtkReconnect(d->DMMLModelNode, modelNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDMML()));
  //qvtkReconnect(d->DMMLModelNode, modelNode, vtkDMMLModelNode::DisplayModifiedEvent,
  //              this, SLOT(updateWidgetFromDMML()));
  d->DMMLModelNode = modelNode;
  this->updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qDMMLModelInfoWidget::showEvent(QShowEvent *)
{
  // Update the widget, now that it becomes becomes visible
  // (we might have missed some updates, because widget contents is not updated
  // if the widget is not visible).
  updateWidgetFromDMML();
}

//------------------------------------------------------------------------------
void qDMMLModelInfoWidget::updateWidgetFromDMML()
{
  Q_D(qDMMLModelInfoWidget);
  if (!this->isVisible())
  {
    // Getting the model information is too expensive,
    // so if the widget is not visible then do not update
    return;
  }
  vtkPointSet *mesh = d->DMMLModelNode ? d->DMMLModelNode->GetMesh() : nullptr;
  if (mesh)
    {
    vtkPolyDataAlgorithm* filter;
    vtkPolyData* poly = vtkPolyData::SafeDownCast(mesh);
    if (poly)
      {
      filter = d->TriangleFilter;
      }
    else
      {
      filter = d->GeometryFilter;
      }
    d->MassProperties->SetInputConnection( filter->GetOutputPort() );
    filter->SetInputData(mesh);
    filter->Update();
    if (filter->GetOutput()->GetNumberOfCells() > 0)
      {
      d->SurfaceAreaDoubleSpinBox->setValue(d->MassProperties->GetSurfaceArea());
      d->VolumeAreaDoubleSpinBox->setValue(d->MassProperties->GetVolume());
      }
    else
      {
      d->SurfaceAreaDoubleSpinBox->setValue(0);
      d->VolumeAreaDoubleSpinBox->setValue(0);
      }

    d->NumberOfPointsSpinBox->setValue(mesh->GetNumberOfPoints());
    d->NumberOfCellsSpinBox->setValue(mesh->GetNumberOfCells());
    if (poly)
      {
      d->MeshTypeLineEdit->setText("Surface Mesh (vtkPolyData)");
      d->NumberOfVertsValueLabel->setText(QString::number(poly->GetNumberOfVerts()));
      d->NumberOfLinesValueLabel->setText(QString::number(poly->GetNumberOfLines()));
      d->NumberOfPolysValueLabel->setText(QString::number(poly->GetNumberOfPolys()));
      d->NumberOfStripsValueLabel->setText(QString::number(poly->GetNumberOfStrips()));
      }
    else
      {
      d->MeshTypeLineEdit->setText("Volumetric Mesh (vtkUnstructuredGrid)");
      d->NumberOfVertsValueLabel->setText("0");
      d->NumberOfLinesValueLabel->setText("0");
      d->NumberOfPolysValueLabel->setText("0");
      d->NumberOfStripsValueLabel->setText("0");
      }
    d->MaxCellSizeValueLabel->setText(QString::number(mesh->GetMaxCellSize()));
    d->NumberOfPointsScalarsSpinBox->setValue(mesh->GetPointData()->GetNumberOfComponents());
    d->NumberOfCellsScalarsSpinBox->setValue(mesh->GetCellData()->GetNumberOfComponents());
    }
  else
    {
    d->SurfaceAreaDoubleSpinBox->setValue(0.);
    d->VolumeAreaDoubleSpinBox->setValue(0.);

    d->NumberOfPointsSpinBox->setValue(0);
    d->NumberOfCellsSpinBox->setValue(0);
    d->NumberOfVertsValueLabel->setText("0");
    d->NumberOfLinesValueLabel->setText("0");
    d->NumberOfPolysValueLabel->setText("0");
    d->NumberOfStripsValueLabel->setText("0");
    d->MaxCellSizeValueLabel->setText("0");
    d->NumberOfPointsScalarsSpinBox->setValue(0);
    d->NumberOfCellsScalarsSpinBox->setValue(0);
    }

  vtkDMMLStorageNode *storageNode = d->DMMLModelNode ? d->DMMLModelNode->GetStorageNode() : nullptr;
  if (storageNode)
    {
    d->FileNameLineEdit->setText(storageNode->GetFileName());
    }
  else
    {
    d->FileNameLineEdit->setText("");
    }
  this->setEnabled(d->DMMLModelNode != nullptr);
}

