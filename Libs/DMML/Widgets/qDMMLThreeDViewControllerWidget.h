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

#ifndef __qDMMLThreeDViewControllerWidget_h
#define __qDMMLThreeDViewControllerWidget_h

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkVTKObject.h>

// qDMMLWidget includes
#include "qDMMLViewControllerBar.h"
class qDMMLThreeDViewControllerWidgetPrivate;
class qDMMLThreeDView;

// DMML includes

// DMMLLogic includes
class vtkDMMLViewLogic;
class vtkDMMLViewNode;

// VTK includes

class QDMML_WIDGETS_EXPORT qDMMLThreeDViewControllerWidget
  : public qDMMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Superclass typedef
  typedef qDMMLViewControllerBar Superclass;

  /// Constructors
  explicit qDMMLThreeDViewControllerWidget(QWidget* parent = nullptr);
  ~qDMMLThreeDViewControllerWidget() override;

  void setQuadBufferStereoSupportEnabled(bool value);

  /// Get ViewLogic
  vtkDMMLViewLogic* viewLogic()const;

  /// Set \a newViewLogic
  /// Use if two instances of the controller need to observe the same logic.
  void setViewLogic(vtkDMMLViewLogic* newViewLogic);

  /// Set the label for the table view (abbreviation for the view name)
  void setViewLabel(const QString& newViewLabel);

  /// Get the label for the view (abbreviation for the view name)
  QString viewLabel()const;

  /// Get 3D view node associated with this ThreeDViewController.
  Q_INVOKABLE vtkDMMLViewNode* dmmlThreeDViewNode() const;

public slots:

  void setDMMLScene(vtkDMMLScene* newScene) override;

  void setThreeDView(qDMMLThreeDView* threeDView);

  /// Link/Unlink the view controls and the cameras across all viewes
  void setViewLink(bool linked);

  void setOrthographicModeEnabled(bool enabled);

  void lookFromAxis(const ctkAxesWidget::Axis& axis);
  void pitchView();
  void rollView();
  void yawView();
  void zoomIn();
  void zoomOut();
  void spinView(bool enabled);
  void rockView(bool enabled);
  void setAnimationMode(int newAnimationMode);

  void resetFocalPoint();
  void set3DAxisVisible(bool visible);
  void set3DAxisLabelVisible(bool visible);

  /// Use or not depth peeling in the first renderer.
  /// False by default.
  void setUseDepthPeeling(bool use);
  /// Show or hide the FPS in the lower right corner.
  /// False by default.
  void setFPSVisible(bool visible);

  /// Utility function to change the color of the background to blue
  void setLightBlueBackground();

  /// Utility function to change the color of the background to black
  void setBlackBackground();

  /// Utility function to change the color of the background to white
  void setWhiteBackground();

  /// If the second color is not set, the first color is used.
  void setBackgroundColor(const QColor& color,
                          QColor color2 = QColor());

  void setStereoType(int newStereoType);
  void setOrientationMarkerType(int type);
  void setOrientationMarkerSize(int size);
  void setRulerType(int type);
  void setRulerColor(int color);

protected slots:
  void updateWidgetFromDMMLViewLogic();
  void updateWidgetFromDMMLView() override;
  void updateViewFromDMMLCamera();

protected:
  void setDMMLViewNode(vtkDMMLAbstractViewNode* viewNode) override;

private:
  Q_DECLARE_PRIVATE(qDMMLThreeDViewControllerWidget);
  Q_DISABLE_COPY(qDMMLThreeDViewControllerWidget);
};

#endif
