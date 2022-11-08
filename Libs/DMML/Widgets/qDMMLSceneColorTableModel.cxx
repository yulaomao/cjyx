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
#include <QPixmap>

// CTK includes
#include <ctkVTKWidgetsUtils.h>

// qDMML includes
#include "qDMMLSceneColorTableModel.h"

// DMML includes
#include <vtkDMMLColorNode.h>

// VTK includes
#include <vtkScalarsToColors.h>

//------------------------------------------------------------------------------
class qDMMLSceneColorTableModelPrivate
{
public:
  struct ColorGradient
  {
    ColorGradient();
    void updatePixmap(vtkScalarsToColors* scalarsToColors);

    vtkMTimeType  MTime;
    QPixmap       Pixmap;
  };

  mutable QMap<QString, ColorGradient> GradientCache;
};

//------------------------------------------------------------------------------
qDMMLSceneColorTableModelPrivate::ColorGradient::ColorGradient()
{
  this->MTime = 0;
  this->Pixmap = QPixmap(50, 31);
}

//------------------------------------------------------------------------------
void qDMMLSceneColorTableModelPrivate::ColorGradient::updatePixmap(vtkScalarsToColors* scalarsToColors)
{
  if (!scalarsToColors ||
      scalarsToColors->GetNumberOfAvailableColors() <= 0)
    {
    return;
    }
  this->Pixmap = QPixmap::fromImage(ctk::scalarsToColorsImage( scalarsToColors, this->Pixmap.size() ));
  this->MTime = scalarsToColors->GetMTime();
}

//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
qDMMLSceneColorTableModel::qDMMLSceneColorTableModel(QObject *vparent)
  : qDMMLSceneCategoryModel(vparent)
  , d_ptr(new qDMMLSceneColorTableModelPrivate)
{
}

//------------------------------------------------------------------------------
qDMMLSceneColorTableModel::~qDMMLSceneColorTableModel() = default;

//------------------------------------------------------------------------------
void qDMMLSceneColorTableModel::updateItemFromNode(QStandardItem* item, vtkDMMLNode* node, int column)
{
  Q_D(const qDMMLSceneColorTableModel);
  this->qDMMLSceneModel::updateItemFromNode(item, node, column);
  vtkDMMLColorNode* colorNode = vtkDMMLColorNode::SafeDownCast(node);
  if (colorNode && column == 0)
    {
    if (this->updateGradientFromNode(colorNode) || item->icon().isNull())
      {
      qDMMLSceneColorTableModelPrivate::ColorGradient& colorGradient =
        d->GradientCache[colorNode->GetID()];
      //item->setBackground(colorGradient.Gradient);
      item->setIcon(colorGradient.Pixmap);
      }
    }
}

//------------------------------------------------------------------------------
bool qDMMLSceneColorTableModel::updateGradientFromNode(vtkDMMLColorNode* node)const
{
  Q_D(const qDMMLSceneColorTableModel);
  Q_ASSERT(node);
  /// TODO: Improve the cache of the pixmaps, right now, they are not shared
  /// between the different qDMMLSceneColorTableModels.
  bool cached = d->GradientCache.contains(node->GetID());
  qDMMLSceneColorTableModelPrivate::ColorGradient& colorGradient = d->GradientCache[node->GetID()];
  if (!node->GetScalarsToColors() ||
      (cached && colorGradient.MTime >= node->GetScalarsToColors()->GetMTime()))
    {
    // pixmap is already up-to-date
    return false;
    }
  /// HACK: The node UserDefined is currently garbage and makes the icon
  /// generation crash.
  if (QString(node->GetName()) == "UserDefined")
    {
    return false;
    }
  colorGradient.updatePixmap(node->GetScalarsToColors());
  return true;
}
