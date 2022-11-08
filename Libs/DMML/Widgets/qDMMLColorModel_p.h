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

#ifndef __qDMMLColorModel_p_h
#define __qDMMLColorModel_p_h

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

// Qt includes
class QStandardItemModel;

// qDMML includes
#include "qDMMLColorModel.h"

// DMML includes
class vtkDMMLColorNode;

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
// qDMMLColorModelPrivate
//------------------------------------------------------------------------------
class QDMML_WIDGETS_EXPORT qDMMLColorModelPrivate
{
  Q_DECLARE_PUBLIC(qDMMLColorModel);
protected:
  qDMMLColorModel* const q_ptr;
public:
  qDMMLColorModelPrivate(qDMMLColorModel& object);
  virtual ~qDMMLColorModelPrivate();
  void init();

  void updateColumnCount();
  virtual int maxColumnId()const;

  vtkSmartPointer<vtkDMMLColorLogic>  ColorLogic;
  vtkSmartPointer<vtkCallbackCommand> CallBack;
  vtkSmartPointer<vtkDMMLColorNode>   DMMLColorNode;

  bool NoneEnabled;
  int ColorColumn;
  int LabelColumn;
  int OpacityColumn;
  int CheckableColumn;

  /// This flag allows to make sure that during updating widget from DMML,
  /// GUI updates will not trigger DMML node updates.
  bool IsUpdatingWidgetFromDMML;
};

#endif
