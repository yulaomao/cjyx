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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qDMMLThreeDView_h
#define __qDMMLThreeDView_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKRenderView.h>

#include "qDMMLWidgetsExport.h"

class QDropEvent;
class qDMMLThreeDViewPrivate;
class vtkDMMLAbstractDisplayableManager;
class vtkDMMLCameraNode;
class vtkDMMLScene;
class vtkDMMLViewNode;
class vtkCollection;

/// \brief 3D view for view nodes.
/// For performance reasons, the view block refreshs when the scene is in
/// batch process state.
/// \sa qDMMLThreeDWidget, qDMMLThreeDViewControllerWidget, qDMMLSliceView
class QDMML_WIDGETS_EXPORT qDMMLThreeDView : public ctkVTKRenderView
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkVTKRenderView Superclass;

  /// Constructors
  explicit qDMMLThreeDView(QWidget* parent = nullptr);
  ~qDMMLThreeDView() override;

  /// Add a displayable manager to the view,
  /// the displayable manager is proper to the 3D view and is not shared
  /// with other views.
  /// If you want to register a displayable manager with all the 3D
  /// views (existing or future), you need to do it via
  /// vtkDMMLThreeDViewDisplayableManagerFactory::RegisterDisplayableManager()
  /// By default: vtkDMMLCameraDisplayableManager,
  /// vtkDMMLViewDisplayableManager and vtkDMMLModelDisplayableManager are
  /// already registered.
  void addDisplayableManager(const QString& displayableManager);
  Q_INVOKABLE void getDisplayableManagers(vtkCollection* displayableManagers);

  /// Return a DisplayableManager given its class name
  Q_INVOKABLE  vtkDMMLAbstractDisplayableManager* displayableManagerByClassName(const char* className);

  /// Get the 3D View node observed by view.
  Q_INVOKABLE vtkDMMLViewNode* dmmlViewNode()const;

  /// Returns the interactor style of the view
  //vtkInteractorObserver* interactorStyle()const;

  /// Methods to rotate/reset the camera,
  /// Can defined a view axis by its index (from 0 to 5)
  /// or its label (defined in vtkDMMLViewNode::AxisLabels)
  /// to rotate to the axis ranged in that order:
  /// -X, +X, -Y, +Y, -Z, +Z
  Q_INVOKABLE void rotateToViewAxis(unsigned int axisId);
  Q_INVOKABLE void rotateToViewAxis(const std::string& axisLabel);
  Q_INVOKABLE void resetCamera(bool resetRotation = true,
                               bool resetTranslation = true,
                               bool resetDistance = true);

  /// Returns camera node of the 3D view
  Q_INVOKABLE vtkDMMLCameraNode* cameraNode();

  /// Set cursor in the view area
  Q_INVOKABLE void setViewCursor(const QCursor &);

  /// Restore default cursor in the view area
  Q_INVOKABLE void unsetViewCursor();

  /// Set default cursor in the view area
  Q_INVOKABLE void setDefaultViewCursor(const QCursor &cursor);

  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;

public slots:

  /// Set the DMML \a scene that should be listened for events
  /// When the scene is in batch process state, the view blocks all refresh.
  /// \sa renderEnabled
  void setDMMLScene(vtkDMMLScene* newScene);

  /// Set the current \a viewNode to observe
  void setDMMLViewNode(vtkDMMLViewNode* newViewNode);

  /// Look from a given axis, need a dmml view node to be set
  void lookFromViewAxis(const ctkAxesWidget::Axis& axis);

  /// Reimplemented to hide items to not take into
  /// account when computing the boundaries
  virtual void resetFocalPoint();

protected:
  QScopedPointer<qDMMLThreeDViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLThreeDView);
  Q_DISABLE_COPY(qDMMLThreeDView);
};

#endif
