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

#ifndef __qDMMLEventLogger_h
#define __qDMMLEventLogger_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

#include "qDMMLWidgetsExport.h"

class qDMMLEventLoggerPrivate;
class vtkDMMLScene;
class vtkObject;

class QDMML_WIDGETS_EXPORT qDMMLEventLogger: public QObject
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  explicit qDMMLEventLogger(QObject* parent = nullptr);
  ~qDMMLEventLogger() override;

  ///
  /// Set the DMML \a scene that should be listened for events
  void setDMMLScene(vtkDMMLScene* scene);

  ///
  /// Return true if the corresponding event if listened by the eventLogger
  bool listeningNodeAddedEvent();
  bool listeningNodeRemovedEvent();
  bool listeningNewSceneEvent();
  bool listeningSceneClosedEvent();
  bool listeningSceneAboutToBeClosedEvent();
  bool listeningMetadataAddedEvent();
  bool listeningImportProgressFeedbackEvent();
  bool listeningSaveProgressFeedbackEvent();
  bool listeningSceneAboutToBeImportedEvent();
  bool listeningSceneImportedEvent();
  bool listeningSceneRestoredEvent();

public slots:
  ///
  /// Allow to enable or disable the listening of specific event
  void listenNodeAddedEvent(bool listen);
  void listenNodeRemovedEvent(bool listen);
  void listenNewSceneEvent(bool listen);
  void listenSceneClosedEvent(bool listen);
  void listenSceneAboutToBeClosedEvent(bool listen);
  void listenMetadataAddedEvent(bool listen);
  void listenImportProgressFeedbackEvent(bool listen);
  void listenSaveProgressFeedbackEvent(bool listen);
  void listenSceneAboutToBeImportedEvent(bool listen);
  void listenSceneImportedEvent(bool listen);
  void listenSceneRestoredEvent(bool listen);

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

  /// Enable / Disable console output
  void setConsoleOutputEnabled(bool enabled);

signals:
  ///
  /// Emitted when the associated DMML scene event is fired
  void signalNodeAddedEvent(vtkObject* caller, vtkObject* call_data);
  void signalNodeRemovedEvent(vtkObject* caller, vtkObject* call_data);
  void signalNewSceneEvent();
  void signalSceneClosedEvent();
  void signalSceneAboutToBeClosedEvent();
  void signalMetadataAddedEvent();
  void signalImportProgressFeedbackEvent();
  void signalSaveProgressFeedbackEvent();
  void signalSceneAboutToBeImportedEvent();
  void signalSceneImportedEvent();
  void signalSceneRestoredEvent();

protected:
  QScopedPointer<qDMMLEventLoggerPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLEventLogger);
  Q_DISABLE_COPY(qDMMLEventLogger);
};

#endif
