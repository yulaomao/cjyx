/*=========================================================================

  Program: 3D Cjyx

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Benjamin LONG, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

=========================================================================*/

// Qt includes
#include <QAbstractItemModel>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

// CTK includes
#include "ctkCheckableModelHelper.h"
#include "ctkComboBox.h"

// qDMML includes
#include "qDMMLNodeComboBox.h"
#include "qDMMLNodeComboBoxEventTranslator.h"

// DMML includes
#include "vtkDMMLNode.h"

// ----------------------------------------------------------------------------
qDMMLNodeComboBoxEventTranslator::qDMMLNodeComboBoxEventTranslator(QObject *parent)
  : pqWidgetEventTranslator(parent)
{
  this->CurrentObject = nullptr;
}

// ----------------------------------------------------------------------------
bool qDMMLNodeComboBoxEventTranslator::translateEvent(QObject *Object,
                                                      QEvent *Event,
                                                      bool &Error)
{
  Q_UNUSED(Error);

  qDMMLNodeComboBox* widget = nullptr;
  for(QObject* test = Object; widget == nullptr && test != nullptr; test = test->parent())
    {
    widget = qobject_cast<qDMMLNodeComboBox*>(test);
    }
  if(!widget)
    {
    return false;
    }

  if(Event->type() == QEvent::Enter && Object == widget)
    {
    if(this->CurrentObject != Object)
      {
      if(this->CurrentObject)
        {
        disconnect(this->CurrentObject, nullptr, this, nullptr);
        }
      this->CurrentObject = Object;
      connect(widget, SIGNAL(destroyed(QObject*)),
              this, SLOT(onDestroyed(QObject*)));
      connect(widget, SIGNAL(nodeAddedByUser(vtkDMMLNode*)),
              this, SLOT(onRowsInserted()));
      connect(widget, SIGNAL(nodeAboutToBeRemoved(vtkDMMLNode*)),
              this, SLOT(onNodeAboutToBeRemoved(vtkDMMLNode*)));
      connect(widget, SIGNAL(currentNodeRenamed(QString)),
              this, SLOT(onCurrentNodeRenamed(QString)));
      }
    if(this->CurrentObject)
      {
      connect(this->CurrentObject, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
              this, SLOT(onCurrentNodeChanged(vtkDMMLNode*)), Qt::UniqueConnection);
      }
    }

  return true;
}

// ----------------------------------------------------------------------------
void qDMMLNodeComboBoxEventTranslator::onDestroyed(QObject* /*Object*/)
{
  this->CurrentObject = nullptr;
}

// ----------------------------------------------------------------------------
void qDMMLNodeComboBoxEventTranslator::onRowsInserted()
{
  disconnect(this->CurrentObject, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
             this, SLOT(onCurrentNodeChanged(vtkDMMLNode*)));
  emit recordEvent(this->CurrentObject, "nodeAddedByUser", "");
}

// ----------------------------------------------------------------------------
void qDMMLNodeComboBoxEventTranslator::onCurrentNodeChanged(vtkDMMLNode* node)
{
  if(node)
    {
    emit recordEvent(this->CurrentObject, "currentNodeChanged", QString(node->GetID()));
    }
  else
    {
    emit recordEvent(this->CurrentObject, "currentNodeChanged", "None");
    }
}

// ----------------------------------------------------------------------------
void qDMMLNodeComboBoxEventTranslator::onNodeAboutToBeRemoved(vtkDMMLNode* node)
{
  if(node)
    {
    disconnect(this->CurrentObject, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
               this, SLOT(onCurrentNodeChanged(vtkDMMLNode*)));
    emit recordEvent(this->CurrentObject, "nodeAboutToBeRemoved", QString(node->GetName()));
    }
  else
    {
    emit recordEvent(this->CurrentObject, "nodeAboutToBeRemoved", "None");
    }
}

// ----------------------------------------------------------------------------
void qDMMLNodeComboBoxEventTranslator::onCurrentNodeRenamed(const QString& newName)
{
  emit recordEvent(this->CurrentObject, "nodeRenamed", newName);
}
