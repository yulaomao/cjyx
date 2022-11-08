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
// Qt includes
#include <QButtonGroup>
#include <QFileDialog>
#include <QMetaProperty>
#include <QPointer>

// CTK includes
#include <ctkVTKWidgetsUtils.h>

// qDMML includes
#include "qDMMLLayoutManager.h"
#include <qDMMLSliceView.h>
#include "qDMMLSliceWidget.h"
#include "qDMMLScreenShotDialog.h"
#include "qDMMLThreeDView.h"
#include "qDMMLThreeDWidget.h"
#include "qDMMLUtils.h"

#include "ui_qDMMLScreenShotDialog.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkRendererCollection.h>
#include <vtkRenderLargeImage.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>

//-----------------------------------------------------------------------------
class qDMMLScreenShotDialogPrivate : public Ui_qDMMLScreenShotDialog
{
  Q_DECLARE_PUBLIC(qDMMLScreenShotDialog);
protected:
  qDMMLScreenShotDialog* const q_ptr;
public:
  qDMMLScreenShotDialogPrivate(qDMMLScreenShotDialog& object);

  void setupUi(QDialog* dialog);
  void setCheckedRadioButton(int type);
  void setWidgetEnabled(bool state);
  QPointer<qDMMLLayoutManager>   LayoutManager;
  vtkSmartPointer<vtkImageData>      ImageData;
  /// The ID of the associated snapshot node.
  /// This is nullptr if the dialog has no associated snapshot node (== new snapshot mode).
  QVariant                           Data;
  QButtonGroup*                      WidgetTypeGroup;

  /// The last selected thumbnail type
  int LastWidgetType;

};

//-----------------------------------------------------------------------------
qDMMLScreenShotDialogPrivate::qDMMLScreenShotDialogPrivate(qDMMLScreenShotDialog &object)
  : q_ptr(&object)
{
  qRegisterMetaType<qDMMLScreenShotDialog::WidgetType>(
      "qDMMLScreenShotDialog::WidgetType");
  this->WidgetTypeGroup = nullptr;

  this->LastWidgetType = qDMMLScreenShotDialog::FullLayout;
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialogPrivate::setupUi(QDialog* dialog)
{
  Q_Q(qDMMLScreenShotDialog);

  this->Ui_qDMMLScreenShotDialog::setupUi(dialog);
  this->WidgetTypeGroup = new QButtonGroup(dialog);
  this->WidgetTypeGroup->setExclusive(true);
  this->WidgetTypeGroup->addButton(this->fullLayoutRadio, qDMMLScreenShotDialog::FullLayout);
  this->WidgetTypeGroup->addButton(this->threeDViewRadio, qDMMLScreenShotDialog::ThreeD);
  this->WidgetTypeGroup->addButton(this->redSliceViewRadio, qDMMLScreenShotDialog::Red);
  this->WidgetTypeGroup->addButton(this->yellowSliceViewRadio, qDMMLScreenShotDialog::Yellow);
  this->WidgetTypeGroup->addButton(this->greenSliceViewRadio, qDMMLScreenShotDialog::Green);

  this->setCheckedRadioButton(this->LastWidgetType);

  QObject::connect(this->saveAsButton, SIGNAL(clicked()),
                   q, SLOT(saveAs()));
  QObject::connect(this->WidgetTypeGroup, SIGNAL(buttonClicked(int)),
                   q, SLOT(setLastWidgetType(int)));
}


//-----------------------------------------------------------------------------
void qDMMLScreenShotDialogPrivate::setCheckedRadioButton(int type)
{
  QRadioButton* widgetButton =
    qobject_cast<QRadioButton*>(this->WidgetTypeGroup->button(type));
  if (widgetButton)
    {
    // this can crash if an invalid type is passed in
    widgetButton->setChecked(true);
    }
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialogPrivate::setWidgetEnabled(bool state)
{
  this->threeDViewRadio->setEnabled(state);
  this->redSliceViewRadio->setEnabled(state);
  this->yellowSliceViewRadio->setEnabled(state);
  this->greenSliceViewRadio->setEnabled(state);
  this->fullLayoutRadio->setEnabled(state);
  this->scaleFactorSpinBox->setEnabled(state);
}

//-----------------------------------------------------------------------------
// qDMMLScreenShotDialog methods

//-----------------------------------------------------------------------------
qDMMLScreenShotDialog::qDMMLScreenShotDialog(QWidget * _parent)
  : Superclass(_parent)
  , d_ptr(new qDMMLScreenShotDialogPrivate(*this))
{
  Q_D(qDMMLScreenShotDialog);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qDMMLScreenShotDialog::~qDMMLScreenShotDialog() = default;

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::setLayoutManager(qDMMLLayoutManager* newlayoutManager)
{
  Q_D(qDMMLScreenShotDialog);
  d->LayoutManager = newlayoutManager;
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::setLastWidgetType(int id)
{
  Q_D(qDMMLScreenShotDialog);
  d->LastWidgetType = id;
}

//-----------------------------------------------------------------------------
qDMMLLayoutManager* qDMMLScreenShotDialog::layoutManager() const
{
  Q_D(const qDMMLScreenShotDialog);
  return d->LayoutManager.data();
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::setNameEdit(const QString& newName)
{
  Q_D(qDMMLScreenShotDialog);
  d->nameEdit->setText(newName);
  d->nameEdit->setFocus();
  d->nameEdit->selectAll();
}

//-----------------------------------------------------------------------------
QString qDMMLScreenShotDialog::nameEdit() const
{
  Q_D(const qDMMLScreenShotDialog);
  return d->nameEdit->text();
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::setDescription(const QString& newDescription)
{
  Q_D(qDMMLScreenShotDialog);
  d->descriptionTextEdit->setPlainText(newDescription);
}

//-----------------------------------------------------------------------------
QString qDMMLScreenShotDialog::description() const
{
  Q_D(const qDMMLScreenShotDialog);
  return d->descriptionTextEdit->toPlainText();
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::setData(const QVariant& newData)
{
  Q_D(qDMMLScreenShotDialog);
  d->Data = newData;
  d->setWidgetEnabled(!d->Data.isValid());
}

//-----------------------------------------------------------------------------
QVariant qDMMLScreenShotDialog::data() const
{
  Q_D(const qDMMLScreenShotDialog);
  return d->Data;
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::setWidgetType(qDMMLScreenShotDialog::WidgetType newType)
{
  Q_D(qDMMLScreenShotDialog);
  d->setCheckedRadioButton(newType);
}

//-----------------------------------------------------------------------------
qDMMLScreenShotDialog::WidgetType qDMMLScreenShotDialog::widgetType() const
{
  Q_D(const qDMMLScreenShotDialog);
  return qDMMLScreenShotDialog::WidgetType(d->WidgetTypeGroup->checkedId());
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::setScaleFactor(const double& newScaleFactor)
{
  Q_D(qDMMLScreenShotDialog);
  d->scaleFactorSpinBox->setValue(newScaleFactor);
}

//-----------------------------------------------------------------------------
double qDMMLScreenShotDialog::scaleFactor() const
{
  Q_D(const qDMMLScreenShotDialog);
  return d->scaleFactorSpinBox->value();
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::setImageData(vtkImageData* screenshot)
{
  Q_D(qDMMLScreenShotDialog);
  d->ImageData = screenshot;
  QImage qimage;
  qDMMLUtils::vtkImageDataToQImage(screenshot,qimage);
  // set preview
  d->ScreenshotWidget->setPixmap(QPixmap::fromImage(qimage));
}

//-----------------------------------------------------------------------------
vtkImageData* qDMMLScreenShotDialog::imageData() const
{
  Q_D(const qDMMLScreenShotDialog);
  return d->ImageData.GetPointer();
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::setShowScaleFactorSpinBox(const bool& state)
{
  Q_D(qDMMLScreenShotDialog);
  d->scaleFactorSpinBox->setVisible(state);
  d->scaleFactorLabel->setVisible(state);
}

//-----------------------------------------------------------------------------
bool qDMMLScreenShotDialog::showScaleFactorSpinBox() const
{
  Q_D(const qDMMLScreenShotDialog);
  return d->scaleFactorSpinBox->isVisible();
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::resetDialog()
{
  Q_D(qDMMLScreenShotDialog);

  d->descriptionTextEdit->clear();
  // we want a default name which is easily overwritable by just typing
  // We set the name
  d->nameEdit->clear();

  // set the widget type to the last one used
  this->setWidgetType(qDMMLScreenShotDialog::WidgetType(d->LastWidgetType));

  this->setScaleFactor(1.0);

  // reset the id
  this->setData(QVariant());
  this->grabScreenShot();
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::grabScreenShot()
{
  if (this->data().isValid())
    {
    // If a data is set, we are in "review" mode, no screenshot can be taken
    return;
    }
  this->grabScreenShot(this->widgetType());
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::grabScreenShot(int screenshotWindow)
{
  Q_D(qDMMLScreenShotDialog);
  QWidget* widget = nullptr;
  vtkRenderWindow* renderWindow = nullptr;
  if (d->LayoutManager.isNull())
    {
    // layout manager not set, can't grab image
    return;
    }
  switch (screenshotWindow)
    {
    case qDMMLScreenShotDialog::ThreeD:
      {
      // Create a screenshot of the first 3DView
      qDMMLThreeDView* threeDView = d->LayoutManager.data()->threeDWidget(0)->threeDView();
      widget = threeDView;
      renderWindow = threeDView->renderWindow();
      }
      break;
    case qDMMLScreenShotDialog::Red:
    case qDMMLScreenShotDialog::Yellow:
    case qDMMLScreenShotDialog::Green:
      // Create a screenshot of a specific sliceView
      {
      QString name = this->enumToString(screenshotWindow);
      qDMMLSliceWidget* sliceWidget = d->LayoutManager.data()->sliceWidget(name);
      qDMMLSliceView* sliceView = sliceWidget->sliceView();
      widget = sliceView;
      renderWindow = sliceView->renderWindow();
      }
      break;
    case qDMMLScreenShotDialog::FullLayout:
    default:
      // Create a screenshot of the full layout
      widget = d->LayoutManager.data()->viewport();
      break;
    }

  double scaleFactor = d->scaleFactorSpinBox->value();

  vtkNew<vtkImageData> newImageData;
  if (!qFuzzyCompare(scaleFactor, 1.0) &&
      screenshotWindow == qDMMLScreenShotDialog::ThreeD)
    {
    // use off screen rendering to magnifiy the VTK widget's image without interpolation
    vtkRenderer *renderer = renderWindow->GetRenderers()->GetFirstRenderer();
    vtkNew<vtkRenderLargeImage> renderLargeImage;
    renderLargeImage->SetInput(renderer);
    renderLargeImage->SetMagnification(scaleFactor);
    renderLargeImage->Update();
    newImageData.GetPointer()->DeepCopy(renderLargeImage->GetOutput());
    }
  else if (!qFuzzyCompare(scaleFactor, 1.0) && renderWindow != nullptr)
    {
    // Render slice widget at high resolution

    // Enable offscreen rendering
    renderWindow->OffScreenRenderingOn();

    // Resize render window and slice widget
    const int* renderWindowSize = renderWindow->GetSize();
    int width = renderWindowSize[0];
    int height = renderWindowSize[1];
    int scaledWidth = width * scaleFactor;
    int scaledHeight = height * scaleFactor;
    renderWindow->SetSize(scaledWidth, scaledHeight);
    widget->resize(scaledWidth, scaledHeight);

    renderWindow->Render();

    vtkNew<vtkWindowToImageFilter> windowToImageFilter;
    windowToImageFilter->SetInput(renderWindow);
    windowToImageFilter->Update();
    newImageData->DeepCopy(windowToImageFilter->GetOutput());

    // Resize slice widget to its original size
    widget->resize(width, height);

    // Disable offscreen rendering; restores original render window size
    renderWindow->OffScreenRenderingOff();
    }
  else
    {
    // no scaling, or for not just the 3D window
    QImage screenShot = ctk::grabVTKWidget(widget);

    if (!qFuzzyCompare(scaleFactor, 1.0))
      {
      // Rescale the image which gets saved
      QImage rescaledScreenShot = screenShot.scaled(screenShot.size().width() * scaleFactor,
                                                    screenShot.size().height() * scaleFactor);

      // convert the scaled screenshot from QPixmap to vtkImageData
      qDMMLUtils::qImageToVtkImageData(rescaledScreenShot,
                                       newImageData.GetPointer());
      }
    else
      {
      // convert the screenshot from QPixmap to vtkImageData
      qDMMLUtils::qImageToVtkImageData(screenShot,
                                       newImageData.GetPointer());
      }
    }
  // save the screen shot image to this class
  this->setImageData(newImageData.GetPointer());
}

//-----------------------------------------------------------------------------
void qDMMLScreenShotDialog::saveAs()
{
  if (this->data().isValid())
    {
    // If a data is set, we are in "review" mode, no screenshot can be taken
    return;
    }
  QString name = nameEdit();
  if (name == "")
    {
    name = "Cjyx Screen Capture";
    }
  QString savePath = QFileDialog::getSaveFileName(this, tr("Save File"),
                           name, tr("Images (*.png *.jpg)"));

  if (savePath != "")
    {
    QImage qimage;
    qDMMLUtils::vtkImageDataToQImage(this->imageData(),qimage);
    qimage.save(savePath);
    }
}

//-----------------------------------------------------------------------------
QString qDMMLScreenShotDialog::enumToString(int type)
{
  int propIndex = this->metaObject()->indexOfProperty("widgetType");
  QMetaProperty widgetTypeProperty = this->metaObject()->property(propIndex);
  QMetaEnum widgetTypeEnum = widgetTypeProperty.enumerator();
  return widgetTypeEnum.valueToKey(type);
}
