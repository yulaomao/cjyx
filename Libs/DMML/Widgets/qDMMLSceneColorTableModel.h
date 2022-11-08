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

#ifndef __qDMMLSceneColorTableModel_h
#define __qDMMLSceneColorTableModel_h

// qDMML includes
#include "qDMMLSceneCategoryModel.h"

// DMML includes
class vtkDMMLColorNode;

class qDMMLSceneColorTableModelPrivate;

class QDMML_WIDGETS_EXPORT qDMMLSceneColorTableModel : public qDMMLSceneCategoryModel
{
  Q_OBJECT

public:
  qDMMLSceneColorTableModel(QObject *parent=nullptr);
  ~qDMMLSceneColorTableModel() override;

protected:
  QScopedPointer<qDMMLSceneColorTableModelPrivate> d_ptr;

  void updateItemFromNode(QStandardItem* item, vtkDMMLNode* node, int column) override;
  bool updateGradientFromNode(vtkDMMLColorNode* node)const;

private:
  Q_DECLARE_PRIVATE(qDMMLSceneColorTableModel);
  Q_DISABLE_COPY(qDMMLSceneColorTableModel);
};

#endif
