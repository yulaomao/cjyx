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

#ifndef __qDMMLColorTableComboBox_h
#define __qDMMLColorTableComboBox_h

// CTK includes
#include <ctkPimpl.h>

// qDMML includes
#include "qDMMLNodeComboBox.h"
#include "qDMMLWidgetsExport.h"

class qDMMLColorTableComboBoxPrivate;

class QDMML_WIDGETS_EXPORT qDMMLColorTableComboBox : public qDMMLNodeComboBox
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qDMMLNodeComboBox Superclass;

  /// Construct an empty qDMMLColorTableComboBox with a null scene,
  /// no nodeType, where the hidden nodes are not forced on display.
  explicit qDMMLColorTableComboBox(QWidget* parent = nullptr);
  ~qDMMLColorTableComboBox() override;
  void setDMMLScene(vtkDMMLScene* scene) override;

protected:
  QAbstractItemModel* createSceneModel();

protected:
  QScopedPointer<qDMMLColorTableComboBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLColorTableComboBox);
  Q_DISABLE_COPY(qDMMLColorTableComboBox);
};

#endif
