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

#ifndef __qDMMLCheckableNodeComboBox_h
#define __qDMMLCheckableNodeComboBox_h

// qDMML includes
#include "qDMMLNodeComboBox.h"

class qDMMLCheckableNodeComboBoxPrivate;

/// This class provides the ability to select(check) multiple nodes at a time
/// instead of one (qDMMLNodeComboBox). Nodes are left untouched, the selection
/// information only resides in the combobox.
/// Nodes are unchecked by default.
/// \sa qDMMLNodeComboBox
class QDMML_WIDGETS_EXPORT qDMMLCheckableNodeComboBox
  : public qDMMLNodeComboBox
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qDMMLNodeComboBox Superclass;

  /// Construct an empty qDMMLCheckableNodeComboBox with a null scene,
  /// no nodeType, where the hidden nodes are not forced on display.
  explicit qDMMLCheckableNodeComboBox(QWidget* parent = nullptr);
  ~qDMMLCheckableNodeComboBox() override;

  /// Return the list of checked nodes.
  /// \sa uncheckedNodes(), nodes()
  Q_INVOKABLE QList<vtkDMMLNode*> checkedNodes()const;

  /// Return the list of unchecked nodes.
  /// \sa checkedNodes(), nodes()
  Q_INVOKABLE QList<vtkDMMLNode*> uncheckedNodes()const;

  /// Return true if all the nodes are checked.
  /// If empty, return true;
  Q_INVOKABLE bool allChecked()const;

  /// Return true if all the nodes are unchecked.
  /// If empty, return true.
  Q_INVOKABLE bool noneChecked()const;

  /// Return the checkstate of the node.
  /// If \a node is invalid (null or not in the scene),
  /// Qt::Unchecked is returned.
  Q_INVOKABLE Qt::CheckState checkState(vtkDMMLNode* node)const;
  Q_INVOKABLE void setCheckState(vtkDMMLNode* node, Qt::CheckState check);

  /// Set the node item as user checkable or not.
  /// By default, the items are user checkable.
  Q_INVOKABLE void setUserCheckable(vtkDMMLNode* node, bool userCheckable);

public slots:
  /// Set the check state of the node to Qt::Checked.
  inline void check(vtkDMMLNode* node);

  /// Set the check state of the node to Qt::Unchecked.
  inline void uncheck(vtkDMMLNode* node);

signals:
  /// Signal fired anytime a node is checked or unchecked.
  void checkedNodesChanged();

private:
  Q_DECLARE_PRIVATE(qDMMLCheckableNodeComboBox);
  Q_DISABLE_COPY(qDMMLCheckableNodeComboBox);
};

//-----------------------------------------------------------------------------
void qDMMLCheckableNodeComboBox::check(vtkDMMLNode* node)
{
  this->setCheckState(node, Qt::Checked);
}

//-----------------------------------------------------------------------------
void qDMMLCheckableNodeComboBox::uncheck(vtkDMMLNode* node)
{
  this->setCheckState(node, Qt::Unchecked);
}

#endif
