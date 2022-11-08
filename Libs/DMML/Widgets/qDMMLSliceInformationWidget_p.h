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

#ifndef __qDMMLSliceInformationWidget_p_h
#define __qDMMLSliceInformationWidget_p_h

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

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLSliceInformationWidget.h"
#include "ui_qDMMLSliceInformationWidget.h"

// DMMLLogic includes
#include <vtkDMMLSliceLogic.h>

/// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkImageData.h>

class QAction;
class ctkVTKSliceView;
class vtkDMMLSliceNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qDMMLSliceInformationWidgetPrivate: public QObject,
                                   public Ui_qDMMLSliceInformationWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLSliceInformationWidget);
protected:
  qDMMLSliceInformationWidget* const q_ptr;
public:
  qDMMLSliceInformationWidgetPrivate(qDMMLSliceInformationWidget& object);
  ~qDMMLSliceInformationWidgetPrivate() override;

  void setupUi(qDMMLWidget* widget);

public slots:
  /// Update widget state using the associated DMML slice node
  void updateWidgetFromDMMLSliceNode();



public:
  vtkWeakPointer<vtkDMMLSliceNode>    DMMLSliceNode;
  QButtonGroup*                       SliceSpacingModeGroup;

};

#endif
