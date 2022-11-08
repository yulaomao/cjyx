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

#ifndef __qDMMLTreeViewEventTranslator_h
#define __qDMMLTreeViewEventTranslator_h

// Qt includes
#include "QModelIndexList"

// QtTesting includes
#include <pqTreeViewEventTranslator.h>

// qDMML includes
#include "qDMMLWidgetsExport.h"

class vtkDMMLNode;

class QDMML_WIDGETS_EXPORT qDMMLTreeViewEventTranslator :
  public pqTreeViewEventTranslator
{
  Q_OBJECT

public:
  typedef pqTreeViewEventTranslator Superclass;
  qDMMLTreeViewEventTranslator(QObject* parent = nullptr);

  using Superclass::translateEvent;
  bool translateEvent(QObject *Object, QEvent *Event, int EventType, bool &Error) override;

private:
  qDMMLTreeViewEventTranslator(const qDMMLTreeViewEventTranslator&); // NOT implemented
  qDMMLTreeViewEventTranslator& operator=(const qDMMLTreeViewEventTranslator&); // NOT implemented

  QObject* CurrentObject;

private slots:
  void onDestroyed(QObject*);
  void onCurrentNodeDeleted(const QModelIndex&);
  void onCurrentNodeRenamed(const QString&);
//  void onEditNodeRequested(vtkDMMLNode*);
  void onDecorationClicked(const QModelIndex&);
  void onAboutToReparentByDnD(vtkDMMLNode*, vtkDMMLNode*);

private:
  QString getIndexAsString(const QModelIndex&);
};

#endif
