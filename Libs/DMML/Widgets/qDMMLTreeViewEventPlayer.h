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

  This file was originally developed by Benjamin LONG, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qDMMLTreeViewEventPlayer_h
#define __qDMMLTreeViewEventPlayer_h

// QtTesting includes
#include <pqTreeViewEventPlayer.h>

// qDMML includes
#include "qDMMLWidgetsExport.h"

/// Concrete implementation of pqWidgetEventPlayer that translates
/// high-level events into low-level Qt events.

class QDMML_WIDGETS_EXPORT qDMMLTreeViewEventPlayer :
  public pqTreeViewEventPlayer
{
  Q_OBJECT

public:
  typedef pqTreeViewEventPlayer Superclass;
  qDMMLTreeViewEventPlayer(QObject* parent = nullptr);

  using Superclass::playEvent;
  bool playEvent(QObject *Object, const QString &Command, const QString &Arguments, int EventType, bool &Error) override;

private:
  qDMMLTreeViewEventPlayer(const qDMMLTreeViewEventPlayer&); // NOT implemented
  qDMMLTreeViewEventPlayer& operator=(const qDMMLTreeViewEventPlayer&); // NOT implemented

};

#endif
