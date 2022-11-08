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

#ifndef __qDMMLViewControllerBar_h
#define __qDMMLViewControllerBar_h

// Qt includes
class QLabel;
class QLayout;
class QToolButton;

// CTK includes
#include <ctkVTKObject.h>

// qDMMLWidget includes
#include "qDMMLWidget.h"
#include "qDMMLWidgetsExport.h"
class qDMMLViewControllerBarPrivate;
class vtkDMMLAbstractViewNode;

/// qDMMLViewControllerBar is the base class of all the bars over views.
/// A controller bar typically contains a pin button, a view label to uniquely
/// define a view, a popup widget to control the view and a unique color per
/// type of view. The popup widget can be made occupy space in the
/// widget and is then displayed below the bar.
//
// Widget layout:
//   VBoxLayout (ControllerLayout)
//      Widget (BarWidget)
//        HBoxLayout (BarLayout)
//      Optional PopupWidget (can be statically displayed under BarWidget if ControllerBar is a panel)
//
//
// To add widgets to the "bar" section, add them to the barLayout().
// To add widgets to the "controller" section (when not using a
// popup), add them to the layout().
//
class QDMML_WIDGETS_EXPORT qDMMLViewControllerBar
  : public qDMMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Superclass typedef
  typedef qDMMLWidget Superclass;

  /// Constructors
  explicit qDMMLViewControllerBar(QWidget* parent = nullptr);
  ~qDMMLViewControllerBar() override;

  enum LayoutBehavior {
    Popup=0,
    Panel
  };

  /// Set the behavior of the controller, i.e. should it be a popup or
  /// should it occupy space within the widget? (not certain that this
  /// method can be called multiple times to toggle between the behaviors)
  Q_INVOKABLE void setLayoutBehavior(LayoutBehavior behavior);

  /// Get the layout for the "bar" in the view controller.
  /// This layout is an HBoxLayout. It is packed in a VBoxLayout that
  /// contains the "bar" and other controllers.
  Q_INVOKABLE QLayout* barLayout();

  /// Get the widget for the "bar" in the view controller.
  /// This is the part of the controller that is visible
  /// even when view controller is not pinned.
  Q_INVOKABLE QWidget* barWidget();

  /// Push-pin icon that shows additional options when clicked.
  Q_INVOKABLE QToolButton* pinButton();

  /// Label that displays the view's name.
  Q_INVOKABLE QLabel* viewLabel();

public slots:
  void maximizeView();

protected slots:
  virtual void updateWidgetFromDMMLView();

protected:
  QScopedPointer<qDMMLViewControllerBarPrivate> d_ptr;
  qDMMLViewControllerBar(qDMMLViewControllerBarPrivate* pimpl, QWidget* parent = nullptr);

  virtual void setDMMLViewNode(vtkDMMLAbstractViewNode* viewNode);
  virtual vtkDMMLAbstractViewNode* dmmlViewNode() const;

private:
  Q_DECLARE_PRIVATE(qDMMLViewControllerBar);
  Q_DISABLE_COPY(qDMMLViewControllerBar);
};

#endif
