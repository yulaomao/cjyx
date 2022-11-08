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

#ifndef __qDMMLSceneDisplayableModel_p_h
#define __qDMMLSceneDisplayableModel_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Cjyx API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// qDMML includes
#include "qDMMLSceneDisplayableModel.h"
#include "qDMMLSceneHierarchyModel_p.h"

// DMML includes
class vtkDMMLDisplayNode;
class vtkDMMLHierarchyNode;

//------------------------------------------------------------------------------
// qDMMLSceneDisplayableModelPrivate
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
class QDMML_WIDGETS_EXPORT qDMMLSceneDisplayableModelPrivate
  : public qDMMLSceneHierarchyModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qDMMLSceneDisplayableModel);
public:
  typedef qDMMLSceneHierarchyModelPrivate Superclass;
  qDMMLSceneDisplayableModelPrivate(qDMMLSceneDisplayableModel& object);
  void init() override;

  vtkDMMLHierarchyNode* CreateHierarchyNode()const override;
  vtkDMMLDisplayNode* displayNode(vtkDMMLNode* node)const;

  int ColorColumn;
  int OpacityColumn;
};

#endif
