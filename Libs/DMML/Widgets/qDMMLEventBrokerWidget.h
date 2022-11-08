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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qDMMLEventBrokerWidget_h
#define __qDMMLEventBrokerWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

#include "qDMMLWidgetsExport.h"

class QModelIndex;
class QTreeWidgetItem;
class qDMMLEventBrokerWidgetPrivate;
class vtkDMMLNode;
class vtkDMMLScene;
class vtkObject;

class QDMML_WIDGETS_EXPORT qDMMLEventBrokerWidget: public QWidget
{
  Q_OBJECT
public:
  typedef QWidget Superclass;
  explicit qDMMLEventBrokerWidget(QWidget *parent = nullptr);
  ~qDMMLEventBrokerWidget() override;

public slots:
  void refresh();
  void resetElapsedTimes();
  void expandElapsedTimeItems();

signals:
  void currentObjectChanged(vtkObject*);

protected slots:
  void onItemChanged(QTreeWidgetItem* item, int column);
  void onCurrentItemChanged(QTreeWidgetItem* currentItem);

protected:
  QScopedPointer<qDMMLEventBrokerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qDMMLEventBrokerWidget);
  Q_DISABLE_COPY(qDMMLEventBrokerWidget);
};

#endif
