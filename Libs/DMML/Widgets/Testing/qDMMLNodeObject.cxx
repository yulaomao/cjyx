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

/// Qt includes
#include <QApplication>
#include <QDebug>

/// qDMML includes
#include "qDMMLNodeObject.h"

/// VTK includes
#include <vtkTimerLog.h>

//-----------------------------------------------------------------------------
qDMMLNodeObject::qDMMLNodeObject(vtkDMMLNode* node, QObject* parent)
  : QObject(parent)
{
  this->Node = node;
  this->ProcessEvents = true;
  this->Message = QString(node ? node->GetName() : "");
}

//-----------------------------------------------------------------------------
void qDMMLNodeObject::modify()
{
  vtkTimerLog* timer = vtkTimerLog::New();
  timer->StartTimer();
  this->Node->Modified();
  if (this->ProcessEvents)
    {
    QApplication::processEvents();
    }
  timer->StopTimer();
  qDebug() << this->Message << " modified: " << timer->GetElapsedTime() << "seconds. FPS:" << 1. / timer->GetElapsedTime();
  timer->Delete();
}

//-----------------------------------------------------------------------------
void qDMMLNodeObject::setProcessEvents(bool process)
{
  this->ProcessEvents = process;
}

//-----------------------------------------------------------------------------
bool qDMMLNodeObject::processEvents()const
{
  return this->ProcessEvents;
}

//-----------------------------------------------------------------------------
void qDMMLNodeObject::setMessage(const QString& message)
{
  this->Message = message;
}

//-----------------------------------------------------------------------------
QString qDMMLNodeObject::message()const
{
  return this->Message;
}
