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

#ifndef __qDMMLLabelComboBox_h
#define __qDMMLLabelComboBox_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qDMML includes
#include "qDMMLWidgetsExport.h"
#include "qDMMLWidget.h"

class vtkDMMLNode;
class vtkDMMLColorNode;
class qDMMLLabelComboBoxPrivate;

class QDMML_WIDGETS_EXPORT qDMMLLabelComboBox : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(bool noneEnabled READ noneEnabled WRITE setNoneEnabled)
  Q_PROPERTY(int currentColor READ currentColor WRITE setCurrentColor NOTIFY currentColorChanged USER true)
  Q_PROPERTY(QString currentColorName READ currentColorName WRITE setCurrentColor NOTIFY currentColorChanged STORED false)
  Q_PROPERTY(int maximumColorCount READ maximumColorCount WRITE setMaximumColorCount)
  Q_PROPERTY(bool colorNameVisible READ colorNameVisible WRITE setColorNameVisible)
  Q_PROPERTY(bool labelValueVisible READ labelValueVisible WRITE setLabelValueVisible)

public:

  typedef qDMMLWidget Superclass;

  /// Construct an empty qDMMLColorTableComboBox with a null scene,
  /// no nodeType, where the hidden nodes are not forced on display.
  explicit qDMMLLabelComboBox(QWidget* newParent = nullptr);
  ~qDMMLLabelComboBox() override;

  /// Set/Get NoneEnabled flags
  /// An additional item is added into the menu list, where the user can select "None".
  bool noneEnabled()const;
  void setNoneEnabled(bool enable);

  ///Display or not the colors names
  bool colorNameVisible() const;
  void setColorNameVisible(bool visible);

  ///Display or not the label values
  bool labelValueVisible() const;
  void setLabelValueVisible(bool visible);

  virtual void printAdditionalInfo();

  vtkDMMLColorNode* dmmlColorNode()const;

  int currentColor()const;
  QString currentColorName()const;

  int maximumColorCount()const;
  void setMaximumColorCount(int maximum);

public slots:

  void setDMMLColorNode(vtkDMMLNode * newDMMLColorNode);

  void setCurrentColor(int index);
  void setCurrentColor(const QString& colorName);

  void updateWidgetFromDMML();

signals:

  void currentColorChanged(const QColor& color);
  void currentColorChanged(const QString& name);
  void currentColorChanged(int index);

private slots:

  void onCurrentIndexChanged(int index);

protected:
  QScopedPointer<qDMMLLabelComboBoxPrivate> d_ptr;
private:
  Q_DECLARE_PRIVATE(qDMMLLabelComboBox);
  Q_DISABLE_COPY(qDMMLLabelComboBox);
};

#endif
