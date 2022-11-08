
/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright 2020 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qDMMLScalarsDisplayWidget_h
#define __qDMMLScalarsDisplayWidget_h

// DMMLWidgets includes
#include "qDMMLWidget.h"

// DMML includes
#include "vtkDMMLDisplayNode.h"

// CTK includes
#include <ctkVTKObject.h>

class qDMMLScalarsDisplayWidgetPrivate;
class vtkDMMLColorNode;
class vtkDMMLNode;

class QDMML_WIDGETS_EXPORT qDMMLScalarsDisplayWidget : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

  Q_PROPERTY(vtkDMMLDisplayNode::ScalarRangeFlagType scalarRangeMode READ scalarRangeMode WRITE setScalarRangeMode)

public:
  /// Constructors
  typedef qDMMLWidget Superclass;
  explicit qDMMLScalarsDisplayWidget(QWidget* parentWidget = nullptr);
  ~qDMMLScalarsDisplayWidget() override;

  /// Get the (first) current display node
  vtkDMMLDisplayNode* dmmlDisplayNode()const;
  /// Get current display nodes. This is for supporting changing properties
  /// in case of multiple nodes are selected (e.g., in subject hierarchy folders).
  QList<vtkDMMLDisplayNode*> dmmlDisplayNodes()const;

  bool scalarsVisibility()const;
  QString activeScalarName()const;
  vtkDMMLColorNode* scalarsColorNode()const;

  /// Set scalar range mode
  void setScalarRangeMode(vtkDMMLDisplayNode::ScalarRangeFlagType mode);
  vtkDMMLDisplayNode::ScalarRangeFlagType scalarRangeMode() const;

  /// Get minimum of the scalar display range
  double minimumValue()const;

  /// Get maximum of the scalar display range
  double maximumValue()const;

signals:
  /// Signal sent if the auto/manual value is updated
  void scalarRangeModeValueChanged(vtkDMMLDisplayNode::ScalarRangeFlagType mode);
  /// Signal sent if the any property in the display node is changed
  void displayNodeChanged();

public slots:
  /// Set the one display node
  void setDMMLDisplayNode(vtkDMMLDisplayNode* node);
  /// Utility function to be connected with generic signals
  void setDMMLDisplayNode(vtkDMMLNode* node);
  /// Set the current display nodes if more are managed.
  /// In case of multi-selection, the first item's display properties are
  /// displayed in the widget, but the changed settings are applied on all
  /// selected items if applicable.
  void setDMMLDisplayNodes(QList<vtkDMMLDisplayNode*> displayNodes);

  void setScalarsVisibility(bool);
  void setActiveScalarName(const QString&);
  void setScalarsColorNode(vtkDMMLNode*);
  void setScalarsColorNode(vtkDMMLColorNode*);
  void setScalarsDisplayRange(double min, double max);
  void setTresholdEnabled(bool b);
  void setThresholdRange(double min, double max);

  /// Set Auto/Manual mode
  void setScalarRangeMode(int scalarRangeMode);

  /// Set min/max of scalar range
  void setMinimumValue(double min);
  void setMaximumValue(double max);

protected slots:
  /// Update the widget from volume display node properties
  void updateWidgetFromDMML();

  /// Set active scalar when the user changes selection
  void onCurrentArrayActivated();

protected:
  QScopedPointer<qDMMLScalarsDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLScalarsDisplayWidget);
  Q_DISABLE_COPY(qDMMLScalarsDisplayWidget);
  friend class qDMMLModelDisplayNodeWidget;
  friend class qDMMLMarkupsDisplayNodeWidget;
};

#endif
