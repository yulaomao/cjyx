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

#ifndef __qDMMLEventLoggerWidget_h
#define __qDMMLEventLoggerWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

#include "qDMMLWidgetsExport.h"

class qDMMLEventLoggerWidgetPrivate;
class vtkDMMLScene;
class vtkObject;

class QDMML_WIDGETS_EXPORT qDMMLEventLoggerWidget: public QWidget
{
  Q_OBJECT
public:
  typedef QWidget Superclass;
  explicit qDMMLEventLoggerWidget(QWidget *parent = nullptr);
  ~qDMMLEventLoggerWidget() override;

public slots:

  ///
  /// Set the DMML scene that should be listened for events
  void setDMMLScene(vtkDMMLScene* scene);

  /// Enable / Disable console output
  void setConsoleOutputEnabled(bool enabled);

protected slots:

  virtual void onNodeAddedEvent(vtkObject* caller, vtkObject* call_data);
  virtual void onNodeRemovedEvent(vtkObject* caller, vtkObject* call_data);
  virtual void onNewSceneEvent();
  virtual void onSceneClosedEvent();
  virtual void onSceneAboutToBeClosedEvent();
  virtual void onMetadataAddedEvent();
  virtual void onImportProgressFeedbackEvent();
  virtual void onSaveProgressFeedbackEvent();
  virtual void onSceneAboutToBeImportedEvent();
  virtual void onSceneImportedEvent();
  virtual void onSceneRestoredEvent();

protected:
  QScopedPointer<qDMMLEventLoggerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLEventLoggerWidget);
  Q_DISABLE_COPY(qDMMLEventLoggerWidget);
};

#endif
