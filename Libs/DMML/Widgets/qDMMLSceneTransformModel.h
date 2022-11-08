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

#ifndef __qDMMLSceneTransformModel_h
#define __qDMMLSceneTransformModel_h

#include "qDMMLSceneModel.h"

class qDMMLSceneTransformModelPrivate;

class QDMML_WIDGETS_EXPORT qDMMLSceneTransformModel : public qDMMLSceneModel
{
  Q_OBJECT

public:
  qDMMLSceneTransformModel(QObject *parent=nullptr);
  ~qDMMLSceneTransformModel() override;

  ///
  vtkDMMLNode* parentNode(vtkDMMLNode* node)const override;
  //virtual int          nodeIndex(vtkDMMLNode* node)const;
  /// fast function that only check the type of the node to know if it can be a child.
  bool         canBeAChild(vtkDMMLNode* node)const override;
  /// fast function that only check the type of the node to know if it can be a parent.
  bool         canBeAParent(vtkDMMLNode* node)const override;
  /// if newParent == 0, set the node into the vtkDMMLScene
  bool         reparent(vtkDMMLNode* node, vtkDMMLNode* newParent) override;


  Qt::DropActions supportedDropActions()const override;

private:
  Q_DECLARE_PRIVATE(qDMMLSceneTransformModel);
  Q_DISABLE_COPY(qDMMLSceneTransformModel);
};

#endif
