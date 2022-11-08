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

// Qt includes

// qDMML includes
#include "qDMMLEventLoggerWidget.h"
#include "ui_qDMMLEventLoggerWidget.h"
#include "qDMMLEventLogger.h"

// DMML includes
#include <vtkDMMLNode.h>

// VTK includes

//-----------------------------------------------------------------------------
class qDMMLEventLoggerWidgetPrivate: public Ui_qDMMLEventLoggerWidget
{
public:
  void log(const QString& text);
  void log(const char* text);

  qDMMLEventLogger* DMMLEventLogger;
};

//------------------------------------------------------------------------------
// Helper macro allowing to connect signal from qDMMLEventLogger with the corresponding
// widget.
//
#define QDMMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(_EVENT_NAME)               \
  QObject::connect(d->DMMLEventLogger, SIGNAL(signal##_EVENT_NAME##Event()), \
    this, SLOT(on##_EVENT_NAME##Event()));

//------------------------------------------------------------------------------
qDMMLEventLoggerWidget::qDMMLEventLoggerWidget(QWidget *_parent):Superclass(_parent)
  , d_ptr(new qDMMLEventLoggerWidgetPrivate)
{
  Q_D(qDMMLEventLoggerWidget);
  d->setupUi(this);
  d->DMMLEventLogger = new qDMMLEventLogger(this);

  this->connect(d->DMMLEventLogger,
                SIGNAL(signalNodeAddedEvent(vtkObject*,vtkObject*)),
                SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));

  this->connect(d->DMMLEventLogger,
                SIGNAL(signalNodeRemovedEvent(vtkObject*,vtkObject*)),
                SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)));

  QDMMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(NewScene);
  QDMMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneClosed);
  QDMMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneAboutToBeClosed);
  QDMMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(MetadataAdded);
  QDMMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(ImportProgressFeedback);
  QDMMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SaveProgressFeedback);
  QDMMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneAboutToBeImported);
  QDMMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneImported);
  QDMMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneRestored);
}

//------------------------------------------------------------------------------
// Unregister helper macro
#undef QDMMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO

//------------------------------------------------------------------------------
qDMMLEventLoggerWidget::~qDMMLEventLoggerWidget() = default;

//------------------------------------------------------------------------------
void qDMMLEventLoggerWidget::setDMMLScene(vtkDMMLScene* scene)
{
  Q_D(qDMMLEventLoggerWidget);
  d->DMMLEventLogger->setDMMLScene(scene);
}

//------------------------------------------------------------------------------
void qDMMLEventLoggerWidget::setConsoleOutputEnabled(bool enabled)
{
  Q_D(qDMMLEventLoggerWidget);
  d->DMMLEventLogger->setConsoleOutputEnabled(enabled);
}

//------------------------------------------------------------------------------
void qDMMLEventLoggerWidget::onNodeAddedEvent(vtkObject* caller,
                                              vtkObject* call_data)
{
  Q_UNUSED(caller);
  Q_D(qDMMLEventLoggerWidget);
  vtkDMMLNode * node = vtkDMMLNode::SafeDownCast(call_data);
  Q_ASSERT(node);
  d->log(QString("NodeAdded: %1").arg(node->GetClassName()));
}

//------------------------------------------------------------------------------
void qDMMLEventLoggerWidget::onNodeRemovedEvent(vtkObject* caller,
                                                vtkObject* call_data)
{
  Q_UNUSED(caller);
  Q_D(qDMMLEventLoggerWidget);
  vtkDMMLNode * node = vtkDMMLNode::SafeDownCast(call_data);
  Q_ASSERT(node);
  d->log(QString("NodeRemoved: %1").arg(node->GetClassName()));
}

//------------------------------------------------------------------------------
// Helper macro allowing to define function of the
// form void listenEVENT_NAMEEvent(bool listen)'
//
#define QDMMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(_EVENT_NAME) \
void qDMMLEventLoggerWidget::on##_EVENT_NAME##Event()          \
{                                                              \
  Q_D(qDMMLEventLoggerWidget);                              \
  d->log(#_EVENT_NAME);                                        \
}

//------------------------------------------------------------------------------
QDMMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(NewScene);
QDMMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneClosed);
QDMMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneAboutToBeClosed);
QDMMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(MetadataAdded);
QDMMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(ImportProgressFeedback);
QDMMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SaveProgressFeedback);
QDMMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneAboutToBeImported);
QDMMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneImported);
QDMMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneRestored);

//------------------------------------------------------------------------------
// Unregister helper macro
#undef QDMMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO

//------------------------------------------------------------------------------
// qDMMLEventLoggerWidgetPrivate methods

//------------------------------------------------------------------------------
void qDMMLEventLoggerWidgetPrivate::log(const char* text)
{
  this->log(QString::fromUtf8(text));
}

//------------------------------------------------------------------------------
void qDMMLEventLoggerWidgetPrivate::log(const QString& text)
{
  this->TextEdit->append(text);
}
