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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qDMMLSliderWidget_h
#define __qDMMLSliderWidget_h

// CTK includes
#include <ctkSliderWidget.h>
#include <ctkVTKObject.h>

// qDMML includes
#include <qDMMLWidgetsExport.h>

// VTK includes
class vtkDMMLNode;
class vtkDMMLScene;

// qDMMLSliderWidget includes
class qDMMLSliderWidgetPrivate;

/// \class qDMMLSliderWidget
/// \brief Extend the ctkSliderWidget to integrate units support
///
/// This custom widgets extends the ctkSliderWidget widget to integrate the
/// unit support within Cjyx. By default, this widget behaves just like
/// a normal ctkSliderWidget.
///
/// To use the units, one needs to set what kind of quantity this widget should
/// look for. For example, when dealing with world positions, the quantity is
/// probably going to be "length". Once a scene is set to this widget,
/// it listens to the changes made upon the selection node to extract the
/// unit properties related to its quantity and update accordingly.
///
/// To allow even more customisation, one can set which properties of the
/// widget are updated by units and which aren't.
///
/// \sa qDMMLCoordinatesWidget, qDMMLSpinBox
class QDMML_WIDGETS_EXPORT qDMMLSliderWidget : public ctkSliderWidget
{
  Q_OBJECT
  QVTK_OBJECT

  /// Get/Set the quantity is used to determine what unit the spinbox is in.
  /// This determines the spinbox properties like minimum, maximum,
  /// single step, prefix and suffix.
  // \sa setQuantity(), quantity()
  Q_PROPERTY(QString quantity READ quantity WRITE setQuantity)

  /// Get/Set the properties that will be determined by units.
  /// If a property is aware of units, it will update itself to the unit's
  /// property value automatically. Otherwise, this property is left to be
  /// changed by its accessors. All flags except MinimumValue and MaximumValue
  /// are on by default.
  /// \sa setQuantity(), quantity()
  // \sa setUnitAwareProperties(), unitAwareProperties()
  Q_FLAGS(UnitAwareProperty UnitAwareProperties)
  Q_PROPERTY(UnitAwareProperties unitAwareProperties READ unitAwareProperties WRITE setUnitAwareProperties)

public:
  typedef ctkSliderWidget Superclass;

  /// Construct an empty qDMMLSliderWidget with a null scene.
  explicit qDMMLSliderWidget(QWidget* parent = nullptr);
  ~qDMMLSliderWidget() override;

  enum UnitAwareProperty
    {
    None = 0x00,
    Prefix = 0x01,
    Suffix = 0x02,
    Precision = 0x04,
    MinimumValue = 0x08,
    MaximumValue = 0x10,
    Scaling = 0x20,
    All = Prefix | Suffix | Precision | MinimumValue | MaximumValue | Scaling
    };
  Q_DECLARE_FLAGS(UnitAwareProperties, UnitAwareProperty)

  /// Get DMML scene that has been set by setDMMLScene(). Default is no scene.
  /// \sa setDMMLScene()
  Q_INVOKABLE vtkDMMLScene* dmmlScene()const;

  /// Get the quantity property value.
  /// \sa quantity
  QString quantity()const;

  /// Get the unitAwareProperties property value.
  /// \sa unitAwareProperties
  UnitAwareProperties unitAwareProperties()const;

  /// Reimplemented for internal reasons.
  /// \sa ctkSliderWidget::setMinimum(), ctkSliderWidget::setMaximum(),
  /// ctkSliderWidget::setRange()
  void setMinimum(double) override;
  void setMaximum(double) override;
  void setRange(double, double) override;

public slots:
  void setQuantity(const QString& baseName);

  /// Set the scene the spinbox listens to.
  /// \sa dmmlScene()
  virtual void setDMMLScene(vtkDMMLScene* scene);

  void setUnitAwareProperties(UnitAwareProperties flags);

protected slots:
  void updateWidgetFromUnitNode();

protected:
  QScopedPointer<qDMMLSliderWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSliderWidget);
  Q_DISABLE_COPY(qDMMLSliderWidget);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(qDMMLSliderWidget::UnitAwareProperties)

#endif
