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

#ifndef __qDMMLSpinBox_h
#define __qDMMLSpinBox_h

// CTK includes
#include <ctkDoubleSpinBox.h>
#include <ctkVTKObject.h>

// qDMML includes
#include <qDMMLWidgetsExport.h>

// VTK includes
class vtkDMMLNode;
class vtkDMMLScene;

// qDMMLSpinBox includes
class qDMMLSpinBoxPrivate;

/// \class qDMMLSpinBox
/// \brief Extend the ctkDoubleSpinBox to integrate units support
///
/// This custom widgets extends the ctkDoubleSpinBox widget to integrate the
/// unit support within Cjyx. By default, this widget behaves just like
/// a normal ctkDoubleSpinBox.
///
/// To use the units, one needs to set what kind of quantity this widget should
/// look for. For example, when dealing with world positions, the quantity is
/// probably going to be "length". Once a scene is set to this widget,
/// it listens to the changes made upon the selection node to extract the
/// unit properties related to its quantity and update accordingly.
///
/// To allow even more customisation, one can set which properties of the
/// spinbox are updated by units and which aren't.
///
/// \sa qDMMLSliderWidget, qDMMLCoordinatesWidget
class QDMML_WIDGETS_EXPORT qDMMLSpinBox : public ctkDoubleSpinBox
{
  Q_OBJECT
  QVTK_OBJECT

  /// Get/Set the quantity is used to determine what unit the spinbox is in.
  /// This determines the spinbox properties like minimum, maximum,
  /// single step, prefix and suffix.
  // \sa setQuantity(), quantity()
  // \sa setUnitAwareProperties(), unitAwareProperties()
  Q_PROPERTY(QString quantity READ quantity WRITE setQuantity)

  /// Get/Set the properties that will be determined by units.
  /// If a property is aware of units, it will update itself to the unit's
  /// property value automatically. Otherwise, this property is left to be
  /// changed by its accessors. All flags are on by default.
  /// \sa setQuantity(), quantity()
  // \sa setUnitAwareProperties(), unitAwareProperties()
  Q_FLAGS(UnitAwareProperty UnitAwareProperties)
  Q_PROPERTY(UnitAwareProperties unitAwareProperties READ unitAwareProperties WRITE setUnitAwareProperties)

public:
  typedef ctkDoubleSpinBox Superclass;

  /// Construct an empty qDMMLSpinBox with a null scene.
  explicit qDMMLSpinBox(QWidget* parent = nullptr);
  ~qDMMLSpinBox() override;

  enum UnitAwareProperty
    {
    None = 0x00,
    Prefix = 0x01,
    Suffix = 0x02,
    Precision = 0x04,
    MinimumValue = 0x08,
    MaximumValue = 0x10,
    Scaling = 0x20,
    };
  Q_DECLARE_FLAGS(UnitAwareProperties, UnitAwareProperty)

  /// Get DMML scene that has been set by setDMMLScene(). Default is no scene.
  /// \sa setDMMLScene()
  Q_INVOKABLE vtkDMMLScene* dmmlScene()const;

  QString quantity()const;

  UnitAwareProperties unitAwareProperties()const;

public slots:
  void setQuantity(const QString& baseName);

  /// Set the scene the spinbox listens to.
  /// \sa dmmlScene()
  virtual void setDMMLScene(vtkDMMLScene* scene);

  void setUnitAwareProperties(UnitAwareProperties flags);

protected slots:
  void updateWidgetFromUnitNode();

protected:
  QScopedPointer<qDMMLSpinBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLSpinBox);
  Q_DISABLE_COPY(qDMMLSpinBox);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(qDMMLSpinBox::UnitAwareProperties)

#endif
