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

#ifndef __qDMMLCaptureToolBar_h
#define __qDMMLCaptureToolBar_h

// Qt includes
//#include <QSignalMapper>
#include <QToolBar>

// CTK includes
#include <ctkPimpl.h>
// no ui begin
#include <ctkVTKObject.h>
// no ui end

// qDMMLWidget includes
#include "qDMMLWidget.h"
#include "qDMMLWidgetsExport.h"

class qDMMLCaptureToolBarPrivate;
class vtkDMMLNode;
class vtkDMMLScene;
class vtkDMMLViewNode;

class QDMML_WIDGETS_EXPORT qDMMLCaptureToolBar : public QToolBar
{
  Q_OBJECT
  QVTK_OBJECT

  Q_PROPERTY(bool popupsTimeOut READ popupsTimeOut WRITE setPopupsTimeOut)

public:
  typedef QToolBar Superclass;

  /// Constructor
  /// Title is the name of the toolbar (can appear using right click on the toolbar area)
  qDMMLCaptureToolBar(const QString& title, QWidget* parent = nullptr);
  qDMMLCaptureToolBar(QWidget* parent = nullptr);
  ~qDMMLCaptureToolBar() override;

  // Get popupsTimeOut setting
  bool popupsTimeOut() const;

public slots:
  virtual void setDMMLScene(vtkDMMLScene* newScene);
  void setActiveDMMLThreeDViewNode(vtkDMMLViewNode * newActiveDMMLThreeDViewNode);

  /// Set flag to time out pop ups, set from the qCjyxAppMainWindow according to the
  /// AA_EnableTesting attribute
  void setPopupsTimeOut(bool flag);

signals:
  void screenshotButtonClicked();
  void sceneViewButtonClicked();
  void dmmlSceneChanged(vtkDMMLScene*);

protected:
  QScopedPointer<qDMMLCaptureToolBarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLCaptureToolBar);
  Q_DISABLE_COPY(qDMMLCaptureToolBar);
};

#endif
