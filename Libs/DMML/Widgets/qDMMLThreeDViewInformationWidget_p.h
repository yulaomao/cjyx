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

#ifndef __qDMMLThreeDViewInformationWidget_p_h
#define __qDMMLThreeDViewInformationWidget_p_h

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
#include "qDMMLThreeDViewInformationWidget.h"
#include "ui_qDMMLThreeDViewInformationWidget.h"

/// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class QAction;
class ctkVTKSliceView;
class vtkDMMLSliceNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qDMMLThreeDViewInformationWidgetPrivate: public QObject,
                                   public Ui_qDMMLThreeDViewInformationWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLThreeDViewInformationWidget);
protected:
  qDMMLThreeDViewInformationWidget* const q_ptr;
public:
  qDMMLThreeDViewInformationWidgetPrivate(qDMMLThreeDViewInformationWidget& object);
  ~qDMMLThreeDViewInformationWidgetPrivate() override;

  void setupUi(qDMMLWidget* widget);

public slots:
  /// Update widget state using the associated DMML node
  void updateWidgetFromDMMLViewNode();

public:
  vtkWeakPointer<vtkDMMLViewNode> DMMLViewNode;

};

#endif
