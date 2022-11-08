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

#ifndef __qDMMLSliceWidget_p_h
#define __qDMMLSliceWidget_p_h

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
#include "qDMMLSliceWidget.h"
#include "ui_qDMMLSliceWidget.h"

// VTK include
#include <vtkSmartPointer.h>

class vtkDMMLDisplayableManagerGroup;
class QResizeEvent;

//-----------------------------------------------------------------------------
class QDMML_WIDGETS_EXPORT qDMMLSliceWidgetPrivate
  : public QObject
  , public Ui_qDMMLSliceWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qDMMLSliceWidget);
protected:
  qDMMLSliceWidget* const q_ptr;
public:
  qDMMLSliceWidgetPrivate(qDMMLSliceWidget& object);
  ~qDMMLSliceWidgetPrivate() override;

  void init();

public slots:
  void setSliceViewSize(const QSize& size);
  void resetSliceViewSize();
  void endProcessing();
  /// Set the image data to the slice view
  void setImageDataConnection(vtkAlgorithmOutput * imageDataConnection);


};

#endif
