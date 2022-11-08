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

#ifndef __qDMMLViewControllerBar_p_h
#define __qDMMLViewControllerBar_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Cjyx API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
#include <QIcon>
#include <QObject>

// qDMML includes
#include "qDMMLViewControllerBar.h"

// DMML includes
#include "vtkDMMLAbstractViewNode.h"
#include "vtkDMMLLayoutNode.h"

// VTK includes
#include "vtkWeakPointer.h"

class ctkPopupWidget;
class QLabel;
class QToolButton;
class QHBoxLayout;
class QVBoxLayout;
class vtkDMMLLayoutNode;

//-----------------------------------------------------------------------------
class QDMML_WIDGETS_EXPORT qDMMLViewControllerBarPrivate: public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qDMMLViewControllerBar);

protected:
  qDMMLViewControllerBar* const q_ptr;

public:
  typedef QObject Superclass;
  qDMMLViewControllerBarPrivate(qDMMLViewControllerBar& object);
  ~qDMMLViewControllerBarPrivate() override;

  virtual void init();
  virtual void setColor(QColor color);
  virtual QColor color()const;

  // Need to observe the view and layout nodes to update maximize/restore button state.
  vtkWeakPointer<vtkDMMLAbstractViewNode> ViewNode;
  vtkWeakPointer<vtkDMMLLayoutNode> LayoutNode;

  QToolButton*                     PinButton;
  QLabel*                          ViewLabel;
  QToolButton*                     MaximizeViewButton;
  ctkPopupWidget*                  PopupWidget;
  QWidget*                         BarWidget;
  QHBoxLayout*                     BarLayout;
  QVBoxLayout*                     ControllerLayout;
  qDMMLViewControllerBar::LayoutBehavior  LayoutBehavior;
  QColor                           BarColor;
  QIcon                            ViewMaximizeIcon;
  QIcon                            ViewRestoreIcon;

  bool eventFilter(QObject* object, QEvent* event) override;

protected:
  virtual void setupPopupUi();
};

#endif
