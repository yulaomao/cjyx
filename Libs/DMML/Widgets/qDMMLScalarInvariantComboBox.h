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

#ifndef __qDMMLScalarInvariantComboBox_h
#define __qDMMLScalarInvariantComboBox_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLWidgetsExport.h"

class qDMMLScalarInvariantComboBoxPrivate;
class vtkDMMLNode;
class vtkDMMLDiffusionTensorDisplayPropertiesNode;

/// Combobox to select a function to compute a scalar from a diffusion tensor.
/// If an optional display properties node is provided, its ColorGlyphBy
/// property is automatically synchronized with the combobox entree.
class QDMML_WIDGETS_EXPORT qDMMLScalarInvariantComboBox : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(int scalarInvariant READ scalarInvariant WRITE setScalarInvariant)

public:
  qDMMLScalarInvariantComboBox(QWidget *parent=nullptr);
  ~qDMMLScalarInvariantComboBox() override;

  /// Return the optionally set display properties node set
  vtkDMMLDiffusionTensorDisplayPropertiesNode* displayPropertiesNode()const;

  /// Returns the currently selected scalar invariant
  int scalarInvariant()const;

public slots:
  /// Set the volume node properties to synchronize the combobox with.
  /// It listens to the ColorGlyphBy property (not ScalarInvariant). If the
  /// property is externally modified, the combobox is updated with the new
  /// value, on the other hand, if the user graphically select a new menu entry,
  /// the node will be updated with the user choice.
  void setDisplayPropertiesNode(vtkDMMLDiffusionTensorDisplayPropertiesNode *node);

  /// Utility function to be connected with generic signals
  void setDisplayPropertiesNode(vtkDMMLNode *node);

  /// Set the combobox entry to the new scalar invariant value. If a display
  /// properties node is set, it sets to the value to the node.
  void setScalarInvariant(int value);

signals:
  /// Fired anytime the scalar invariant is changed (by the user,
  /// programmatically or via the node)
  void scalarInvariantChanged(int scalarInvariant);

protected slots:
  void updateWidgetFromDMML();
  void onCurrentScalarInvariantChanged(int index);

protected:
  QScopedPointer<qDMMLScalarInvariantComboBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLScalarInvariantComboBox);
  Q_DISABLE_COPY(qDMMLScalarInvariantComboBox);
};

#endif
