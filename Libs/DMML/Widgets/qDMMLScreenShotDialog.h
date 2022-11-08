/*==============================================================================

  Program: 3D Cjyx

  Copyright (c) 2010 Kitware Inc.

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

#ifndef __qDMMLScreenShotDialog_h
#define __qDMMLScreenShotDialog_h

// Qt includes
#include <QDialog>
#include <QMetaType>
#include <QString>

// qDMMLWidget includes
#include "qDMMLWidgetsExport.h"
#include "qDMMLLayoutManager.h"

class vtkImageData;

class qDMMLScreenShotDialogPrivate;

class QDMML_WIDGETS_EXPORT qDMMLScreenShotDialog : public QDialog
{
  Q_OBJECT
  Q_ENUMS(WidgetType)
  Q_PROPERTY(WidgetType widgetType READ widgetType WRITE setWidgetType)
  Q_PROPERTY(QString nameEdit READ nameEdit WRITE setNameEdit)
  Q_PROPERTY(double scaleFactor READ scaleFactor WRITE setScaleFactor)
  Q_PROPERTY(bool showScaleFactorSpinBox READ showScaleFactorSpinBox WRITE setShowScaleFactorSpinBox)
public:
  typedef QDialog Superclass;

  enum WidgetType{
    ThreeD = 0,
    Red = 1,
    Yellow = 2,
    Green = 3,
    FullLayout = 4
    };

  qDMMLScreenShotDialog(QWidget *parent = nullptr);
  ~qDMMLScreenShotDialog() override;

  /// Set/Get layout manager
  Q_INVOKABLE void setLayoutManager(qDMMLLayoutManager* newlayoutManager);
  Q_INVOKABLE qDMMLLayoutManager* layoutManager()const;

  void setNameEdit(const QString& newName);
  QString nameEdit()const;

  void setDescription(const QString& description);
  QString description()const;

  /// Setting the data prevent the dialog from automatically taking a screenshot
  /// each time the widgettype or scaleFactor is changed.
  void setData(const QVariant& newData);
  QVariant data()const;

  void setWidgetType(WidgetType newType);
  WidgetType widgetType()const;

  void setScaleFactor(const double& newScaleFactor);
  double scaleFactor()const;

  void setShowScaleFactorSpinBox(const bool& state);
  bool showScaleFactorSpinBox()const;

  /// set/return the image data of the screenshot
  void setImageData(vtkImageData* screenshot);
  vtkImageData* imageData()const;

protected slots:

  /// Grab a screenshot of the 3DView or any sliceView.
  /// The screenshotWindow is Red, Green, Yellow for a sliceView or empty for
  /// a ThreeDView
  void grabScreenShot(int screenshotWindow);
  void grabScreenShot();
  void setLastWidgetType(int id);
  /// Reset the dialog
  void resetDialog();

  /// Present save as dialog to directly save image
  void saveAs();


private:
  QString enumToString(int type);

protected:
  QScopedPointer<qDMMLScreenShotDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLScreenShotDialog);
  Q_DISABLE_COPY(qDMMLScreenShotDialog);
};

Q_DECLARE_METATYPE(qDMMLScreenShotDialog::WidgetType)

#endif
