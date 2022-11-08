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

#ifndef __qDMMLDisplayNodeViewComboBox_h
#define __qDMMLDisplayNodeViewComboBox_h

// CTK includes
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLCheckableNodeComboBox.h"
class qDMMLDisplayNodeViewComboBoxPrivate;

// VTK includes
class vtkDMMLDisplayNode;
class vtkDMMLAbstractViewNode;

/// \brief Combobox of display node view nodes.
/// Observe the view nodes of a display node and mark them as checked in the
/// scene view node list. View nodes can be vtkDMMLViewNode for the 3D view
/// or vtkDMMLSliceNode for the 2d case, or vtkDMMLPlotChartViewNodes for charts
/// \sa vtkDMMLDisplayNode::GetNthViewNodeID()
class QDMML_WIDGETS_EXPORT qDMMLDisplayNodeViewComboBox
  : public qDMMLCheckableNodeComboBox
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Superclass typedef
  typedef qDMMLCheckableNodeComboBox Superclass;

  /// Construct an empty qDMMLDisplayNodeViewComboBox with a null scene,
  /// no nodeType, where the hidden nodes are not forced on display.
  explicit qDMMLDisplayNodeViewComboBox(QWidget* parent = nullptr);
  ~qDMMLDisplayNodeViewComboBox() override;

  vtkDMMLDisplayNode* dmmlDisplayNode()const;

  /// Return a list of view nodes the display node is visible into.
  QList<vtkDMMLAbstractViewNode*> checkedViewNodes()const;
  /// Return a list of view nodes the display node is not visible into.
  QList<vtkDMMLAbstractViewNode*> uncheckedViewNodes()const;

public slots:
  /// Set the display node to observe.
  void setDMMLDisplayNode(vtkDMMLDisplayNode* node);
  /// Utility function to conveniently connect the combobox with other
  /// qDMMLWidgets.
  void setDMMLDisplayNode(vtkDMMLNode* displayNode);

protected slots:
  void updateWidgetFromDMML();
  void updateDMMLFromWidget();

protected:
  QScopedPointer<qDMMLDisplayNodeViewComboBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLDisplayNodeViewComboBox);
  Q_DISABLE_COPY(qDMMLDisplayNodeViewComboBox);
};

#endif
