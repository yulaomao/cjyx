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

#ifndef __qDMMLColorPickerWidget_h
#define __qDMMLColorPickerWidget_h

// Qt include
#include <QModelIndex>

// qDMML includes
#include "qDMMLWidget.h"
#include "qDMMLWidgetsExport.h"

// CTK includes
#include <ctkVTKObject.h>

class qDMMLColorPickerWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLColorNode;
class vtkDMMLColorLogic;

/// Given a dmml scene, qDMMLColorPickerWidget allows the selection of
/// a color/label from all the vtkDMMLColorNode in the scene.
class QDMML_WIDGETS_EXPORT qDMMLColorPickerWidget : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  qDMMLColorPickerWidget(QWidget *parent=nullptr);
  ~qDMMLColorPickerWidget() override;

  /// A color logic is needed to select the color default nodes.
  /// A default color logic is created.
  Q_INVOKABLE void setDMMLColorLogic(vtkDMMLColorLogic* colorLogic);
  Q_INVOKABLE vtkDMMLColorLogic* dmmlColorLogic()const;

  Q_INVOKABLE vtkDMMLColorNode* currentColorNode()const;

  void setDMMLScene(vtkDMMLScene* scene) override;
  bool eventFilter(QObject* target, QEvent* event) override;

public slots:
  void setCurrentColorNode(vtkDMMLNode* node);
  void setCurrentColorNodeToDefault();

signals:
  /// Fired wen the current color table node is selected
  void currentColorNodeChanged(vtkDMMLNode* node);

  /// Fired when the user selects a color in the list. index is the selected
  /// color node entry.
  void colorEntrySelected(int index);

  /// Fired when the user selects a color in the list
  void colorSelected(const QColor& color);

  /// Fired when the user selects a color in the list.
  /// name is the name of the color node entry.
  void colorNameSelected(const QString& name);

protected slots:
  void onNodeAdded(vtkObject*, vtkObject*);
  void onCurrentColorNodeChanged(vtkDMMLNode* node);
  void onTextChanged(const QString& colorText);

protected:
  QScopedPointer<qDMMLColorPickerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLColorPickerWidget);
  Q_DISABLE_COPY(qDMMLColorPickerWidget);
};

#endif
