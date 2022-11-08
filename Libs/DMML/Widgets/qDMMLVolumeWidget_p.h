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

#ifndef __qDMMLVolumeWidget_p_h
#define __qDMMLVolumeWidget_p_h

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
#include "qDMMLVolumeWidget.h"

// VTK includes
#include <vtkWeakPointer.h>

class QMenu;
class qDMMLSpinBox;

// -----------------------------------------------------------------------------
class qDMMLVolumeWidgetPrivate : public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qDMMLVolumeWidget);
protected:
  qDMMLVolumeWidget* const q_ptr;

public:
  qDMMLVolumeWidgetPrivate(qDMMLVolumeWidget& object);
  ~qDMMLVolumeWidgetPrivate() override;

  virtual void init();

  /// Block all the signals emitted by the widgets that are impacted by the
  /// range.
  /// To be reimplemented in subclasses
  virtual bool blockSignals(bool block);
  /// Compute the ideal singleStep based on a range.
  /// Set the single step to the widgets such as sliders, spinboxes...
  void updateSingleStep(double min, double max);

public slots:
  virtual void setRange(double min, double max);
  virtual void setDecimals(int decimals);
  virtual void setSingleStep(double singleStep);
  virtual void updateRangeFromSpinBox();

protected:
  vtkWeakPointer<vtkDMMLScalarVolumeNode> VolumeNode;
  vtkWeakPointer<vtkDMMLScalarVolumeDisplayNode> VolumeDisplayNode;
  QMenu* OptionsMenu;
  qDMMLSpinBox* MinRangeSpinBox;
  qDMMLSpinBox* MaxRangeSpinBox;
  double DisplayScalarRange[2];
};

#endif
