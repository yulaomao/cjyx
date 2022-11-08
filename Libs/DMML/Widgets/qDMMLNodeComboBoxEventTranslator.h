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

#ifndef __qDMMLNodeComboBoxEventTranslator_h
#define __qDMMLNodeComboBoxEventTranslator_h

// Qt includes
#include "QModelIndexList"

// QtTesting includes
#include <pqWidgetEventTranslator.h>

// qDMML includes
#include "qDMMLWidgetsExport.h"

class vtkDMMLNode;

class QDMML_WIDGETS_EXPORT qDMMLNodeComboBoxEventTranslator :
  public pqWidgetEventTranslator
{
  Q_OBJECT

public:
  typedef pqWidgetEventTranslator Superclass;
  qDMMLNodeComboBoxEventTranslator(QObject* parent = nullptr);

  using Superclass::translateEvent;
  bool translateEvent(QObject *Object, QEvent *Event, bool &Error) override;

private:
  qDMMLNodeComboBoxEventTranslator(const qDMMLNodeComboBoxEventTranslator&); // NOT implemented
  qDMMLNodeComboBoxEventTranslator& operator=(const qDMMLNodeComboBoxEventTranslator&); // NOT implemented

  QObject* CurrentObject;

private slots:
  void onDestroyed(QObject*);
  void onRowsInserted();
  void onCurrentNodeChanged(vtkDMMLNode*);
  void onNodeAboutToBeRemoved(vtkDMMLNode*);
  void onCurrentNodeRenamed(const QString&);
};

#endif
