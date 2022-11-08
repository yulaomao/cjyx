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

#ifndef __qDMMLRangeWidget_h
#define __qDMMLRangeWidget_h

// CTK includes
#include <ctkDoubleRangeSlider.h>
#include <ctkRangeSlider.h>
#include <ctkRangeWidget.h>

// qDMML includes
#include "qDMMLWidgetsExport.h"

class qDMMLRangeSliderPrivate;
class qDMMLSpinBox;
class vtkDMMLScene;

/// qDMMLRangeWidget is a wrapper around a ctkRangeWidget
/// It adds QSpinBoxes (in a menu) for controlling the range of the values
/// and supports for the units.
class QDMML_WIDGETS_EXPORT qDMMLRangeWidget : public ctkRangeWidget
{
  Q_OBJECT
  Q_PROPERTY(QPalette minimumHandlePalette READ minimumHandlePalette WRITE setMinimumHandlePalette)
  Q_PROPERTY(QPalette maximumHandlePalette READ maximumHandlePalette WRITE setMaximumHandlePalette)
  Q_PROPERTY(vtkDMMLScene* dmmlScene READ dmmlScene WRITE setDMMLScene)
  Q_PROPERTY(QString quantity READ quantity WRITE setQuantity)

public:
  /// Constructor
  /// If \li parent is null, qDMMLRangeWidget will be a top-level widget
  /// \note The \li parent can be set later using QWidget::setParent()
  explicit qDMMLRangeWidget(QWidget* parent = nullptr);

  QPalette minimumHandlePalette()const;
  QPalette maximumHandlePalette()const;

  vtkDMMLScene* dmmlScene()const;
  QString quantity()const;

  /// When symmetricMoves is true, moving a handle will move the other handle
  /// symmetrically, otherwise the handles are independent. False by default.
  void setSymmetricMoves(bool symmetry) override;

public slots:
  /// Set the palette of the minimum handle
  void setMinimumHandlePalette(const QPalette& palette);

  /// Set the palette of the minimum handle
  void setMaximumHandlePalette(const QPalette& palette);

  /// Set the quantity the widget should look for.
  /// \sa quantity()
  void setQuantity(const QString& baseName);

  /// Set the scene the spinboxes listens to.
  /// \sa dmmlScene()
  virtual void setDMMLScene(vtkDMMLScene* scene);

protected slots:
  void updateSpinBoxRange(double min, double max);
  void updateRange();
  void updateSymmetricMoves(bool symmetric);

protected:
  qDMMLSpinBox* MinSpinBox;
  qDMMLSpinBox* MaxSpinBox;
  QAction* SymmetricAction;
};

/// qDMMLDoubleRangeSlider is a wrapper around a ctkDoubleRangeSlider
class QDMML_WIDGETS_EXPORT qDMMLDoubleRangeSlider : public ctkDoubleRangeSlider
{
  Q_OBJECT;
public:
  qDMMLDoubleRangeSlider(QWidget* parentWidget);
  QPalette minimumHandlePalette()const;
  QPalette maximumHandlePalette()const;

  vtkDMMLScene* dmmlScene()const;
  QString quantity()const;

public slots:
  /// Set the palette of the minimum handle
  void setMinimumHandlePalette(const QPalette& palette);

  /// Set the palette of the minimum handle
  void setMaximumHandlePalette(const QPalette& palette);
};

/// qDMMLRangeSlider is a wrapper around a ctkRangeSlider
class QDMML_WIDGETS_EXPORT qDMMLRangeSlider : public ctkRangeSlider
{
  Q_OBJECT;
public:
  qDMMLRangeSlider(QWidget* parentWidget);
  ~qDMMLRangeSlider() override;
  QPalette minimumHandlePalette()const;
  QPalette maximumHandlePalette()const;

public slots:
  /// Set the palette of the minimum handle
  void setMinimumHandlePalette(const QPalette& palette);

  /// Set the palette of the minimum handle
  void setMaximumHandlePalette(const QPalette& palette);
protected:
  void initMinimumSliderStyleOption(QStyleOptionSlider* option) const override;
  void initMaximumSliderStyleOption(QStyleOptionSlider* option) const override;

protected:
  QScopedPointer<qDMMLRangeSliderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLRangeSlider);
  Q_DISABLE_COPY(qDMMLRangeSlider);
};

#endif
