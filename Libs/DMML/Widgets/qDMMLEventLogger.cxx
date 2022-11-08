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

// qDMML includes
#include "qDMMLEventLogger_p.h"

// DMML includes
#include <vtkDMMLScene.h>

//------------------------------------------------------------------------------
// qDMMLEventLoggerPrivate methods

//------------------------------------------------------------------------------
void qDMMLEventLoggerPrivate::init()
{
  Q_Q(qDMMLEventLogger);
  q->listenNodeAddedEvent(true);
  q->listenNodeRemovedEvent(true);
  q->listenNewSceneEvent(true);
  q->listenSceneClosedEvent(true);
  q->listenSceneAboutToBeClosedEvent(true);
  q->listenMetadataAddedEvent(true);
  q->listenImportProgressFeedbackEvent(true);
  q->listenSaveProgressFeedbackEvent(true);
  q->listenSceneAboutToBeImportedEvent(true);
  q->listenSceneImportedEvent(true);
  q->listenSceneRestoredEvent(true);
}

//------------------------------------------------------------------------------
qDMMLEventLoggerPrivate::qDMMLEventLoggerPrivate(qDMMLEventLogger& object)
  : q_ptr(&object)
{
  this->DMMLScene = nullptr;
  this->ConsoleOutputEnabled = true;
}

//------------------------------------------------------------------------------
void qDMMLEventLoggerPrivate::setDMMLScene(vtkDMMLScene* scene)
{
  Q_Q(qDMMLEventLogger);

  if (scene == this->DMMLScene)
    {
    return;
    }

  QString cid; // connectionId

  // Set a high priority, doing so will force the Logger to be first to catch and
  // display the event associated with the scene.
  float priority = 100.0;

  this->EventNameToConnectionIdMap["NodeAdded"] = this->qvtkReconnect(
    this->DMMLScene, scene,
    vtkDMMLScene::NodeAddedEvent, q,
    SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)), priority);

  this->EventNameToConnectionIdMap["NodeRemoved"] = this->qvtkReconnect(
    this->DMMLScene, scene,
    vtkDMMLScene::NodeRemovedEvent, q,
    SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)), priority);

  this->EventNameToConnectionIdMap["NewScene"] = this->qvtkReconnect(
    this->DMMLScene, scene,
    vtkDMMLScene::NewSceneEvent, q, SLOT(onNewSceneEvent()), priority);

  this->EventNameToConnectionIdMap["SceneClosed"] = this->qvtkReconnect(
    this->DMMLScene, scene,
    vtkDMMLScene::EndCloseEvent, q, SLOT(onSceneClosedEvent()), priority);

  this->EventNameToConnectionIdMap["SceneAboutToBeClosed"] = this->qvtkReconnect(
    this->DMMLScene, scene,
    vtkDMMLScene::StartCloseEvent, q, SLOT(onSceneAboutToBeClosedEvent()), priority);

  this->EventNameToConnectionIdMap["MetadataAdded"] = this->qvtkReconnect(
    this->DMMLScene, scene,
    vtkDMMLScene::MetadataAddedEvent, q, SLOT(onMetadataAddedEvent()), priority);

  this->EventNameToConnectionIdMap["ImportProgressFeedback"] = this->qvtkReconnect(
    this->DMMLScene, scene,
    vtkDMMLScene::ImportProgressFeedbackEvent, q, SLOT(onImportProgressFeedbackEvent()), priority);

  this->EventNameToConnectionIdMap["SaveProgressFeedback"] = this->qvtkReconnect(
    this->DMMLScene, scene,
    vtkDMMLScene::SaveProgressFeedbackEvent, q, SLOT(onSaveProgressFeedbackEvent()), priority);

  this->EventNameToConnectionIdMap["SceneAboutToBeImported"] = this->qvtkReconnect(
    this->DMMLScene, scene,
    vtkDMMLScene::StartImportEvent, q, SLOT(onSceneAboutToBeImportedEvent()), priority);

  this->EventNameToConnectionIdMap["SceneImported"] = this->qvtkReconnect(
    this->DMMLScene, scene,
    vtkDMMLScene::EndImportEvent, q, SLOT(onSceneImportedEvent()), priority);

  this->EventNameToConnectionIdMap["SceneRestored"] = this->qvtkReconnect(
    this->DMMLScene, scene,
    vtkDMMLScene::EndRestoreEvent, q, SLOT(onSceneRestoredEvent()), priority);

  this->DMMLScene = scene;
}

//------------------------------------------------------------------------------
// qDMMLEventLogger methods

//------------------------------------------------------------------------------
qDMMLEventLogger::qDMMLEventLogger(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qDMMLEventLoggerPrivate(*this))
{
  Q_D(qDMMLEventLogger);
  d->init();
}

//------------------------------------------------------------------------------
qDMMLEventLogger::~qDMMLEventLogger() = default;

//------------------------------------------------------------------------------
void qDMMLEventLogger::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLEventLogger);
  d->setDMMLScene(scene);
}

//------------------------------------------------------------------------------
// Helper macro allowing to define function of the form 'bool ListeningEVENT_NAME()'
//
#define QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO(_EVENT_NAME)   \
bool qDMMLEventLogger::listening##_EVENT_NAME##Event()        \
{                                                             \
  Q_D(qDMMLEventLogger);                                   \
  return d->EventToListen.contains(#_EVENT_NAME);             \
}

//------------------------------------------------------------------------------
QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO(NodeAdded);
QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO(NodeRemoved);
QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO(NewScene);
QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneClosed);
QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneAboutToBeClosed);
QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO(MetadataAdded);
QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO(ImportProgressFeedback);
QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO(SaveProgressFeedback);
QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneAboutToBeImported);
QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneImported);
QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneRestored);

//------------------------------------------------------------------------------
// Unregister helper macro
#undef QDMMLEVENTLOGGER_LISTENING_EVENT_MACRO

//------------------------------------------------------------------------------
// Helper macro allowing to define function of the
// form void listenEVENT_NAMEEvent(bool listen)'
//
#define QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO(_EVENT_NAME)            \
void qDMMLEventLogger::listen##_EVENT_NAME##Event(bool listen)      \
{                                                                   \
  Q_D(qDMMLEventLogger);                                         \
                                                                    \
  Q_ASSERT(!d->EventNameToConnectionIdMap.contains(#_EVENT_NAME));  \
  QString cid = d->EventNameToConnectionIdMap[#_EVENT_NAME];        \
                                                                    \
  if (listen && !d->EventToListen.contains(#_EVENT_NAME))           \
    {                                                               \
    d->EventToListen << #_EVENT_NAME;                               \
    d->qvtkBlock(cid, false);                                       \
    }                                                               \
  if (!listen)                                                      \
    {                                                               \
    d->EventToListen.removeOne(#_EVENT_NAME);                       \
    d->qvtkBlock(cid, true);                                        \
    }                                                               \
}

//------------------------------------------------------------------------------
QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO(NodeAdded);
QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO(NodeRemoved);
QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO(NewScene);
QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO(SceneClosed);
QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO(SceneAboutToBeClosed);
QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO(MetadataAdded);
QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO(ImportProgressFeedback);
QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO(SaveProgressFeedback);
QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO(SceneAboutToBeImported);
QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO(SceneImported);
QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO(SceneRestored);

//------------------------------------------------------------------------------
// Unregister helper macro
#undef QDMMLEVENTLOGGER_LISTEN_EVENT_MACRO

//------------------------------------------------------------------------------
void qDMMLEventLogger::onNodeAddedEvent(vtkObject* caller, vtkObject* call_data)
{
  Q_D(qDMMLEventLogger);
  if (d->ConsoleOutputEnabled)
    {
    std::cout << qPrintable(QString("onNodeAddedEvent: %1").arg(call_data->GetClassName())) << std::endl;
    }
  emit this->signalNodeAddedEvent(caller, call_data);
}

//------------------------------------------------------------------------------
void qDMMLEventLogger::onNodeRemovedEvent(vtkObject* caller, vtkObject* call_data)
{
  Q_D(qDMMLEventLogger);
  if (d->ConsoleOutputEnabled)
    {
    std::cout << qPrintable(QString("onNodeRemovedEvent: %1").arg(call_data->GetClassName())) << std::endl;
    }
  emit this->signalNodeRemovedEvent(caller, call_data);
}

//------------------------------------------------------------------------------
// Helper macro allowing to define function of the
// form void onEVENT_NAMEEvent()'
//
#define QDMMLEVENTLOGGER_ONEVENT_SLOT_MACRO(_EVENT_NAME)    \
void qDMMLEventLogger::on##_EVENT_NAME##Event()             \
{                                                           \
  Q_D(qDMMLEventLogger);                                    \
  if (d->ConsoleOutputEnabled)                              \
    {                                                       \
    std::cout << qPrintable(                                \
      QString("qDMMLEventLogger::on%1Event").               \
        arg(#_EVENT_NAME)) << std::endl;                    \
    }                                                       \
  emit signal##_EVENT_NAME##Event();                        \
}

QDMMLEVENTLOGGER_ONEVENT_SLOT_MACRO(NewScene);
QDMMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneClosed);
QDMMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneAboutToBeClosed);
QDMMLEVENTLOGGER_ONEVENT_SLOT_MACRO(MetadataAdded);
QDMMLEVENTLOGGER_ONEVENT_SLOT_MACRO(ImportProgressFeedback);
QDMMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SaveProgressFeedback);
QDMMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneAboutToBeImported);
QDMMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneImported);
QDMMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneRestored);

//------------------------------------------------------------------------------
// Unregister helper macro
#undef QDMMLEVENTLOGGER_ONEVENT_SLOT_MACRO

//------------------------------------------------------------------------------
void qDMMLEventLogger::setConsoleOutputEnabled(bool enabled)
{
  Q_D(qDMMLEventLogger);
  d->ConsoleOutputEnabled = enabled;
}

