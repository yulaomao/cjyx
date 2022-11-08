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
#include <QDebug>

// qDMML includes
#include "qDMMLNodeComboBox.h"
#include "qDMMLNodeComboBoxEventPlayer.h"

// DMML includes
#include "vtkDMMLNode.h"

// ----------------------------------------------------------------------------
qDMMLNodeComboBoxEventPlayer::qDMMLNodeComboBoxEventPlayer(QObject *parent)
  : pqWidgetEventPlayer(parent)
{

}

// ----------------------------------------------------------------------------
bool qDMMLNodeComboBoxEventPlayer::playEvent(QObject *Object,
                                    const QString &Command,
                                    const QString &Arguments,
                                    bool &Error)
{
  if (Command != "nodeAddedByUser" && Command != "currentNodeChanged" &&
      Command != "nodeAboutToBeRemoved" && Command != "nodeRenamed")
    {
    return false;
    }

  if (qDMMLNodeComboBox* const comboBox =
      qobject_cast<qDMMLNodeComboBox*>(Object))
    {
    if (Command == "nodeAddedByUser")
      {
      comboBox->addNode();
      return true;
      }
    if (Command == "currentNodeChanged")
      {
      if (Arguments == "None")
        {
        comboBox->setCurrentNodeIndex(0);
        }
      comboBox->setCurrentNodeID(Arguments);
      return true;
      }
    if (Command == "nodeAboutToBeRemoved")
      {
      comboBox->removeCurrentNode();
      return true;
      }
    if (Command == "nodeRenamed")
      {
      comboBox->currentNode()->SetName(Arguments.toUtf8());
      return true;
      }
    }

  qCritical() << "calling nodeAddedByUser/currentNodeChanged/nodeAboutToBeRemoved/nodeRenamed on unhandled type " << Object;
  Error = true;
  return true;
}

