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

// Qt includes
#include <QMimeData>
#include <QPainter>
#include <QStyle>
#include <QUrl>
#include <QUrlQuery>

// CTK includes
#include "ctkVTKWidgetsUtils.h"

// qDMML includes
#include "qDMMLUtils.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLTransformNode.h>
#include <vtkDMMLViewNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkQImageToImageSource.h>
#include <vtkTransform.h>

//-----------------------------------------------------------------------------
qDMMLUtils::qDMMLUtils(QObject* _parent)
  :QObject(_parent)
{
}

//-----------------------------------------------------------------------------
qDMMLUtils::~qDMMLUtils() = default;

//------------------------------------------------------------------------------
void qDMMLUtils::vtkMatrixToQVector(vtkMatrix4x4* matrix, QVector<double> & vector)
{
  if (!matrix) { return; }

  vector.clear();

  for (int i=0; i < 4; i++)
    {
    for (int j=0; j < 4; j++)
      {
      vector.append(matrix->GetElement(i,j));
      }
    }
}

//------------------------------------------------------------------------------
void qDMMLUtils::getTransformInCoordinateSystem(vtkDMMLNode* node, bool global,
    vtkTransform* transform)
{
  Self::getTransformInCoordinateSystem(vtkDMMLTransformNode::SafeDownCast( node ),
    global, transform);
}

//------------------------------------------------------------------------------
void qDMMLUtils::getTransformInCoordinateSystem(vtkDMMLTransformNode* transformNode,
  bool global, vtkTransform* transform)
{
  Q_ASSERT(transform);
  if (!transform)
    {
    return;
    }

  transform->Identity();

  if (!transformNode || !transformNode->IsLinear())
    {
    return;
    }

  vtkNew<vtkMatrix4x4> matrix;
  int matrixDefined=transformNode->GetMatrixTransformToParent(matrix.GetPointer());
  Q_ASSERT(matrixDefined);
  if (!matrixDefined)
    {
    return;
    }

  transform->SetMatrix(matrix.GetPointer());

  if ( global )
    {
    transform->PostMultiply();
    }
  else
    {
    transform->PreMultiply();
    }
}

//------------------------------------------------------------------------------
int qDMMLUtils::countVisibleViewNode(vtkDMMLScene* scene)
{
  Q_ASSERT(scene);
  int numberOfVisibleNodes = 0;
  const char* className = "vtkDMMLViewNode";
  int nnodes = scene->GetNumberOfNodesByClass(className);
  for (int n = 0; n < nnodes; n++)
    {
    vtkDMMLViewNode * node = vtkDMMLViewNode::SafeDownCast(scene->GetNthNodeByClass(n, className));
    if (node && node->GetVisibility())
      {
      numberOfVisibleNodes++;
      }
    }
  return numberOfVisibleNodes;
}

// ----------------------------------------------------------------
QPixmap qDMMLUtils::createColorPixmap(QStyle * style, const QColor &color)
{
  if (!style)
    {
    return QPixmap();
    }

  const int size = style->pixelMetric(QStyle::PM_SmallIconSize) - 4;

  // Create a pixmap
  QPixmap colorFieldPixmap(size, size);

  // Fill it with the color
  colorFieldPixmap.fill(color);

  // Make a black rectangle on the border
  QPainter painter(&colorFieldPixmap);
  painter.drawRect(0, 0, size - 1, size - 1);

  return colorFieldPixmap;
}

//---------------------------------------------------------------------------
bool qDMMLUtils::qImageToVtkImageData(const QImage& qImage, vtkImageData* vtkimage)
{
  return ctk::qImageToVTKImageData(qImage, vtkimage);
}

//---------------------------------------------------------------------------
bool qDMMLUtils::vtkImageDataToQImage(vtkImageData* vtkimage, QImage& img)
{
  img = ctk::vtkImageDataToQImage(vtkimage);
  return !img.isNull();
}

//-----------------------------------------------------------------------------
void qDMMLUtils::colorToQColor(const double* color, QColor &qcolor)
{
  if (color)
    {
    qcolor = QColor::fromRgbF(color[0], color[1], color[2]);
    }
}

//-----------------------------------------------------------------------------
void qDMMLUtils::qColorToColor(const QColor &qcolor, double* color)
{
  if (color)
    {
    color[0] = qcolor.redF();
    color[1] = qcolor.greenF();
    color[2] = qcolor.blueF();
    }
}

//------------------------------------------------------------------------------
void qDMMLUtils::mimeDataToSubjectHierarchyItemIDs(const QMimeData* mimeData, vtkIdList* idList)
{
  if (!mimeData->hasFormat("text/uri-list") || !idList)
    {
    return;
    }
  idList->Reset();
  foreach(QUrl url, mimeData->urls())
    {
    if (!url.isValid() || url.isEmpty())
      {
      continue;
      }
    if (url.scheme() != "dmml" || url.host() != "scene" || url.path() != "/subjecthierarchy/item")
      {
      continue;
      }
    QUrlQuery query(url.query());
    idList->InsertNextId(query.queryItemValue("id").toLong());
    }
}
